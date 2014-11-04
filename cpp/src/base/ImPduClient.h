/*
 * ImPduClient.h
 *
 *  Created on: 2013-8-27
 *      Author: ziteng@mogujie.com
 */

#ifndef IMPDUCLIENT_H_
#define IMPDUCLIENT_H_

#include "ImPduBase.h"

#define IM_PDU_TYPE_HEARTBEAT						1


#define IM_PDU_TYPE_MSG_SERV_REQUEST				11
#define IM_PDU_TYPE_MSG_SERV_RESPONSE				12
#define IM_PDU_TYPE_LOGIN_REQUEST					13
#define IM_PDU_TYPE_LOGIN_RESPONSE					14
#define IM_PDU_TYPE_CLIENT_KICK_USER				15
#define IM_PDU_TYPE_KICK_USER		IM_PDU_TYPE_CLIENT_KICK_USER

//
#define IM_PDU_TYPE_CLIENT_BUDDY_LIST_REQUEST		21	// 
#define IM_PDU_TYPE_CLIENT_FRIEND_LIST				23	//
#define IM_PDU_TYPE_CLIENT_ONLINE_FRIEND_LIST		24
#define IM_PDU_TYPE_CLIENT_FRIEND_NOTIFY			25
#define IM_PDU_TYPE_CLIENT_USER_STATUS_REQUEST		28	// query a user's status
#define IM_PDU_TYPE_CLIENT_USER_STATUS_RESPONSE		29	// return a user's status

#define IM_PDU_TYPE_CLIENT_USERS_STATUS_REQUEST		280	// query a user's status
#define IM_PDU_TYPE_CLIENT_USERS_STATUS_RESPONSE		290	// return a user's status

//
#define IM_PDU_TYPE_CLIENT_MSG_DATA					31	//
#define IM_PDU_TYPE_CLIENT_MSG_DATA_ACK				32
#define IM_PDU_TYPE_CLIENT_MSG_READ_ACK				33
#define IM_PDU_TYPE_CLIENT_P2P_CMD_MSG				34
#define IM_PDU_TYPE_CLIENT_TIME_REQUEST				35
#define IM_PDU_TYPE_CLIENT_TIME_RESPONSE			36
#define IM_PDU_TYPE_CLIENT_UNREAD_MSG_CNT_REQUEST	37
#define IM_PDU_TYPE_CLIENT_UNREAD_MSG_CNT_RESPONSE	38
#define IM_PDU_TYPE_CLIENT_UNREAD_MSG_REQUEST		39
#define IM_PDU_TYPE_CLIENT_MSG_LIST_RESPONSE		41
#define IM_PDU_TYPE_CLIENT_DEPARTMENT_REQUEST       42
#define IM_PDU_TYPE_CLIENT_DEPARTMENT_RESPONSE      43
// 
#define IM_PDU_TYPE_CLIENT_ALL_USER_REQUEST			50
#define IM_PDU_TYPE_CLIENT_USER_INFO_REQUEST		51
#define IM_PDU_TYPE_CLIENT_USER_INFO_RESPONSE		52
#define IM_PDU_TYPE_CLIENT_DB_QUERY_REQUEST			53
#define IM_PDU_TYPE_CLIENT_DB_QUERY_RESPONSE		54
#define IM_PDU_TYPE_CLIENT_REMOVE_SESSION_REQUEST	55
#define IM_PDU_TYPE_CLIENT_REMOVE_SESSION_RESPONSE	56


#define IM_PDU_TYPE_CLIENT_SERVICE_SETTING			61	//  online/leave


class DLL_MODIFIER CImPduHeartbeat : public CImPdu
{
public:
	CImPduHeartbeat(uchar_t* buf, uint32_t len) {}
	CImPduHeartbeat();
	virtual ~CImPduHeartbeat() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_HEARTBEAT; }
};

///////////////////////////////////////////
class DLL_MODIFIER CImPduMsgServRequest : public CImPdu
{
public:
	CImPduMsgServRequest(uchar_t* buf, uint32_t len);
	CImPduMsgServRequest();
	virtual ~CImPduMsgServRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_MSG_SERV_REQUEST; }
private:
};

