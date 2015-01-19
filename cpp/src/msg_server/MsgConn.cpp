/*
 * MsgConn.cpp
 *
 *  Created on: 2013-7-5
 *      Author: ziteng@mogujie.com
 */

#include "MsgConn.h"
#include "DBServConn.h"
#include "LoginServConn.h"
#include "RouteServConn.h"
#include "FileHandler.h"
#include "GroupChat.h"
#include "ImUser.h"
#include "AttachData.h"


#define TIMEOUT_WATI_LOGIN_RESPONSE		15000	// 15 seconds
#define TIMEOUT_WAITING_MSG_DATA_ACK	15000	// 15 seconds
#define LOG_MSG_STAT_INTERVAL			300000	// log message miss status in every 5 minutes;
#define MAX_MSG_CNT_PER_SECOND			20		// user can not send more than 20 msg in one second
static ConnMap_t g_msg_conn_map;

static uint64_t g_last_stat_tick;	// 上次显示丢包率信息的时间
static uint32_t g_up_msg_total_cnt = 0;		// 上行消息包总数
static uint32_t g_up_msg_miss_cnt = 0;		// 上行消息包丢数
static uint32_t g_down_msg_total_cnt = 0;	// 下行消息包总数
static uint32_t g_down_msg_miss_cnt = 0;	// 下行消息丢包数

static bool g_log_msg_toggle = true;	// 是否把收到的MsgData写入Log的开关，通过kill -SIGUSR2 pid 打开/关闭

static uint64_t g_last_sensitive_word_tick;
static CFileHandler* s_file_handler = NULL;
static CGroupChat* s_group_chat = NULL;

void msg_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	ConnMap_t::iterator it_old;
	CMsgConn* pConn = NULL;
	uint64_t cur_time = get_tick_count();

	for (ConnMap_t::iterator it = g_msg_conn_map.begin(); it != g_msg_conn_map.end();)
    {
        it_old = it;
        it++;
        pConn = (CMsgConn*)it_old->second;
        pConn->OnTimer(cur_time);
	}

	if (cur_time > g_last_stat_tick + LOG_MSG_STAT_INTERVAL) {
		g_last_stat_tick = cur_time;
		log("up_msg_cnt=%u, up_msg_miss_cnt=%u, down_msg_cnt=%u, down_msg_miss_cnt=%u\n",
			g_up_msg_total_cnt, g_up_msg_miss_cnt, g_down_msg_total_cnt, g_down_msg_miss_cnt);
	}
}

static void signal_handler_usr1(int sig_no)
{
	if (sig_no == SIGUSR1) {
		log("receive SIGUSR1\n");
		g_up_msg_total_cnt = 0;
		g_up_msg_miss_cnt = 0;
		g_down_msg_total_cnt = 0;
		g_down_msg_miss_cnt = 0;
	}
}

static void signal_handler_usr2(int sig_no)
{
	if (sig_no == SIGUSR2) {
		log("receive SIGUSR2\n");
		g_log_msg_toggle = !g_log_msg_toggle;
	}
}

static void signal_handler_hup(int sig_no)
{
	if (sig_no == SIGHUP) {
		CMsgConn* pConn = NULL;
		for (ConnMap_t::iterator it = g_msg_conn_map.begin(); it != g_msg_conn_map.end(); it++) {
			pConn = (CMsgConn*)it->second;
			if (pConn->IsOpen()) {
				pConn->SendUserActionLog(USER_ACTION_TYPE_LOGOUT);
			}
		}

		log("receive SIGHUP, send USER_ACTION_TYPE_LOGOUT and exit...\n");
		exit(0);
	}
}

void init_msg_conn()
{
	g_last_stat_tick = g_last_sensitive_word_tick = get_tick_count();
	signal(SIGUSR1, signal_handler_usr1);
	signal(SIGUSR2, signal_handler_usr2);
	signal(SIGHUP, signal_handler_hup);
	netlib_register_timer(msg_conn_timer_callback, NULL, 1000);
	s_file_handler = CFileHandler::getInstance();
	s_group_chat = CGroupChat::GetInstance();
}

////////////////////////////
CMsgConn::CMsgConn()
{
	m_bOpen = false;
    m_last_seq_no = 0;
	m_msg_cnt_per_sec = 0;
	m_send_msg_list.clear();
}

CMsgConn::~CMsgConn()
{

}

