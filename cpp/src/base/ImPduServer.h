/*
 * ImPduServer.h
 *
 *  Created on: 2013-8-27
 *      Author: ziteng@mogujie.com
 */

#ifndef IMPDUSERVER_H_
#define IMPDUSERVER_H_

#include "ImPduBase.h"
#include "util.h"

// DB storage server related
#define IM_PDU_TYPE_STOP_RECEIVE_PACKET			100  // business server stop service
#define IM_PDU_TYPE_VALIDATE_REQUEST			110
#define IM_PDU_TYPE_VALIDATE_RESPONSE			111
#define IM_PDU_TYPE_FRIEND_LIST_REQUEST			122
#define IM_PDU_TYPE_FRIEND_LIST_RESPONSE		123
#define IM_PDU_TYPE_REMOVE_SESSION_REQUEST		124
#define IM_PDU_TYPE_REMOVE_SESSION_RESPONSE		125
#define IM_PDU_TYPE_DEPARTMENT_REQUEST          126
#define IM_PDU_TYPE_DEPARTMENT_RESPONSE         127


#define IM_PDU_TYPE_USERS_INFO_REQUEST			130
#define IM_PDU_TYPE_USERS_INFO_RESPONSE			131
#define IM_PDU_TYPE_ALL_USER_REQUEST			132	// 内部所有员工信息请求
#define IM_PDU_TYPE_ALL_USER_RESPONSE			133	//

#define IM_PDU_TYPE_MSG_DATA					140
#define IM_PDU_TYPE_UNREAD_MSG_COUNT_REQUEST	141
#define IM_PDU_TYPE_UNREAD_MSG_COUNT_RESPONSE	142
#define IM_PDU_TYPE_UNREAD_MSG_REQUEST			143
#define IM_PDU_TYPE_MSG_LIST_RESPONSE			145
#define IM_PDU_TYPE_MSG_READ_ACK				146
#define IM_PDU_TYPE_DB_QUERY_REQUEST			170
#define IM_PDU_TYPE_DB_QUERY_RESPONSE			171
#define IM_PDU_TYPE_DB_WRITE_RESPONSE			180
#define IM_PDU_TYPE_USER_ACTION_LOG				190



// message server internal
#define IM_PDU_TYPE_ROLE_SET					200	// 设置RouteServer的Master
#define IM_PDU_TYPE_ONLINE_USER_INFO			201	// MsgServer刚连上RouteServer发送的用户在线状态信息
#define IM_PDU_TYPE_MSG_SERV_INFO				202	// MsgServer刚连上LoginServer发送的在线人数信息
#define IM_PDU_TYPE_USER_STATUS_UPDATE			203	// MsgServer发送给RouteServer的用户状态更新
#define IM_PDU_TYPE_USER_CNT_UPDATE				204	// MsgServer发送给LoginServer的用户人数更新
#define IM_PDU_TYPE_USER_CONN_INFO				205 // MsgServer刚连上时发送给LoginServer的每个用户连接数信息

#define IM_PDU_TYPE_FRIEND_STATUS_QUERY			210
#define IM_PDU_TYPE_FRIEND_STATUS_LIST			211
#define IM_PDU_TYPE_FRIEND_STATUS_NOTIFY		212
#define IM_PDU_TYPE_P2P_MSG						213	// a user defined message from user1 to user2

#define IM_PDU_TYPE_GROUP_P2P_MSG_REQUEST       214 //
#define IM_PDU_TYPE_GROUP_P2P_MSG_RESPONSE      215



#define IM_PDU_TYPE_USER_STATUS_REQUEST			223
#define IM_PDU_TYPE_USER_STATUS_RESPONSE		224
#define IM_PDU_TYPE_USER_LIST_STATUS_REQUEST	225
#define IM_PDU_TYPE_USER_LIST_STATUS_RESPONSE	226
#define IM_PDU_TYPE_USERS_STATUS_REQUEST			227
#define IM_PDU_TYPE_USERS_STATUS_RESPONSE		228

