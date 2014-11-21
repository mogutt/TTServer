/*
 * GroupChat.cpp
 *
 *  Created on: 2014-1-3
 *      Author: ziteng@mogujie.com
 */

#include "GroupChat.h"
#include "MsgConn.h"
#include "DBServConn.h"
#include "RouteServConn.h"
#include "AttachData.h"
#include "ImUser.h"
CGroupChat* CGroupChat::s_group_chat_instance = NULL;

CGroupChat* CGroupChat::GetInstance()
{
	if (!s_group_chat_instance) {
		s_group_chat_instance = new CGroupChat();
	}

	return s_group_chat_instance;
}

void CGroupChat::HandleClientGroupListRequest(CImPduClientGroupListRequest* pPdu, CMsgConn* pFromConn)
{
	uint16_t cmd_id = pPdu->GetCommandId();
	uint16_t serv_cmd_id = (cmd_id == CID_GROUP_LIST_REQUEST) ?
						IM_PDU_TYPE_GROUP_LIST_REQUEST : IM_PDU_TYPE_GROUP_DIALOG_LIST_REQUEST;
	uint16_t cli_res_cmd_id = (cmd_id == CID_GROUP_LIST_REQUEST) ?
						CID_GROUP_LIST_RESPONSE : CID_GROUP_DIALOG_LIST_RESPONSE;
	uint32_t user_id = pFromConn->GetUserId();
	log("HandleClientGroupListReq, cmd_id=%u, user_id=%u\n", cmd_id, user_id);

	CDBServConn* pDbConn = get_db_serv_conn();
	if (pDbConn) {
        CDbAttachData attach_data(ATTACH_TYPE_HANDLE, pFromConn->GetHandle(), 0);
		CImPduGroupListRequest pdu(serv_cmd_id, user_id, attach_data.GetLength(), attach_data.GetBuffer());
		pdu.SetReserved(pPdu->GetReserved());
		pDbConn->SendPdu(&pdu);
	} else {
		CImPduClientGroupListResponse pdu(pFromConn->GetPduVersion(), cli_res_cmd_id, 0);
		pdu.SetReserved(pPdu->GetReserved());
		pFromConn->SendPdu(&pdu);
	}
}

void CGroupChat::HandleGroupListResponse(CImPduGroupListResponse* pPdu)
{
	uint16_t serv_cmd_id = pPdu->GetCommandId();
	uint16_t resp_cmd_id = (serv_cmd_id == IM_PDU_TYPE_GROUP_LIST_RESPONSE) ?
			CID_GROUP_LIST_RESPONSE : CID_GROUP_DIALOG_LIST_RESPONSE;
	uint32_t req_user_id = pPdu->GetReqUserId();
	uint32_t group_cnt = pPdu->GetGroupCnt();
    CDbAttachData attach_data(pPdu->GetAttachData(), pPdu->GetAttachLen());
	uint32_t handle = attach_data.GetHandle();
    
	group_info_t* group_list = pPdu->GetGroupList();
	log("HandleGroupListRes, cmd_id=%u, req_id=%u, group_cnt=%u\n", serv_cmd_id, req_user_id, group_cnt);

	for (uint32_t i = 0; i < group_cnt; i++) {
		_UpdateGroupMap(group_list[i].group_id, &(group_list[i].group_memeber_list));
	}

    CMsgConn* pConn = CImUserManager::GetInstance()->GetMsgConnByHandle(req_user_id, handle);
	if (pConn) {
		CImPduClientGroupListResponse pdu(resp_cmd_id, group_cnt, group_list);
		pdu.SetReserved(pPdu->GetReserved());
		pConn->SendPdu(&pdu);
	}

	CRouteServConn* pRouteConn = get_route_serv_conn();
	if (pRouteConn && group_cnt > 0) {
		set<uint32_t> all_id_set;
		for (uint32_t i = 0; i < group_cnt; i++) {
			for (list<uint32_t>::iterator it = group_list[i].group_memeber_list.begin();
					it != group_list[i].group_memeber_list.end();
					it++) {
				if (*it != req_user_id) {
					all_id_set.insert(*it);
				}
			}
		}

		list<uint32_t> all_id_list;
		for (set<uint32_t>::iterator it = all_id_set.begin(); it != all_id_set.end(); it++) {
			all_id_list.push_back(*it);
		}
		CImPduFriendStatusQuery pdu(2, req_user_id, &all_id_list);
		pRouteConn->SendPdu(&pdu);
	}
}