enum {
	REFUSE_REASON_NONE				= 0,
	REFUSE_REASON_NO_MSG_SERVER		= 1,
	REFUSE_REASON_MSG_SERVER_FULL 	= 2,
	REFUSE_REASON_NO_DB_SERVER		= 3,
	REFUSE_REASON_NO_LOGIN_SERVER	= 4,
	REFUSE_REASON_NO_ROUTE_SERVER	= 5,
	REFUSE_REASON_DB_VALIDATE_FAILED = 6,
	RESUSE_REASON_VERSION_TOO_OLD	= 7,
    REFUSE_REASON_NO_FILE_SERVER = 8,
};

class DLL_MODIFIER CImPduMsgServResponse : public CImPdu
{
public:
	CImPduMsgServResponse(uchar_t* buf, uint32_t len);
	CImPduMsgServResponse(uint32_t result, const char* ip1_addr = NULL, const char* ip2_addr = NULL, uint16_t port = 0);
	virtual ~CImPduMsgServResponse() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_MSG_SERV_RESPONSE; }

	uint32_t GetResult() { return m_result; }
	uint32_t GetIP1Len() { return m_ip1_len; }
	char* GetIP1Addr() { return m_ip1_addr; }
	uint32_t GetIP2Len() { return m_ip2_len; }
	char* GetIP2Addr() { return m_ip2_addr; }
	uint16_t GetPort() { return m_port; }
private:
	uint32_t	m_result;
	uint32_t	m_ip1_len;
	char*		m_ip1_addr;
	uint32_t	m_ip2_len;
	char*		m_ip2_addr;
	uint16_t	m_port;
};

class DLL_MODIFIER CImPduLoginRequest : public CImPdu
{
public:
	CImPduLoginRequest(uchar_t* buf, uint32_t len);
	CImPduLoginRequest(const char* user_name, const char* password, uint32_t online_status,
                       uint32_t client_type, const char* client_version = NULL);
	virtual ~CImPduLoginRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_LOGIN_REQUEST; }

	uint32_t GetUserNameLen() { return m_user_name_len; }
	char* GetUserName() { return m_user_name; }
	uint32_t GetPasswordLen() { return m_password_len; }
	char* GetPassword() { return m_password; }
	uint32_t GetOnlineStatus() { return m_online_status; }
    uint32_t GetClientType() { return m_client_type; }
	uint32_t GetClientVersionLen() { return m_client_version_len; }
	char* GetClientVersion() { return m_client_version; }
private:
	uint32_t 	m_user_name_len;
	char*		m_user_name;
	uint32_t	m_password_len;
	char*		m_password;
	uint32_t	m_online_status;
    uint32_t    m_client_type;
	uint32_t	m_client_version_len;
	char*		m_client_version;
};


class DLL_MODIFIER CImPduLoginResponse : public CImPdu
{
public:
	CImPduLoginResponse(uchar_t* buf, uint32_t len);	// for client use
	CImPduLoginResponse(uint32_t result, uint32_t online_status = 0,
			user_info_t* user = NULL, char* token = NULL); // for server user
	virtual ~CImPduLoginResponse() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_LOGIN_RESPONSE; }

	uint32_t GetServerTime() { return m_server_time; }
	uint32_t GetResult() { return m_result; }
	uint32_t GetOnlineStatus() { return m_online_status; }
	uint32_t GetUserIdUrllen() { return m_user_id_url_len; }
	char* GetUserIdUrl() { return m_user_id_url; }
	uint32_t GetNicknameLen() { return m_nickname_len; }
	char* GetNickname() { return m_nickname; }
	uint32_t GetAvatarLen() { return m_avatar_len; }
	char* GetAvatarUrl() { return m_avatar_url; }
    
    uint32_t GetTitleLen() { return m_title_len; }
    char* GetTitle() { return m_title; }
    uint32_t GetPositionLen() { return m_position_len; }
    char* GetPosition() { return m_position; }
    uint32_t GetRoleStatus() { return m_role_status; }
    uint32_t GetSex() { return m_sex; }
    uint32_t GetDepartIdUrlLen() { return m_depart_id_url_len; }
    char* GetDepartIdUrl() { return m_depart_id_url; }
    uint32_t GetJobNum() { return m_job_num; }
    uint32_t GetTelphoneLen() { return m_telphone_len; }
    char* GetTelphone() { return m_telphone; }
    uint32_t GetEmailLen() { return m_email_len; }
    char* GetEmail() { return m_email; }
    uint32_t GetTokenLen() { return m_token_len; }
    char* GetToken() { return m_token; }
