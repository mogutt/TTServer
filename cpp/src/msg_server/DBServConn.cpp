/*
 * DBServConn.cpp
 *
 *  Created on: 2013-7-8
 *      Author: ziteng@mogujie.com
 */

#include "DBServConn.h"
#include "MsgConn.h"
#include "RouteServConn.h"
#include "GroupChat.h"
#include "FileHandler.h"
#include "ImUser.h"
#include "AttachData.h"
#include <uuid/uuid.h>
static ConnMap_t g_db_server_conn_map;

static serv_info_t* g_db_server_list = NULL;
static uint32_t		g_db_server_count = 0;			// 到DBServer的总连接数
static uint32_t		g_db_server_login_count = 0;	// 到进行登录处理的DBServer的总连接数
static CGroupChat*	s_group_chat = NULL;
static CFileHandler* s_file_handler = NULL;

static void db_server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	ConnMap_t::iterator it_old;
	CDBServConn* pConn = NULL;
	uint64_t cur_time = get_tick_count();

	for (ConnMap_t::iterator it = g_db_server_conn_map.begin(); it != g_db_server_conn_map.end();
         )
    {
        it_old = it;
        it++;
        pConn = (CDBServConn*)it_old->second;
        if (pConn->IsOpen())
        {
            pConn->OnTimer(cur_time);
        }
	}

	// reconnect DB Storage Server
	// will reconnect in 4s, 8s, 16s, 32s, 64s, 4s 8s ...
	serv_check_reconnect<CDBServConn>(g_db_server_list, g_db_server_count);
}

void init_db_serv_conn(serv_info_t* server_list, uint32_t server_count, uint32_t concur_conn_cnt)
{
	g_db_server_list = server_list;
	g_db_server_count = server_count;

	uint32_t total_db_instance = server_count / concur_conn_cnt;
	g_db_server_login_count = (total_db_instance / 2) * concur_conn_cnt;
	log("DB server connection index for login business: [0, %u), for other business: [%u, %u)\n",
			g_db_server_login_count, g_db_server_login_count, g_db_server_count);

	serv_init<CDBServConn>(g_db_server_list, g_db_server_count);

	netlib_register_timer(db_server_conn_timer_callback, NULL, 1000);
	s_group_chat = CGroupChat::GetInstance();
	s_file_handler = CFileHandler::getInstance();
}

string create_uuid()
{
    uuid_t uu;
#ifdef __APPLE__
    uuid_string_t uuid;
#else
    char uuid[37];    
#endif
    uuid_generate( uu );
    uuid_unparse(uu, uuid);
    return string(uuid);
}


// get a random db server connection in the range [start_pos, stop_pos)
static CDBServConn* get_db_server_conn_in_range(uint32_t start_pos, uint32_t stop_pos)
{
	uint32_t i = 0;
	CDBServConn* pDbConn = NULL;

	// determine if there is a valid DB server connection
	for (i = start_pos; i < stop_pos; i++) {
		pDbConn = (CDBServConn*)g_db_server_list[i].serv_conn;
		if (pDbConn && pDbConn->IsOpen()) {
			break;
		}
	}

	// no valid DB server connection
	if (i == stop_pos) {
		return NULL;
	}

	// return a random valid DB server connection
	while (true) {
		int i = rand() % (stop_pos - start_pos) + start_pos;
		pDbConn = (CDBServConn*)g_db_server_list[i].serv_conn;
		if (pDbConn && pDbConn->IsOpen()) {
			break;
		}
	}

	return pDbConn;
}

CDBServConn* get_db_serv_conn_for_login()
{
	// 先获取login业务的实例，没有就去获取其他业务流程的实例
	CDBServConn* pDBConn = get_db_server_conn_in_range(0, g_db_server_login_count);
	if (!pDBConn) {
		pDBConn = get_db_server_conn_in_range(g_db_server_login_count, g_db_server_count);
	}

	return pDBConn;
}

CDBServConn* get_db_serv_conn()
{
	// 先获取其他业务流程的实例，没有就去获取login业务的实例
	CDBServConn* pDBConn = get_db_server_conn_in_range(g_db_server_login_count, g_db_server_count);
	if (!pDBConn) {
		pDBConn = get_db_server_conn_in_range(0, g_db_server_login_count);
	}

	return pDBConn;
}


