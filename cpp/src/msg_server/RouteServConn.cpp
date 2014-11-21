/*
 * RouteServConn.cpp
 *
 *  Created on: 2013-7-8
 *      Author: ziteng@mogujie.com
 */

#include "RouteServConn.h"
#include "MsgConn.h"
#include "LoginServConn.h"
#include "DBServConn.h"
#include "FileHandler.h"
#include "GroupChat.h"
#include "ImUser.h"
#include "AttachData.h"
#include "FileServConn.h"
static ConnMap_t g_route_server_conn_map;

static serv_info_t* g_route_server_list;
static uint32_t g_route_server_count;
static CRouteServConn* g_master_rs_conn = NULL;
static CFileHandler* s_file_handler = NULL;
static CGroupChat* s_group_chat = NULL;

void route_server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	ConnMap_t::iterator it_old;
	CRouteServConn* pConn = NULL;
	uint64_t cur_time = get_tick_count();

	for (ConnMap_t::iterator it = g_route_server_conn_map.begin(); it != g_route_server_conn_map.end(); ) {
        it_old = it;
        it++;
        pConn = (CRouteServConn*)it_old->second;
        pConn->OnTimer(cur_time);
	}

	// reconnect RouteServer
	serv_check_reconnect<CRouteServConn>(g_route_server_list, g_route_server_count);
}

void init_route_serv_conn(serv_info_t* server_list, uint32_t server_count)
{
	g_route_server_list = server_list;
	g_route_server_count = server_count;

	serv_init<CRouteServConn>(g_route_server_list, g_route_server_count);

	netlib_register_timer(route_server_conn_timer_callback, NULL, 1000);
	s_file_handler = CFileHandler::getInstance();
	s_group_chat = CGroupChat::GetInstance();
}

bool is_route_server_available()
{
	CRouteServConn* pConn = NULL;

	for (uint32_t i = 0; i < g_route_server_count; i++) {
		pConn = (CRouteServConn*)g_route_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen()) {
			return true;
		}
	}

	return false;
}

void send_to_all_route_server(CImPdu* pPdu)
{
	CRouteServConn* pConn = NULL;

	for (uint32_t i = 0; i < g_route_server_count; i++) {
		pConn = (CRouteServConn*)g_route_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen()) {
			pConn->SendPdu(pPdu);
		}
	}
}

// get the oldest route server connection
CRouteServConn* get_route_serv_conn()
{
	return g_master_rs_conn;
}

void update_master_route_serv_conn()
{
	uint64_t oldest_connect_time = (uint64_t)-1;
	CRouteServConn* pOldestConn = NULL;

	CRouteServConn* pConn = NULL;

	for (uint32_t i = 0; i < g_route_server_count; i++) {
		pConn = (CRouteServConn*)g_route_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen() && (pConn->GetConnectTime() < oldest_connect_time) ){
			pOldestConn = pConn;
			oldest_connect_time = pConn->GetConnectTime();
		}
	}

	g_master_rs_conn =  pOldestConn;

	if (g_master_rs_conn) {
		CImPduRoleSet pdu(1);
		g_master_rs_conn->SendPdu(&pdu);
	}
}


CRouteServConn::CRouteServConn()
{
	m_bOpen = false;
	m_serv_idx = 0;
}

CRouteServConn::~CRouteServConn()
{

}

void CRouteServConn::Connect(const char* server_ip, uint16_t server_port, uint32_t idx)
{
	log("Connecting to RouteServer %s:%d\n", server_ip, server_port);

	m_serv_idx = idx;
	m_handle = netlib_connect(server_ip, server_port, imconn_callback, (void*)&g_route_server_conn_map);

	if (m_handle != NETLIB_INVALID_HANDLE) {
		g_route_server_conn_map.insert(make_pair(m_handle, this));
	}
}

void CRouteServConn::Close()
{
	serv_reset<CRouteServConn>(g_route_server_list, g_route_server_count, m_serv_idx);

	m_bOpen = false;
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_route_server_conn_map.erase(m_handle);
	}

	ReleaseRef();

	if (g_master_rs_conn == this) {
		update_master_route_serv_conn();
	}
}