private:
	uint32_t	m_server_time;		//
	uint32_t	m_result;	// 0 -- success, non-0 -- fail reason
	uint32_t	m_online_status;
	uint32_t 	m_user_id_url_len;
	char*		m_user_id_url;
	uint32_t	m_nickname_len;
	char*		m_nickname;
	uint32_t	m_avatar_len;
	char*		m_avatar_url;
    
    uint32_t    m_title_len;
    char*       m_title;
    uint32_t    m_position_len;
    char*       m_position;       // 地址
    uint32_t    m_role_status;     // 用户在职状态 0:在职 1:离职
    uint32_t    m_sex;            // 0:女 1:男
    uint32_t    m_depart_id_url_len;
    char*       m_depart_id_url;  // 部门id
    uint32_t    m_job_num;        // 工号
    uint32_t    m_telphone_len;
    char*       m_telphone;       // 电话
    uint32_t    m_email_len;
    char*       m_email;          // 邮箱
    uint32_t    m_token_len;
    char*       m_token;
};

#define KICK_REASON_DUPLICATE_USER 1

class CImPduKickUser : public CImPdu
{
public:
	CImPduKickUser(uchar_t* buf, uint32_t len);
	CImPduKickUser(uint32_t user_id, uint32_t reason);
	virtual ~CImPduKickUser() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_KICK_USER; }

	uint32_t GetUserId() { return m_user_id; }
	uint32_t GetReason() { return m_reason; }
private:
	uint32_t	m_user_id;
	uint32_t	m_reason;
};

///////////////////////////////////////////
class DLL_MODIFIER CImPduClientBuddyListRequest : public CImPdu
{
public:
	CImPduClientBuddyListRequest(uchar_t* buf, uint32_t len);
	CImPduClientBuddyListRequest(uint32_t req_type);
	virtual ~CImPduClientBuddyListRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_BUDDY_LIST_REQUEST; }
	uint32_t GetReqType() { return m_req_type; }
private:
	uint32_t m_req_type;
};

class DLL_MODIFIER CImPduClientFriendList : public CImPdu
{
public:
	CImPduClientFriendList(uchar_t* buf, uint32_t len);
	CImPduClientFriendList(uint32_t friend_cnt, user_info_t* friend_list); // for server
	virtual ~CImPduClientFriendList();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FRIEND_LIST; }

	uint32_t GetFriendCnt() { return m_friend_cnt; }
	client_user_info_t* GetFriendList() { return m_friend_list; }
private:
	uint32_t		 	m_friend_cnt;
	client_user_info_t* m_friend_list;   //现在只有id_url和user_updated有用
    
};

typedef struct {
	uint32_t 	id_len;
	char*		id_url;
} client_id_t;

typedef struct {
	uint32_t 	id_len;
	char*		id_url;
	uint32_t	status;
} client_stat_t;

enum {
	ONLINE_LIST_TYPE_FRIEND_LIST = 2,
};

class DLL_MODIFIER CImPduClientOnlineFriendList : public CImPdu
{
public:
	CImPduClientOnlineFriendList(uchar_t* buf, uint32_t len);
	CImPduClientOnlineFriendList(uint16_t list_type, uint32_t friend_cnt, user_stat_t* friend_stat_list); // for server use
	virtual ~CImPduClientOnlineFriendList();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_ONLINE_FRIEND_LIST; }
	uint16_t GetListType() { return m_list_type; }
	uint32_t GetFriendCnt() { return m_friend_cnt; }
	client_stat_t* GetFriendStatList() { return m_friend_stat_list; }
private:
	uint16_t		m_list_type;
	uint32_t		m_friend_cnt;
	client_stat_t* 	m_friend_stat_list;
};

enum {
	USER_STATUS_ONLINE 	= 1,
	USER_STATUS_OFFLINE	= 2,
	USER_STATUS_LEAVE	= 3,
};

