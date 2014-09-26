/*
 * RouteConn.cpp
 *
 *  Created on: 2013-7-4
 *      Author: ziteng@mogujie.com
 */

#include "netlib.h"
#include "RouteConn.h"

/* key: RouteConn*, value: client_type_flag */
typedef map<CRouteConn*, uint32_t> RouteConnMap_t;
typedef struct {
    uint32_t 	status;
    RouteConnMap_t  	conns;
} UserStat_t;

typedef hash_map<uint32_t, UserStat_t> UserStatMap_t;

static ConnMap_t g_route_conn_map;
static UserStatMap_t g_rs_user_map;

RouteConnMap_t* get_route_conns(uint32_t user_id)
{
	RouteConnMap_t* pConnMap = NULL;
	UserStatMap_t::iterator it = g_rs_user_map.find(user_id);
	if (it != g_rs_user_map.end()) {
		pConnMap = &(it->second.conns);
	}
	return pConnMap;
}

void route_serv_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	uint64_t cur_time = get_tick_count();
	for (ConnMap_t::iterator it = g_route_conn_map.begin(); it != g_route_conn_map.end(); ) {
		ConnMap_t::iterator it_old = it;
		it++;

		CRouteConn* pConn = (CRouteConn*)it_old->second;
		pConn->OnTimer(cur_time);
	}
}

void init_routeconn_timer_callback()
{
	//s_route_file_handler = CRouteFileHandler::getInstance();
	netlib_register_timer(route_serv_timer_callback, NULL, 1000);
}

CRouteConn::CRouteConn()
{
	m_bMaster = false;
}

CRouteConn::~CRouteConn()
{

}

void CRouteConn::Close()
{
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_route_conn_map.erase(m_handle);
	}

	// remove all user info from this MessageServer
	UserStatMap_t::iterator it_old;
	for (UserStatMap_t::iterator it = g_rs_user_map.begin(); it != g_rs_user_map.end(); ) {
		it_old = it;
		it++;
        
		RouteConnMap_t::iterator conn_it = it_old->second.conns.find(this);
		if (conn_it != it_old->second.conns.end()) {
			it_old->second.conns.erase(conn_it);
            
			if (it_old->second.conns.empty()) {
				g_rs_user_map.erase(it_old);
			}
		}
	}

	ReleaseRef();
}

void CRouteConn::OnConnect(net_handle_t handle)
{
	m_handle = handle;

	g_route_conn_map.insert(make_pair(handle, this));

	netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)imconn_callback);
	netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)&g_route_conn_map);
}

void CRouteConn::OnClose()
{
	log("MsgServer onclose: handle=%d\n", m_handle);
	Close();
}

void CRouteConn::OnTimer(uint64_t curr_tick)
{
	if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {
		CImPduHeartbeat pdu;
		SendPdu(&pdu);
	}

	if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
		log("message server timeout\n");
		Close();
	}
}

void CRouteConn::HandlePdu(CImPdu* pPdu)
{
	switch (pPdu->GetPduType()) {
	case IM_PDU_TYPE_HEARTBEAT:
		// do not take any action, heart beat only update m_last_recv_tick
		break;
	case IM_PDU_TYPE_ONLINE_USER_INFO:
		_HandleOnlineUserInfo( (CImPduOnlineUserInfo*)pPdu );
		break;
	case IM_PDU_TYPE_USER_STATUS_UPDATE:
		_HandleUserStatusUpdate( (CImPduUserStatusUpdate*)pPdu );
		break;
	case IM_PDU_TYPE_FRIEND_STATUS_QUERY:
		_HandleFriendStatusQuery( (CImPduFriendStatusQuery*)pPdu );
		break;
	case IM_PDU_TYPE_FRIEND_STATUS_NOTIFY:
		_HandleFriendStatusNotify( (CImPduFriendStatusNotify*)pPdu );
		break;
	case IM_PDU_TYPE_MSG_DATA:
		_HandleMsgData( (CImPduMsgData*)pPdu );
		break;
	case IM_PDU_TYPE_P2P_MSG:
		_HandleP2PMsg( (CImPduP2PMsg*)pPdu );
		break;
	case IM_PDU_TYPE_ROLE_SET:
		_HandleRoleSet( (CImPduRoleSet*)pPdu );
		break;
	case IM_PDU_TYPE_USER_STATUS_REQUEST:
		_HandleUserStatusRequest( (CImPduUserStatusRequest*)pPdu );
		break;
	case IM_PDU_TYPE_USERS_STATUS_REQUEST:
		_HandleUsersStatusRequest( (CImPduUsersStatusRequest*)pPdu );
		break;
    case IM_PDU_TYPE_SERVER_KICK_USER:
        _HandleKickOut((CImPduServerKickUser *)pPdu);
        break;
   
    case IM_PDU_TYPE_USER_CLIENT_TYPE_REQUEST:
        _HandleUserClientType((CImPduUserClientTypeRequest *)pPdu);
        break;
     
	case IM_PDU_TYPE_FILE_NOTIFY:
		_HandleFileNotify((CImPduFileNotify*)pPdu);
		break;
	case IM_PDU_TYPE_GROUP_CREATE_TMP_GROUP_RESPONSE:
	case IM_PDU_TYPE_GROUP_CHANGE_MEMBER_RESPONSE:
    case IM_PDU_TYPE_GROUP_CREATE_NORMAL_GROUP_NOTIFY:
    case IM_PDU_TYPE_GROUP_CHANGE_MEMBER_NOTIFY:
		_BroadcastMsg(pPdu);
		break;
    
    case IM_PDU_TYPE_GROUP_P2P_MSG_RESPONSE:
        _BroadcastMsg(pPdu);
        break;
        
	default:
		log("CRouteConn::HandlePdu, wrong pdu type: %d\n", pPdu->GetPduType());
		break;
	}
}