void CRouteServConn::OnConfirm()
{
	log("connect to route server success\n");
	m_bOpen = true;
	m_connect_time = get_tick_count();
	g_route_server_list[m_serv_idx].reconnect_cnt = MIN_RECONNECT_CNT / 2;

	if (g_master_rs_conn == NULL) {
		update_master_route_serv_conn();
	}

	list<user_conn_stat_t> online_user_list;
    CImUserManager::GetInstance()->GetOnlineUserInfo(&online_user_list);
	CImPduOnlineUserInfo pdu(&online_user_list);
	SendPdu(&pdu);
}

void CRouteServConn::OnClose()
{
	log("onclose from route server handle=%d\n", m_handle);
	Close();
}

void CRouteServConn::OnTimer(uint64_t curr_tick)
{
	if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {
		CImPduHeartbeat pdu;
		SendPdu(&pdu);
	}

	if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
		log("conn to route server timeout\n");
		Close();
	}
}

void CRouteServConn::HandlePdu(CImPdu* pPdu)
{
	switch (pPdu->GetPduType()) {
	case IM_PDU_TYPE_HEARTBEAT:
		break;
	case IM_PDU_TYPE_SERVER_KICK_USER:
		_HandleKickUser( (CImPduServerKickUser*)pPdu );
		break;
	case IM_PDU_TYPE_FRIEND_STATUS_LIST:
		_HandleFriendStatusList( (CImPduFriendStatusList*)pPdu );
		break;
	case IM_PDU_TYPE_FRIEND_STATUS_NOTIFY:
		_HandleFriendStatusNotify( (CImPduFriendStatusNotify*)pPdu );
		break;
	case IM_PDU_TYPE_MSG_DATA:
	{
		CImPduMsgData* pPduMsg = (CImPduMsgData*)pPdu;
		if (CHECK_MSG_TYPE_GROUP(pPduMsg->GetMsgType()))
            s_group_chat->HandleGroupMessage(pPduMsg);
		else
            _HandleMsgData(pPduMsg);
		break;
	}
	case IM_PDU_TYPE_P2P_MSG:
		_HandleP2PMsg( (CImPduP2PMsg*)pPdu );
		break;
    case IM_PDU_TYPE_GROUP_P2P_MSG_RESPONSE:
        _HandleGroupP2PMsg((CImPduGroupP2PMessageResponse*)pPdu);
        break;
	case IM_PDU_TYPE_USER_STATUS_RESPONSE:
		_HandleUserStatusResponse( (CImPduUserStatusResponse*)pPdu );
		break;
	case IM_PDU_TYPE_USERS_STATUS_RESPONSE:
		_HandleUsersStatusResponse( (CImPduUsersStatusResponse*)pPdu );
		break;
    case IM_PDU_TYPE_USER_CLIENT_TYPE_RESPONSE:
        _HandleUserClientTypeResponse((CImPduUserClientTypeResponse*) pPdu);
        break;
    case IM_PDU_TYPE_FILE_NOTIFY:
        s_file_handler->HandleFileNotify((CImPduFileNotify *)pPdu);
        break;
    /*
	case IM_PDU_TYPE_FILE_REQUEST:
		s_file_handler->HandleFileRequest( (CImPduFileRequest*)pPdu );
		break;
	case IM_PDU_TYPE_FILE_RESPONSE:
		s_file_handler->HandleFileResponse( (CImPduFileResponse*)pPdu );
		break;
	case IM_PDU_TYPE_FILE_RECV_READY:
		s_file_handler->HandleFileRecvReady( (CImPduFileRecvReady*)pPdu );
		break;
	case IM_PDU_TYPE_FILE_ABORT:
		s_file_handler->HandleFileAbort( (CImPduFileAbort*)pPdu );
		break;
	case IM_PDU_TYPE_FILE_UPLOAD_OFFLINE_NOTIFY:
		s_file_handler->HandleFileUploadOfflineNotify( (CImPduFileUploadOfflineNotify*)pPdu );
		break;
	case IM_PDU_TYPE_FILE_DOWNLOAD_OFFLINE_NOTIFY:
		s_file_handler->HandleFileDownloadOfflineNotify( (CImPduFileDownloadOfflineNotify*)pPdu );
		break;
    */
	case IM_PDU_TYPE_GROUP_CREATE_TMP_GROUP_RESPONSE:
		s_group_chat->HandleGroupCreateTmpGroupBroadcast((CImPduGroupCreateTmpGroupResponse*)pPdu);
		break;
	case IM_PDU_TYPE_GROUP_CHANGE_MEMBER_RESPONSE:
		s_group_chat->HandleGroupChangeMemberBroadcast((CImPduGroupChangeMemberResponse*)pPdu);
		break;
    case IM_PDU_TYPE_GROUP_CREATE_NORMAL_GROUP_NOTIFY:
        s_group_chat->HandleGroupCreateNormalGroupNotify((CImPduGroupCreateNormalGroupNotify*)pPdu);
        break;
    case IM_PDU_TYPE_GROUP_CHANGE_MEMBER_NOTIFY:
        s_group_chat->HandleGroupChangeMemberNotify((CImPduGroupChangeMemberNotify*)pPdu);
        break;
	default:
		log("unknown pdu_type=%d\n", pPdu->GetPduType());
		break;
	}
}

