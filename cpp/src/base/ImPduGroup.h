/*
 * ImPduGroup.h
 *
 *  Created on: 2014-1-2
 *      Author: ziteng@mogujie.com
 */

#ifndef IMPDUGROUP_H_
#define IMPDUGROUP_H_

#include "ImPduBase.h"

// How client and server pdu can merge to the same one in future
// pdu with client
#define IM_PDU_TYPE_CLIENT_GROUP_LIST_REQUEST				400
#define IM_PDU_TYPE_CLIENT_GROUP_LIST_RESPONSE				401
#define IM_PDU_TYPE_CLIENT_GROUP_USER_LIST_REQUEST			402
#define IM_PDU_TYPE_CLIENT_GROUP_USER_LIST_RESPONSE			403
#define IM_PDU_TYPE_CLIENT_GROUP_UNREAD_MSG_CNT_REQUEST		404
#define IM_PDU_TYPE_CLIENT_GROUP_UNREAD_MSG_CNT_RESPONSE	405
#define IM_PDU_TYPE_CLIENT_GROUP_UNREAD_MSG_REQUEST			406
#define IM_PDU_TYPE_CLIENT_GROUP_MSG_LIST_RESPONSE			408
#define IM_PDU_TYPE_CLIENT_GROUP_MSG_READ_ACK				409
#define IM_PDU_TYPE_CLIENT_GROUP_CREATE_TMP_GROUP_REQUEST	410
#define IM_PDU_TYPE_CLIENT_GROUP_CREATE_TMP_GROUP_RESPONSE	411
#define IM_PDU_TYPE_CLIENT_GROUP_CHANGE_MEMBER_REQUEST		412
#define IM_PDU_TYPE_CLIENT_GROUP_CHANGE_MEMBER_RESPONSE		413
#define IM_PDU_TYPE_CLIENT_DIALOG_GROUP_LIST_REQUEST		414
#define IM_PDU_TYPE_CLIENT_DIALOG_GROUP_LIST_RESPONSE		415

#define IM_PDU_TYPE_CLIENT_GROUP_CREATE_NORMAL_GROUP_NOTIFY 416
#define IM_PDU_TYPE_CLIENT_GROUP_CHANGE_MEMBER_NOTIFY       417

// pdu with business server
#define IM_PDU_TYPE_GROUP_LIST_REQUEST				300
#define IM_PDU_TYPE_GROUP_LIST_RESPONSE				301
#define IM_PDU_TYPE_GROUP_USER_LIST_REQUEST			302
#define IM_PDU_TYPE_GROUP_USER_LIST_RESPONSE		303
#define IM_PDU_TYPE_GROUP_UNREAD_MSG_CNT_REQUEST	304
#define IM_PDU_TYPE_GROUP_UNREAD_MSG_CNT_RESPONSE	305
#define IM_PDU_TYPE_GROUP_UNREAD_MSG_REQUEST		306
#define IM_PDU_TYPE_GROUP_MSG_LIST_RESPONSE			308
#define IM_PDU_TYPE_GROUP_MSG_READ_ACK				309
#define IM_PDU_TYPE_GROUP_CREATE_TMP_GROUP_REQUEST	310
#define IM_PDU_TYPE_GROUP_CREATE_TMP_GROUP_RESPONSE	311
#define IM_PDU_TYPE_GROUP_CHANGE_MEMBER_REQUEST		312	// for internal use
#define IM_PDU_TYPE_GROUP_CHANGE_MEMBER_RESPONSE	313 // for internal use
#define IM_PDU_TYPE_GROUP_DIALOG_LIST_REQUEST		314 // recent contact group(normal group & tmp group)
#define IM_PDU_TYPE_GROUP_DIALOG_LIST_RESPONSE		315

#define IM_PDU_TYPE_GROUP_CREATE_NORMAL_GROUP_NOTIFY    316
#define IM_PDU_TYPE_GROUP_CHANGE_MEMBER_NOTIFY          317

typedef struct {
	uint32_t 	id_len;
	char*		id_url;
} group_id_t;

typedef struct {
	uint32_t 	id_len;
	char*		id_url;
} user_id_t;