#define IM_PDU_TYPE_SERVER_KICK_USER            230
#define IM_PDU_TYPE_USER_CLIENT_TYPE_REQUEST    231
#define IM_PDU_TYPE_USER_CLIENT_TYPE_RESPONSE   232

class CImPduStopReceivePacket : public CImPdu
{
public:
	CImPduStopReceivePacket(uchar_t* buf, uint32_t len);
	virtual ~CImPduStopReceivePacket() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_STOP_RECEIVE_PACKET; }
	uint32_t GetCode() { return m_code; }
private:
	uint32_t	m_code;
};

// 到BusinessServer的请求和回复都添加了attach_len, attach_data，这是为了HttpMsgServer添加，
// 因为每个请求都是对应不同的socket，可以把handle放在这儿，收到回复是就知道发给哪个TCP连接了
class CImPduValidateRequest : public CImPdu
{
public:
	CImPduValidateRequest(const char* user_name, const char* password, uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduValidateRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_VALIDATE_REQUEST; }
};

class CImPduValidateResponse : public CImPdu
{
public:
	CImPduValidateResponse(uchar_t* buf, uint32_t len);
    CImPduValidateResponse(const char* user_name, uint32_t result, const user_info_t* user, uint32_t attach_len = 0, uchar_t* attach_data = NULL, uint32_t reserved = 0);
	virtual ~CImPduValidateResponse() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_VALIDATE_RESPONSE; }

	uint32_t GetUserNameLen() { return m_user_name_len; }
    char* GetUserName() { return m_user_name; }
	uint32_t GetResult() { return m_result; }
	user_info_t* GetUserInfo() { return &m_user; }
	uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
    uint32_t    m_user_name_len;
    char*       m_user_name;
	uint32_t	m_result;	// 0 -- success, non-0 -- fail reason
	user_info_t	m_user;		// if result != 0, no this item
	uint32_t	m_attach_len;
	uchar_t*	m_attach_data;
};

// 获取最近联系人列表请求
class CImPduFriendListRequest : public CImPdu
{
public:
	CImPduFriendListRequest(uint32_t user_id, uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduFriendListRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_FRIEND_LIST_REQUEST; }
};

// 获取最近联系人列表回复
class CImPduFriendListResponse : public CImPdu
{
public:
	CImPduFriendListResponse(uchar_t* buf, uint32_t len);
	virtual ~CImPduFriendListResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_FRIEND_LIST_RESPONSE; }
	uint32_t GetFromUserId() { return m_from_user_id; }
	uint32_t GetFriendCnt() { return m_friend_cnt; }
	user_info_t* GetFriendList() { return m_friend_list; }
	uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t		m_from_user_id;
	uint32_t		m_friend_cnt;
	user_info_t*	m_friend_list;    //只有user_id和 user_updated有效
	uint32_t		m_attach_len;
	uchar_t*		m_attach_data;
};


class CImPduDepartmentRequest : public CImPdu
{
public:
    CImPduDepartmentRequest(uint32_t user_id, uint32_t attach_len = 0, uchar_t* attach_data = NULL);
    virtual ~CImPduDepartmentRequest() {}
    
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_DEPARTMENT_REQUEST; }
};

class CImPduDepartmentResponse : public CImPdu
{
public:
    CImPduDepartmentResponse(uchar_t* buf, uint32_t len);
    virtual ~CImPduDepartmentResponse();
    
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_DEPARTMENT_RESPONSE; }
    
    uint32_t GetReqUserId() { return m_req_user_id; }
    uint32_t GetDepartCnt() { return m_depart_cnt; }
    department_info_t* GetDepartList() { return m_depart_list; }
    uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
    uint32_t m_req_user_id;
    uint32_t m_depart_cnt;
    department_info_t* m_depart_list;
    uint32_t		m_attach_len;
	uchar_t*		m_attach_data;
};

