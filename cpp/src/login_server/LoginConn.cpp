/*
 * LoginConn.cpp
 *
 *  Created on: 2013-6-21
 *      Author: ziteng@mogujie.com
 */

#include "LoginConn.h"

static ConnMap_t g_client_conn_map;
static ConnMap_t g_msg_serv_conn_map;
static UserConnCntMap_t g_user_conn_cnt_map;
static uint32_t g_total_online_user_cnt = 0;	// 并发在线总人数

typedef struct  {
	string		ip_addr1;	// 网通IP
	string		ip_addr2;	// 电信IP
	uint16_t	port;
	uint32_t	max_conn_cnt;
	uint32_t	cur_conn_cnt;
    uint32_t	cur_user_cnt;	// 当前的用户数- 由于允许用户多点登陆，cur_user_cnt != cur_conn_cnt
	string 		hostname;	// 消息服务器的主机名
	uint32_t	server_type;
    UserConnCntMap_t user_cnt_map;
} msg_serv_info_t;

static map<uint32_t, msg_serv_info_t*> g_msg_serv_info;

void login_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	uint64_t cur_time = get_tick_count();
	for (ConnMap_t::iterator it = g_client_conn_map.begin(); it != g_client_conn_map.end(); ) {
		ConnMap_t::iterator it_old = it;
		it++;

		CLoginConn* pConn = (CLoginConn*)it_old->second;
		pConn->OnTimer(cur_time);
	}

	for (ConnMap_t::iterator it = g_msg_serv_conn_map.begin(); it != g_msg_serv_conn_map.end(); ) {
		ConnMap_t::iterator it_old = it;
		it++;

		CLoginConn* pConn = (CLoginConn*)it_old->second;
		pConn->OnTimer(cur_time);
	}
}

void init_login_conn()
{
	netlib_register_timer(login_conn_timer_callback, NULL, 1000);
}

CLoginConn::CLoginConn()
{
}

CLoginConn::~CLoginConn()
{

}

void CLoginConn::Close()
{
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		if (m_conn_type == LOGIN_CONN_TYPE_CLIENT) {
			g_client_conn_map.erase(m_handle);
		} else {
			g_msg_serv_conn_map.erase(m_handle);

			// remove all user count from this message server
			map<uint32_t, msg_serv_info_t*>::iterator it = g_msg_serv_info.find(m_handle);
			if (it != g_msg_serv_info.end()) {
				msg_serv_info_t* pMsgServInfo = it->second;

				g_total_online_user_cnt -= pMsgServInfo->cur_conn_cnt;
				log("onclose from MsgServer: %s:%u\n", pMsgServInfo->hostname.c_str(), pMsgServInfo->port);
				delete pMsgServInfo;
				g_msg_serv_info.erase(it);
			}
		}
	}

	ReleaseRef();
}

void CLoginConn::OnConnect2(net_handle_t handle, int conn_type)
{
	m_handle = handle;
	m_conn_type = conn_type;

	ConnMap_t* conn_map = &g_msg_serv_conn_map;
	if (conn_type == LOGIN_CONN_TYPE_CLIENT) {
		conn_map = &g_client_conn_map;
	}

	conn_map->insert(make_pair(handle, this));

	netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)imconn_callback);
	netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)conn_map);
}

void CLoginConn::OnClose()
{
	Close();
}

void CLoginConn::OnTimer(uint64_t curr_tick)
{
	if (m_conn_type == LOGIN_CONN_TYPE_CLIENT) {
		if (curr_tick > m_last_recv_tick + CLIENT_TIMEOUT) {
			Close();
		}
	} else {
		if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {
			CImPduHeartbeat pdu;
			SendPdu(&pdu);
		}

		if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
			log("connection to MsgServer timeout\n");
			Close();
		}
	}
}