class DLL_MODIFIER CImPduClientFriendNotify : public CImPdu
{
public:
	CImPduClientFriendNotify(uchar_t* buf, uint32_t len);
	CImPduClientFriendNotify(const char* id_url, uint32_t status);
	virtual ~CImPduClientFriendNotify() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FRIEND_NOTIFY; }

	uint32_t GetIdLen() { return m_id_len; }
	char* GetIdUrl() { return m_id_url; }
	uint32_t GetStatus() { return m_status; }
private:
	uint32_t	m_id_len;
	char*		m_id_url;
	uint32_t 	m_status;
};

class DLL_MODIFIER CImPduClientUserStatusRequest : public CImPdu
{
public:
	CImPduClientUserStatusRequest(uchar_t* buf, uint32_t len);
	CImPduClientUserStatusRequest(const char* user_id);
	virtual ~CImPduClientUserStatusRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_USER_STATUS_REQUEST; }
	uint32_t GetUserIdLen() { return m_user_id_len; }
	char* GetUserId() { return m_user_id; }
private:
	uint32_t 	m_user_id_len;
	char* 		m_user_id;
};

class DLL_MODIFIER CImPduClientUserStatusResponse : public CImPdu
{
public:
	CImPduClientUserStatusResponse(uchar_t* buf, uint32_t len);
	CImPduClientUserStatusResponse(const char* user_id, uint32_t status);
	virtual ~CImPduClientUserStatusResponse() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_USER_STATUS_RESPONSE; }
	uint32_t GetUserIdLen() { return m_user_id_len; }
	char* GetUserId() { return m_user_id; }
	uint32_t GetStatus() { return m_status; }
private:
	uint32_t 	m_user_id_len;
	char* 		m_user_id;
	uint32_t	m_status;
};


class DLL_MODIFIER CImPduClientUsersStatusRequest : public CImPdu
{
public:
	CImPduClientUsersStatusRequest(uchar_t* buf, uint32_t len);
	CImPduClientUsersStatusRequest(list<string>& users);
	virtual ~CImPduClientUsersStatusRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_USERS_STATUS_REQUEST; }
	uint32_t GetUserCount() {return m_user_count;}
	const list<string>& GetUsers() { return m_users;}
private:
	uint32_t    m_user_count;
	list<string> 	m_users;
};

class DLL_MODIFIER CImPduClientUsersStatusResponse : public CImPdu
{
public:
	CImPduClientUsersStatusResponse(uchar_t* buf, uint32_t len);
	CImPduClientUsersStatusResponse(const list<user_stat_t>& user_status_list);
	virtual ~CImPduClientUsersStatusResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_USERS_STATUS_RESPONSE; }
	uint32_t GetUserCount() { return m_user_count; }
	client_user_stat_t* GetUserStatusList() { return m_user_status_list; }
private:
	uint32_t				m_user_count;
	client_user_stat_t* 	m_user_status_list;
};

/////////////////////// ////////////////////
class DLL_MODIFIER CImPduClientMsgData : public CImPdu
{
public:
	CImPduClientMsgData(uchar_t* buf, uint32_t len);
	CImPduClientMsgData(uint32_t seq_no, const char* from_id_url, const char* to_id_url, uint32_t create_time,
			uint8_t msg_type, uint32_t msg_len, uchar_t* msg_data, uint32_t attach_len, char* attach_data);
	virtual ~CImPduClientMsgData() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_MSG_DATA; }

	void AddTime();	//

	uint32_t GetSeqNo() { return m_seq_no; }
	uint32_t GetFromIdLen() { return m_from_id_len; }
	char* GetFromIdUrl() { return m_from_id_url; }
	uint32_t GetToIdLen() { return m_to_id_len; }
	char* GetToIdUrl() { return m_to_id_url; }
	uint32_t GetCreateTime() { return m_create_time; }
	uint8_t GetMsgType() { return m_msg_type; }
	uint32_t GetMsgLen() { return m_msg_len; }
	uchar_t* GetMsgData() { return m_msg_data; }
	uint32_t GetAttachLen() { return m_attach_len; }
	char* GetAttachData() { return m_attach_data; }
private:
	uint32_t	m_seq_no;
	uint32_t	m_from_id_len;
	char*		m_from_id_url;
	uint32_t	m_to_id_len;
	char*		m_to_id_url;
	uint32_t	m_create_time;
	uint8_t		m_msg_type;
	uint32_t 	m_msg_len;
	uchar_t* 	m_msg_data;
	uint32_t	m_attach_len;
	char*		m_attach_data;
};