class CImPduRemoveSessionRequest : public CImPdu
{
public:
	CImPduRemoveSessionRequest(uint32_t req_user_id, uint32_t session_id, uint32_t session_type,
			uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduRemoveSessionRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_REMOVE_SESSION_REQUEST; }
};

class CImPduRemoveSessionResponse : public CImPdu
{
public:
	CImPduRemoveSessionResponse(uchar_t* buf, uint32_t len);
	virtual ~CImPduRemoveSessionResponse() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_REMOVE_SESSION_RESPONSE; }
	uint32_t GetReqUserId() { return m_req_user_id; }
	uint32_t GetResult() { return m_result; }
	uint32_t GetSessionType() { return m_session_type; }
	uint32_t GetSessionId() { return m_session_id; }
	uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t	m_req_user_id;
	uint32_t	m_result;		// 0 - success, 1 - fail
	uint32_t 	m_session_id;
	uint32_t	m_session_type;	//
	uint32_t 	m_attach_len;
	uchar_t*	m_attach_data;
};

class CImPduUsersInfoRequest : public CImPdu
{
public:
	CImPduUsersInfoRequest(uint32_t from_user_id, uint32_t user_cnt, uint32_t* user_id_list,
			 uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduUsersInfoRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_USERS_INFO_REQUEST; }
};

class CImPduUsersInfoResponse : public CImPdu
{
public:
	CImPduUsersInfoResponse(uchar_t* buf, uint32_t len);
	virtual ~CImPduUsersInfoResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_USERS_INFO_RESPONSE; }
	uint32_t GetFromUserId() { return m_from_user_id; }
	uint32_t GetUserCnt() { return m_user_cnt; }
	user_info_t* GetUserInfoList() { return m_user_info_list; }
	uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t		m_from_user_id;
	uint32_t		m_user_cnt;
	user_info_t* 	m_user_info_list;
	uint32_t		m_attach_len;
	uchar_t*		m_attach_data;
};

class CImPduAllUserRequest : public CImPdu
{
public:
	CImPduAllUserRequest(uint32_t req_user_id, uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduAllUserRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_ALL_USER_REQUEST; }
};

class DLL_MODIFIER CImPduDBQueryRequest : public CImPdu
{
public:
	CImPduDBQueryRequest(uint32_t seq_no, uint32_t user_id, uint32_t query_len, uchar_t* query_data,
			uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduDBQueryRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_DB_QUERY_REQUEST; }
};

class DLL_MODIFIER CImPduDBQueryResponse : public CImPdu
{
public:
	CImPduDBQueryResponse(uchar_t* buf, uint32_t len);
	virtual ~CImPduDBQueryResponse() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_DB_QUERY_RESPONSE; }
	uint32_t GetSeqNo() { return m_seq_no; }
	uint32_t GetUserid() { return m_user_id; }
	uint32_t GetResultLen() { return m_result_len; }
	uchar_t* GetResultData() { return m_result_data; }
	uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t 	m_seq_no;
	uint32_t	m_user_id;
	uint32_t 	m_result_len;
	uchar_t* 	m_result_data;
	uint32_t	m_attach_len;
	uchar_t*	m_attach_data;
};

class CImPduMsgData : public CImPdu
{
public:
	CImPduMsgData(uchar_t* buf, uint32_t len);
	CImPduMsgData(uint32_t request_id, uint32_t from_user_id, uint32_t to_user_id,
                  uint32_t create_time, uint8_t msg_type, uint32_t msg_len, uchar_t* msg_data,
                  uint32_t client_type, uint32_t attach_len, char* attach_data);
	virtual ~CImPduMsgData() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_MSG_DATA; }

	uint32_t GetRequestId() { return m_request_id; }
	uint32_t GetFromUserId() { return m_from_user_id; }
	uint32_t GetToUserId() { return m_to_user_id; }
	uint32_t GetCreateTime() { return m_create_time; }
	uint8_t GetMsgType() { return m_msg_type; }
	uint32_t GetMsgLen() { return m_msg_len; }
	uchar_t* GetMsgData() { return m_msg_data; }
    uint32_t GetClientType() { return m_client_type; }
	uint32_t GetAttachLen() { return m_attach_len; }
	char* GetAttachData() { return m_attach_data; }