void CLoginConn::HandlePdu(CImPdu* pPdu)
{
	switch (pPdu->GetPduType()) {
	case IM_PDU_TYPE_HEARTBEAT:
		break;
	case IM_PDU_TYPE_MSG_SERV_INFO:
		_HandleMsgServInfo((CImPduMsgServInfo*)pPdu);
		break;
	case IM_PDU_TYPE_USER_CNT_UPDATE:
		_HandleUserCntUpdate((CImPduUserCntUpdate*)pPdu);
		break;
	case IM_PDU_TYPE_MSG_SERV_REQUEST:
		_HandleMsgServRequest((CImPduMsgServRequest*)pPdu);
		break;
    case IM_PDU_TYPE_USER_CONN_INFO:
        _HandleUserConnInfo((CImPduUserConnInfo*)pPdu);
        break;
	default:
		log("wrong msg, type=%d\n", pPdu->GetPduType());
		break;
	}
}

void CLoginConn::_HandleMsgServInfo(CImPduMsgServInfo* pPdu)
{
	msg_serv_info_t* pMsgServInfo = new msg_serv_info_t;

	pMsgServInfo->ip_addr1 = string(pPdu->GetIp1Addr(), pPdu->GetIp1Len());
	pMsgServInfo->ip_addr2 = string(pPdu->GetIp2Addr(), pPdu->GetIp2Len());
	pMsgServInfo->port = pPdu->GetPort();
	pMsgServInfo->max_conn_cnt = pPdu->GetMaxConnCnt();
	pMsgServInfo->cur_conn_cnt = pPdu->GetCurConnCnt();
	pMsgServInfo->hostname = string(pPdu->GetHostname(), pPdu->GetHostnameLen());
	pMsgServInfo->server_type = pPdu->GetServerType();
	g_msg_serv_info.insert(make_pair(m_handle, pMsgServInfo));

	g_total_online_user_cnt += pMsgServInfo->cur_conn_cnt;

	log("MsgServInfo, ip_addr1=%s, ip_addr2=%s, port=%d, max_conn_cnt=%d, cur_conn_cnt=%d, "\
		"hostname: %s, server_type: %d\n",
		pMsgServInfo->ip_addr1.c_str(), pMsgServInfo->ip_addr2.c_str(), pMsgServInfo->port,pMsgServInfo->max_conn_cnt,
		pMsgServInfo->cur_conn_cnt, pMsgServInfo->hostname.c_str(), pMsgServInfo->server_type);
}

void CLoginConn::_HandleUserCntUpdate(CImPduUserCntUpdate* pPdu)
{
	map<uint32_t, msg_serv_info_t*>::iterator it = g_msg_serv_info.find(m_handle);
	if (it != g_msg_serv_info.end()) {
		msg_serv_info_t* pMsgServInfo = it->second;
		uint32_t action = pPdu->GetUserAction();
        uint32_t user_id = pPdu->GetUserId();
		if (action == USER_CNT_INC) {
			pMsgServInfo->cur_conn_cnt++;
            UserConnCntMap_t::iterator iter = g_user_conn_cnt_map.find(user_id);
            if(iter == g_user_conn_cnt_map.end()) {
                g_user_conn_cnt_map.insert(make_pair(user_id, 1));
                g_total_online_user_cnt++;
            } else {
                ++iter->second;
            }
            
            iter = pMsgServInfo->user_cnt_map.find(user_id);
            if (iter == pMsgServInfo->user_cnt_map.end()) {
            	pMsgServInfo->user_cnt_map.insert(make_pair(user_id, 1));
            	pMsgServInfo->cur_user_cnt++;
            }
            else
            {
            	++iter->second;
            }
        }
        else
        {
			pMsgServInfo->cur_conn_cnt--;
            UserConnCntMap_t::iterator iter = g_user_conn_cnt_map.find(user_id);
            if((iter == g_user_conn_cnt_map.end())) {
                log("user_id is not exist, id=%u\n", user_id);
            }
            else
            {
				if(--iter->second <= 0)
                {
					g_user_conn_cnt_map.erase(iter);
					--g_total_online_user_cnt;
				}
            }
            
            iter = pMsgServInfo->user_cnt_map.find(user_id);
            if (iter == pMsgServInfo->user_cnt_map.end())
            {
            	log("no user_id in MsgServer\n");
            }
            else
            {
            	if (--iter->second <= 0)
                {
            		pMsgServInfo->cur_user_cnt--;
            		pMsgServInfo->user_cnt_map.erase(iter);
            	}
            }
        }

        log("%s:%d, conn_cnt=%u, user_cnt=%u, total_cnt=%u\n", pMsgServInfo->hostname.c_str(),
            pMsgServInfo->port, pMsgServInfo->cur_conn_cnt, pMsgServInfo->cur_user_cnt,
            g_total_online_user_cnt);
	}
}