CDBServConn::CDBServConn()
{
	m_bOpen = false;
}

CDBServConn::~CDBServConn()
{

}

void CDBServConn::Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx)
{
	log("Connecting to DB Storage Server %s:%d\n", server_ip, server_port);

	m_serv_idx = serv_idx;
	m_handle = netlib_connect(server_ip, server_port, imconn_callback, (void*)&g_db_server_conn_map);

	if (m_handle != NETLIB_INVALID_HANDLE) {
		g_db_server_conn_map.insert(make_pair(m_handle, this));
	}
}

void CDBServConn::Close()
{
	// reset server information for the next connect
	serv_reset<CDBServConn>(g_db_server_list, g_db_server_count, m_serv_idx);

	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_db_server_conn_map.erase(m_handle);
	}

	ReleaseRef();
}

void CDBServConn::OnConfirm()
{
	log("connect to db server success\n");
	m_bOpen = true;
	g_db_server_list[m_serv_idx].reconnect_cnt = MIN_RECONNECT_CNT / 2;
}

void CDBServConn::OnClose()
{
	log("onclose from db server handle=%d\n", m_handle);
	Close();
}

void CDBServConn::OnTimer(uint64_t curr_tick)
{
	if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {
		CImPduHeartbeat pdu;
		SendPdu(&pdu);
	}

	if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
		log("conn to db server timeout\n");
		Close();
	}
}

void CDBServConn::HandlePdu(CImPdu* pPdu)
{
	switch (pPdu->GetPduType()) {
	case IM_PDU_TYPE_HEARTBEAT:
		break;
	case IM_PDU_TYPE_VALIDATE_RESPONSE:
		_HandleValidateResponse( (CImPduValidateResponse*)pPdu );
		break;
	case IM_PDU_TYPE_FRIEND_LIST_RESPONSE:
		_HandleFriendListResponse( (CImPduFriendListResponse*)pPdu );
		break;
	case IM_PDU_TYPE_UNREAD_MSG_COUNT_RESPONSE:
		_HandleUnreadMsgCountResponse( (CImPduUnreadMsgCountResponse*)pPdu );
		break;
	case IM_PDU_TYPE_MSG_LIST_RESPONSE:
		_HandleMsgListResponse( (CImPduMsgListResponse*)pPdu );
		break;
	case IM_PDU_TYPE_DB_WRITE_RESPONSE:
		_HandleDBWriteResponse( (CImPduDBWriteResponse*)pPdu );
		break;
	case IM_PDU_TYPE_USERS_INFO_RESPONSE:
		_HandleUsersInfoResponse( (CImPduUsersInfoResponse*)pPdu );
		break;
	case IM_PDU_TYPE_STOP_RECEIVE_PACKET:
		_HandleStopReceivePacket( (CImPduStopReceivePacket*)pPdu);
		break;
	case IM_PDU_TYPE_DB_QUERY_RESPONSE:
		_HandleDBQueryResponse( (CImPduDBQueryResponse*)pPdu );
		break;
	case IM_PDU_TYPE_REMOVE_SESSION_RESPONSE:
		_HandleRemoveSessionResponse( (CImPduRemoveSessionResponse*)pPdu );
		break;
    case IM_PDU_TYPE_DEPARTMENT_RESPONSE:
        _HandleDepartmentResponse((CImPduDepartmentResponse*)pPdu);
        break;
	case IM_PDU_TYPE_GROUP_LIST_RESPONSE:
		s_group_chat->HandleGroupListResponse( (CImPduGroupListResponse*)pPdu );
		break;
	case IM_PDU_TYPE_GROUP_USER_LIST_RESPONSE:
		s_group_chat->HandleGroupUserListResponse( (CImPduGroupUserListResponse*)pPdu );
		break;
	case IM_PDU_TYPE_GROUP_UNREAD_MSG_CNT_RESPONSE:
		s_group_chat->HandleGroupUnreadMsgCntResponse( (CImPduGroupUnreadMsgCntResponse*)pPdu );
		break;
	case IM_PDU_TYPE_GROUP_MSG_LIST_RESPONSE:
		s_group_chat->HandleGroupMsgListResponse( (CImPduGroupMsgListResponse*)pPdu );
		break;
	case IM_PDU_TYPE_GROUP_CREATE_TMP_GROUP_RESPONSE:
		s_group_chat->HandleGroupCreateTmpGroupResponse((CImPduGroupCreateTmpGroupResponse*)pPdu);
		break;
	case IM_PDU_TYPE_GROUP_CHANGE_MEMBER_RESPONSE:
		s_group_chat->HandleGroupChangeMemberResponse((CImPduGroupChangeMemberResponse*)pPdu);
		break;

	case IM_PDU_TYPE_FILE_HAS_OFFLINE_RES:
		s_file_handler->HandleFileHasOfflineRes( (CImPduFileHasOfflineRes*)pPdu );
		break;
	default:
		log("db server, wrong pdu_type=%d\n", pPdu->GetPduType());
	}
}