// must be called when the client is validated(m_bOpen=true);
void CMsgConn::SendUserStatusUpdate(uint32_t user_status)
{
    
	if (!m_bOpen) {
		return;
	}
    CImUser* pImUser = CImUserManager::GetInstance()->GetImUserByName(m_user_name);
    if (!pImUser) {
        return;
    }

	// 只有上下线通知才通知LoginServer
	if (user_status == USER_STATUS_ONLINE) {
		CImPduUserCntUpdate pdu(USER_CNT_INC, pImUser->GetUserId());
		send_to_all_login_server(&pdu);
        
        //if (pImUser->IsMsgConnEmpty()) {
        
        CImPduUserStatusUpdate pdu2(USER_STATUS_ONLINE, pImUser->GetUserId(), GetClientTypeFlag());
        send_to_all_route_server(&pdu2);
        //}
	} else if (user_status == USER_STATUS_OFFLINE) {
		CImPduUserCntUpdate pdu(USER_CNT_DEC, pImUser->GetUserId());
		send_to_all_login_server(&pdu);
        
        //if (pImUser->IsMsgConnEmpty()) {
        CImPduUserStatusUpdate pdu2(USER_STATUS_OFFLINE, pImUser->GetUserId(), GetClientTypeFlag());
        send_to_all_route_server(&pdu2);
        //}
	}
}

void CMsgConn::Close(bool kick_user)
{
	log("Close client, handle=%d, user_id=%u\n", m_handle, GetUserId());

	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_msg_conn_map.erase(m_handle);
	}
    
	if (m_user_name.length() > 0) {
        CImUser *pImUser = CImUserManager::GetInstance()->GetImUserByName(m_user_name);
        if (pImUser) {
            pImUser->DelMsgConn(GetToken());
                
            SendUserStatusUpdate(USER_STATUS_OFFLINE);
            if (pImUser->IsMsgConnEmpty()) {
                pImUser->SetIMOnlineStatus(USER_STATUS_OFFLINE);
                pImUser->SendStatusChangeToFriend(USER_STATUS_OFFLINE);
                CImUserManager::GetInstance()->RemoveImUser(pImUser);
            }
        }
    }

	if (IsOpen()) {
		if (kick_user)
			SendUserActionLog(USER_ACTION_TYPE_KICK);
		else
			SendUserActionLog(USER_ACTION_TYPE_LOGOUT);
	} else {
		SendUserActionLog(USER_ACTION_TYPE_FAIL);
	}

	ReleaseRef();
}

void CMsgConn::OnConnect(net_handle_t handle)
{
	m_handle = handle;
	m_login_time = get_tick_count();

	g_msg_conn_map.insert(make_pair(handle, this));

	netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)imconn_callback);
	netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)&g_msg_conn_map);
	netlib_option(handle, NETLIB_OPT_GET_REMOTE_IP, (void*)&m_peer_ip);
	netlib_option(handle, NETLIB_OPT_GET_REMOTE_PORT, (void*)&m_peer_port);
}

void CMsgConn::OnClose()
{
	Close();
}

void CMsgConn::OnTimer(uint64_t curr_tick)
{
	m_msg_cnt_per_sec = 0;

	//if (curr_tick > m_last_send_tick + CLIENT_HEARTBEAT_INTERVAL) {
	//	CImPduHeartbeat pdu;
	//	SendPdu(&pdu);
	//}

    if (CHECK_CLIENT_TYPE_MOBILE(GetClientType()))
    {
        if (curr_tick > m_last_recv_tick + MOBILE_CLIENT_TIMEOUT) {
            log("mobile client timeout, handle=%d, uid=%u\n", m_handle, GetUserId());
            Close();
            return;
        }
    }
    else
    {
        if (curr_tick > m_last_recv_tick + CLIENT_TIMEOUT) {
            log("client timeout, handle=%d, uid=%u\n", m_handle, GetUserId());
            Close();
            return;
        }
    }
	

	if (!IsOpen()) {
		if (curr_tick > m_login_time + TIMEOUT_WATI_LOGIN_RESPONSE) {
			log("login timeout, handle=%d, uid=%u\n", m_handle, GetUserId());
			Close();
			return;
		}
	}

	list<msg_ack_t>::iterator it_old;
	for (list<msg_ack_t>::iterator it = m_send_msg_list.begin(); it != m_send_msg_list.end(); ) {
		msg_ack_t msg = *it;
		it_old = it;
		it++;
		if (curr_tick >= msg.timestamp + TIMEOUT_WAITING_MSG_DATA_ACK) {
			log("!!!a msg missed, seq=%u, %u->%u\n", msg.seq_no, msg.from_id, GetUserId());
			g_down_msg_miss_cnt++;
			m_send_msg_list.erase(it_old);
		} else {
			break;
		}
	}
}