void CGroupChat::HandleClientGroupUserListRequest(CImPduClientGroupUserListRequest* pPdu, CMsgConn* pFromConn)
{
	uint32_t user_id = pFromConn->GetUserId();
	string group_id_url(pPdu->GetGroupId(), pPdu->GetGroupIdLen());
	uint32_t group_id = urltoid(group_id_url.c_str());
	log("HandleClientGroupUserListReq, user_id=%u, group_id=%u\n", user_id, group_id);

	CDBServConn* pDbConn = get_db_serv_conn();
	if (pDbConn) {
        CPduAttachData pduAttachData(ATTACH_TYPE_HANDLE_AND_PDU, pFromConn->GetHandle(), 0, (uchar_t*)pPdu);
		CImPduGroupUserListRequest pdu(user_id, group_id, pduAttachData.GetLength(), pduAttachData.GetBuffer());
		pdu.SetReserved(pPdu->GetReserved());
		pDbConn->SendPdu(&pdu);
	} else {
		CImPduClientGroupUserListResponse pdu(pFromConn->GetPduVersion(), group_id_url.c_str(), 2);
		pdu.SetReserved(pPdu->GetReserved());
		pFromConn->SendPdu(&pdu);
	}
}

void CGroupChat::HandleGroupUserListResponse(CImPduGroupUserListResponse* pPdu)
{
	uint32_t req_user_id = pPdu->GetReqUserId();
	uint32_t group_id = pPdu->GetGroupId();
	uint32_t result = pPdu->GetResult();
	string group_name(pPdu->GetGroupName(), pPdu->GetGroupNameLen());
	string group_avatar(pPdu->GetGroupAvatar(), pPdu->GetGroupAvatarLen());
	uint32_t group_creator_id = pPdu->GetGroupCreatorId();
	uint32_t group_type = pPdu->GetGroupType();
	uint32_t user_cnt = pPdu->GetUserCnt();
	uint32_t* user_list = pPdu->GetUserList();
	uint32_t attach_len = pPdu->GetAttachLen();
	uchar_t* attach_data = pPdu->GetAttachData();
	log("HandleGroupUserListRes, req_user_id=%u, group_id=%u, group_name=%s, group_avatar=%s, creator_id=%u, user_cnt=%u\n",
			req_user_id, group_id, group_name.c_str(), group_avatar.c_str(), group_creator_id, user_cnt);

    CPduAttachData pduAttachData(attach_data, attach_len);

	if (pduAttachData.GetPduLength() > 0) {
		log("GroupUserListRequest is send by server, group_id=%u\n", group_id);

		if (result) {
			log("something very bad happened!!!\n");
			return;
		}

		_UpdateGroupMap(group_id, user_cnt, user_list);

		group_member_t* group_member = _GetGroupMember(group_id);
		if (group_member) {
			group_member_t::iterator it;
			for (it = group_member->begin(); it != group_member->end(); it++) {
				uint32_t member_user_id = *it;
                CImUser* pImUser = CImUserManager::GetInstance()->GetImUserById(member_user_id);
                if (pImUser) {
                    CMsgConn* pFromConn = NULL;
                    if( member_user_id == req_user_id ){
                        uint32_t reqHandle = pduAttachData.GetHandle();
                        if(reqHandle != 0)
                            pFromConn = CImUserManager::GetInstance()->GetMsgConnByHandle(req_user_id, reqHandle);
                    }
                    pImUser->BroadcastData(pduAttachData.GetPdu(), pduAttachData.GetPduLength(), pFromConn);
                }
			}
		}
		return;
	}


	//CMsgConn* pConn = get_msg_conn(req_user_id);
	CMsgConn* pConn = CImUserManager::GetInstance()->GetMsgConnByHandle(req_user_id, pduAttachData.GetHandle());
    if (pConn) {
		string group_id_url = idtourl(group_id);
		string group_creator_id_url = idtourl(group_creator_id);
		CImPduClientGroupUserListResponse pdu(pConn->GetPduVersion(), group_id_url.c_str(), result, group_name.c_str(), group_avatar.c_str(),
				group_creator_id_url.c_str(), group_type, user_cnt, user_list);
		pdu.SetReserved(pPdu->GetReserved());
		pConn->SendPdu(&pdu);
	}

	if (result)
		return;

	_UpdateGroupMap(group_id, user_cnt, user_list);

	// query group member's online status
	CRouteServConn* pRouteConn = get_route_serv_conn();
	if (pRouteConn) {
		list<uint32_t> member_id_list;
		for (uint32_t i = 0; i < user_cnt; i++) {
			if (user_list[i] != req_user_id) {
				member_id_list.push_back(user_list[i]);
			}
		}

		CImPduFriendStatusQuery pdu(2, req_user_id, &member_id_list);
		pRouteConn->SendPdu(&pdu);
	}
}