private:
	uint32_t  	m_request_id;
	uint32_t	m_from_user_id;
	uint32_t 	m_to_user_id;
	uint32_t	m_create_time;
	uint8_t		m_msg_type;
	uint32_t 	m_msg_len;
	uchar_t* 	m_msg_data;
    uint32_t    m_client_type;
	uint32_t	m_attach_len;	// 这两项是PC client添加，不是用于HttpMsgServer
	char*		m_attach_data;
};

class CImPduUnreadMsgCountRequest : public CImPdu
{
public:
	CImPduUnreadMsgCountRequest(uint32_t user_id, uint32_t client_type, uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduUnreadMsgCountRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_UNREAD_MSG_COUNT_REQUEST; }
};

class CImPduUnreadMsgCountResponse : public CImPdu
{
public:
	CImPduUnreadMsgCountResponse(uchar_t* buf, uint32_t len);
	virtual ~CImPduUnreadMsgCountResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_UNREAD_MSG_COUNT_RESPONSE; }
	uint32_t GetToUserId() { return m_to_user_id; }
	uint32_t GetUserUnreadCnt() { return m_user_unread_cnt; }
	UserUnreadMsgCnt_t* GetUserUnreadList() { return m_user_unread_list; }
	uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t	m_to_user_id;
	uint32_t	m_user_unread_cnt;
	UserUnreadMsgCnt_t* m_user_unread_list;
	uint32_t	m_attach_len;
	uchar_t*	m_attach_data;
};

class CImPduUnreadMsgRequest : public CImPdu
{
public:
	CImPduUnreadMsgRequest(uint32_t from_user_id, uint32_t to_user_id, uint32_t client_type, uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduUnreadMsgRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_UNREAD_MSG_REQUEST; }
};


class CImPduMsgListResponse : public CImPdu
{
public:
	CImPduMsgListResponse(uchar_t* buf, uint32_t len);
	virtual ~CImPduMsgListResponse();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_MSG_LIST_RESPONSE; }
	uint32_t GetRequestCmdId() { return m_request_cmd_id; }
	uint32_t GetFromUserId() { return m_from_user_id; }
	uint32_t GetToUserId() { return m_to_user_id; }
	uint32_t GetMsgCount() { return m_msg_count; }
	server_msg_t* GetMsgList() { return m_msg_list; }
	uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t 	m_request_cmd_id;
	uint32_t	m_from_user_id;
	uint32_t	m_to_user_id;
	uint32_t	m_msg_count;
	server_msg_t* m_msg_list;
	uint32_t	m_attach_len;
	uchar_t*	m_attach_data;
};

class CImPduMsgReadAck : public CImPdu
{
public:
	CImPduMsgReadAck(uint32_t request_id, uint32_t from_user_id, uint32_t to_user_id, uint32_t client_type);
	virtual ~CImPduMsgReadAck() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_MSG_READ_ACK; }
};

class CImPduDBWriteResponse : public CImPdu
{
public:
	CImPduDBWriteResponse(uchar_t* buf, uint32_t len);
	CImPduDBWriteResponse(uint32_t request_id, uint32_t result, uint16_t request_type, uint32_t from_id, uint32_t to_id);
	virtual ~CImPduDBWriteResponse() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_DB_WRITE_RESPONSE; }
	uint32_t GetRequestId() { return m_request_id; }
	uint32_t GetResult() { return m_result; }
	uint16_t GetRequestPduType() { return m_request_pdu_type; }
	uint32_t GetFromId() { return m_from_id; }
	uint32_t GetToId() { return m_to_id; }
private:
	uint32_t	m_request_id;
	uint32_t	m_result;
	uint16_t	m_request_pdu_type;
	uint32_t	m_from_id;
	uint32_t	m_to_id;
};