class DLL_MODIFIER CImPduClientMsgDataAck : public CImPdu
{
public:
	CImPduClientMsgDataAck(uchar_t* buf, uint32_t len);
	CImPduClientMsgDataAck(uint32_t seq_no, uint32_t from_id_len, const char* from_id_url);
	virtual ~CImPduClientMsgDataAck() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_MSG_DATA_ACK; }
	uint32_t GetSeqNo() { return m_seq_no; }
	uint32_t GetFromIdLen() { return m_from_id_len; }
	char* GetFromIdUrl() { return m_from_id_url; }
private:
	uint32_t	m_seq_no;
	uint32_t	m_from_id_len;
	char*		m_from_id_url;
};

class DLL_MODIFIER CImPduClientMsgReadAck : public CImPdu
{
public:
	CImPduClientMsgReadAck(uchar_t* buf, uint32_t len);
	CImPduClientMsgReadAck(const char* from_id_url);
	~CImPduClientMsgReadAck() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_MSG_READ_ACK; }
	uint32_t GetFromIdLen() { return m_from_id_len; }
	char* GetFromIdUrl() { return m_from_id_url; }
private:
	uint32_t	m_from_id_len;
	char*		m_from_id_url;
};

class DLL_MODIFIER CImPduClientP2PCmdMsg : public CImPdu
{
public:
	CImPduClientP2PCmdMsg(uchar_t* buf, uint32_t len);
	CImPduClientP2PCmdMsg(uint32_t seq_no, const char* from_id, const char* to_id, uint32_t cmd_msg_len, uchar_t* cmd_msg_data);
	virtual ~CImPduClientP2PCmdMsg() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_P2P_CMD_MSG; }

	uint32_t GetSeqNo() { return m_seq_no; }
	uint32_t GetFromIdLen() { return m_from_id_len; }
	char* GetFromId() { return m_from_id; }
	uint32_t GetToIdLen() { return m_to_id_len; }
	char* GetToId() { return m_to_id; }
	uint32_t GetCmdMsgLen() { return m_cmd_msg_len; }
	uchar_t* GetCmdMsgData() { return m_cmd_msg_data; }
private:
	uint32_t	m_seq_no;
	uint32_t	m_from_id_len;
	char* 		m_from_id;
	uint32_t 	m_to_id_len;
	char* 		m_to_id;
	uint32_t 	m_cmd_msg_len;
	uchar_t* 	m_cmd_msg_data;
};

class DLL_MODIFIER CImPduClientTimeRequest : public CImPdu
{
public:
	CImPduClientTimeRequest(uchar_t* buf, uint32_t len) {}
	CImPduClientTimeRequest();
	virtual ~CImPduClientTimeRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_TIME_REQUEST; }
};

class DLL_MODIFIER CImPduClientTimeResponse : public CImPdu
{
public:
	CImPduClientTimeResponse(uchar_t* buf, uint32_t len);
	CImPduClientTimeResponse();
	virtual ~CImPduClientTimeResponse() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_TIME_RESPONSE; }
	uint32_t GetServerTime() { return m_server_time; }
private:
	uint32_t m_server_time;
};

class DLL_MODIFIER CImPduClientUnreadMsgCntRequest : public CImPdu
{
public:
	CImPduClientUnreadMsgCntRequest(uchar_t* buf, uint32_t len);
	CImPduClientUnreadMsgCntRequest();
	virtual ~CImPduClientUnreadMsgCntRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_UNREAD_MSG_CNT_REQUEST; }
};

typedef struct {
	uint32_t	id_len;
	char*		id_url;
	uint32_t	unread_cnt;
} unread_info_t;

class DLL_MODIFIER CImPduClientUnreadMsgCntResponse : public CImPdu
{
public:
	CImPduClientUnreadMsgCntResponse(uchar_t* buf, uint32_t len);
	CImPduClientUnreadMsgCntResponse(uint32_t unread_cnt, UserUnreadMsgCnt_t* unread_list);	// for server use
	virtual ~CImPduClientUnreadMsgCntResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_UNREAD_MSG_CNT_RESPONSE; }

	uint32_t GetUnreadCnt() { return m_unread_cnt; }
	unread_info_t* GetUnreadList() { return m_unread_list; }