void CGroupChat::HandleClientGroupMessage(CImPduClientMsgData* pPdu, CMsgConn* pFromConn)
{
	if (pPdu->GetMsgLen() == 0)
		return;

	// send back ACK
	CImPduClientMsgDataAck pduAck(pPdu->GetSeqNo(), pPdu->GetFromIdLen(), pPdu->GetFromIdUrl());
	pduAck.SetReserved(pPdu->GetReserved());
	pFromConn->SendPdu(&pduAck);

	uint32_t from_user_id = pFromConn->GetUserId();
	string to_group_id_url(pPdu->GetToIdUrl(), pPdu->GetToIdLen());
	uint32_t to_group_id = urltoid(to_group_id_url.c_str());
	uint8_t msg_type = pPdu->GetMsgType();
	string msg_data((char*)pPdu->GetMsgData(), pPdu->GetMsgLen());
	log("HandleClientGroupMsg, %d->%d, content: %s\n", from_user_id, to_group_id, msg_data.c_str());

	uint32_t cur_time = time(NULL);
	CImPduMsgData msg_pdu(pPdu->GetSeqNo(), from_user_id, to_group_id, cur_time, msg_type,
			pPdu->GetMsgLen(), pPdu->GetMsgData(), pFromConn->GetClientType(), pPdu->GetAttachLen(), pPdu->GetAttachData());

	// send to DB storage server
	CDBServConn* pDbConn = get_db_serv_conn();
	if (pDbConn) {
		pDbConn->SendPdu(&msg_pdu);
	}

	// send to route server
	CRouteServConn* pRouteConn = get_route_serv_conn();
	if (pRouteConn) {
		pRouteConn->SendPdu(&msg_pdu);
	}

	// send to group members in this server
	group_member_t* group_member = _GetGroupMember(to_group_id);
	if (group_member) {
		pPdu->AddTime();
		group_member_t::iterator it;
		for (it = group_member->begin(); it != group_member->end(); it++) {
			uint32_t member_user_id = *it;
            CImUser* pImUser = CImUserManager::GetInstance()->GetImUserById(member_user_id);
            if (pImUser) {
                pImUser->BroadcastPdu(pPdu, pFromConn);
            }
		}
	} else {
		// 服务器没有该群的信息，向DB服务器请求群信息，并带上消息作为附件，返回时在发送该消息给其他群成员
        log("this server does not have enough group info\n");
        CPduAttachData pduAttachData(ATTACH_TYPE_HANDLE_AND_PDU, pFromConn->GetHandle(), pPdu->GetLength(), pPdu->GetBuffer());
		CImPduGroupUserListRequest pdu(from_user_id, to_group_id, pduAttachData.GetLength(), pduAttachData.GetBuffer());
        
		pDbConn->SendPdu(&pdu);
	}
}