void CDBServConn::_HandleValidateResponse(CImPduValidateResponse* pPdu)
{
    string user_name(pPdu->GetUserName(), pPdu->GetUserNameLen());
	uint32_t result = pPdu->GetResult();
    CDbAttachData attach_data(pPdu->GetAttachData(), pPdu->GetAttachLen());
	log("HandleValidateResp, user_name=%s, result=%d\n", user_name.c_str(), result);

    CImUser* pImUser = CImUserManager::GetInstance()->GetImUserByName(user_name);
	CMsgConn* pMsgConn = NULL;
	if (!pImUser) {
		// can not find the client connection,
		// maybe the client is closed before the DB response arrived
		// do nothing
		log("ImUser for user_name=%s not exist\n", user_name.c_str());
		return;
	} else {
        pMsgConn = pImUser->GetUnValidateMsgConn(attach_data.GetHandle());
		if (!pMsgConn || pMsgConn->IsOpen()) {
			log("no such connection or is validated, user_name=%s\n", user_name.c_str());
			return;
		}
	}

	if (result != 0) {
		result = REFUSE_REASON_DB_VALIDATE_FAILED;
	}
    
	// validate OK, set client validate past, and send FriendListRequest to db storage server
	// else close the client connection
	if (result == 0) {
        user_info_t* user = pPdu->GetUserInfo();
        pImUser->SetUser(user);
        pImUser->SetValidated();
        
        uint32_t user_id = user->user_id;
        CImUserManager::GetInstance()->AddImUserById(user_id, pImUser);
        pImUser->KickOutSameClientType(pMsgConn->GetClientType(), pMsgConn);
        
        CRouteServConn* pRouteConn = get_route_serv_conn();
        if (pRouteConn) {
            CImPduServerKickUser kickPdu(user_id, pMsgConn->GetClientType(), KICK_REASON_DUPLICATE_USER);
            pRouteConn->SendPdu(&kickPdu);
        }
        
        string token = create_uuid();
        log("user_name: %s, uid: %d, token:%s\n", user_name.c_str(), user->user_id, token.c_str());
        pMsgConn->SetToken(token);
        pMsgConn->SetOpen();
        pMsgConn->SendUserActionLog(USER_ACTION_TYPE_LOGIN);
        pMsgConn->SendUserStatusUpdate(USER_STATUS_ONLINE);
        pImUser->ValidateMsgConn(token, pMsgConn);

        CImPduLoginResponse pduLR(result, pImUser->GetIMOnlineStatus(), user, (char*)token.c_str());
        pduLR.SetReserved(pPdu->GetReserved());
        pMsgConn->SendPdu(&pduLR);
	} else {
        CImPduLoginResponse pduLR(result);
        pduLR.SetReserved(pPdu->GetReserved());
        pMsgConn->SendPdu(&pduLR);
        //pMsgConn->Close();
	}
}