enum {
	USER_ACTION_TYPE_LOGIN 	= 100,
	USER_ACTION_TYPE_LOGOUT	= 101,
	USER_ACTION_TYPE_FAIL	= 102,
	USER_ACTION_TYPE_KICK	= 103,
	USER_ACTION_TYPE_ONLINE	= 104,
	USER_ACTION_TYPE_LEAVE	= 105,
};

class CImPduUserActionLog : public CImPdu
{
public:
	CImPduUserActionLog(uint32_t source, uint32_t protocol, const char* ip, uint32_t user_id, uint32_t action_type,
	const char* os_type = NULL, const char* user_agent = NULL, const char* flash_ver = NULL, const char* client_ver = NULL);
	virtual ~CImPduUserActionLog() {}
};

//////////// Message Server internal //////////
class CImPduRoleSet : public CImPdu
{
public:
	CImPduRoleSet(uchar_t* buf, uint32_t len);
	CImPduRoleSet(uint32_t master);
	virtual ~CImPduRoleSet() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_ROLE_SET; }
	uint32_t GetMaster() { return m_master; }
public:
	uint32_t	m_master;	// 0-slave, 1-master
};

class CImPduOnlineUserInfo : public CImPdu
{
public:
	CImPduOnlineUserInfo(uchar_t* buf, uint32_t len);
	CImPduOnlineUserInfo(list<user_conn_stat_t>* user_status_list);
	virtual ~CImPduOnlineUserInfo();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_ONLINE_USER_INFO; }

	uint32_t GetUserCount() { return m_user_count; }
	user_conn_stat_t* GetUserStatusList() { return m_user_status_list; }
private:
	uint32_t		m_user_count;
	user_conn_stat_t* 	m_user_status_list;
};

enum {
	MSG_SERVER_TYPE_TCP		= 1,
};

class CImPduMsgServInfo : public CImPdu
{
public:
	CImPduMsgServInfo(uchar_t* buf, uint32_t len);
	CImPduMsgServInfo(const char* ip_addr1, const char* ip_addr2, uint16_t port, uint32_t max_conn_cnt,
			uint32_t cur_conn_cnt, const char* hostname, uint32_t server_type);
	virtual ~CImPduMsgServInfo() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_MSG_SERV_INFO; }

	uint32_t GetIp1Len() { return m_ip1_len; }
	char* GetIp1Addr() { return m_ip1_addr; }
	uint32_t GetIp2Len() { return m_ip2_len; }
	char* GetIp2Addr() { return m_ip2_addr; }
	uint16_t GetPort() { return m_port; }
	uint32_t GetMaxConnCnt() { return m_max_conn_cnt; }
	uint32_t GetCurConnCnt() { return m_cur_conn_cnt; }
	uint32_t GetHostnameLen() { return m_hostname_len; }
	char* GetHostname() { return m_hostname; }
	uint32_t GetServerType() { return m_server_type; }
private:
	uint32_t	m_ip1_len;
	char*		m_ip1_addr;
	uint32_t 	m_ip2_len;
	char*		m_ip2_addr;
	uint16_t	m_port;
	uint32_t	m_max_conn_cnt;
	uint32_t	m_cur_conn_cnt;
	uint32_t	m_hostname_len;
	char* 		m_hostname;
	uint32_t	m_server_type;	// 1 - tcp server, 2-http long-polling server
};


class CImPduUserStatusUpdate : public CImPdu
{
public:
	CImPduUserStatusUpdate(uchar_t* buf, uint32_t len);
	CImPduUserStatusUpdate(uint32_t user_status, uint32_t user_id, uint32_t client_type_flag);
	virtual ~CImPduUserStatusUpdate() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_USER_STATUS_UPDATE; }

	uint32_t GetUserStatus() { return m_user_status; }
	uint32_t GetUserId() { return m_user_id; }
    uint32_t GetClientTypeFlag() { return m_client_type_flag; }
