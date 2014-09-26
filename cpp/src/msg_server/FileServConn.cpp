//
//  FileServConn.cpp
//  public_TTServer
//
//  Created by luoning on 14-8-19.
//  Copyright (c) 2014年 luoning. All rights reserved.
//

#include "FileServConn.h"
#include "FileHandler.h"
#include "util.h"
#include "ImUser.h"
#include "AttachData.h"
#include "RouteServConn.h"
#include "MsgConn.h"
static ConnMap_t g_file_server_conn_map;

static serv_info_t* g_file_server_list;
static uint32_t g_file_server_count;
static CFileHandler* s_file_handler = NULL;

void file_server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	ConnMap_t::iterator it_old;
	CFileServConn* pConn = NULL;
	uint64_t cur_time = get_tick_count();
    
	for (ConnMap_t::iterator it = g_file_server_conn_map.begin(); it != g_file_server_conn_map.end();
         )
    {
        it_old = it;
        it++;
		pConn = (CFileServConn*)it_old->second;
		pConn->OnTimer(cur_time);
	}
    
	// reconnect FileServer
	serv_check_reconnect<CFileServConn>(g_file_server_list, g_file_server_count);
}

void init_file_serv_conn(serv_info_t* server_list, uint32_t server_count)
{
	g_file_server_list = server_list;
	g_file_server_count = server_count;
    
	serv_init<CFileServConn>(g_file_server_list, g_file_server_count);
    
	netlib_register_timer(file_server_conn_timer_callback, NULL, 1000);
	s_file_handler = CFileHandler::getInstance();
}

bool is_file_server_available()
{
	CFileServConn* pConn = NULL;
    
	for (uint32_t i = 0; i < g_file_server_count; i++) {
		pConn = (CFileServConn*)g_file_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen()) {
			return true;
		}
	}
	return false;
}

//
CFileServConn* get_random_file_serv_conn()
{
    CFileServConn* pConn = NULL;
    CFileServConn* pConnTmp = NULL;
    if (0 == g_file_server_count) {
        return pConn;
    }
    int32_t random_num = rand() % g_file_server_count;
    pConnTmp = (CFileServConn*)g_file_server_list[random_num].serv_conn;
    if (pConnTmp && pConnTmp->IsOpen())
    {
        pConn = pConnTmp;
    }
    else
    {
        for (uint32_t i = 0; i < g_file_server_count; i++)
        {
            int j = (random_num + 1) % g_file_server_count;
            pConnTmp = (CFileServConn*)g_file_server_list[j].serv_conn;
            if (pConnTmp && pConnTmp->IsOpen())
            {
                pConn = pConnTmp;
            }
        }
    }
	return pConn;
}


CFileServConn::CFileServConn()
{
	m_bOpen = false;
	m_serv_idx = 0;
}

CFileServConn::~CFileServConn()
{
}

void CFileServConn::Connect(const char* server_ip, uint16_t server_port, uint32_t idx)
{
	log("Connecting to FileServer %s:%d\n", server_ip, server_port);
    
	m_serv_idx = idx;
	m_handle = netlib_connect(server_ip, server_port, imconn_callback, (void*)&g_file_server_conn_map);
    
	if (m_handle != NETLIB_INVALID_HANDLE) {
		g_file_server_conn_map.insert(make_pair(m_handle, this));
	}
}

void CFileServConn::Close()
{
	serv_reset<CFileServConn>(g_file_server_list, g_file_server_count, m_serv_idx);
    
	m_bOpen = false;
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_file_server_conn_map.erase(m_handle);
	}
    
	ReleaseRef();
}

void CFileServConn::OnConfirm()
{
	log("connect to file server success\n");
	m_bOpen = true;
	m_connect_time = get_tick_count();
	g_file_server_list[m_serv_idx].reconnect_cnt = MIN_RECONNECT_CNT / 2;
    
    
    CImPduFileServerIPReq pdu;
    SendPdu(&pdu);
}

void CFileServConn::OnClose()
{
	log("onclose from file server handle=%d\n", m_handle);
	Close();
}