void CDBServConn::_HandleFriendListResponse(CImPduFriendListResponse* pPdu)
{
	uint32_t user_id = pPdu->GetFromUserId();
	uint32_t friend_cnt = pPdu->GetFriendCnt();
	user_info_t* friend_list = pPdu->GetFriendList();
    CDbAttachData attach_data(pPdu->GetAttachData(), pPdu->GetAttachLen());
	uint32_t handle = attach_data.GetHandle();
	log("HandleFriendListResp, user_id=%d, friend_cnt=%d\n", user_id, friend_cnt);

	CMsgConn* pToConn = CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);
	if (!pToConn || !pToConn->IsOpen()) {
		return;
	}

	CRouteServConn* pRouteConn = get_route_serv_conn();
	if (!pRouteConn) {
		CImPduLoginResponse pduLR(REFUSE_REASON_NO_ROUTE_SERVER);
		pduLR.SetReserved(pPdu->GetReserved());
		pToConn->SendPdu(&pduLR);
		pToConn->Close();
		return;
	}

	// transfer from server friend list form to client friend list form (uid->url)
	CImPduClientFriendList pduCFL(friend_cnt, friend_list);
	pduCFL.SetReserved(pPdu->GetReserved());
	pToConn->SendPdu(&pduCFL);
}

void CDBServConn::_HandleUnreadMsgCountResponse(CImPduUnreadMsgCountResponse* pPdu)
{
	uint32_t to_user_id = pPdu->GetToUserId();
	UserUnreadMsgCnt_t* user_unread_list = pPdu->GetUserUnreadList();
	uint32_t user_unread_cnt = pPdu->GetUserUnreadCnt();
    CDbAttachData attach_data(pPdu->GetAttachData(), pPdu->GetAttachLen());
	uint32_t handle = attach_data.GetHandle();
	log("HandleUnreadMsgCntResp, userId=%u, cnt=%u\n", to_user_id,  user_unread_cnt);

	CMsgConn* pMsgConn = CImUserManager::GetInstance()->GetMsgConnByHandle(to_user_id, handle);
    
	if (pMsgConn && pMsgConn->IsOpen()) {
		CImPduClientUnreadMsgCntResponse pdu(user_unread_cnt, user_unread_list);
		pdu.SetReserved(pPdu->GetReserved());
		pMsgConn->SendPdu(&pdu);
	}
}

void CDBServConn::_HandleMsgListResponse(CImPduMsgListResponse* pPdu)
{
	uint32_t request_cmd_id = pPdu->GetRequestCmdId();
	uint32_t from_user_id = pPdu->GetFromUserId();
	uint32_t to_user_id = pPdu->GetToUserId();
	uint32_t msg_cnt = pPdu->GetMsgCount();
	server_msg_t* server_msg_list = pPdu->GetMsgList();
	string to_id_url = idtourl(to_user_id);
    CDbAttachData attach_data(pPdu->GetAttachData(), pPdu->GetAttachLen());
	uint32_t handle = attach_data.GetHandle();
    CMsgConn* pMsgConn = CImUserManager::GetInstance()->GetMsgConnByHandle(from_user_id, handle);


	log("HandleMsgListResp, req_cmd_id=%u, %u->%u, msg_cnt=%u\n",
			request_cmd_id, from_user_id, to_user_id, msg_cnt);

	if (pMsgConn && pMsgConn->IsOpen()) {
		uint32_t client_cmd_id = CID_MSG_UNREAD_MSG_RESPONSE;

		CImPduClientMsgListResponse pdu(client_cmd_id, to_id_url.c_str(), msg_cnt, server_msg_list);
		pdu.SetReserved(pPdu->GetReserved());
		pMsgConn->SendPdu(&pdu);
	}
}

void CDBServConn::_HandleDBWriteResponse(CImPduDBWriteResponse* pPdu)
{
	if (pPdu->GetResult() != 0) {
		log("DB write failed, req_id=%u, req_type=%u, %u->%u\n",
				pPdu->GetRequestId(), pPdu->GetRequestPduType(), pPdu->GetFromId(), pPdu->GetToId());
	}
}