typedef struct {
	uint32_t 	group_id_len;
	char*		group_id;
	uint32_t	group_name_len;
	char*		group_name;
	uint32_t	group_avatar_len;
	char*		group_avatar;
	uint32_t	group_creator_id_len;
	char*		group_creator_id;
	uint32_t	group_type;		// 1--normal group, 2--temporary group
	uint32_t	group_updated;
	list<user_id_t>	group_memeber_list;
} client_group_info_t;

typedef struct {
	uint32_t 	group_id;
	uint32_t	group_name_len;
	char*		group_name;
	uint32_t	group_avatar_len;
	char*		group_avatar;
	uint32_t	group_creator_id;
	uint32_t	group_type;		// 1--normal group, 2--temporary group
	uint32_t	group_updated;
	list<uint32_t> group_memeber_list;
} group_info_t;

typedef struct {
	uint32_t group_id_len;
	char*	 group_id_url;
	uint32_t unread_msg_cnt;
} client_group_unread_cnt_t;

typedef struct {
	uint32_t group_id;
	uint32_t unread_msg_cnt;
} group_unread_cnt_t;

typedef struct {
	uint32_t from_user_id_len;
	char*	 from_user_id_url;
	uint32_t create_time;
    uint8_t msg_type;
	uint32_t msg_len;
	uchar_t* msg_content;
} client_group_msg_t;

typedef struct {
	uint32_t from_user_id;
	uint32_t create_time;
    uint8_t msg_type;
	uint32_t msg_len;
	uchar_t* msg_content;
} group_msg_t;

class DLL_MODIFIER CImPduClientGroupListRequest : public CImPdu
{
public:
	CImPduClientGroupListRequest(uchar_t* buf, uint32_t len);
	CImPduClientGroupListRequest(uint16_t cmd_id);
	virtual ~CImPduClientGroupListRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_GROUP_LIST_REQUEST; }
};

class DLL_MODIFIER CImPduClientGroupListResponse : public CImPdu
{
public:
	CImPduClientGroupListResponse(uchar_t* buf, uint32_t len);
	CImPduClientGroupListResponse(uint16_t cmd_id, uint32_t group_cnt, group_info_t* group_list = NULL);
	virtual ~CImPduClientGroupListResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_GROUP_LIST_RESPONSE; }
	uint32_t GetGroupCnt() { return m_group_cnt; }
	client_group_info_t* GetGroupList() { return m_group_list; }
private:
	uint32_t				m_group_cnt;
	client_group_info_t*	m_group_list;
};

class DLL_MODIFIER CImPduClientGroupUserListRequest : public CImPdu
{
public:
	CImPduClientGroupUserListRequest(uchar_t* buf, uint32_t len);
	CImPduClientGroupUserListRequest(const char* group_id);
	virtual ~CImPduClientGroupUserListRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_GROUP_USER_LIST_REQUEST; }
	uint32_t GetGroupIdLen() { return m_group_id_len; }
	char* GetGroupId() { return m_group_id; }
private:
	uint32_t	m_group_id_len;
	char*		m_group_id;
};

class DLL_MODIFIER CImPduClientGroupUserListResponse : public CImPdu
{
public:
	CImPduClientGroupUserListResponse(uchar_t* buf, uint32_t len);
	CImPduClientGroupUserListResponse(uint16_t version, const char* group_id, uint32_t result, const char* group_name = NULL,
		const char* group_avatar = NULL, const char* group_creator_id = NULL, uint32_t group_type = 0,
		uint32_t user_cnt = 0, uint32_t* user_list = NULL);
	virtual ~CImPduClientGroupUserListResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_GROUP_USER_LIST_RESPONSE; }
	uint32_t GetGroupIdLen() { return m_group_id_len; }
	char* GetGroupId() { return m_group_id; }
	uint32_t GetResult() { return m_result; }
	uint32_t GetGroupNameLen() { return m_group_name_len; }
	char* GetGroupName() { return m_group_name; }
	uint32_t GetGroupAvatarLen() { return m_group_avatar_len; }
	char* GetGroupAvatar() { return m_group_avatar; }
	uint32_t GetGroupType() { return m_group_type; }
	uint32_t GetUserCnt() { return m_user_cnt; }
	user_id_t* GetUserList() { return m_user_list; }