void CMsgConn::HandlePdu(CImPdu* pPdu)
{
	// request authorization check
	if ( (pPdu->GetPduType() != IM_PDU_TYPE_LOGIN_REQUEST) && !IsOpen() ) {
		return;
	}

	switch (pPdu->GetPduType()) {
	case IM_PDU_TYPE_HEARTBEAT:
        _HandleHeartBeat((CImPduHeartbeat*)pPdu);
		break;
	case IM_PDU_TYPE_LOGIN_REQUEST:
		_HandleLoginRequest( (CImPduLoginRequest*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_BUDDY_LIST_REQUEST:
		_HandleBuddyListRequest( (CImPduClientBuddyListRequest*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_MSG_DATA:
	{
		CImPduClientMsgData* pPduMsg = (CImPduClientMsgData*)pPdu;
		if (CHECK_MSG_TYPE_GROUP(pPduMsg->GetMsgType()))
            s_group_chat->HandleClientGroupMessage(pPduMsg, this);
		else
            _HandleClientMsgData(pPduMsg);
		break;
	}
	case IM_PDU_TYPE_CLIENT_MSG_DATA_ACK:
		_HandleClientMsgDataAck( (CImPduClientMsgDataAck*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_TIME_REQUEST:
		_HandleClientTimeRequest( (CImPduClientTimeRequest*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_UNREAD_MSG_CNT_REQUEST:
		_HandleClientUnreadMsgCntRequest( (CImPduClientUnreadMsgCntRequest*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_UNREAD_MSG_REQUEST:
		_HandleClientUnreadMsgRequest( (CImPduClientUnreadMsgRequest*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_MSG_READ_ACK:
		_HandleClientMsgReadAck( (CImPduClientMsgReadAck*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_P2P_CMD_MSG:
		_HandleClientP2PCmdMsg( (CImPduClientP2PCmdMsg*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_USER_INFO_REQUEST:
		_HandleClientUserInfoRequest( (CImPduClientUserInfoRequest*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_DB_QUERY_REQUEST:
		_HandleClientDBQueryRequest( (CImPduClientDBQueryRequest*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_REMOVE_SESSION_REQUEST:
		_HandleClientRemoveSessionRequest( (CImPduClientRemoveSessionRequest*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_ALL_USER_REQUEST:
		_HandleClientAllUserRequest( (CImPduClientAllUserRequest*)pPdu );
		break;

	case IM_PDU_TYPE_CLIENT_SERVICE_SETTING:
		_HandleClientServiceSetting( (CImPduClientServiceSetting*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_USER_STATUS_REQUEST:
		_HandleClientUserStatusRequest( (CImPduClientUserStatusRequest*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_USERS_STATUS_REQUEST:
		_HandleClientUsersStatusRequest((CImPduClientUsersStatusRequest*) pPdu);
		break;
    case IM_PDU_TYPE_CLIENT_DEPARTMENT_REQUEST:
        _HandleClientDepartmentRequest((CImPduClientDepartmentRequest*)pPdu);
        break;
    
	// for file process
	case IM_PDU_TYPE_CLIENT_FILE_REQUEST:
		s_file_handler->HandleClientFileRequest(this, (CImPduClientFileRequest*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_FILE_HAS_OFFLINE_REQ:
		s_file_handler->HandleClientFileHasOfflineReq(this, (CImPduClientFileHasOfflineReq*)pPdu);
		break;
	case IM_PDU_TYPE_CLIENT_FILE_ADD_OFFLINE_REQ:
		s_file_handler->HandleClientFileAddOfflineReq( (CImPduClientFileAddOfflineReq*)pPdu );
		break;
	case IM_PDU_TYPE_CLIENT_FILE_DEL_OFFLINE_REQ:
		s_file_handler->HandleClientFileDelOfflineReq( (CImPduClientFileDelOfflineReq*)pPdu );
		break;
            
            
	// for group process
	case IM_PDU_TYPE_CLIENT_GROUP_LIST_REQUEST:
		s_group_chat->HandleClientGroupListRequest( (CImPduClientGroupListRequest*)pPdu, this );
		break;
	case IM_PDU_TYPE_CLIENT_GROUP_USER_LIST_REQUEST:
		s_group_chat->HandleClientGroupUserListRequest( (CImPduClientGroupUserListRequest*)pPdu, this);
		break;
	case IM_PDU_TYPE_CLIENT_GROUP_UNREAD_MSG_CNT_REQUEST:
		s_group_chat->HandleClientGroupUnreadMsgCntRequest((CImPduClientGroupUnreadMsgCntRequest*)pPdu, this);
		break;
	case IM_PDU_TYPE_CLIENT_GROUP_UNREAD_MSG_REQUEST:
		s_group_chat->HandleClientGroupUnreadMsgRequest((CImPduClientGroupUnreadMsgRequest*)pPdu, this);
		break;
	case IM_PDU_TYPE_CLIENT_GROUP_MSG_READ_ACK:
		s_group_chat->HandleClientGroupMsgReadAck((CImPduClientGroupMsgReadAck*)pPdu, this);
		break;
	case IM_PDU_TYPE_CLIENT_GROUP_CREATE_TMP_GROUP_REQUEST:
		s_group_chat->HandleClientGroupCreateTmpGroupRequest((CImPduClientGroupCreateTmpGroupRequest*)pPdu, this);
		break;
	case IM_PDU_TYPE_CLIENT_GROUP_CHANGE_MEMBER_REQUEST:
		s_group_chat->HandleClientGroupChangeMemberRequest((CImPduClientGroupChangeMemberRequest*)pPdu, this);
		break;

	default:
		log("wrong msg, type=%d\n", pPdu->GetPduType());
		break;
	}
}

void CMsgConn::_HandleHeartBeat(CImPduHeartbeat* pPdu)
{
    CImPduHeartbeat pdu;
    SendPdu(&pdu);
}


// process: send validate request to db server
void CMsgConn::_HandleLoginRequest(CImPduLoginRequest* pPdu)
{
	// refuse second validate request
	if (m_user_name.length() != 0) {
        log("duplicate LoginRequest in the same conn\n");
		return;
    }

	// check if all server connection are OK
	uint32_t result = 0;
	CDBServConn* pDbConn = get_db_serv_conn_for_login();
	if (!pDbConn) {
		result = REFUSE_REASON_NO_DB_SERVER;
	}
	else if (!is_login_server_available()) {
		result = REFUSE_REASON_NO_LOGIN_SERVER;
	}
	else if (!is_route_server_available()) {
		result = REFUSE_REASON_NO_ROUTE_SERVER;
	}

	if (result) {
		CImPduLoginResponse pdu(result);
		pdu.SetReserved(pPdu->GetReserved());
		SendPdu(&pdu);
		Close();
		return;
	}
    
    m_user_name.append(pPdu->GetUserName(), pPdu->GetUserNameLen());
    string password(pPdu->GetPassword(), pPdu->GetPasswordLen());
	uint32_t online_status = pPdu->GetOnlineStatus();
    if (online_status < USER_STATUS_ONLINE || online_status > USER_STATUS_LEAVE) {
        log("HandleLoginReq, online status wrong: %u\n", online_status);
        online_status = USER_STATUS_ONLINE;
    }
	m_pdu_version = pPdu->GetVersion();
	m_client_version.append(pPdu->GetClientVersion(), pPdu->GetClientVersionLen());
    m_client_type = pPdu->GetClientType();

	log("HandleLoginReq, version=%u, user_name=%s, status=%u, client_type=%u, client=%s\n",
			m_pdu_version, m_user_name.c_str(), online_status, m_client_type, m_client_version.c_str());

    CImUser* pImUser = CImUserManager::GetInstance()->GetImUserByName(m_user_name);
    if (!pImUser) {
		pImUser = new CImUser(m_user_name);
        CImUserManager::GetInstance()->AddImUserByName(m_user_name, pImUser);
	}
    pImUser->SetIMOnlineStatus(online_status);
    pImUser->AddUnValidateMsgConn(this);

    CDbAttachData attach_data(ATTACH_TYPE_HANDLE, m_handle, 0);
    // continue to validate if the user is OK
    
	CImPduValidateRequest pduVR(m_user_name.c_str(), password.c_str(), attach_data.GetLength(),
                                attach_data.GetBuffer());
	pduVR.SetReserved(pPdu->GetReserved());
	pDbConn->SendPdu(&pduVR);
    
}

void CMsgConn::_HandleBuddyListRequest(CImPduClientBuddyListRequest* pPdu)
{
	log("HandleClientBuddyListReq, req_type=%u\n", pPdu->GetReqType());

	CDBServConn* pConn = get_db_serv_conn_for_login();
	if (!pConn) {
		return;
	}

	// 请求最近联系人列表
    CDbAttachData attach_data(ATTACH_TYPE_HANDLE, m_handle, 0);
	CImPduFriendListRequest pduFLR(GetUserId(), attach_data.GetLength(), attach_data.GetBuffer());
	pduFLR.SetReserved(pPdu->GetReserved());
	pConn->SendPdu(&pduFLR);
}

void CMsgConn::_HandleClientMsgData(CImPduClientMsgData* pPdu)
{
	if (pPdu->GetMsgLen() == 0) {
		log("discard an empty message, uid=%u\n", GetUserId());
		return;
	}

	if (m_msg_cnt_per_sec >= MAX_MSG_CNT_PER_SECOND) {
		log("!!!too much msg cnt in one second, uid=%u\n", GetUserId());
		return;
	}

	m_msg_cnt_per_sec++;

	// send back ACK
	CImPduClientMsgDataAck pduAck(pPdu->GetSeqNo(), pPdu->GetFromIdLen(), pPdu->GetFromIdUrl());
	pduAck.SetReserved(pPdu->GetReserved());
	SendPdu(&pduAck);

	uint32_t seq_no = pPdu->GetSeqNo();
	g_up_msg_total_cnt++;
	if (m_last_seq_no == 0) {
		m_last_seq_no = seq_no;
	} else {
		if (seq_no != m_last_seq_no + 1) {
			log("up msg miss, last_seq_no=%u, curr_seq_no=%u\n", m_last_seq_no, seq_no);
			g_up_msg_miss_cnt += seq_no - m_last_seq_no - 1;
		}

		m_last_seq_no = seq_no;
	}

	string to_id_url(pPdu->GetToIdUrl(), pPdu->GetToIdLen());
	uint32_t to_user_id = urltoid(to_id_url.c_str());
	uint8_t msg_type = pPdu->GetMsgType();
	string msg_data((char*)pPdu->GetMsgData(), pPdu->GetMsgLen());

	// 为了隐私，去掉消息内容log
	if (g_log_msg_toggle) {
		log("HandleClientMsgData, %d->%d\n", GetUserId(), to_user_id);
	}

	uint32_t cur_time = time(NULL);
	CImPduMsgData pdu(pPdu->GetSeqNo(), GetUserId(), to_user_id, cur_time, msg_type,
			pPdu->GetMsgLen(), (uchar_t*)msg_data.c_str(), GetClientType(),pPdu->GetAttachLen(),
                      pPdu->GetAttachData());

	// send to DB storage server
	CDBServConn* pDbConn = get_db_serv_conn();
	if (pDbConn) {
		pDbConn->SendPdu(&pdu);
	}

    CImUser* pFromImUser = CImUserManager::GetInstance()->GetImUserByName(m_user_name);
    CImUser* pToImUser = CImUserManager::GetInstance()->GetImUserById(to_user_id);
    pPdu->AddTime();
    pPdu->SetReserved(0);   //转发消息不带序号
	if (pFromImUser) {
		pFromImUser->BroadcastClientMsgData(pPdu, this, GetUserId());
		pFromImUser->AddOnlineFriend(to_user_id); // 聊过天就通知上下线消息
	}
    
	if (pToImUser) {
		pToImUser->BroadcastClientMsgData(pPdu, NULL, GetUserId());
		pToImUser->AddOnlineFriend(GetUserId());
	}
    
    // send to RouteServer
	CRouteServConn* pRouteConn = get_route_serv_conn();
	if (pRouteConn) {
		pRouteConn->SendPdu(&pdu);
	}
}

void CMsgConn::_HandleClientMsgDataAck(CImPduClientMsgDataAck* pPdu)
{
	uint32_t seq_no = pPdu->GetSeqNo();
	string from_id_url(pPdu->GetFromIdUrl(), pPdu->GetFromIdLen());
	uint32_t from_id = urltoid(from_id_url.c_str());
    
	DelFromSendList(seq_no, from_id);
}

void CMsgConn::_HandleClientTimeRequest(CImPduClientTimeRequest* pPdu)
{
	CImPduClientTimeResponse pdu;
	SendPdu(&pdu);
}

void CMsgConn::_HandleClientUnreadMsgCntRequest(CImPduClientUnreadMsgCntRequest* pPdu)
{
	log("HandleClientUnreadMsgCntReq, from_id=%u\n", GetUserId());
	CDBServConn* pDBConn = get_db_serv_conn_for_login();
	if (pDBConn) {
        CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
		CImPduUnreadMsgCountRequest pdu(GetUserId(), GetClientType(), attach.GetLength(), attach.GetBuffer());
		pdu.SetReserved(pPdu->GetReserved());
		pDBConn->SendPdu(&pdu);
	}
}

void CMsgConn::_HandleClientUnreadMsgRequest(CImPduClientUnreadMsgRequest* pPdu)
{
	string from_id_url(pPdu->GetFromIdUrl(), pPdu->GetFromIdLen());
	uint32_t from_id = urltoid(from_id_url.c_str());

	log("HandleClientUnreadMsgReq, %u->%u\n", from_id, GetUserId());
	CDBServConn* pDBConn = get_db_serv_conn();
	if (pDBConn) {
        CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
		CImPduUnreadMsgRequest pdu(GetUserId(), from_id, GetClientType(), attach.GetLength(), attach.GetBuffer());
		pdu.SetReserved(pPdu->GetReserved());
		pDBConn->SendPdu(&pdu);
	}
}

void CMsgConn::_HandleClientMsgReadAck(CImPduClientMsgReadAck* pPdu)
{
	string from_id_url(pPdu->GetFromIdUrl(), pPdu->GetFromIdLen());
	uint32_t from_id = urltoid(from_id_url.c_str());
	log("HandleClientMsgReadAck, %u->%u\n", from_id, GetUserId());

	CDBServConn* pDBConn = get_db_serv_conn();
	if (pDBConn) {
		CImPduMsgReadAck pdu(0, GetUserId(), from_id, GetClientType());
		pdu.SetReserved(pPdu->GetReserved());
		pDBConn->SendPdu(&pdu);
	}
}

void CMsgConn::_HandleClientP2PCmdMsg(CImPduClientP2PCmdMsg* pPdu)
{
	string from_id_url(pPdu->GetFromId(), pPdu->GetFromIdLen());
	string to_id_url(pPdu->GetToId(), pPdu->GetToIdLen());
	string cmd_msg((char*)pPdu->GetCmdMsgData(), pPdu->GetCmdMsgLen());
	uint32_t from_user_id = urltoid(from_id_url.c_str());
	uint32_t to_user_id = urltoid(to_id_url.c_str());

	log("HandleClientP2PCmdMsg, %u->%u, cmd_msg: %s\n", from_user_id, to_user_id, cmd_msg.c_str());
    if (to_user_id == GetUserId()) {
		log("***filter self P2PCmdMsg, user_id=%u\n", GetUserId());
		return;
	}
    
    CImUser* pFromImUser = CImUserManager::GetInstance()->GetImUserByName(m_user_name);
	CImUser* pToImUser = CImUserManager::GetInstance()->GetImUserById(to_user_id);
    
    if (pFromImUser) {
		pFromImUser->BroadcastPdu(pPdu, this);
	}
    
	if (pToImUser) {
		pToImUser->BroadcastPdu(pPdu, NULL);
	}
    
	CRouteServConn* pRouteConn = get_route_serv_conn();
	if (pRouteConn) {
		CImPduP2PMsg pdu(pPdu->GetSeqNo(), from_user_id, to_user_id,
                         pPdu->GetCmdMsgLen(), pPdu->GetCmdMsgData());
		pdu.SetReserved(pPdu->GetReserved());
		pRouteConn->SendPdu(&pdu);
	}
}

void CMsgConn::_HandleClientUserInfoRequest(CImPduClientUserInfoRequest* pPdu)
{
	uint32_t user_cnt = pPdu->GetUserCnt();
	log("HandleClientUserInfoReq, req_id=%u, user_cnt=%u\n", GetUserId(), user_cnt);
	CDBServConn* pDBConn = get_db_serv_conn_for_login();
	if (pDBConn) {
		client_id_t* url_id_list= pPdu->GetClientIdList();
		uint32_t* user_id_list = (uint32_t*)malloc(user_cnt * sizeof(uint32_t));
		for (uint32_t i = 0; i < user_cnt; i++) {
			string id_url(url_id_list[i].id_url, url_id_list[i].id_len);

			user_id_list[i] = urltoid(id_url.c_str());
		}

       CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
       CImPduUsersInfoRequest pdu(GetUserId(), user_cnt, user_id_list, attach.GetLength(),
                                   attach.GetBuffer());
		pdu.SetReserved(pPdu->GetReserved());
		pDBConn->SendPdu(&pdu);

		free(user_id_list);
	}
}

void CMsgConn::_HandleClientDBQueryRequest(CImPduClientDBQueryRequest* pPdu)
{
	uint32_t seq_no = pPdu->GetSeqNo();
	uint32_t query_len = pPdu->GetQueryLen();
	uchar_t* query_data = pPdu->GetQeuryData();
	log("HandleClientDBQueryReq, uid=%u, seq_no=%u\n", GetUserId(), seq_no);

	CDBServConn* pDBConn = get_db_serv_conn();
	if (pDBConn) {
		CImPduDBQueryRequest pdu(seq_no, GetUserId(), query_len, query_data);
		pdu.SetReserved(pPdu->GetReserved());
		pDBConn->SendPdu(&pdu);
	}
}

void CMsgConn::_HandleClientServiceSetting(CImPduClientServiceSetting* pPdu)
{
	uint32_t key = pPdu->GetKey();
	uint32_t value = pPdu->GetValue();
	log("HandleClientServiceSetting, user_id=%u, key=%u, value=%u\n", GetUserId(), key, value);

    CImUser* pImUser = CImUserManager::GetInstance()->GetImUserById(GetUserId());
	uint32_t online_status = pImUser->GetIMOnlineStatus();
	if ( (key == SERVICE_KEY_ONLINE_STATUS) && (value <= USER_STATUS_LEAVE) &&
        (online_status != value)) {
        pImUser->SetIMOnlineStatus(value);
        CImPduUserStatusUpdate pdu(value, GetUserId(), GetClientTypeFlag());
		pdu.SetReserved(pPdu->GetReserved());
		send_to_all_route_server(&pdu);

        pImUser->SendStatusChangeToFriend(value);

        // 把修改的状态通知给自己
		CImPduClientFriendNotify pduCFN(idtourl(GetUserId()), value);
		pduCFN.SetReserved(pPdu->GetReserved());
		pImUser->BroadcastPdu(&pduCFN, NULL);
        
		if (value == USER_STATUS_ONLINE) {
			SendUserActionLog(USER_ACTION_TYPE_ONLINE);
		} else {
			SendUserActionLog(USER_ACTION_TYPE_LEAVE);
		}

	}
}

void CMsgConn::_HandleClientUserStatusRequest(CImPduClientUserStatusRequest* pPdu)
{
	string query_user_id_url(pPdu->GetUserId(), pPdu->GetUserIdLen());
	uint32_t query_user_id = urltoid(query_user_id_url.c_str());
	log("HandleClientUserStatusReq, query_id=%u\n", query_user_id);

	CImUser* pImUser = CImUserManager::GetInstance()->GetImUserById(query_user_id);
	if (pImUser) {
		uint32_t status = pImUser->GetIMOnlineStatus();
		CImPduClientUserStatusResponse pdu(query_user_id_url.c_str(), status);
		pdu.SetReserved(pPdu->GetReserved());
		SendPdu(&pdu);	// 发给查询者，不是被查询者pConn
	} else {
		CRouteServConn* pRouteConn = get_route_serv_conn();
		if (pRouteConn) {
            CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
			CImPduUserStatusRequest pdu(GetUserId(), query_user_id, attach.GetLength(), attach.GetBuffer());
			pdu.SetReserved(pPdu->GetReserved());
			pRouteConn->SendPdu(&pdu);
		}
	}
}

void CMsgConn::_HandleClientUsersStatusRequest(CImPduClientUsersStatusRequest* pPdu)
{
	uint32_t user_count = pPdu->GetUserCount();
	log("HandleClientUsersStatusReq, query_count=%u\n", user_count);

	list<uint32_t> unavalible_list;
	list<user_stat_t> list_response;
	list<string>::const_iterator it;
	const list<string>& list = pPdu->GetUsers();
	user_stat_t status;
	uint32_t user_id = 0;
	for(it=list.begin(); it!=list.end(); ++it)
	{
		user_id = urltoid(it->c_str());
        CImUser* pUser = CImUserManager::GetInstance()->GetImUserById(user_id);
        if (pUser) {
            status.user_id = user_id;
        	status.status = pUser->GetIMOnlineStatus();
        	list_response.push_back(status);
        }
        else
        {
            unavalible_list.push_back(urltoid(it->c_str()));
        }
	}
    
	CImPduClientUsersStatusResponse pdu(list_response);
	pdu.SetReserved(pPdu->GetReserved());
	SendPdu(&pdu);

	if(unavalible_list.size() != 0)
	{
		CRouteServConn* pRouteConn = get_route_serv_conn();
		if(pRouteConn)
		{
            log("QueryUsersStatus on RouteServer, query_count=%u\n", user_count);
            CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle,0);
			CImPduUsersStatusRequest pdu(GetUserId(), unavalible_list, attach.GetLength(),
                                         attach.GetBuffer());
			pdu.SetReserved(pPdu->GetReserved());
			pRouteConn->SendPdu(&pdu);
		}
	}
}

void CMsgConn::_HandleClientRemoveSessionRequest(CImPduClientRemoveSessionRequest* pPdu)
{
	uint32_t session_type = pPdu->GetSessionType();
	string session_id_url(pPdu->GetSessionId(), pPdu->GetSessionIdLen());
	uint32_t session_id = urltoid(session_id_url.c_str());
	log("HandleClientRemoveSessionReq, user_id=%u, session_id=%u, type=%u\n", GetUserId(), session_id, session_type);

	CDBServConn* pConn = get_db_serv_conn();
	if (pConn) {
        CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
		CImPduRemoveSessionRequest pdu(GetUserId(), session_id, session_type, attach.GetLength(),
                                       attach.GetBuffer());
		pdu.SetReserved(pPdu->GetReserved());
		pConn->SendPdu(&pdu);
	}
}

void CMsgConn::_HandleClientAllUserRequest(CImPduClientAllUserRequest* pPdu)
{
	log("HandleClientAllUserReq, user_id=%u\n", GetUserId());

	CDBServConn* pConn = get_db_serv_conn();
	if (pConn) {
        CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);

		CImPduAllUserRequest pdu(GetUserId(), attach.GetLength(), attach.GetBuffer());
		pdu.SetReserved(pPdu->GetReserved());
        pConn->SendPdu(&pdu);
	}
}

void CMsgConn::_HandleClientDepartmentRequest(CImPduClientDepartmentRequest *pPdu)
{
    uint32_t user_id = GetUserId();
    log("HandleClientDepartmentReq, user_id=%u\n", user_id);
    CDBServConn* pConn = get_db_serv_conn();
    if (pConn) {
        CDbAttachData attach(ATTACH_TYPE_HANDLE, m_handle, 0);
        CImPduDepartmentRequest pdu(user_id, attach.GetLength(), attach.GetBuffer());
        pdu.SetReserved(pPdu->GetReserved());
        pConn->SendPdu(&pdu);
        
    }
}

void CMsgConn::AddToSendList(uint32_t seq_no, uint32_t from_id)
{
	//log("AddSendMsg, seq_no=%u, from_id=%u\n", seq_no, from_id);
	msg_ack_t msg;
	msg.seq_no = seq_no;
	msg.from_id = from_id;
	msg.timestamp = get_tick_count();
	m_send_msg_list.push_back(msg);

	g_down_msg_total_cnt++;
}

void CMsgConn::DelFromSendList(uint32_t seq_no, uint32_t from_id)
{
	//log("DelSendMsg, seq_no=%u, from_id=%u\n", seq_no, from_id);
	for (list<msg_ack_t>::iterator it = m_send_msg_list.begin(); it != m_send_msg_list.end(); it++) {
		msg_ack_t msg = *it;
		if ( (msg.seq_no == seq_no) && (msg.from_id == from_id) ) {
			m_send_msg_list.erase(it);
			break;
		}
	}
}

void CMsgConn::SendUserActionLog(uint32_t action_type)
{
}

uint32_t CMsgConn::GetUserId()
{
    uint32_t user_id = 0;
    CImUser* pUser = CImUserManager::GetInstance()->GetImUserByName(GetUserName());
    if (pUser) {
        user_id = pUser->GetUserId();
    }
    return user_id;
}

uint32_t CMsgConn::GetClientTypeFlag()
{
    uint32_t client_type_flag = 0x00;
    if (CHECK_CLIENT_TYPE_PC(GetClientType()))
    {
        client_type_flag = CLIENT_TYPE_FLAG_PC;
    }
    else if (CHECK_CLIENT_TYPE_MOBILE(GetClientType()))
    {
        client_type_flag = CLIENT_TYPE_FLAG_MOBILE;
    }
    return client_type_flag;
}