void CRouteConn::_HandleOnlineUserInfo(CImPduOnlineUserInfo* pPdu)
{
	uint32_t user_count = pPdu->GetUserCount();
	user_conn_stat_t* user_status_list = pPdu->GetUserStatusList();

	log("HandleOnlineUserInfo, user_cnt=%u\n", user_count);

	for (uint32_t i = 0; i < user_count; i++) {
		_UpdateUserStatus(user_status_list[i].user_id, user_status_list[i].status,
                          user_status_list[i].client_type_flag);
	}
}

void CRouteConn::_HandleUserStatusUpdate(CImPduUserStatusUpdate* pPdu)
{
	uint32_t user_status = pPdu->GetUserStatus();
	uint32_t user_id = pPdu->GetUserId();

	log("HandleUserStatusUpdate, action=%d, uid=%d\n", user_status, user_id);

	_UpdateUserStatus(pPdu->GetUserId(), pPdu->GetUserStatus(), pPdu->GetClientTypeFlag());
}

void CRouteConn::_HandleFriendStatusQuery(CImPduFriendStatusQuery* pPdu)
{
	uint32_t from_user_id = pPdu->GetFromUserId();
	uint32_t friend_cnt = pPdu->GetFriendCnt();
	uint32_t* friend_id_list = pPdu->GetFriendIdList();
	list<user_stat_t> friend_status_list;
	user_stat_t stat;

	log("HandleOnlineQuery, from_id=%d\n", from_user_id);

	for (uint32_t i = 0; i < friend_cnt; i++) {
		uint32_t friend_id = friend_id_list[i];
		UserStatMap_t::iterator it = g_rs_user_map.find(friend_id);
		if (it != g_rs_user_map.end()) {
			stat.user_id = friend_id;
			stat.status = it->second.status;
			friend_status_list.push_back(stat);
		}
	}

	CImPduFriendStatusList pdu(pPdu->GetRequestType(), from_user_id, &friend_status_list);
	SendPdu(&pdu);
}

void CRouteConn::_HandleFriendStatusNotify(CImPduFriendStatusNotify* pPdu)
{
	uint32_t from_user_id = pPdu->GetFromUserId();
	uint32_t from_user_status = pPdu->GetFromUserStatus();
	uint32_t friend_cnt = pPdu->GetFriendCnt();

	log("HandleFriendNotify, from_id=%u, status=%u, friend_cnt=%u\n", from_user_id, from_user_status,
        friend_cnt);
    if (from_user_status == USER_STATUS_OFFLINE) {
        UserStatMap_t::iterator it = g_rs_user_map.find(from_user_id);
        if (it != g_rs_user_map.end()) {
            uint32_t status = it->second.status;
            if (status != USER_STATUS_OFFLINE) {
                // user is still online, do not send status leave status to other
                return;
            }
        }
	}
	for (ConnMap_t::iterator it = g_route_conn_map.begin();  it != g_route_conn_map.end(); it++) {
		CRouteConn* pConn = (CRouteConn*)it->second;
		if (pConn) {
			pConn->SendPdu(pPdu);
		}
	}
}