void CRouteServConn::_HandleKickUser(CImPduServerKickUser* pPdu)
{
	uint32_t to_user_id = pPdu->GetUserId();
    uint32_t client_type = pPdu->GetClientType();
	log("HandleKickUser, user_id=%u\n", to_user_id);

    //TODO
    CImUser* pImUser = CImUserManager::GetInstance()->GetImUserById(to_user_id);
	if (pImUser) {
		pImUser->KickOutSameClientType(client_type);
	}
}

void CRouteServConn::_HandleFriendStatusList(CImPduFriendStatusList* pPdu)
{
	uint32_t user_id = pPdu->GetFromUserId();
	uint32_t friend_cnt = pPdu->GetFriendCnt();
	user_stat_t* friend_status_list = pPdu->GetFriendStatList();
	string user_id_url = idtourl(user_id);

	log("HandleOnlineFriendList, from_id=%d\n", user_id);

    CImUser* pImUser = CImUserManager::GetInstance()->GetImUserById(user_id);
    if (!pImUser) {
        return;
    }

	// send online friend status list to client
	CImPduClientOnlineFriendList pduCOFL(pPdu->GetRequestType(), friend_cnt, friend_status_list);
    pImUser->BroadcastPduWithOutMobile(&pduCOFL);
    
    // user don't need to send self status to friend list
	//if (pPdu->GetRequestType() == ONLINE_LIST_TYPE_FRIEND_LIST) {
	//	return;
	//}
    
	// send online friend notify packet to the client in the same message server
	CImPduClientFriendNotify pduCFN(user_id_url.c_str(), pImUser->GetIMOnlineStatus());
	for (uint32_t i = 0; i < friend_cnt; i++) {
		pImUser->AddOnlineFriend(friend_status_list[i].user_id);

        CImUser* pFriendUser = CImUserManager::GetInstance()->GetImUserById(friend_status_list[i].user_id);
		if (pFriendUser) {
			//log("notify %u online to %u\n", user_id, friend_status_list[i].user_id);
            pFriendUser->BroadcastPduWithOutMobile(&pduCFN);
			pFriendUser->AddOnlineFriend(user_id);
		}
	}
}

// friend online/off-line notify
void CRouteServConn::_HandleFriendStatusNotify(CImPduFriendStatusNotify* pPdu)
{
	uint32_t user_id = pPdu->GetFromUserId();
	uint32_t user_status = pPdu->GetFromUserStatus();
	uint32_t friend_cnt = pPdu->GetFriendCnt();
	uint32_t* friend_id_list = pPdu->GetFriendIdList();
	string user_id_url = idtourl(user_id);

	log("HandleFriendStatusNotify, from_id=%u, status=%u\n", user_id, user_status);

	// send friend online message to client
	CImPduClientFriendNotify pdu(user_id_url.c_str(), user_status);

	for (uint32_t i = 0; i < friend_cnt; ++i) {
        CImUser* pImUser = CImUserManager::GetInstance()->GetImUserById(friend_id_list[i]);
		if (pImUser) {
			pImUser->BroadcastPduWithOutMobile(&pdu);

			if (user_status == USER_STATUS_ONLINE) {
				pImUser->AddOnlineFriend(user_id);
			} else if (user_status == USER_STATUS_OFFLINE){
				pImUser->RemoveOnlineFriend(user_id);
			}
		}
	}
}