private:
	uint32_t	m_user_status;	// 1-ONLINE, 2-OFFLINE, 3-Leave
	uint32_t	m_user_id;
    uint32_t  m_client_type_flag;
};

enum {
	USER_CNT_INC = 1,
	USER_CNT_DEC = 2,
};

typedef struct {
	uint32_t 	user_id;
	uint32_t	conn_cnt;
} user_conn_t;

class CImPduUserConnInfo : public CImPdu
{
public:
	CImPduUserConnInfo(uchar_t* buf, uint32_t len);
	CImPduUserConnInfo(list<user_conn_t>* user_conn_list);
	virtual ~CImPduUserConnInfo();
    
	virtual uint16_t GetPduType() { return IM_PDU_TYPE_USER_CONN_INFO; }
    uint32_t GetUserCnt() { return m_user_cnt; }
    user_conn_t* GetUserConnList() { return m_user_conn_list; }
private:
	uint32_t 		m_user_cnt;
	user_conn_t* 	m_user_conn_list;
};

class CImPduUserCntUpdate : public CImPdu
{
public:
	CImPduUserCntUpdate(uchar_t* buf, uint32_t len);
	CImPduUserCntUpdate(uint32_t action, uint32_t user_id);
	virtual ~CImPduUserCntUpdate() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_USER_CNT_UPDATE; }
	uint32_t GetUserAction() { return m_user_action; }
    uint32_t GetUserId() { return m_user_id; }
private:
	uint32_t	m_user_action;	// 1-inc, 2-dec
    uint32_t    m_user_id;
};

class CImPduFriendStatusQuery : public CImPdu
{
public:
	CImPduFriendStatusQuery(uchar_t* buf, uint32_t len);
	CImPduFriendStatusQuery(uint16_t request_type, uint32_t from_user_id, list<uint32_t>* friend_id_list);
	virtual ~CImPduFriendStatusQuery();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_FRIEND_STATUS_QUERY; }

	uint16_t GetRequestType() { return m_request_type; }
	uint32_t GetFromUserId() { return m_from_user_id; }
	uint32_t GetFriendCnt() { return m_friend_cnt; }
	uint32_t* GetFriendIdList() { return m_friend_id_list; }
private:
	uint16_t	m_request_type;
	uint32_t	m_from_user_id;
	uint32_t	m_friend_cnt;
	uint32_t*   m_friend_id_list;
};

class CImPduFriendStatusList : public CImPdu
{
public:
	CImPduFriendStatusList(uchar_t* buf, uint32_t len);
	CImPduFriendStatusList(uint16_t request_type, uint32_t from_user_id, list<user_stat_t>* friend_stat_list);
	virtual ~CImPduFriendStatusList();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_FRIEND_STATUS_LIST; }

	uint16_t GetRequestType() { return m_request_type; }
	uint32_t GetFromUserId() { return m_from_user_id; }
	uint32_t GetFriendCnt() { return m_friend_cnt; }
	user_stat_t* GetFriendStatList() { return m_friend_stat_list; }
private:
	uint16_t	m_request_type;
	uint32_t	m_from_user_id;
	uint32_t	m_friend_cnt;
	user_stat_t* m_friend_stat_list;
};

// notify other friend my current status
class CImPduFriendStatusNotify : public CImPdu
{
public:
	CImPduFriendStatusNotify(uchar_t* buf, uint32_t len);
	CImPduFriendStatusNotify(uint32_t from_user_id, uint32_t from_user_status, list<uint32_t>* friend_id_list);
	virtual ~CImPduFriendStatusNotify();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_FRIEND_STATUS_NOTIFY; }

	uint32_t GetFromUserId() { return m_from_user_id; }
	uint32_t GetFromUserStatus() { return m_from_user_status; }
	uint32_t GetFriendCnt() { return m_friend_cnt; }
	uint32_t* GetFriendIdList() { return m_friend_id_list; }