private:
	uint32_t		m_unread_cnt;
	unread_info_t*	m_unread_list;
};

// 
// 
class DLL_MODIFIER CImPduClientUnreadMsgRequest : public CImPdu
{
public:
	CImPduClientUnreadMsgRequest(uchar_t* buf, uint32_t len);
	CImPduClientUnreadMsgRequest(const char* from_id_url);
	~CImPduClientUnreadMsgRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_UNREAD_MSG_REQUEST; }

	uint32_t GetFromIdLen() { return m_from_id_len; }
	char* GetFromIdUrl() { return m_from_id_url; }
private:
	uint32_t	m_from_id_len;
	char*		m_from_id_url;
};

typedef struct {
	uint32_t	from_id_len;
	char*		from_id_url;
	uint32_t	from_name_len;
	char*		from_name;
	uint32_t 	from_nick_name_len;
	char* 		from_nick_name;
	uint32_t	from_avatar_len;
	char*		from_avatar_url;

	uint32_t	create_time;
	uint8_t		msg_type;
	uint32_t	msg_len;
	uchar_t*	msg_content;
} client_msg_t;

class DLL_MODIFIER CImPduClientMsgListResponse : public CImPdu
{
public:
	CImPduClientMsgListResponse(uchar_t* buf, uint32_t len);
	CImPduClientMsgListResponse(uint16_t cmd_id, const char* display_id_url, uint32_t msg_cnt, server_msg_t* msg_list);
	virtual ~CImPduClientMsgListResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_MSG_LIST_RESPONSE; }
	uint32_t GetDisplayIdLen() { return m_display_id_len; }
	char* GetDisplayId() { return m_display_id; }
	uint32_t GetMsgCnt() { return m_msg_cnt; }
	client_msg_t* GetMsgList() { return m_msg_list; }
private:
	uint32_t		m_display_id_len;
	char*			m_display_id;
	uint32_t		m_msg_cnt;
	client_msg_t*	m_msg_list;
};


///////////////////////////////////////////
class DLL_MODIFIER CImPduClientAllUserRequest : public CImPdu
{
public:
	CImPduClientAllUserRequest(uchar_t* buf, uint32_t len);
	CImPduClientAllUserRequest();
	virtual ~CImPduClientAllUserRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_ALL_USER_REQUEST; }
};

class DLL_MODIFIER CImPduClientUserInfoRequest : public CImPdu
{
public:
	CImPduClientUserInfoRequest(uchar_t* buf, uint32_t len);
	CImPduClientUserInfoRequest(uint32_t user_cnt, client_id_t* client_id_list);
	virtual ~CImPduClientUserInfoRequest();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_USER_INFO_REQUEST; }

	uint32_t GetUserCnt() { return m_user_cnt; }
	client_id_t* GetClientIdList() { return m_client_id_list; }
private:
	uint32_t		m_user_cnt;
	client_id_t*	m_client_id_list;
};

class DLL_MODIFIER CImPduClientUserInfoResponse : public CImPdu
{
public:
	CImPduClientUserInfoResponse(uchar_t* buf, uint32_t len);
	CImPduClientUserInfoResponse(uint16_t cmd_id, uint32_t user_cnt, user_info_t* user_info_list);
	virtual ~CImPduClientUserInfoResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_USER_INFO_RESPONSE; }

	uint32_t GetUserCnt() { return m_user_cnt; }
	client_user_info_t* GetUserInfoList() { return m_user_info_list; }
private:
	uint32_t			m_user_cnt;
	client_user_info_t*	m_user_info_list;
};

class DLL_MODIFIER CImPduClientDBQueryRequest : public CImPdu
{
public:
	CImPduClientDBQueryRequest(uchar_t* buf, uint32_t len);
	CImPduClientDBQueryRequest(uint32_t seq_no, uint32_t query_len, uchar_t* query_data);
	virtual ~CImPduClientDBQueryRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_DB_QUERY_REQUEST; }
	uint32_t GetSeqNo() { return m_seq_no; }
	uint32_t GetQueryLen() { return m_query_len; }
	uchar_t* GetQeuryData() { return m_query_data; }