void CRouteConn::_HandleMsgData(CImPduMsgData* pPdu)
{
	uint32_t from_user_id = pPdu->GetFromUserId();
	uint32_t to_user_id = pPdu->GetToUserId();
	uint8_t msg_type = pPdu->GetMsgType();
	log("HandleMsgData, %u->%u, msg_type=%u\n", from_user_id, to_user_id, msg_type);

    _DeliverMsgData(from_user_id, to_user_id, pPdu);
}

void CRouteConn::_HandleP2PMsg(CImPduP2PMsg* pPdu)
{
	uint32_t from_user_id = pPdu->GetFromUserId();
	uint32_t to_user_id = pPdu->GetToUserId();

	log("HandleP2PMsg, %u->%u\n", from_user_id, to_user_id);

	_DeliverMsgData(from_user_id, to_user_id, pPdu);
}

void CRouteConn::_HandleRoleSet(CImPduRoleSet* pPdu)
{
	uint32_t master = pPdu->GetMaster();

	log("HandleRoleSet, master=%u, handle=%u\n", master, m_handle);
	if (master == 1) {
		m_bMaster = true;
	} else {
		m_bMaster = false;
	}
}

void CRouteConn::_HandleUserStatusRequest(CImPduUserStatusRequest* pPdu)
{
	uint32_t request_id = pPdu->GetRequestId();
	uint32_t query_id = pPdu->GetQueryId();
	uint32_t attach_len = pPdu->GetAttachLen();
	uchar_t* attach_data = pPdu->GetAttachData();
	log("HandleUserStatusReq, req_id=%u, query_id=%u\n", request_id, query_id);

	uint32_t status = USER_STATUS_OFFLINE;
	UserStatMap_t::iterator it = g_rs_user_map.find(query_id);
	if (it != g_rs_user_map.end()) {
		status = it->second.status;
	}

	// send back query user status
	CImPduUserStatusResponse pdu(request_id, query_id, status, attach_len, attach_data);
	pdu.SetReserved(pPdu->GetReserved());
	SendPdu(&pdu);
}

void CRouteConn::_HandleUsersStatusRequest(CImPduUsersStatusRequest* pPdu)
{
	uint32_t request_id = pPdu->GetRequestId();
	uint32_t query_count = pPdu->GetQueryCount();
	log("HandleUserStatusReq, req_id=%u, query_count=%u\n", request_id, query_count);

	const list<uint32_t>& query_list = pPdu->GetQueryList();
	list<user_stat_t> result_list;
	user_stat_t status;
	for(list<uint32_t>::const_iterator itQ=query_list.begin(); itQ!=query_list.end(); ++itQ)
	{
		UserStatMap_t::iterator it = g_rs_user_map.find(*itQ);
		if (it != g_rs_user_map.end()) {
			status.status = it->second.status;
		}
		else
		{
			status.status = USER_STATUS_OFFLINE;
		}
		status.user_id = *itQ;
		result_list.push_back(status);
	}

	// send back query user status
	CImPduUsersStatusResponse pdu(request_id, result_list);
	pdu.SetReserved(pPdu->GetReserved());
	SendPdu(&pdu);
}

void CRouteConn::_HandleKickOut(CImPduServerKickUser *pPdu)
{
    uint32_t user_id = pPdu->GetUserId();
	log("HandleKickUser, user_id=%u\n", user_id);
	_BroadcastMsg(pPdu);
}

void CRouteConn::_HandleUserClientType(CImPduUserClientTypeRequest *pPdu)
{
    uint32_t user_id = pPdu->GetUserId();
    uint32_t attach_len = pPdu->GetAttachLen();
    uchar_t* attach_data = pPdu->GetAttachData();
    log("HandleUserClientType, user_id = %u\n", user_id);
    RouteConnMap_t* pConnMap = get_route_conns(user_id);
    uint32_t client_type_flag = 0x00;
    if (pConnMap != NULL)
    {
        RouteConnMap_t::iterator it = pConnMap->begin();
        for (; it != pConnMap->end(); it++)
        {
            if (it->first != this)
            {
                uint32_t client_type_flag_tmp = it->second;
                client_type_flag |= client_type_flag_tmp;
            }
        }
    }
    
    CImPduUserClientTypeResponse pdu(user_id, client_type_flag, attach_len, attach_data);
    pdu.SetReserved(pPdu->GetReserved());
    SendPdu(&pdu);
}