private:
	uint32_t	m_group_id_len;
	char*		m_group_id;
	uint32_t	m_result;	// 0--success, other--group not exist
	// the following fields only return when result = 0;
	uint32_t	m_group_name_len;
	char*		m_group_name;
	uint32_t	m_group_avatar_len;
	char*		m_group_avatar;
	uint32_t 	m_group_creator_id_len;
	char*		m_group_creator_id;
	uint32_t	m_group_type;
	uint32_t	m_user_cnt;
	user_id_t*	m_user_list;
};

class DLL_MODIFIER CImPduClientGroupUnreadMsgCntRequest : public CImPdu
{
public:
	CImPduClientGroupUnreadMsgCntRequest(uchar_t* buf, uint32_t len) { ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header); }
	CImPduClientGroupUnreadMsgCntRequest();
	virtual ~CImPduClientGroupUnreadMsgCntRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_GROUP_UNREAD_MSG_CNT_REQUEST; }
};

class DLL_MODIFIER CImPduClientGroupUnreadMsgCntResponse : public CImPdu
{
public:
	CImPduClientGroupUnreadMsgCntResponse(uchar_t* buf, uint32_t len);
	CImPduClientGroupUnreadMsgCntResponse(uint32_t unread_cnt, group_unread_cnt_t* group_unread_list = NULL);
	virtual ~CImPduClientGroupUnreadMsgCntResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_GROUP_UNREAD_MSG_CNT_RESPONSE; }
	uint32_t GetGroupUnreadCnt() { return m_group_unread_cnt; }
	client_group_unread_cnt_t* GetGroupUnreadList() { return m_group_unread_list; }
private:
	uint32_t 					m_group_unread_cnt;
	client_group_unread_cnt_t* 	m_group_unread_list;
};

class DLL_MODIFIER CImPduClientGroupUnreadMsgRequest : public CImPdu
{
public:
	CImPduClientGroupUnreadMsgRequest(uchar_t* buf, uint32_t len);
	CImPduClientGroupUnreadMsgRequest(const char* group_id_url);
	virtual ~CImPduClientGroupUnreadMsgRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_GROUP_UNREAD_MSG_REQUEST; }
	uint32_t GetGroupIdLen() { return m_group_id_len; }
	char* GetGroupIdUrl() { return m_group_id_url; }
private:
	uint32_t 	m_group_id_len;
	char* 	 	m_group_id_url;
};

class DLL_MODIFIER CImPduClientGroupMsgListResponse : public CImPdu
{
public:
	CImPduClientGroupMsgListResponse(uchar_t* buf, uint32_t len);
	CImPduClientGroupMsgListResponse(uint32_t cmd_id, const char* group_id_url,
			uint32_t msg_cnt, group_msg_t* msg_list);
	virtual ~CImPduClientGroupMsgListResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_MSG_LIST_RESPONSE; }
	uint32_t GetGroupIdLen() { return m_group_id_len; }
	char* GetGroupIdUrl() { return m_group_id_url; }
	uint32_t GetMsgCnt() { return m_msg_cnt; }
	client_group_msg_t* GetMsgList() { return m_msg_list; }
 private:
	uint32_t 	m_group_id_len;
	char*		m_group_id_url;
	uint32_t	m_msg_cnt;
	client_group_msg_t* m_msg_list;
};

class DLL_MODIFIER CImPduClientGroupMsgReadAck : public CImPdu
{
public:
	CImPduClientGroupMsgReadAck(uchar_t* buf, uint32_t len);
	CImPduClientGroupMsgReadAck(const char* group_id_url);
	virtual ~CImPduClientGroupMsgReadAck() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_GROUP_MSG_READ_ACK; }
	uint32_t GetGroupIdLen() { return m_group_id_len; }
	char* GetGroupIdUrl() { return m_group_id_url; }