void CGroupChat::HandleGroupMessage(CImPduMsgData* pPdu)
{
	uint32_t from_user_id = pPdu->GetFromUserId();
	uint32_t to_group_id = pPdu->GetToUserId();
	string msg_data((char*)pPdu->GetMsgData(), pPdu->GetMsgLen());
	log("HandleGroupMsg, %u->%u\n", from_user_id, to_group_id);

    uint32_t seq_no = pPdu->GetRequestId();
    string from_id_url = idtourl(from_user_id);
    string to_id_url = idtourl(to_group_id);
    uint8_t msg_type = pPdu->GetMsgType();
    
    CImPduClientMsgData pdu(seq_no, from_id_url.c_str(), to_id_url.c_str(), pPdu->GetCreateTime(),
                            msg_type, pPdu->GetMsgLen(), pPdu->GetMsgData(),
                            pPdu->GetAttachLen(), pPdu->GetAttachData());
    
	group_member_t* group_member = _GetGroupMember(to_group_id);
	if (group_member)
    {
		group_member_t::iterator it;
		for (it = group_member->begin(); it != group_member->end(); it++) {
			uint32_t member_user_id = *it;
            
            CImUser* pImUser = CImUserManager::GetInstance()->GetImUserById(member_user_id);
            if (pImUser) {
                pImUser->BroadcastPdu(&pdu);
            }
		}
	}
    else
    {
        // 服务器没有该群的信息，向DB服务器请求群信息，并带上消息作为附件，返回时在发送该消息给其他群成员
        log("this server does not have enough group info\n");
        CPduAttachData pduAttachData(ATTACH_TYPE_HANDLE_AND_PDU, 0, pdu.GetLength(), pdu.GetBuffer());
		CImPduGroupUserListRequest pduLR(from_user_id, to_group_id, pduAttachData.GetLength(), pduAttachData.GetBuffer());
        CDBServConn* pDbConn = get_db_serv_conn();
        if(pDbConn)
            pDbConn->SendPdu(&pduLR);
    }
}

void CGroupChat::HandleClientGroupUnreadMsgCntRequest(CImPduClientGroupUnreadMsgCntRequest* pPdu,
		CMsgConn* pFromConn)
{
	uint32_t req_user_id = pFromConn->GetUserId();
	log("HandleClientGroupUnreadMsgCntReq, user_id=%u\n", req_user_id);

	CDBServConn* pDbConn = get_db_serv_conn();
	if (pDbConn) {
        CDbAttachData attach_data(ATTACH_TYPE_HANDLE, pFromConn->GetHandle(), 0);
		CImPduGroupUnreadMsgCntRequest pdu(req_user_id, pFromConn->GetClientType(),
                            attach_data.GetLength(), attach_data.GetBuffer());
		pdu.SetReserved(pPdu->GetReserved());
		pDbConn->SendPdu(&pdu);
	} else {
		CImPduClientGroupUnreadMsgCntResponse pdu(0);
		pdu.SetReserved(pPdu->GetReserved());
		pFromConn->SendPdu(&pdu);
	}
}

void CGroupChat::HandleGroupUnreadMsgCntResponse(CImPduGroupUnreadMsgCntResponse* pPdu)
{
	uint32_t req_user_id = pPdu->GetReqUserId();
	uint32_t unread_cnt = pPdu->GetGroupUnreadCnt();
	group_unread_cnt_t* unread_list = pPdu->GetGroupUnreadList();
	log("HandleGroupUnreadMsgCntResp, req_user_id=%u, unread_cnt=%u\n", req_user_id, unread_cnt);
    CDbAttachData attach_data(pPdu->GetAttachData(), pPdu->GetAttachLen());

	CMsgConn* pConn = CImUserManager::GetInstance()->GetMsgConnByHandle(req_user_id, attach_data.GetHandle());
	if (pConn) {
		CImPduClientGroupUnreadMsgCntResponse pdu(unread_cnt, unread_list);
		pdu.SetReserved(pPdu->GetReserved());
		pConn->SendPdu(&pdu);
	}
}

void CGroupChat::HandleClientGroupUnreadMsgRequest(CImPduClientGroupUnreadMsgRequest* pPdu, CMsgConn* pFromConn)
{
	uint32_t req_user_id = pFromConn->GetUserId();
	string group_id_url(pPdu->GetGroupIdUrl(), pPdu->GetGroupIdLen());
	uint32_t group_id = urltoid(group_id_url.c_str());
	log("HandleClientGroupUnreadMsgReq, req_user_id=%u, group_id=%u\n", req_user_id, group_id);

	CDBServConn* pDbConn = get_db_serv_conn();
	if (pDbConn) {
        CDbAttachData attach_data(ATTACH_TYPE_HANDLE, pFromConn->GetHandle(), 0);
		CImPduGroupUnreadMsgRequest pdu(req_user_id, group_id, pFromConn->GetClientType(),
                attach_data.GetLength(), attach_data.GetBuffer());
		pdu.SetReserved(pPdu->GetReserved());
		pDbConn->SendPdu(&pdu);
	}
}