private:
	uint32_t	m_from_user_id;
	uint32_t	m_from_user_status;
	uint32_t	m_friend_cnt;
	uint32_t*   m_friend_id_list;
};

class CImPduP2PMsg : public CImPdu
{
public:
	CImPduP2PMsg(uchar_t* buf, uint32_t len);
	CImPduP2PMsg(uint32_t seq_no, uint32_t from_user_id, uint32_t to_user_id, uint32_t msg_len, uchar_t* msg_data);
	virtual ~CImPduP2PMsg() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_P2P_MSG; }

	uint32_t GetSeqNo() { return m_seq_no; }
	uint32_t GetFromUserId() { return m_from_user_id; }
	uint32_t GetToUserId() { return m_to_user_id; }
	uint32_t GetMsgLen() { return m_msg_len; }
	uchar_t* GetMsgData() { return m_msg_data; }
private:
	uint32_t 	m_seq_no;
	uint32_t	m_from_user_id;
	uint32_t	m_to_user_id;
	uint32_t	m_msg_len;
	uchar_t*	m_msg_data;
};

class CImPduGroupP2PMessageRequest : public CImPdu
{
public:
    CImPduGroupP2PMessageRequest(uint32_t from_user_id, uint32_t flag, list<string>& to_user_name_list, uint32_t create_time, uint32_t msg_len, uchar_t* msg_data);
    virtual ~CImPduGroupP2PMessageRequest() {}
    
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_P2P_MSG_REQUEST; }
};

class CImPduGroupP2PMessageResponse : public CImPdu
{
public:
    CImPduGroupP2PMessageResponse(uchar_t* buf, uint32_t len);
    virtual ~CImPduGroupP2PMessageResponse();

    virtual uint16_t GetPduType() { return IM_PDU_TYPE_GROUP_P2P_MSG_RESPONSE; }
    uint32_t  GetReqUserId() { return m_req_user_id; }
    uint32_t  GetResult() { return m_result; }
    uint32_t  GetUserCnt() { return m_user_cnt; }
    uint32_t* GetUserList() { return m_user_list; }
    uint32_t  GetMsgDataLen() { return m_msg_len; }
    uchar_t*  GetMsgData() { return m_msg_data; }
    uint32_t  GetCreateTime() { return m_create_time; }
    uint32_t  GetFlag() { return m_flag; }
private:
    uint32_t 	m_req_user_id;
    uint32_t	m_result;
    uint32_t    m_create_time;
    uint32_t	m_user_cnt;
    uint32_t*	m_user_list;
    uint32_t 	m_msg_len;
    uchar_t* 	m_msg_data;
    uint32_t    m_flag;

};

class CImPduUserStatusRequest : public CImPdu
{
public:
	CImPduUserStatusRequest(uchar_t* buf, uint32_t len);
	CImPduUserStatusRequest(uint32_t request_id, uint32_t query_id, uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduUserStatusRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_USER_STATUS_REQUEST; }
	uint32_t GetRequestId() { return m_request_id; }
	uint32_t GetQueryId() { return m_query_id; }
	uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t	m_request_id;	// 哪个用户发起的查询
	uint32_t	m_query_id;		// 要查询哪个用户
	uint32_t 	m_attach_len;
	uchar_t*	m_attach_data;
};

class CImPduUsersStatusRequest : public CImPdu
{
public:
	CImPduUsersStatusRequest(uchar_t* buf, uint32_t len);
	CImPduUsersStatusRequest(uint32_t request_id, list<uint32_t>& query_list,
                             uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduUsersStatusRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_USERS_STATUS_REQUEST; }
	uint32_t GetRequestId() { return m_request_id; }
	uint32_t GetQueryCount() { return m_query_count;}
	const list<uint32_t>& GetQueryList() { return m_query_list; }
    uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t	m_request_id;	// 哪个用户发起的查询
	uint32_t    m_query_count;
	list<uint32_t> m_query_list;
    uint32_t 	m_attach_len;
	uchar_t*	m_attach_data;
};