private:
	uint32_t	m_group_id_len;
	char*		m_group_id_url;
};

class DLL_MODIFIER CImPduClientGroupCreateTmpGroupRequest : public CImPdu
{
public:
	CImPduClientGroupCreateTmpGroupRequest(uchar_t* buf, uint32_t len);
	CImPduClientGroupCreateTmpGroupRequest(const char* group_name, const char* group_avatar,
			list<user_id_t>* user_list);
	virtual ~CImPduClientGroupCreateTmpGroupRequest();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_GROUP_CREATE_TMP_GROUP_REQUEST; }
	uint32_t GetGroupNameLen() { return m_group_name_len; }
	char* GetGroupName() { return m_group_name; }
	uint32_t GetGroupAvatarLen() { return m_group_avatar_len; }
	char* GetGroupAvatar() { return m_group_avatar; }
	uint32_t GetUserCnt() { return m_user_cnt; }
	user_id_t* GetUserList() { return m_user_list; }
private:
	uint32_t	m_group_name_len;
	char*		m_group_name;
	uint32_t	m_group_avatar_len;
	char*		m_group_avatar;
	uint32_t	m_user_cnt;
	user_id_t*	m_user_list;
};

class DLL_MODIFIER CImPduClientGroupCreateTmpGroupResponse : public CImPdu
{
public:
	CImPduClientGroupCreateTmpGroupResponse(uchar_t* buf, uint32_t len);
	CImPduClientGroupCreateTmpGroupResponse(uint32_t result, const char* group_id, const char* group_name,
			uint32_t user_cnt = 0, uint32_t* user_list = NULL);
	virtual ~CImPduClientGroupCreateTmpGroupResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_GROUP_CREATE_TMP_GROUP_RESPONSE; }
	uint32_t GetGroupIdLen() { return m_group_id_len; }
	char* GetGroupId() { return m_group_id; }
	uint32_t GetGroupNameLen() { return m_group_name_len; }
	char* GetGroupName() { return m_group_name; }
	uint32_t GetUserCnt() { return m_user_cnt; }
	user_id_t* GetUserList() { return m_user_list; }
private:
	uint32_t	m_result;
	uint32_t	m_group_id_len;
	char*		m_group_id;
	uint32_t	m_group_name_len;
	char*		m_group_name;
	uint32_t	m_user_cnt;
	user_id_t*	m_user_list;
};

class DLL_MODIFIER CImPduClientGroupChangeMemberRequest : public CImPdu
{
public:
	CImPduClientGroupChangeMemberRequest(uchar_t* buf, uint32_t len);
	CImPduClientGroupChangeMemberRequest(uint32_t change_type, const char* group_id, list<user_id_t>* user_list);
	virtual ~CImPduClientGroupChangeMemberRequest();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_GROUP_CHANGE_MEMBER_REQUEST; }

    uint32_t GetChangeType() { return m_change_type; }
	uint32_t GetGroupIdLen() { return m_group_id_len; }
	char* GetGroupId() { return m_group_id; }
	uint32_t GetUserCnt() { return m_user_cnt; }
	user_id_t* GetUserList() { return m_user_list; }
private:
	uint32_t	m_group_id_len;
	char*		m_group_id;
    uint32_t   m_change_type;
	uint32_t	m_user_cnt;
	user_id_t*	m_user_list;
};

class DLL_MODIFIER CImPduClientGroupChangeMemberResponse : public CImPdu
{
public:
	CImPduClientGroupChangeMemberResponse(uchar_t* buf, uint32_t len);
	CImPduClientGroupChangeMemberResponse(uint32_t change_type, uint32_t result,
                                          const char* group_id,
			uint32_t user_cnt = 0, uint32_t* user_list = NULL);	// for server
	virtual ~CImPduClientGroupChangeMemberResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_GROUP_CHANGE_MEMBER_RESPONSE; }
	uint32_t GetResult() { return m_result; }
    uint32_t GetChangeType() { return m_change_type;}
	uint32_t GetGroupIdLen() { return m_group_id_len; }
	char* GetGroupId() { return m_group_id; }