void CGroupChat::HandleGroupMsgListResponse(CImPduGroupMsgListResponse* pPdu)
{
	uint32_t req_user_id = pPdu->GetReqUserId();
	uint32_t req_cmd_id = pPdu->GetReqCmdId();
	uint32_t group_id = pPdu->GetGroupId();
	uint32_t msg_cnt = pPdu->GetMsgCnt();
	group_msg_t* msg_list = pPdu->GetMsgList();
	log("HandleGroupMsgListResp, req_user_id=%u, req_cmd_id=%u, group_id=%u, msg_cnt=%u\n",
			req_user_id, req_cmd_id, group_id, msg_cnt);
    CDbAttachData attach_data(pPdu->GetAttachData(), pPdu->GetAttachLen());
	CMsgConn* pMsgConn = CImUserManager::GetInstance()->GetMsgConnByHandle(req_user_id,
                                                                           attach_data.GetHandle());
	if (pMsgConn) {
		uint32_t cmd_id = CID_GROUP_UNREAD_MSG_RESPONSE;

		string group_id_url = idtourl(group_id);
		CImPduClientGroupMsgListResponse pdu(cmd_id, group_id_url.c_str(), msg_cnt, msg_list);
		pdu.SetReserved(pPdu->GetReserved());
		pMsgConn->SendPdu(&pdu);
	}
}

void CGroupChat::HandleClientGroupMsgReadAck(CImPduClientGroupMsgReadAck* pPdu, CMsgConn* pFromConn)
{
	uint32_t req_user_id = pFromConn->GetUserId();
	string group_id_url(pPdu->GetGroupIdUrl(), pPdu->GetGroupIdLen());
	uint32_t group_id = urltoid(group_id_url.c_str());
	log("HandleClientGroupMsgReadAck, req_id=%u, group_id=%u\n", req_user_id, group_id);

	CDBServConn* pDbConn = get_db_serv_conn();
	if (pDbConn) {
		CImPduGroupMsgReadAck pdu(req_user_id, group_id, pFromConn->GetClientType());
		pdu.SetReserved(pPdu->GetReserved());
		pDbConn->SendPdu(&pdu);
	}
}

void CGroupChat::HandleClientGroupCreateTmpGroupRequest(CImPduClientGroupCreateTmpGroupRequest* pPdu, CMsgConn* pFromConn)
{
	uint32_t req_user_id = pFromConn->GetUserId();
	string group_name(pPdu->GetGroupName(), pPdu->GetGroupNameLen());
	string group_avatar(pPdu->GetGroupAvatar(), pPdu->GetGroupAvatarLen());
	uint32_t user_cnt = pPdu->GetUserCnt();
	user_id_t* user_list = pPdu->GetUserList();
	log("HandleClientCreateTmpGroupReq, req_id=%u, group_name=%s, avatar_url=%s, user_cnt=%u\n",
			req_user_id, group_name.c_str(), group_avatar.c_str(), user_cnt);

	CDBServConn* pDbConn = get_db_serv_conn();
	if (pDbConn) {
		list<uint32_t> user_id_list;
		for (uint32_t i = 0; i < user_cnt; i++) {
			string user_id_url(user_list[i].id_url, user_list[i].id_len);
			uint32_t user_id = urltoid(user_id_url.c_str());
			user_id_list.push_back(user_id);
		}
        CDbAttachData attach_data(ATTACH_TYPE_HANDLE, pFromConn->GetHandle(), 0);
		CImPduGroupCreateTmpGroupRequest pdu(req_user_id, group_name.c_str(), group_avatar.c_str(),
                    &user_id_list, attach_data.GetLength(), attach_data.GetBuffer());
		pdu.SetReserved(pPdu->GetReserved());
		pDbConn->SendPdu(&pdu);
	} else {
		log("no DB connection\n");
		CImPduClientGroupCreateTmpGroupResponse pdu(1, NULL, group_name.c_str());
		pdu.SetReserved(pPdu->GetReserved());
		pFromConn->SendPdu(&pdu);
	}
}