void CDBServConn::_HandleUsersInfoResponse(CImPduUsersInfoResponse* pPdu)
{
	uint16_t serv_cmd_id = pPdu->GetCommandId();
	uint32_t from_user_id = pPdu->GetFromUserId();
	uint32_t user_cnt = pPdu->GetUserCnt();
	user_info_t* user_info_list = pPdu->GetUserInfoList();
    CDbAttachData attach_data(pPdu->GetAttachData(), pPdu->GetAttachLen());
	uint32_t handle = attach_data.GetHandle();
    CMsgConn* pMsgConn = CImUserManager::GetInstance()->GetMsgConnByHandle(from_user_id, handle);
    
	log("HandleUsersInfoResp, cmd_id=%u, from_id=%u, user_cnt=%u\n",
			serv_cmd_id, from_user_id, user_cnt);

	if (pMsgConn && pMsgConn->IsOpen()) {
        uint16_t cmd_id = (pPdu->GetCommandId() == IM_PDU_TYPE_USERS_INFO_RESPONSE) ?
            CID_BUDDY_LIST_USER_INFO_RESPONSE : CID_BUDDY_LIST_ALL_USER_RESPONSE;

        CImPduClientUserInfoResponse pdu(cmd_id, user_cnt, user_info_list);
        pdu.SetReserved(pPdu->GetReserved());
        pMsgConn->SendPdu(&pdu);
	}
}

void CDBServConn::_HandleStopReceivePacket(CImPduStopReceivePacket* pPdu)
{
	log("HandleStopReceivePacket, from %s:%d\n",
			g_db_server_list[m_serv_idx].server_ip.c_str(), g_db_server_list[m_serv_idx].server_port);

	m_bOpen = false;
}

void CDBServConn::_HandleDBQueryResponse(CImPduDBQueryResponse* pPdu)
{
	uint32_t user_id = pPdu->GetUserid();
	uint32_t seq_no = pPdu->GetSeqNo();
    CDbAttachData attach_data(pPdu->GetAttachData(), pPdu->GetAttachLen());
	uint32_t handle = attach_data.GetHandle();
    CMsgConn* pMsgConn = CImUserManager::GetInstance()->GetMsgConnByHandle(user_id, handle);
	log("HandleDBQueryResp, user_id=%u, seq_no=%u\n", user_id, seq_no);

	if (pMsgConn && pMsgConn->IsOpen()) {
		CImPduClientDBQueryResponse pdu(seq_no, pPdu->GetResultLen(), pPdu->GetResultData());
		pMsgConn->SendPdu(&pdu);
	}
}

void CDBServConn::_HandleRemoveSessionResponse(CImPduRemoveSessionResponse* pPdu)
{
	uint32_t req_user_id = pPdu->GetReqUserId();
	uint32_t result = pPdu->GetResult();
	uint32_t session_type = pPdu->GetSessionType();
	uint32_t session_id = pPdu->GetSessionId();
	log("HandleRemoveSessionResp, req_user_id=%u, result=%u, session_id=%u, type=%u\n",
			req_user_id, result, session_id, session_type);

    
    CDbAttachData attach_data(pPdu->GetAttachData(), pPdu->GetAttachLen());
	uint32_t handle = attach_data.GetHandle();
    CMsgConn* pMsgConn = CImUserManager::GetInstance()->GetMsgConnByHandle(req_user_id, handle);
	if (pMsgConn && pMsgConn->IsOpen()) {
		char* session_id_url = idtourl(session_id);
		CImPduClientRemoveSessionResponse pdu(result, session_type, session_id_url);
		pdu.SetReserved(pPdu->GetReserved());
		pMsgConn->SendPdu(&pdu);
	}
}

void CDBServConn::_HandleDepartmentResponse(CImPduDepartmentResponse *pPdu)
{
    uint32_t req_user_id = pPdu->GetReqUserId();
    uint32_t depart_cnt = pPdu->GetDepartCnt();
    department_info_t* depart_list = pPdu->GetDepartList();
    log("HandleDepartmentResp, req_user_id=%u, depart_cnt=%u\n", req_user_id, depart_cnt);
    CDbAttachData attach_data(pPdu->GetAttachData(), pPdu->GetAttachLen());
    uint32_t handle = attach_data.GetHandle();
    CMsgConn* pMsgConn = CImUserManager::GetInstance()->GetMsgConnByHandle(req_user_id, handle);
	if (pMsgConn && pMsgConn->IsOpen()) {
		CImPduClientDepartmentResponse pdu(depart_cnt, depart_list);
		pdu.SetReserved(pPdu->GetReserved());
		pMsgConn->SendPdu(&pdu);
	}

}