private:
	uint32_t	m_result;
	uint32_t	m_group_id_len;
	char*		m_group_id;
    uint32_t   m_change_type;
	uint32_t	m_user_cnt;
	user_id_t*	m_user_list;
};

class DLL_MODIFIER CImPduClientGroupCreateNormalGroupNotify : public CImPdu
{
public:
    CImPduClientGroupCreateNormalGroupNotify(uchar_t* buf, uint32_t len);
    CImPduClientGroupCreateNormalGroupNotify(const char* group_id_url, const char* group_name,
                            const char* group_avatar, uint32_t user_cnt, uint32_t* user_list);
    virtual ~CImPduClientGroupCreateNormalGroupNotify();
    
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_GROUP_CREATE_NORMAL_GROUP_NOTIFY; }
    uint32_t GetGroupIdLen() { return m_group_id_url_len; }
    char*   GetGroupId() { return m_group_id_url; }
    uint32_t GetGroupNameLen() { return m_group_name_len; }
    char*   GetGroupName() { return m_group_name; }
    uint32_t GetGroupAvatarLen() { return m_group_avatar_len; }
    char* GetGroupAvatar() { return m_group_avatar; }
    uint32_t GetUserCnt() { return m_user_cnt; }
    user_id_t* GetUserList() { return m_user_list; }
private:
    uint32_t m_group_id_url_len;
    char* m_group_id_url;
    uint32_t m_group_name_len;
    char* m_group_name;
    uint32_t m_group_avatar_len;
    char* m_group_avatar;
    uint32_t	m_user_cnt;
    user_id_t*	m_user_list;
    
};

class DLL_MODIFIER CImPduClientGroupChangeMemberNotify : public CImPdu
{
public:
    CImPduClientGroupChangeMemberNotify(uchar_t* buf, uint32_t len);
    CImPduClientGroupChangeMemberNotify(const char* group_id_url, uint32_t user_cnt, uint32_t* user_list);
    virtual ~CImPduClientGroupChangeMemberNotify();
    
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_GROUP_CHANGE_MEMBER_NOTIFY; }
    uint32_t GetGroupIdUrlLen() { return m_group_id_url_len; }
    char* GetGroupIdUrl() { return m_group_id_url; }
    uint32_t GetUserCnt() { return m_user_cnt; }
    user_id_t* GetUserList() { return m_user_list; }
private:
    uint32_t m_group_id_url_len;
    char* m_group_id_url;
    uint32_t m_user_cnt;
    user_id_t* m_user_list;
};

// interactive packet with business server
#ifndef WIN32
class CImPduGroupListRequest : public CImPdu
{
public:
	CImPduGroupListRequest(uint16_t cmd_id, uint32_t user_id, uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduGroupListRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_LIST_REQUEST; }
};

class CImPduGroupListResponse : public CImPdu
{
public:
	CImPduGroupListResponse(uchar_t* buf, uint32_t len);
	virtual ~CImPduGroupListResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_LIST_RESPONSE; }
	uint32_t GetReqUserId() { return m_req_user_id; }
	uint32_t GetGroupCnt() { return m_group_cnt; }
	group_info_t* GetGroupList() { return m_group_list; }
	uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t 		m_req_user_id;
	uint32_t		m_group_cnt;
	group_info_t*	m_group_list;
	uint32_t		m_attach_len;
	uchar_t*		m_attach_data;
};

class CImPduGroupUserListRequest : public CImPdu
{
public:
	CImPduGroupUserListRequest(uint32_t req_user_id, uint32_t group_id,
			uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduGroupUserListRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_USER_LIST_REQUEST; }
};