void CLoginConn::_HandleMsgServRequest(CImPduMsgServRequest* pPdu)
{
	log("HandleMsgServReq\n");

	// no MessageServer available
	if (g_msg_serv_info.size() == 0) {
		CImPduMsgServResponse pdu(REFUSE_REASON_NO_MSG_SERVER);
		pdu.SetReserved(pPdu->GetReserved());
		SendPdu(&pdu);
		Close();
		return;
	}

	// return a message server with minimum concurrent connection count
	msg_serv_info_t* pMsgServInfo;
	uint32_t min_user_cnt = (uint32_t)-1;
	map<uint32_t, msg_serv_info_t*>::iterator it_min_conn = g_msg_serv_info.end(),it;

	for (it = g_msg_serv_info.begin() ; it != g_msg_serv_info.end(); it++) {
		pMsgServInfo = it->second;
		if ( (pMsgServInfo->cur_conn_cnt < pMsgServInfo->max_conn_cnt) &&
			 (pMsgServInfo->cur_conn_cnt < min_user_cnt) &&
			 (pMsgServInfo->server_type == MSG_SERVER_TYPE_TCP) ) {
			it_min_conn = it;
			min_user_cnt = pMsgServInfo->cur_conn_cnt;
		}
	}

	if (it_min_conn == g_msg_serv_info.end()) {
		log("All TCP MsgServer are full\n");
		CImPduMsgServResponse pdu(REFUSE_REASON_MSG_SERVER_FULL);
		pdu.SetReserved(pPdu->GetReserved());
		SendPdu(&pdu);
	} else {
		CImPduMsgServResponse pdu(0, it_min_conn->second->ip_addr1.c_str(),
				it_min_conn->second->ip_addr2.c_str(), it_min_conn->second->port);
		pdu.SetReserved(pPdu->GetReserved());
		SendPdu(&pdu);
	}

	Close();	// after send MsgServResponse, active close the connection
}

void CLoginConn::_HandleUserConnInfo(CImPduUserConnInfo *pPdu)
{
    uint32_t user_cnt = pPdu->GetUserCnt();
	user_conn_t* user_conn_list = pPdu->GetUserConnList();
    
	map<uint32_t, msg_serv_info_t*>::iterator it = g_msg_serv_info.find(m_handle);
	if (it != g_msg_serv_info.end()) {
		msg_serv_info_t* pMsgServInfo = it->second;
		pMsgServInfo->cur_user_cnt = user_cnt;
		for (uint32_t i = 0; i < user_cnt; i++) {
			pMsgServInfo->user_cnt_map.insert(make_pair(user_conn_list[i].user_id, user_conn_list[i].conn_cnt));
            
			UserConnCntMap_t::iterator it_user = g_user_conn_cnt_map.find(user_conn_list[i].user_id);
			if(it_user == g_user_conn_cnt_map.end()) {
				g_user_conn_cnt_map.insert(make_pair(user_conn_list[i].user_id, 1));
				g_total_online_user_cnt++;
			} else {
				++it_user->second;
			}
		}
	}
    
	log("HandleUserConnInfo, user_cnt=%u, total_user_cnt=%u\n", user_cnt, g_total_online_user_cnt);
}