class CImPduUserStatusResponse : public CImPdu
{
public:
	CImPduUserStatusResponse(uchar_t* buf, uint32_t len);
	CImPduUserStatusResponse(uint32_t request_id, uint32_t query_id, uint32_t status,
			uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduUserStatusResponse() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_USER_STATUS_RESPONSE; }
	uint32_t GetRequestId() { return m_request_id; }
	uint32_t GetQueryId() { return m_query_id; }
	uint32_t GetStatus() { return m_status; }
	uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t	m_request_id;	// 哪个用户发起的查询
	uint32_t	m_query_id;		// 要查询哪个用户
	uint32_t 	m_status;
	uint32_t 	m_attach_len;
	uchar_t*	m_attach_data;
};

class CImPduUsersStatusResponse : public CImPdu
{
public:
	CImPduUsersStatusResponse(uchar_t* buf, uint32_t len);
	CImPduUsersStatusResponse(uint32_t request_id, const list<user_stat_t>& result_list,
                              uint32_t attach_len = 0, uchar_t* attach_data = NULL);
	virtual ~CImPduUsersStatusResponse() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_USERS_STATUS_RESPONSE; }
	uint32_t GetRequestId() { return m_request_id; }
	uint32_t GetResultCount() { return m_result_count;}
	const list<user_stat_t>& GetQueryResult() { return m_result_list; }
    uint32_t GetAttachLen() { return m_attach_len; }
	uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t	m_request_id;	// 哪个用户发起的查询
	uint32_t    m_result_count;
	list<user_stat_t>	m_result_list;		// 要查询哪个用户
    uint32_t 	m_attach_len;
	uchar_t*	m_attach_data;
};

class CImPduServerKickUser : public CImPdu
{
public:
    CImPduServerKickUser(uchar_t* buf, uint32_t len);
    CImPduServerKickUser(uint32_t user_id, uint32_t client_type, uint32_t kick_reason);
    virtual ~CImPduServerKickUser() {}
    
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_SERVER_KICK_USER; }
    
    uint32_t GetUserId() { return m_user_id; }
    uint32_t GetClientType() { return m_client_type; }
    uint32_t GetKickReason() { return m_reason; }
private:
    uint32_t  m_user_id;
    uint32_t  m_client_type;
    uint32_t  m_reason;
};

class CImPduUserClientTypeRequest : public CImPdu
{
public:
    CImPduUserClientTypeRequest(uchar_t* buf, uint32_t len);
    CImPduUserClientTypeRequest(uint32_t user_id, uint32_t attach_len, uchar_t* attach_data);
    virtual ~CImPduUserClientTypeRequest() {}
  
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_USER_CLIENT_TYPE_REQUEST; }
    
    uint32_t GetUserId() { return m_user_id; }
    uint32_t GetAttachLen() { return m_attach_len; }
    uchar_t* GetAttachData() { return m_attach_data; }
private:
    uint32_t m_user_id;
    uint32_t m_attach_len;
    uchar_t* m_attach_data;
};

class CImPduUserClientTypeResponse : public CImPdu
{
public:
    CImPduUserClientTypeResponse(uchar_t* buf, uint32_t len);
    CImPduUserClientTypeResponse(uint32_t user_id, uint32_t client_type_flag, uint32_t attach_len,
                                 uchar_t* attach_data);
    virtual ~CImPduUserClientTypeResponse() {}
    
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_USER_CLIENT_TYPE_RESPONSE; }
  
    uint32_t GetUserId() { return m_user_id; }
    uint32_t GetClientTypeFlag() { return m_client_type_flag; }
    uint32_t GetAttachLen() { return m_attach_len; }
    uchar_t* GetAttachData() { return m_attach_data; }
private:
    uint32_t m_user_id;
    uint32_t m_client_type_flag;
    uint32_t m_attach_len;
    uchar_t* m_attach_data;
};

#endif /* IMPDUSERVER_H_ */