void CGroupChat::HandleGroupCreateTmpGroupResponse(CImPduGroupCreateTmpGroupResponse* pPdu)
{
	uint32_t req_user_id = pPdu->GetReqUserId();
	uint32_t result = pPdu->GetResult();
	uint32_t group_id = pPdu->GetGroupId();
	string group_name(pPdu->GetGroupName(), pPdu->GetGroupNameLen());
	uint32_t user_cnt = pPdu->GetUserCnt();
	uint32_t* user_list = pPdu->GetUserList();
	log("HandleCreateTmpGroupResp, req_id=%u, result=%u, group_id=%u\n", req_user_id, result, group_id);

	string group_id_url = idtourl(group_id);
	CImPduClientGroupCreateTmpGroupResponse pdu(result, group_id_url.c_str(), group_name.c_str(),
			user_cnt, user_list);
	pdu.SetReserved(pPdu->GetReserved());

    CDbAttachData attach_data(pPdu->GetAttachData(), pPdu->GetAttachLen());
	if (!result) {
		_UpdateGroupMap(group_id, user_cnt, user_list);

		CRouteServConn* pRouteConn = get_route_serv_conn();
		if (pRouteConn) {
			pRouteConn->SendPdu(pPdu);
		}

		// push create tmp group info to other group members
		for (uint32_t i = 0; i < user_cnt; i++) {
			if (user_list[i] != req_user_id) {
				_SendPduToUser(&pdu, user_list[i]);
			}
		}
	}

	_SendPduToUser(&pdu, req_user_id);
}

void CGroupChat::HandleGroupCreateTmpGroupBroadcast(CImPduGroupCreateTmpGroupResponse* pPdu)
{
	uint32_t group_id = pPdu->GetGroupId();
	string group_name(pPdu->GetGroupName(), pPdu->GetGroupNameLen());
	uint32_t user_cnt = pPdu->GetUserCnt();
	uint32_t* user_list = pPdu->GetUserList();
	log("HandleCreateTmpGroupBroadcast, group_id=%u, user_cnt=%u\n", group_id, user_cnt);

	_UpdateGroupMap(group_id, user_cnt, user_list);

	CImPduClientGroupCreateTmpGroupResponse pdu(0, idtourl(group_id), group_name.c_str(), user_cnt, user_list);
	for (uint32_t i = 0; i < user_cnt; i++) {
		_SendPduToUser(&pdu, user_list[i]);
	}
}

void CGroupChat::HandleClientGroupChangeMemberRequest(CImPduClientGroupChangeMemberRequest* pPdu, CMsgConn* pFromConn)
{
    uint32_t change_type = pPdu->GetChangeType();
	uint32_t req_user_id = pFromConn->GetUserId();
	string group_id_url(pPdu->GetGroupId(), pPdu->GetGroupIdLen());
	uint32_t group_id = urltoid(group_id_url.c_str());
	uint32_t user_cnt = pPdu->GetUserCnt();
	user_id_t* user_list = pPdu->GetUserList();
	log("HandleClientChangeMemberReq, change_type=%u, req_id=%u, group_id=%u, user_cnt=%u\n",
			change_type, req_user_id, group_id, user_cnt);

	CDBServConn* pDbConn = get_db_serv_conn();
	if (pDbConn) {
		list<uint32_t> user_id_list;
		for (uint32_t i = 0; i < user_cnt; i++) {
			string user_id_url(user_list[i].id_url, user_list[i].id_len);
			uint32_t user_id = urltoid(user_id_url.c_str());
			user_id_list.push_back(user_id);
		}

       CDbAttachData attach_data(ATTACH_TYPE_HANDLE, pFromConn->GetHandle(), 0);
		CImPduGroupChangeMemberRequest pdu(change_type, req_user_id, group_id, &user_id_list,
                                           attach_data.GetLength(), attach_data.GetBuffer());
		pdu.SetReserved(pPdu->GetReserved());
		pDbConn->SendPdu(&pdu);
	} else {
		log("no DB connection\n");
		CImPduClientGroupChangeMemberResponse pdu(change_type, 1, group_id_url.c_str());
		pdu.SetReserved(pPdu->GetReserved());
		pFromConn->SendPdu(&pdu);
	}
}