void CFileServConn::OnTimer(uint64_t curr_tick)
{
	if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {
		CImPduHeartbeat pdu;
		SendPdu(&pdu);
	}
    
	if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
		log("conn to file server timeout\n");
		Close();
	}
}

void CFileServConn::HandlePdu(CImPdu* pPdu)
{
	switch (pPdu->GetPduType()) {
        case IM_PDU_TYPE_HEARTBEAT:
            break;
        case IM_PDU_TYPE_MSG_FILE_TRANSFER_RSP:
            _HandleFileMsgTransRsp((CImPduMsgFileTransferRsp*) pPdu);
            break;
        case IM_PDU_TYPE_FILE_SERVER_IP_RESPONSE:
            _HandleFileServerIPRsp((CImPduFileServerIPRsp*)pPdu);
            break;
        default:
            log("unknown pdu_type=%d\n", pPdu->GetPduType());
            break;
	}
}

void CFileServConn::_HandleFileMsgTransRsp(CImPduMsgFileTransferRsp* pPdu)
{
    uint32_t result = pPdu->GetResult();
    uint32_t from_id = pPdu->GetFromId();
    uint32_t to_id = pPdu->GetToId();
    string file_name(pPdu->GetFileName(), pPdu->GetFileNameLen());
    uint32_t file_size = pPdu->GetFileLen();
    
    string task_id(pPdu->GetTaskId(), pPdu->GetTaskIdLen());
    uint32_t trans_mode = pPdu->GetTransMode();
    CPduAttachData attach(pPdu->GetAttachData(), pPdu->GetAttachLen());
    
    const list<svr_ip_addr_t>* ip_addr_list = GetFileServerIPList();
    
    string from_id_url(idtourl(from_id));
    string to_id_url(idtourl(to_id));
    CImPduClientFileResponse pdu(result, from_id_url.c_str(), to_id_url.c_str(), file_name.c_str(),
                                 task_id.c_str(), ip_addr_list, trans_mode);
    pdu.SetReserved(pPdu->GetReserved());
    uint32_t handle = attach.GetHandle();
    log("HandleFileMsgTransRsp, result: %u, from_user_id: %u, to_user_id: %u, file_name: %s, \
        task_id: %s, trans_mode: %u.\n", result, from_id, to_id,
        file_name.c_str(), task_id.c_str(), trans_mode);
    
    CMsgConn* pFromConn = CImUserManager::GetInstance()->GetMsgConnByHandle(from_id, handle);
    if (pFromConn)
    {
        pFromConn->SendPdu(&pdu);
    }
    
    const list<svr_ip_addr_t>* file_addr_list = GetFileServerIPList();
    if (result == 0)
    {
        //send notify to target user
        CImUser* pToUser = CImUserManager::GetInstance()->GetImUserById(to_id);
        if (pToUser)
        {
            
            CImPduClientFileNotify pdu2(from_id_url.c_str(), to_id_url.c_str(), file_name.c_str(),
                    file_size, task_id.c_str(), file_addr_list, trans_mode, 0);
            pToUser->BroadcastPdu(&pdu2);
        }
        
        //send to route server
        CRouteServConn* pRouteConn = get_route_serv_conn();
        if (pRouteConn) {
            CImPduFileNotify pdu3(from_id, to_id, file_name.c_str(), file_size, task_id.c_str(),
                                  file_addr_list, trans_mode, 0);
            pRouteConn->SendPdu(&pdu3);
        }
    }
}

void CFileServConn::_HandleFileServerIPRsp(CImPduFileServerIPRsp* pPdu)
{
    uint32_t ip_addr_cnt = pPdu->GetIPCnt();
    ip_addr_t* ip_addr_list = pPdu->GetIPList();
    
    for (uint32_t i = 0; i < ip_addr_cnt ; i++) {
        svr_ip_addr_t svr_ip_addr;
        svr_ip_addr.ip = string(ip_addr_list[i].ip, ip_addr_list[i].ip_len);
        svr_ip_addr.port = ip_addr_list[i].port;
        log("_HandleFileServerIPRsp -> %s : %d\n", svr_ip_addr.ip.c_str(), svr_ip_addr.port);
        m_ip_list.push_back(svr_ip_addr);
    }
}