void CRouteServConn::_HandleMsgData(CImPduMsgData* pPdu)
{
	uint32_t from_user_id = pPdu->GetFromUserId();
	uint32_t to_user_id = pPdu->GetToUserId();
	log("HandleMsgData, %u->%u\n", from_user_id, to_user_id);
   
    uint32_t seq_no = pPdu->GetRequestId();
    string from_id_url = idtourl(from_user_id);
    string to_id_url = idtourl(to_user_id);
    uint8_t msg_type = pPdu->GetMsgType();
    
    CImPduClientMsgData pdu(seq_no, from_id_url.c_str(), to_id_url.c_str(), pPdu->GetCreateTime(),
    msg_type, pPdu->GetMsgLen(), pPdu->GetMsgData(), pPdu->GetAttachLen(), pPdu->GetAttachData());
  
    CImUser* pFromImUser = CImUserManager::GetInstance()->GetImUserById(from_user_id);
    CImUser* pToImUser = CImUserManager::GetInstance()->GetImUserById(to_user_id);
    if (pFromImUser) {
        pFromImUser->BroadcastClientMsgData(&pdu, NULL, from_user_id);
        pFromImUser->AddOnlineFriend(to_user_id); // 聊过天就通知上下线消息
	}

    if (pToImUser) {
        pToImUser->BroadcastClientMsgData(&pdu, NULL, from_user_id);
        pToImUser->AddOnlineFriend(from_user_id);
    }
}

void CRouteServConn::_HandleP2PMsg(CImPduP2PMsg* pPdu)
{
	uint32_t from_user_id = pPdu->GetFromUserId();
	uint32_t to_user_id = pPdu->GetToUserId();
    uint32_t seq_no = pPdu->GetSeqNo();
    string from_id_url = idtourl(from_user_id);
    string to_id_url = idtourl(to_user_id);
    CImPduClientP2PCmdMsg pdu(seq_no, from_id_url.c_str(), to_id_url.c_str(), pPdu->GetMsgLen(), pPdu->GetMsgData());
	log("HandleP2PMsg, %u->%u\n", from_user_id, to_user_id);

    CImUser* pFromImUser = CImUserManager::GetInstance()->GetImUserById(from_user_id);
    CImUser* pToImUser = CImUserManager::GetInstance()->GetImUserById(to_user_id);
	if (pFromImUser) {
		pFromImUser->BroadcastPdu(&pdu);
	}
    
	if (pToImUser) {
		pToImUser->BroadcastPdu(&pdu);
	}
}

void CRouteServConn::_HandleGroupP2PMsg(CImPduGroupP2PMessageResponse* pPdu)
{
    uint32_t  result = pPdu->GetResult();
    uint32_t  from_user_id = pPdu->GetReqUserId();
    uint32_t* toUserList = pPdu->GetUserList();
    log("_HandleGroupP2PMsg, from_user_id = %u, result = %u, to_user_cnt = %u.\n", from_user_id, result, pPdu->GetUserCnt());
    
    if(result != 0 || !toUserList)
    return;
    
    for (int i = 0; i < pPdu->GetUserCnt(); ++i) {
        uint32_t to_user_id = toUserList[i];
        CImUser* pToImUser = CImUserManager::GetInstance()->GetImUserById(to_user_id);
        if (pToImUser) {
            uint32_t seq_no = 0;
            string from_id_url = idtourl(from_user_id);
            string to_id_url = idtourl(to_user_id);
            
            CImPduClientP2PCmdMsg pdu(seq_no, from_id_url.c_str(), to_id_url.c_str(), pPdu->GetMsgDataLen(),
                                      pPdu->GetMsgData());
            pToImUser->BroadcastPdu(&pdu);
        }
    }

}