void CGroupChat::HandleGroupChangeMemberResponse(CImPduGroupChangeMemberResponse* pPdu)
{
	uint32_t change_type = pPdu->GetChangeType();
    uint32_t req_user_id = pPdu->GetReqUserId();
	uint32_t result = pPdu->GetResult();
	uint32_t group_id = pPdu->GetGroupId();
	uint32_t user_cnt = pPdu->GetUserCnt();
	uint32_t* user_list = pPdu->GetUserList();
	log("HandleChangeMemberResp, change_type=%u, req_id=%u, group_id=%u, result=%u\n",
			change_type, req_user_id, group_id, result);
    CDbAttachData attach_data(pPdu->GetAttachData(), pPdu->GetAttachLen());
	string group_id_url = idtourl(group_id);
	CImPduClientGroupChangeMemberResponse pdu(change_type, result, group_id_url.c_str(), user_cnt, user_list);
	pdu.SetReserved(pPdu->GetReserved());

	if (!result) {
		if (change_type == GROUP_MEMBER_ADD_TYPE) {
			_JoinGroup(group_id, user_cnt, user_list);
		} else {
			_QuitGroup(group_id, user_cnt, user_list);
		}

		CRouteServConn* pRouteConn = get_route_serv_conn();
		if (pRouteConn) {
			pRouteConn->SendPdu(pPdu);
		}

		// push join/quit group response to other group member
		group_member_t* member = _GetGroupMember(group_id);
		if (member) {
			for (set<uint32_t>::iterator it = member->begin(); it != member->end(); it++) {
				uint32_t user_id = *it;
				if (user_id != req_user_id) {
					_SendPduToUser(&pdu, user_id);
				}
			}
		}

		// push join/quit group response to join/quit user list
		for (uint32_t i = 0; i < user_cnt; i++) {
			uint32_t user_id = user_list[i];
			if (user_id != req_user_id) {
				_SendPduToUser(&pdu, user_id);
			}
		}
	}

	_SendPduToUser(&pdu, req_user_id);
}

void CGroupChat::HandleGroupChangeMemberBroadcast(CImPduGroupChangeMemberResponse* pPdu)
{
	uint32_t change_type = pPdu->GetChangeType();
	uint32_t group_id = pPdu->GetGroupId();
	uint32_t user_cnt = pPdu->GetUserCnt();
	uint32_t* user_list = pPdu->GetUserList();
	log("HandleChangeMemberBroadcast, change_type=%u, group_id=%u, user_cnt=%u\n", change_type, group_id, user_cnt);

	if (change_type == GROUP_MEMBER_ADD_TYPE) {
		_JoinGroup(group_id, user_cnt, user_list);
	} else {
		_QuitGroup(group_id, user_cnt, user_list);
	}

	// push join/quit group response to other group member
	string group_id_url = idtourl(group_id);

	CImPduClientGroupChangeMemberResponse pdu(change_type, 0, group_id_url.c_str(), user_cnt, user_list);
	group_member_t* member = _GetGroupMember(group_id);
	if (member) {
		for (set<uint32_t>::iterator it = member->begin(); it != member->end(); it++) {
			uint32_t user_id = *it;
			_SendPduToUser(&pdu, user_id);
		}
	}

	// push join/quit group response to join/quit user list
	for (uint32_t i = 0; i < user_cnt; i++) {
		uint32_t user_id = user_list[i];
		_SendPduToUser(&pdu, user_id);
	}
}

void CGroupChat::HandleGroupChangeMemberNotify(CImPduGroupChangeMemberNotify *pPdu)
{
	uint32_t group_id = pPdu->GetGroupId();
	uint32_t user_cnt = pPdu->GetUserCnt();
	uint32_t* user_list = pPdu->GetUserList();
	log("HandleGroupChangeMemberNotify, group_id=%u, user_cnt=%u\n", group_id, user_cnt);
    
	HandleGroupChangeMemberNotify(group_id, user_cnt, user_list);
}