void CRouteConn::_HandleFileNotify(CImPduFileNotify *pPdu)
{
    uint32_t from_id = pPdu->GetFromId();
	uint32_t to_id = pPdu->GetToId();
	string file_name(pPdu->GetFileName(), pPdu->GetFileNameLen());
    
	log("HandleFileNotify, %u->%u, fileName: %s\n", from_id, to_id, file_name.c_str());
    
	RouteConnMap_t* pConnMap = get_route_conns(to_id);
    if (pConnMap) {
        for (RouteConnMap_t::iterator it = pConnMap->begin(); it != pConnMap->end(); it++)
        {
            CRouteConn* pToConn = it->first;
            if (pToConn && pToConn != this) {
                pToConn->SendPdu(pPdu);
            }
        }
    }
}

/*
 * update user status info, the logic seems complex
 */
void CRouteConn::_UpdateUserStatus(uint32_t user_id, uint32_t status, uint32_t client_type_flag)
{
	UserStatMap_t::iterator it = g_rs_user_map.find(user_id);
	if (it != g_rs_user_map.end())
    {
        RouteConnMap_t::iterator conn_it = it->second.conns.find(this);
        if (conn_it != it->second.conns.end())
        {
            uint32_t client_type_flag_tmp = conn_it->second ^ client_type_flag;
            
            if (status == USER_STATUS_OFFLINE)
            {
                //the unique client of msg_server offline
                if (client_type_flag_tmp == CLIENT_TYPE_FLAG_NONE)
                {
                    it->second.conns.erase(conn_it);
                    if (it->second.conns.empty())
                    {
                        g_rs_user_map.erase(it);
                    }
                }
                else
                {
                    //still have other client
                    conn_it->second = client_type_flag_tmp;
                }
            }
            else
            {
                if (client_type_flag_tmp == CLIENT_TYPE_FLAG_NONE)
                {
                    log("update user status online wrong, client type flag is none.\n");
                }
                else
                {
                    conn_it->second = client_type_flag_tmp;
                    it->second.status = status;
                }
			}
		}
        else
        {
			if (status != USER_STATUS_OFFLINE)
           {	// the connection is from different MsgServer
				//log("the same user from different server, user_id=%u\n", user_id);
               it->second.status = status;
				it->second.conns.insert(make_pair(this, client_type_flag));
			}
		}
	}
    else
    {
		if (status != USER_STATUS_OFFLINE) {
			UserStat_t user_stat;
			user_stat.status = status;
			user_stat.conns.insert(make_pair(this, client_type_flag));
			g_rs_user_map.insert(make_pair(user_id, user_stat));
		}
	}
}

void CRouteConn::_BroadcastMsg(CImPdu* pPdu)
{
	ConnMap_t::iterator it;
	for (it = g_route_conn_map.begin(); it != g_route_conn_map.end(); it++) {
		CRouteConn* pRouteConn = (CRouteConn*)it->second;
		if (pRouteConn != this) {
			pRouteConn->SendPdu(pPdu);
		}
	}
}

void CRouteConn::_DeliverMsgData(uint32_t from_id, uint32_t to_id, CImPdu* pPdu)
{
	uint32_t pdu_type = pPdu->GetPduType();
	if (pdu_type == IM_PDU_TYPE_MSG_DATA) {
		CImPduMsgData* pMsgPdu = (CImPduMsgData*)pPdu;
		uint8_t msg_type = pMsgPdu->GetMsgType();
        if(CHECK_MSG_TYPE_GROUP(msg_type)) { //group msg should broadcast
            _BroadcastMsg(pPdu);
            return;
        }
	}
    
    set<CRouteConn*> conn_set;
    UserStatMap_t::iterator it;
     
    it = g_rs_user_map.find(from_id);
    if (it != g_rs_user_map.end())
    {
        for (RouteConnMap_t::iterator conn_iter = it->second.conns.begin(); conn_iter != it->second.conns.end();
         conn_iter++)
        {
            CRouteConn* pConn = conn_iter->first;
            if (pConn != this)
            {
                conn_set.insert(pConn);
            }
        }
    }
     
     
    it = g_rs_user_map.find(to_id);
    if (it != g_rs_user_map.end())
    {
        for (RouteConnMap_t::iterator conn_iter = it->second.conns.begin();
             conn_iter != it->second.conns.end();
             conn_iter++)
        {
            CRouteConn* pConn = conn_iter->first;
            if (pConn != this)
            {
                conn_set.insert(pConn);
            }
        }
    }
     
    for (set<CRouteConn*>::iterator conn_iter = conn_set.begin(); conn_iter != conn_set.end();
         conn_iter++)
    {
        CRouteConn* pConn = *conn_iter;
        pConn->SendPdu(pPdu);
    }
    
}