class CImPduGroupUserListResponse : public CImPdu
{
public:
	CImPduGroupUserListResponse(uchar_t* buf, uint32_t len);
	virtual ~CImPduGroupUserListResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_USER_LIST_RESPONSE; }
	uint32_t GetReqUserId() { return m_req_user_id; }
	uint32_t GetGroupId() { return m_group_id; }
	uint32_t GetResult() { return m_result; }
	uint32_t GetGroupNameLen() { return m_group_name_len; }
	char* GetGroupName() { return m_group_name; }
	uint32_t GetGroupAvatarLen() { return m_group_avatar_len; }
	char* GetGroupAvatar() { return m_group_avatar; }
	uint32_t GetGroupCreatorId() { return m_group_creator_id; }
	uint32_t GetGroupType() { return m_group_type; }
	uint32_t GetUserCnt() { return m_user_cnt; }
	uint32_t* GetUserList() { return m_user_list; }
	uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t	m_req_user_id;
	uint32_t	m_group_id;
	uint32_t	m_result;	// 0--success, other--group not exist
	// the following fields only return when result = 0;
	uint32_t	m_group_name_len;
	char*		m_group_name;
	uint32_t	m_group_avatar_len;
	char*		m_group_avatar;
	uint32_t 	m_group_creator_id;
	uint32_t 	m_group_type;
	uint32_t	m_user_cnt;
	uint32_t*	m_user_list;
	uint32_t	m_attach_len;
	uchar_t*	m_attach_data;
};

class CImPduGroupUnreadMsgCntRequest : public CImPdu
{
public:
	CImPduGroupUnreadMsgCntRequest(uint32_t req_user_id, uint32_t client_type, uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduGroupUnreadMsgCntRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_UNREAD_MSG_CNT_REQUEST; }
};

class CImPduGroupUnreadMsgCntResponse : public CImPdu
{
public:
	CImPduGroupUnreadMsgCntResponse(uchar_t* buf, uint32_t len);
	virtual ~CImPduGroupUnreadMsgCntResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_UNREAD_MSG_CNT_RESPONSE; }
	uint32_t GetReqUserId() { return m_req_user_id; }
	uint32_t GetGroupUnreadCnt() { return m_group_unread_cnt; }
	group_unread_cnt_t* GetGroupUnreadList() { return m_group_unread_list; }
	uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t 	m_req_user_id;
	uint32_t 	m_group_unread_cnt;
	group_unread_cnt_t* m_group_unread_list;
	uint32_t 	m_attach_len;
	uchar_t* 	m_attach_data;
};

class CImPduGroupUnreadMsgRequest : public CImPdu
{
public:
	CImPduGroupUnreadMsgRequest(uint32_t req_user_id, uint32_t group_id, uint32_t client_type,
			uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduGroupUnreadMsgRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_UNREAD_MSG_REQUEST; }
};

class CImPduGroupMsgListResponse : public CImPdu
{
public:
	CImPduGroupMsgListResponse(uchar_t* buf, uint32_t len);
	virtual ~CImPduGroupMsgListResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_MSG_LIST_RESPONSE; }
	uint32_t GetReqUserId() { return m_req_user_id; }
	uint32_t GetReqCmdId() { return m_req_cmd_id; }
	uint32_t GetGroupId() { return m_group_id; }
	uint32_t GetMsgCnt() { return m_msg_cnt; }
	group_msg_t* GetMsgList() { return m_msg_list; }
	uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
 private:
	uint32_t 	m_req_user_id;
	uint32_t	m_req_cmd_id;
	uint32_t 	m_group_id;
	uint32_t	m_msg_cnt;
	group_msg_t* m_msg_list;
	uint32_t	m_attach_len;
	uchar_t*	m_attach_data;
};

class CImPduGroupMsgReadAck : public CImPdu
{
public:
	CImPduGroupMsgReadAck(uint32_t req_user_id, uint32_t group_id, uint32_t client_type);
	virtual ~CImPduGroupMsgReadAck() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_MSG_READ_ACK; }
};

class CImPduGroupCreateTmpGroupRequest : public CImPdu
{
public:
	CImPduGroupCreateTmpGroupRequest(uint32_t req_user_id, const char* group_name, const char* group_avatar,
			list<uint32_t>* user_list, uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduGroupCreateTmpGroupRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_CREATE_TMP_GROUP_REQUEST; }
};