void CGroupChat::HandleGroupChangeMemberNotify(uint32_t group_id, uint32_t user_cnt,
                                               uint32_t* user_list)
{
    group_member_t* old_group = _GetGroupMember(group_id);
    group_member_t group_tmp;
    if (old_group != NULL) {
        group_tmp = *old_group;
    }
    
    for (uint32_t i = 0; i < user_cnt; i++) {
        group_tmp.insert(user_list[i]);
    }
    
    _UpdateGroupMap(group_id, user_cnt, user_list);

	// push join/quit group response to other group member
	string group_id_url = idtourl(group_id);
    
	CImPduClientGroupChangeMemberNotify pdu( group_id_url.c_str(), user_cnt, user_list);
    for (group_member_t::iterator it = group_tmp.begin(); it != group_tmp.end(); it++) {
        uint32_t user_id = *it;
        _SendPduToUser(&pdu, user_id);
	}
}

void CGroupChat::HandleGroupCreateNormalGroupNotify(CImPduGroupCreateNormalGroupNotify *pPdu)
{
    uint32_t group_id = pPdu->GetGroupId();
	string group_name(pPdu->GetGroupName(), pPdu->GetGroupNameLen());
    string group_avatar(pPdu->GetGroupAvatar(), pPdu->GetGroupAvatarLen());
	uint32_t user_cnt = pPdu->GetUserCnt();
	uint32_t* user_list = pPdu->GetUserList();
	log("HandleGroupCreateNormalGroupNotify, group_id=%u, user_cnt=%u\n", group_id, user_cnt);
    
	HandleGroupCreateNormalGroupNotify(group_id, group_name.c_str(), group_avatar.c_str(),
                                       user_cnt, user_list);
}

void CGroupChat::HandleGroupCreateNormalGroupNotify(uint32_t group_id, const char* group_name,
                                const char* group_avatar, uint32_t user_cnt, uint32_t* user_list)
{
    _UpdateGroupMap(group_id, user_cnt, user_list);
    
	CImPduClientGroupCreateNormalGroupNotify pdu(idtourl(group_id), group_name, group_avatar, user_cnt, user_list);
	for (uint32_t i = 0; i < user_cnt; i++) {
		_SendPduToUser(&pdu, user_list[i]);
	}
}
// caution: update group member every time, can use cache in future
void CGroupChat::_UpdateGroupMap(uint32_t group_id, list<uint32_t>* user_list)
{
	if (user_list->size() == 0)
		return;

	group_member_t* group_member = new group_member_t;
	if (!group_member)
		return;

	for (list<uint32_t>::iterator it = user_list->begin(); it != user_list->end(); it++) {
		group_member->insert(*it);
	}

	group_map_t::iterator it = m_group_map.find(group_id);
	if (it == m_group_map.end()) {
		m_group_map.insert(make_pair(group_id, group_member));
	} else {
		group_member_t* old_group = it->second;
		delete old_group;

		it->second = group_member;
	}
}

void CGroupChat::_UpdateGroupMap(uint32_t group_id, uint32_t user_cnt, uint32_t* user_list)
{
	list<uint32_t> user_id_list;
	for (uint32_t i = 0; i < user_cnt; i++) {
		user_id_list.push_back(user_list[i]);
	}

	_UpdateGroupMap(group_id, &user_id_list);
}

group_member_t* CGroupChat::_GetGroupMember(uint32_t group_id)
{
	group_member_t* group_member = NULL;
	group_map_t::iterator it = m_group_map.find(group_id);
	if (it != m_group_map.end()) {
		group_member = it->second;
	}

	return group_member;
}

void CGroupChat::_JoinGroup(uint32_t group_id, uint32_t user_cnt, uint32_t* user_list)
{
	group_member_t* group_member = _GetGroupMember(group_id);

	if (group_member) {
		for (uint32_t i = 0; i < user_cnt; i++) {
			group_member->insert(user_list[i]);
		}
	}
}

void CGroupChat::_QuitGroup(uint32_t group_id, uint32_t user_cnt, uint32_t* user_list)
{
	group_member_t* group_member = _GetGroupMember(group_id);

	if (group_member) {
		for (uint32_t i = 0; i < user_cnt; i++) {
			group_member->erase(user_list[i]);
		}
	}
}

void CGroupChat::_SendPduToUser(CImPdu* pPdu, uint32_t user_id)
{
    CImUser* pToUser = CImUserManager::GetInstance()->GetImUserById(user_id);
    if(pToUser) {
        pToUser->BroadcastPdu(pPdu);
    }
}