private:
	uint32_t m_seq_no;
	uint32_t m_query_len;
	uchar_t* m_query_data;
};

class DLL_MODIFIER CImPduClientDBQueryResponse : public CImPdu
{
public:
	CImPduClientDBQueryResponse(uchar_t* buf, uint32_t len);
	CImPduClientDBQueryResponse(uint32_t seq_no, uint32_t result_len, uchar_t* result_data);
	virtual ~CImPduClientDBQueryResponse() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_DB_QUERY_RESPONSE; }
	uint32_t GetSeqNo() { return m_seq_no; }
	uint32_t GetResultLen() { return m_result_len; }
	uchar_t* GetResultData() { return m_result_data; }
private:
	uint32_t m_seq_no;
	uint32_t m_result_len;
	uchar_t* m_result_data;
};

enum
{
	SESSION_TYPE_SINGLE = 1,          //单个用户会话
	SESSION_TYPE_GROUP = 2,           //群会话
	SESSION_TYPE_TEMP_GROUP = 3,      //临时群会话
};

class DLL_MODIFIER CImPduClientRemoveSessionRequest : public CImPdu
{
public:
	CImPduClientRemoveSessionRequest(uchar_t* buf, uint32_t len);
	CImPduClientRemoveSessionRequest(uint32_t session_type, const char* session_id);
	virtual ~CImPduClientRemoveSessionRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_REMOVE_SESSION_REQUEST; }
	uint32_t GetSessionType() { return m_session_type; }
	uint32_t GetSessionIdLen() { return m_session_id_len; }
	char* GetSessionId() { return m_session_id; }
private:
	uint32_t	m_session_type;
	uint32_t 	m_session_id_len;
	char*		m_session_id;
};

class DLL_MODIFIER CImPduClientRemoveSessionResponse : public CImPdu
{
public:
	CImPduClientRemoveSessionResponse(uchar_t* buf, uint32_t len);
	CImPduClientRemoveSessionResponse(uint32_t result, uint32_t session_type, const char* session_id);
	virtual ~CImPduClientRemoveSessionResponse() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_REMOVE_SESSION_RESPONSE; }
	uint32_t GetResult() { return m_result; }
	uint32_t GetSessionType() { return m_session_type; }
	uint32_t GetSessionIdLen() { return m_session_id_len; }
	char* GetSessionId() { return m_session_id; }
private:
	uint32_t	m_result;		// 0 - success, 1 - fail
	uint32_t	m_session_type;	//
	uint32_t 	m_session_id_len;
	char*		m_session_id;
};

///////////////////////////////////////////
enum {
	SERVICE_KEY_MAX_SERVICE_CNT = 1,
	SERVICE_KEY_ONLINE_STATUS = 2,
};

class DLL_MODIFIER CImPduClientServiceSetting : public CImPdu
{
public:
	CImPduClientServiceSetting(uchar_t* buf, uint32_t len);
	CImPduClientServiceSetting(uint32_t key, uint32_t value);
	virtual ~CImPduClientServiceSetting() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_SERVICE_SETTING; }

	uint32_t GetKey() { return m_key; }
	uint32_t GetValue() { return m_value; }
private:
	uint32_t		m_key;
	uint32_t		m_value;
};

class DLL_MODIFIER CImPduClientDepartmentRequest : public CImPdu
{
public:
    CImPduClientDepartmentRequest(uchar_t* buf, uint32_t len);
    CImPduClientDepartmentRequest();
    
    virtual ~CImPduClientDepartmentRequest() {}
    
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_DEPARTMENT_REQUEST; }
    
};

class DLL_MODIFIER CImPduClientDepartmentResponse : public CImPdu
{
public:
    CImPduClientDepartmentResponse(uchar_t* buf, uint32_t len);
    CImPduClientDepartmentResponse(uint32_t depart_cnt, department_info_t* depart_list);
    virtual ~CImPduClientDepartmentResponse();
    
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_DEPARTMENT_RESPONSE; }
    uint32_t GetDepartCnt() { return m_depart_cnt; }
    client_department_info_t* GetDepartmentList() { return m_depart_list; }
private:
    uint32_t m_depart_cnt;
    client_department_info_t* m_depart_list;
};

#endif /* IMPDUCLIENT_H_ */