void CRouteServConn::_HandleUserStatusResponse(CImPduUserStatusResponse* pPdu)
{
	uint32_t request_id = pPdu->GetRequestId();
	uint32_t query_id = pPdu->GetQueryId();
	uint32_t status = pPdu->GetStatus();
    CDbAttachData attach_data(pPdu->GetAttachData(), pPdu->GetAttachLen());
	uint32_t handle = attach_data.GetHandle();
	log("HandleUserStatusResp, req_id=%u, query_id=%u, status=%u\n", request_id, query_id, status);

	CMsgConn* pToConn = CImUserManager::GetInstance()->GetMsgConnByHandle(request_id,handle);
	if (pToConn) {
		char* query_id_url = idtourl(query_id);
		CImPduClientUserStatusResponse pdu(query_id_url, status);
		pToConn->SendPdu(&pdu);
	}
}

void CRouteServConn::_HandleUsersStatusResponse(CImPduUsersStatusResponse* pPdu)
{
	uint32_t request_id = pPdu->GetRequestId();
	uint32_t result_count = pPdu->GetResultCount();
	const list<user_stat_t>& result_list = pPdu->GetQueryResult();
	log("HandleUsersStatusResp, req_id=%u, query_count=%u\n", request_id, result_count);
    CDbAttachData attach_data(pPdu->GetAttachData(), pPdu->GetAttachLen());
	uint32_t handle = attach_data.GetHandle();

    CMsgConn* pToConn = CImUserManager::GetInstance()->GetMsgConnByHandle(request_id,handle);
	if (pToConn) {
		CImPduClientUsersStatusResponse pdu(result_list);
		pToConn->SendPdu(&pdu);
	}
}

void CRouteServConn::_HandleUserClientTypeResponse(CImPduUserClientTypeResponse *pPdu)
{
    uint32_t client_type_flag = pPdu->GetClientTypeFlag();
    uint32_t attach_len = pPdu->GetAttachLen();
    uchar_t* attach_data = pPdu->GetAttachData();
    CPduAttachData pduAttachData(attach_data, attach_len);
    
    CImPduClientFileRequest pdu(pduAttachData.GetPdu(), pduAttachData.GetPduLength());
    string from_id_url(pdu.GetFromId(), pdu.GetFromIdLen());
	uint32_t from_id = urltoid(from_id_url.c_str());
	string to_id_url(pdu.GetToId(), pdu.GetToIdLen());
	uint32_t to_id = urltoid(to_id_url.c_str());
	string file_name(pdu.GetFileName(), pdu.GetFileNameLen());
	uint32_t file_size = pdu.GetFileSize();
    uint32_t handle = pduAttachData.GetHandle();
    
    log("HandleUserClientTypeResponse, from_user_id: %u, to_user_id: %u, file_name: %s\n",
        from_id, to_id, file_name.c_str());

    uint32_t trans_mode = FILE_TYPE_OFFLINE;
    if ((client_type_flag & CLIENT_TYPE_FLAG_BOTH) == CLIENT_TYPE_FLAG_PC)
    {
        trans_mode = FILE_TYPE_ONLINE;
    }

    CPduAttachData attach(ATTACH_TYPE_HANDLE, handle, 0, NULL, 0);
    CImPduMsgFileTransferReq pdu2(from_id, to_id, file_name.c_str(), file_size,
                                 trans_mode, attach.GetLength(), attach.GetBuffer());
    pdu2.SetReserved(pPdu->GetReserved());
    CFileServConn* pConn = get_random_file_serv_conn();
    if (pConn) {
        pConn->SendPdu(&pdu2);
    }
    else
    {
        log("HandleUserClientTypeResponse, no file server\n");
        CImPduClientFileResponse pdu3(REFUSE_REASON_NO_FILE_SERVER, from_id_url.c_str(), to_id_url.c_str(), file_name.c_str(), NULL, NULL, 0);
        pdu3.SetReserved(pPdu->GetReserved());
        CMsgConn* pMsgConn = CImUserManager::GetInstance()->GetMsgConnByHandle(from_id,handle);
        if (pMsgConn)
        {
            pMsgConn->SendPdu(&pdu3);
        }
    }
}
