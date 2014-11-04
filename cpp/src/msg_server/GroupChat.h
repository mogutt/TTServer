/*
 * GroupChat.h
 *
 *  Created on: 2014-1-3
 *      Author: ziteng@mogujie.com
 */

#ifndef GROUPCHAT_H_
#define GROUPCHAT_H_

#include "impdu.h"

typedef set<uint32_t> group_member_t;
typedef hash_map<uint32_t, group_member_t*> group_map_t;

class CMsgConn;

class CGroupChat
{
public:
	virtual ~CGroupChat() {}

	static CGroupChat* GetInstance();

	void HandleClientGroupListRequest(CImPduClientGroupListRequest* pPdu, CMsgConn* pFromConn);
	void HandleGroupListResponse(CImPduGroupListResponse* pPdu);

	void HandleClientGroupUserListRequest(CImPduClientGroupUserListRequest* pPdu, CMsgConn* pFromConn);
	void HandleGroupUserListResponse(CImPduGroupUserListResponse* pPdu);

	void HandleClientGroupMessage(CImPduClientMsgData* pPdu, CMsgConn* pFromConn);
	void HandleGroupMessage(CImPduMsgData* pPdu);

	void HandleClientGroupUnreadMsgCntRequest(CImPduClientGroupUnreadMsgCntRequest* pPdu, CMsgConn* pFromConn);
	void HandleGroupUnreadMsgCntResponse(CImPduGroupUnreadMsgCntResponse* pPdu);

	void HandleClientGroupUnreadMsgRequest(CImPduClientGroupUnreadMsgRequest* pPdu, CMsgConn* pFromConn);
	
	void HandleGroupMsgListResponse(CImPduGroupMsgListResponse* pPdu);

	void HandleClientGroupMsgReadAck(CImPduClientGroupMsgReadAck* pPdu, CMsgConn* pFromConn);

	void HandleClientGroupCreateTmpGroupRequest(CImPduClientGroupCreateTmpGroupRequest* pPdu, CMsgConn* pFromConn);
	void HandleGroupCreateTmpGroupResponse(CImPduGroupCreateTmpGroupResponse* pPdu);
	void HandleGroupCreateTmpGroupBroadcast(CImPduGroupCreateTmpGroupResponse* pPdu);
	void HandleClientGroupChangeMemberRequest(CImPduClientGroupChangeMemberRequest* pPdu, CMsgConn* pFromConn);
	void HandleGroupChangeMemberResponse(CImPduGroupChangeMemberResponse* pPdu);
	void HandleGroupChangeMemberBroadcast(CImPduGroupChangeMemberResponse* pPdu);
    void HandleGroupCreateNormalGroupNotify(CImPduGroupCreateNormalGroupNotify* pdu);
    void HandleGroupChangeMemberNotify(CImPduGroupChangeMemberNotify* pdu);
    void HandleGroupCreateNormalGroupNotify(uint32_t group_id, const char* group_name,
                    const char* group_avatar, uint32_t user_cnt, uint32_t* user_list);
    void HandleGroupChangeMemberNotify(uint32_t group_id, uint32_t user_cnt, uint32_t* user_list);

private:
	CGroupChat() {}	// for singleton;

	void _UpdateGroupMap(uint32_t group_id, list<uint32_t>* user_list);
	void _UpdateGroupMap(uint32_t group_id, uint32_t user_cnt, uint32_t* user_list);
	group_member_t* _GetGroupMember(uint32_t group_id);
	void _JoinGroup(uint32_t group_id, uint32_t user_cnt, uint32_t* user_list);
	void _QuitGroup(uint32_t group_id, uint32_t user_cnt, uint32_t* user_list);
	void _SendPduToUser(CImPdu* pPdu, uint32_t user_id);
private:

	static CGroupChat* s_group_chat_instance;

	group_map_t m_group_map;
};


#endif /* GROUPCHAT_H_ */