class CImPduGroupCreateTmpGroupResponse : public CImPdu
{
public:
	CImPduGroupCreateTmpGroupResponse(uchar_t* buf, uint32_t len);
	virtual ~CImPduGroupCreateTmpGroupResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_CREATE_TMP_GROUP_RESPONSE; }
	uint32_t GetReqUserId() { return m_req_user_id; }
	uint32_t GetResult() { return m_result; }
	uint32_t GetGroupId() { return m_group_id; }
	uint32_t GetGroupNameLen() { return m_group_name_len; }
	char* GetGroupName() { return m_group_name; }
	uint32_t GetUserCnt() { return m_user_cnt; }
	uint32_t* GetUserList() { return m_user_list; }
	uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t	m_req_user_id;
	uint32_t	m_result;
	uint32_t	m_group_id;
	uint32_t	m_group_name_len;
	char*		m_group_name;
	uint32_t	m_user_cnt;
	uint32_t*	m_user_list;
	uint32_t 	m_attach_len;
	uchar_t* 	m_attach_data;
};

class CImPduGroupChangeMemberRequest : public CImPdu
{
public:
	CImPduGroupChangeMemberRequest(uint32_t change_type, uint32_t req_user_id, uint32_t group_id, list<uint32_t>* user_list,
			uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduGroupChangeMemberRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_CHANGE_MEMBER_REQUEST; }
};

class CImPduGroupChangeMemberResponse : public CImPdu
{
public:
	CImPduGroupChangeMemberResponse(uchar_t* buf, uint32_t len);
	virtual ~CImPduGroupChangeMemberResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_CHANGE_MEMBER_RESPONSE; }
	uint32_t GetReqUserId() { return m_req_user_id; }
	uint32_t GetResult() { return m_result; }
    uint32_t GetChangeType() { return m_change_type; }
	uint32_t GetGroupId() { return m_group_id; }
	uint32_t GetUserCnt() { return m_user_cnt; }
	uint32_t* GetUserList() { return m_user_list; }
	uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t 	m_req_user_id;
	uint32_t	m_result;
	uint32_t	m_group_id;
    uint32_t   m_change_type;
	uint32_t	m_user_cnt;
	uint32_t*	m_user_list;
	uint32_t 	m_attach_len;
	uchar_t* 	m_attach_data;
};

class CImPduGroupCreateNormalGroupNotify : public CImPdu
{
public:
	CImPduGroupCreateNormalGroupNotify(uchar_t* buf, uint32_t len);
    CImPduGroupCreateNormalGroupNotify(uint32_t group_id, const char* group_name,
            const char* group_avatar, uint32_t user_cnt, uint32_t* user_list);
	virtual ~CImPduGroupCreateNormalGroupNotify();
    
	virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_CREATE_NORMAL_GROUP_NOTIFY; }
	uint32_t GetGroupId() { return m_group_id; }
	uint32_t GetGroupNameLen() { return m_group_name_len; }
	char* GetGroupName() { return m_group_name; }
    uint32_t GetGroupAvatarLen() { return m_group_avatar_len; }
    char* GetGroupAvatar() { return m_group_avatar; }
	uint32_t GetUserCnt() { return m_user_cnt; }
	uint32_t* GetUserList() { return m_user_list; }
private:
	uint32_t	m_group_id;
	uint32_t	m_group_name_len;
	char*		m_group_name;
    uint32_t   m_group_avatar_len;
    char*      m_group_avatar;
	uint32_t	m_user_cnt;
	uint32_t*	m_user_list;
};

class CImPduGroupChangeMemberNotify : public CImPdu
{
public:
    CImPduGroupChangeMemberNotify(uchar_t* buf, uint32_t len);
    CImPduGroupChangeMemberNotify(uint32_t group_id, uint32_t user_cnt, uint32_t* user_list);
    virtual ~CImPduGroupChangeMemberNotify();
    
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_CHANGE_MEMBER_NOTIFY; }
    uint32_t GetGroupId() { return m_group_id; }
	uint32_t GetUserCnt() { return m_user_cnt; }
	uint32_t* GetUserList() { return m_user_list; }
private:
    uint32_t    m_group_id;
    uint32_t    m_user_cnt;
    uint32_t*   m_user_list;
};

#endif // WIN32

#endif /* IMPDUGROUP_H_ */
