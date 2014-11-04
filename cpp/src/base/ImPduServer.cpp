/*
 * ImPduServer.cpp
 * Interactive packet between servers
 *
 *  Created on: 2013-8-27
 *      Author: ziteng@mogujie.com
 */

#include "ImPduServer.h"

////// Business Server Packet //////
CImPduStopReceivePacket::CImPduStopReceivePacket(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_code;
	PARSE_PACKET_ASSERT
}

CImPduValidateRequest::CImPduValidateRequest(const char* user_name, const char* password, uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_VALIDATE_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(user_name);
	os.WriteString(password);
	os.WriteData(attach_data, attach_len);
	WriteHeader();
}

//////
CImPduValidateResponse::CImPduValidateResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
	memset(&m_user, 0, sizeof(m_user));

    m_user_name = is.ReadString(m_user_name_len);
	is >> m_result;
	if (m_result == 0) {
        is >> m_user.user_id;
		m_user.nick_name = is.ReadString(m_user.nick_name_len);
		m_user.avatar_url = is.ReadString(m_user.avatar_len);
        m_user.title = is.ReadString(m_user.title_len);
        m_user.position = is.ReadString(m_user.position_len);
        is >> m_user.role_status;
        is >> m_user.sex;
        is >> m_user.depart_id;
        is >> m_user.job_num;
        m_user.telphone = is.ReadString(m_user.telphone_len);
        m_user.email = is.ReadString(m_user.email_len);
	}

	m_attach_data = is.ReadData(m_attach_len);
	PARSE_PACKET_ASSERT
}

CImPduValidateResponse::CImPduValidateResponse(const char* user_name, uint32_t result, const user_info_t* user, uint32_t attach_len, uchar_t* attach_data , uint32_t reserved)
{
    
    m_pdu_header.reserved = reserved;
    m_user_name = (char*)user_name;
    m_user_name_len = strlen(user_name);
    m_result = result = 0;
    m_user = *user;
    m_attach_len = attach_len;
    m_attach_data = attach_data;
    /*
    m_pdu_header.command_id = IM_PDU_TYPE_VALIDATE_RESPONSE;
    CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
    m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    os.WriteString(user_name);
	os << result;
    if (user != NULL)
    {
        os << user->user_id;
        os.WriteString(user->nick_name, user->nick_name_len);
        os.WriteString(user->avatar_url, user->avatar_len);
        os.WriteString(user->title, user->title_len);
        os.WriteString(user->position, user->position_len);
        os << user->role_status;
        os << user->sex;
        os << user->depart_id;
        os << user->job_num;
        os.WriteString(user->telphone, user->telphone_len);
        os.WriteString(user->email, user->email_len);
        os << user->create_time;
    }
    
	os.WriteData(attach_data, attach_len);
	WriteHeader();
    */
}

CImPduFriendListRequest::CImPduFriendListRequest(uint32_t user_id, uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_FRIEND_LIST_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << user_id;
	os.WriteData(attach_data, attach_len);
	WriteHeader();
}

//////
CImPduFriendListResponse::CImPduFriendListResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_friend_list = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_from_user_id;
	is >> m_friend_cnt;

	if (m_friend_cnt > 0) {
		m_friend_list = new user_info_t[m_friend_cnt]();
		ALLOC_FAIL_ASSERT(m_friend_list)
        
		for (uint32_t i = 0; i < m_friend_cnt; i++) {
			is >> m_friend_list[i].user_id;
			is >> m_friend_list[i].user_updated;
		}
	}
	m_attach_data = is.ReadData(m_attach_len);

	PARSE_PACKET_ASSERT
}

CImPduFriendListResponse::~CImPduFriendListResponse()
{
	if (m_friend_list) {
		delete [] m_friend_list;
		m_friend_list = NULL;
	}
}

CImPduDepartmentRequest::CImPduDepartmentRequest(uint32_t user_id, uint32_t attach_len, uchar_t* attach_data)
{
    m_pdu_header.command_id = IM_PDU_TYPE_DEPARTMENT_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
	os << user_id;
	os.WriteData(attach_data, attach_len);
	WriteHeader();
}

CImPduDepartmentResponse::CImPduDepartmentResponse(uchar_t* buf, uint32_t len)
{
    ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    m_depart_list = NULL;
    is >> m_req_user_id;
    is >> m_depart_cnt;
    if (m_depart_cnt > 0) {
        m_depart_list = new department_info_t[m_depart_cnt];
        ALLOC_FAIL_ASSERT(m_depart_list)
        
        for (uint32_t i = 0; i < m_depart_cnt; i++) {
            is >> m_depart_list[i].depart_id;
            m_depart_list[i].title = is.ReadString(m_depart_list[i].title_len);
            m_depart_list[i].description = is.ReadString(m_depart_list[i].description_len);
            is >> m_depart_list[i].parent_depart_id;
            is >> m_depart_list[i].leader;
            is >> m_depart_list[i].status;
        }
    }
	
	m_attach_data = is.ReadData(m_attach_len);
	PARSE_PACKET_ASSERT
}

CImPduDepartmentResponse::~CImPduDepartmentResponse()
{
    if (m_depart_list != NULL) {
        delete []m_depart_list;
        m_depart_list = NULL;
    }
}

CImPduRemoveSessionRequest::CImPduRemoveSessionRequest(uint32_t req_user_id, uint32_t session_id,
		uint32_t session_type, uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_REMOVE_SESSION_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << req_user_id;
	os << session_id;
	os << session_type;
	os.WriteData(attach_data, attach_len);
	WriteHeader();
}

CImPduRemoveSessionResponse::CImPduRemoveSessionResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_req_user_id;
	is >> m_result;
	is >> m_session_id;
	is >> m_session_type;
	m_attach_data = is.ReadData(m_attach_len);
	PARSE_PACKET_ASSERT
}

CImPduUsersInfoRequest::CImPduUsersInfoRequest(uint32_t from_user_id,uint32_t user_cnt, uint32_t* user_id_list
                                               ,uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_USERS_INFO_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << from_user_id;
	os << user_cnt;
	for (uint32_t i = 0; i < user_cnt; i++) {
		os << user_id_list[i];
	}
	os.WriteData(attach_data, attach_len);

	WriteHeader();
}

CImPduUsersInfoResponse::CImPduUsersInfoResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_user_info_list = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_from_user_id;
	is >> m_user_cnt;
	if (m_user_cnt > 0) {
		m_user_info_list = (user_info_t*) malloc( m_user_cnt * sizeof(user_info_t) );
		ALLOC_FAIL_ASSERT(m_user_info_list)

		for (uint32_t i = 0; i < m_user_cnt; i++) {
			is >> m_user_info_list[i].user_id;
			m_user_info_list[i].name = is.ReadString(m_user_info_list[i].name_len);
			m_user_info_list[i].nick_name = is.ReadString(m_user_info_list[i].nick_name_len);
			m_user_info_list[i].avatar_url = is.ReadString(m_user_info_list[i].avatar_len);
            m_user_info_list[i].title = is.ReadString(m_user_info_list[i].title_len);
            m_user_info_list[i].position = is.ReadString(m_user_info_list[i].position_len);
            is >> m_user_info_list[i].role_status;
            is >> m_user_info_list[i].sex;
            is >> m_user_info_list[i].depart_id;
            is >> m_user_info_list[i].job_num;
            m_user_info_list[i].telphone = is.ReadString(m_user_info_list[i].telphone_len);
            m_user_info_list[i].email = is.ReadString(m_user_info_list[i].email_len);
        }
	}
	m_attach_data = is.ReadData(m_attach_len);

	PARSE_PACKET_ASSERT
}

CImPduUsersInfoResponse::~CImPduUsersInfoResponse()
{
	if (m_user_info_list) {
		free(m_user_info_list);
		m_user_info_list = NULL;
	}
}

CImPduAllUserRequest::CImPduAllUserRequest(uint32_t req_user_id, uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_ALL_USER_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << req_user_id;
	os.WriteData(attach_data, attach_len);
	WriteHeader();
}


CImPduDBQueryRequest::CImPduDBQueryRequest(uint32_t seq_no, uint32_t user_id, uint32_t query_len, uchar_t* query_data,
		uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_DB_QUERY_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << seq_no;
	os << user_id;
	os.WriteData(query_data, query_len);
	WriteHeader();
}

CImPduDBQueryResponse::CImPduDBQueryResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_seq_no;
	is >> m_user_id;
	m_result_data = is.ReadData(m_result_len);
	m_attach_data = is.ReadData(m_attach_len);
	PARSE_PACKET_ASSERT
}

CImPduMsgData::CImPduMsgData(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_msg_data = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_request_id;
	is >> m_from_user_id;
	is >> m_to_user_id;
	is >> m_create_time;
	is >> m_msg_type;
	m_msg_data = is.ReadData(m_msg_len);
    is >> m_client_type;
	m_attach_data = is.ReadString(m_attach_len);

	PARSE_PACKET_ASSERT
}


CImPduMsgData::CImPduMsgData(uint32_t request_id,  uint32_t from_user_id, uint32_t to_user_id,
		uint32_t create_time, uint8_t msg_type, uint32_t msg_len, uchar_t* msg_data,
		uint32_t client_type, uint32_t attach_len, char* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_MSG_DATA;
	m_msg_data = NULL;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << request_id;
	os << from_user_id;
	os << to_user_id;
	os << create_time;
	os << msg_type;
	os.WriteData(msg_data, msg_len);
    os << client_type;
	os.WriteString(attach_data, attach_len);

	WriteHeader();
}

CImPduUnreadMsgCountRequest::CImPduUnreadMsgCountRequest(uint32_t user_id, uint32_t client_type, uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_UNREAD_MSG_COUNT_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << user_id;
    os << client_type;
	os.WriteData(attach_data, attach_len);
	WriteHeader();
}


CImPduUnreadMsgCountResponse::CImPduUnreadMsgCountResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_user_unread_list = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_to_user_id;
	is >> m_user_unread_cnt;

	if (m_user_unread_cnt > 0) {
		m_user_unread_list = (UserUnreadMsgCnt_t*) malloc(m_user_unread_cnt * sizeof(UserUnreadMsgCnt_t));
		ALLOC_FAIL_ASSERT(m_user_unread_list)

		for (uint32_t i = 0; i < m_user_unread_cnt; ++i) {
			is >> m_user_unread_list[i].from_user_id;
			is >> m_user_unread_list[i].unread_msg_cnt;
		}
	}
	m_attach_data = is.ReadData(m_attach_len);

	PARSE_PACKET_ASSERT
}

CImPduUnreadMsgCountResponse::~CImPduUnreadMsgCountResponse()
{
	if (m_user_unread_list) {
		free(m_user_unread_list);
		m_user_unread_list = NULL;
	}
}

CImPduUnreadMsgRequest::CImPduUnreadMsgRequest(uint32_t from_user_id, uint32_t to_user_id,
                uint32_t client_type, uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_UNREAD_MSG_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << from_user_id;
	os << to_user_id;
    os << client_type;
	os.WriteData(attach_data, attach_len);
	WriteHeader();
}

CImPduMsgListResponse::CImPduMsgListResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_msg_list = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_request_cmd_id;
	is >> m_from_user_id;
	is >> m_to_user_id;
	is >> m_msg_count;
	if (m_msg_count > 0) {
		m_msg_list = (server_msg_t*) malloc(m_msg_count * sizeof(server_msg_t));
		ALLOC_FAIL_ASSERT(m_msg_list)

		for (uint32_t i = 0; i < m_msg_count; i++) {
			is >> m_msg_list[i].from_user_id;
			m_msg_list[i].from_name = is.ReadString(m_msg_list[i].from_name_len);
			m_msg_list[i].from_nick_name = is.ReadString(m_msg_list[i].from_nick_name_len);
			m_msg_list[i].from_avatar_url = is.ReadString(m_msg_list[i].from_avatar_len);

			is >> m_msg_list[i].create_time;
			is >> m_msg_list[i].msg_type;
			m_msg_list[i].msg_data = is.ReadData(m_msg_list[i].msg_len);
		}
	}
	m_attach_data = is.ReadData(m_attach_len);

	PARSE_PACKET_ASSERT
}

CImPduMsgListResponse::~CImPduMsgListResponse()
{
	if (m_msg_list) {
		free(m_msg_list);
		m_msg_list = NULL;
	}
}

CImPduMsgReadAck::CImPduMsgReadAck(uint32_t request_id, uint32_t from_user_id, uint32_t to_user_id, uint32_t client_type)
{
	m_pdu_header.command_id = IM_PDU_TYPE_MSG_READ_ACK;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << request_id;
	os << from_user_id;
	os << to_user_id;
    os << client_type;
	WriteHeader();
}

CImPduDBWriteResponse::CImPduDBWriteResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_request_id;
	is >> m_result;
	is >> m_request_pdu_type;
	is >> m_from_id;
	is >> m_to_id;
	PARSE_PACKET_ASSERT
}

CImPduDBWriteResponse::CImPduDBWriteResponse(uint32_t request_id, uint32_t result, uint16_t request_type, uint32_t from_id, uint32_t to_id)
{
	m_pdu_header.command_id = IM_PDU_TYPE_DB_WRITE_RESPONSE;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << request_id;
	os << result;
	os << request_type;
	os << from_id;
	os << to_id;
	WriteHeader();
}

CImPduUserActionLog::CImPduUserActionLog(uint32_t source, uint32_t protocol, const char* ip, uint32_t user_id,
		uint32_t action_type, const char* os_type, const char* user_agent, const char* flash_ver, const char* client_ver)
{
	m_pdu_header.command_id = IM_PDU_TYPE_USER_ACTION_LOG;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << source;
	os << protocol;
	os.WriteString(ip);
	os << user_id;
	os << action_type;
	os.WriteString(os_type);
	os.WriteString(user_agent);
	os.WriteString(flash_ver);
	os.WriteString(client_ver);
	WriteHeader();
}

////////////////// Message Server Internal Packet///////////////
CImPduRoleSet::CImPduRoleSet(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_master;
	PARSE_PACKET_ASSERT
}

CImPduRoleSet::CImPduRoleSet(uint32_t master)
{
	m_pdu_header.command_id = IM_PDU_TYPE_ROLE_SET;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << master;
	WriteHeader();
}

CImPduOnlineUserInfo::CImPduOnlineUserInfo(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_user_status_list = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_user_count;
	if (m_user_count > 0) {
		m_user_status_list = new user_conn_stat_t [ m_user_count];
		ALLOC_FAIL_ASSERT(m_user_status_list)

		for (uint32_t i = 0; i < m_user_count; i++) {
			is >> m_user_status_list[i].user_id;
			is >> m_user_status_list[i].status;
           is >> m_user_status_list[i].client_type_flag;
		}
	}

	PARSE_PACKET_ASSERT
}

CImPduOnlineUserInfo::CImPduOnlineUserInfo(list<user_conn_stat_t>* user_status_list)
{
	m_pdu_header.command_id = IM_PDU_TYPE_ONLINE_USER_INFO;
	m_user_status_list = NULL;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << (uint32_t)user_status_list->size();
	for (list<user_conn_stat_t>::iterator it = user_status_list->begin(); it != user_status_list->end();
         it++) {
        os << it->user_id;
        os << it->status;
        os << it->client_type_flag;
	}

	WriteHeader();
}

CImPduOnlineUserInfo::~CImPduOnlineUserInfo()
{
	if (m_user_status_list) {
		free(m_user_status_list);
		m_user_status_list = NULL;
	}
}

CImPduMsgServInfo::CImPduMsgServInfo(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_ip1_addr = is.ReadString(m_ip1_len);
	m_ip2_addr = is.ReadString(m_ip2_len);
	is >> m_port;
	is >> m_max_conn_cnt;
	is >> m_cur_conn_cnt;
	m_hostname = is.ReadString(m_hostname_len);
	is >> m_server_type;

	PARSE_PACKET_ASSERT
}

CImPduMsgServInfo::CImPduMsgServInfo(const char* ip_addr1, const char* ip_addr2, uint16_t port, uint32_t max_conn_cnt,
		uint32_t cur_conn_cnt, const char* hostname, uint32_t server_type)
{
	m_pdu_header.command_id = IM_PDU_TYPE_MSG_SERV_INFO;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(ip_addr1);
	os.WriteString(ip_addr2);
	os << port;
	os << max_conn_cnt;
	os << cur_conn_cnt;
	os.WriteString(hostname);
	os << server_type;

	WriteHeader();
}


CImPduUserStatusUpdate::CImPduUserStatusUpdate(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_user_status;
	is >> m_user_id;
    is >> m_client_type_flag;
	PARSE_PACKET_ASSERT
}

CImPduUserStatusUpdate::CImPduUserStatusUpdate(uint32_t user_status, uint32_t user_id,
                                               uint32_t client_type_flag)
{
	m_pdu_header.command_id = IM_PDU_TYPE_USER_STATUS_UPDATE;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << user_status;
	os << user_id;
    os << client_type_flag;
	WriteHeader();
}

CImPduUserConnInfo::CImPduUserConnInfo(uchar_t* buf, uint32_t len) {
	m_user_conn_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    
	is >> m_user_cnt;
	if (m_user_cnt > 0) {
		m_user_conn_list = new user_conn_t[m_user_cnt];
		ALLOC_FAIL_ASSERT(m_user_conn_list);
		for (uint32_t i = 0; i < m_user_cnt; i++) {
			is >> m_user_conn_list[i].user_id;
			is >> m_user_conn_list[i].conn_cnt;
		}
	}
    
	PARSE_PACKET_ASSERT
}

CImPduUserConnInfo::CImPduUserConnInfo(list<user_conn_t>* user_conn_list) {
	m_user_conn_list = NULL;
	m_pdu_header.command_id = IM_PDU_TYPE_USER_CONN_INFO;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
    if (user_conn_list != NULL) {
        os << (uint32_t) user_conn_list->size();
        for (list<user_conn_t>::iterator it = user_conn_list->begin(); it != user_conn_list->end();
             it++) {
            os << it->user_id;
            os << it->conn_cnt;
        }
    }
    else
    {
        os << 0;
    }
    
	WriteHeader();
}

CImPduUserConnInfo::~CImPduUserConnInfo() {
	if (m_user_conn_list) {
		delete[] m_user_conn_list;
		m_user_conn_list = NULL;
	}
}


CImPduUserCntUpdate::CImPduUserCntUpdate(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_user_action;
    is >> m_user_id;
	PARSE_PACKET_ASSERT
}

CImPduUserCntUpdate::CImPduUserCntUpdate(uint32_t action, uint32_t user_id)
{
	m_pdu_header.command_id = IM_PDU_TYPE_USER_CNT_UPDATE;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << action;
    os << user_id;
	WriteHeader();
}

CImPduFriendStatusQuery::CImPduFriendStatusQuery(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_friend_id_list = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_request_type;
	is >> m_from_user_id;
	is >> m_friend_cnt;

	if (m_friend_cnt > 0) {
		m_friend_id_list = new uint32_t [m_friend_cnt];
		ALLOC_FAIL_ASSERT(m_friend_id_list)

		for (uint32_t i = 0; i < m_friend_cnt; i++) {
			is >> m_friend_id_list[i];
		}
	}

	PARSE_PACKET_ASSERT
}

CImPduFriendStatusQuery::CImPduFriendStatusQuery(uint16_t request_type, uint32_t from_user_id, list<uint32_t>* friend_id_list)
{
	m_pdu_header.command_id = IM_PDU_TYPE_FRIEND_STATUS_QUERY;
	m_friend_id_list = NULL;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << request_type;
	os << from_user_id;
	os << (uint32_t)friend_id_list->size();
	for (list<uint32_t>::iterator it = friend_id_list->begin(); it != friend_id_list->end(); it++) {
		os << *it;
	}

	WriteHeader();
}

CImPduFriendStatusQuery::~CImPduFriendStatusQuery()
{
	if (m_friend_id_list) {
		delete [] m_friend_id_list;
		m_friend_id_list = NULL;
	}
}

CImPduFriendStatusList::CImPduFriendStatusList(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_friend_stat_list = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_request_type;
	is >> m_from_user_id;
	is >> m_friend_cnt;

	if (m_friend_cnt > 0) {
		m_friend_stat_list =  new user_stat_t [m_friend_cnt];
		ALLOC_FAIL_ASSERT(m_friend_stat_list)

		for (uint32_t i = 0; i < m_friend_cnt; i++) {
			is >> m_friend_stat_list[i].user_id;
			is >> m_friend_stat_list[i].status;
		}
	}

	PARSE_PACKET_ASSERT
}

CImPduFriendStatusList::CImPduFriendStatusList(uint16_t request_type, uint32_t from_user_id,
		list<user_stat_t>* friend_stat_list)
{
	m_pdu_header.command_id = IM_PDU_TYPE_FRIEND_STATUS_LIST;
	m_friend_stat_list = NULL;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << request_type;
	os << from_user_id;
	os << (uint32_t)friend_stat_list->size();
	for (list<user_stat_t>::iterator it = friend_stat_list->begin(); it != friend_stat_list->end(); it++) {
		os << it->user_id;
		os << it->status;
	}

	WriteHeader();
}

CImPduFriendStatusList::~CImPduFriendStatusList()
{
	if (m_friend_stat_list) {
		delete [] m_friend_stat_list;
		m_friend_stat_list = NULL;
	}
}

CImPduFriendStatusNotify::CImPduFriendStatusNotify(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_friend_id_list = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_from_user_id;
	is >> m_from_user_status;
	is >> m_friend_cnt;
	if (m_friend_cnt > 0) {
		m_friend_id_list = new uint32_t [m_friend_cnt];
		ALLOC_FAIL_ASSERT(m_friend_id_list)

		for (uint32_t i = 0; i < m_friend_cnt; i++) {
			is >> m_friend_id_list[i];
		}
	}

	PARSE_PACKET_ASSERT
}

CImPduFriendStatusNotify::CImPduFriendStatusNotify(uint32_t from_user_id, uint32_t from_user_status, list<uint32_t>* friend_id_list)
{
	m_pdu_header.command_id = IM_PDU_TYPE_FRIEND_STATUS_NOTIFY;
	m_friend_id_list = NULL;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << from_user_id;
	os << from_user_status;
	os << (uint32_t)friend_id_list->size();
	for (list<uint32_t>::iterator it = friend_id_list->begin(); it != friend_id_list->end(); it++) {
		os << *it;
	}

	WriteHeader();
}

CImPduFriendStatusNotify::~CImPduFriendStatusNotify()
{
	if (m_friend_id_list) {
		delete [] m_friend_id_list;
		m_friend_id_list = NULL;
	}
}


CImPduP2PMsg::CImPduP2PMsg(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_msg_data = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_seq_no;
	is >> m_from_user_id;
	is >> m_to_user_id;
	m_msg_data = is.ReadData(m_msg_len);

	PARSE_PACKET_ASSERT
}

CImPduP2PMsg::CImPduP2PMsg(uint32_t seq_no, uint32_t from_user_id, uint32_t to_user_id, uint32_t msg_len, uchar_t* msg_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_P2P_MSG;
	m_msg_data = NULL;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << seq_no;
	os << from_user_id;
	os << to_user_id;
	os.WriteData(msg_data, msg_len);
	WriteHeader();
}

CImPduGroupP2PMessageRequest::CImPduGroupP2PMessageRequest(uint32_t from_user_id, uint32_t flag, list<string>& to_user_name_list, uint32_t create_time, uint32_t msg_len, uchar_t* msg_data)
{
    m_pdu_header.command_id = IM_PDU_TYPE_GROUP_P2P_MSG_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
	os << from_user_id;
    os << flag;
    uint32_t toUserCount = to_user_name_list.size();
    os << toUserCount;
    
    list<string>::iterator itor = to_user_name_list.begin();
    for (; itor != to_user_name_list.end(); ++itor) {
        os.WriteString((*itor).c_str());
    }
	os << create_time;
	os.WriteData(msg_data, msg_len);
	WriteHeader();
}

CImPduGroupP2PMessageResponse::CImPduGroupP2PMessageResponse(uchar_t* buf, uint32_t len)
{
  	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    
	is >> m_req_user_id;
    is >> m_flag;
	is >> m_result;
    is >> m_create_time;
    
    m_msg_data = is.ReadData(m_msg_len);
    
    is >> m_user_cnt;
    
    if(m_result == 0 && m_user_cnt != 0) {
		m_user_list = new uint32_t [m_user_cnt];
		ALLOC_FAIL_ASSERT(m_user_list);
		for (uint32_t i = 0; i < m_user_cnt; i++) {
			is >> m_user_list[i];
		}
    } else {
        m_user_cnt = 0;
        m_user_list = NULL;
    }

	PARSE_PACKET_ASSERT   
}

CImPduGroupP2PMessageResponse::~CImPduGroupP2PMessageResponse()
{
    if (m_user_list) {
		delete [] m_user_list;
		m_user_list = NULL;
	}

}

CImPduUserStatusRequest::CImPduUserStatusRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_request_id;
	is >> m_query_id;
	m_attach_data = is.ReadData(m_attach_len);
	PARSE_PACKET_ASSERT
}

CImPduUserStatusRequest::CImPduUserStatusRequest(uint32_t request_id, uint32_t query_id,
		uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_USER_STATUS_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << request_id;
	os << query_id;
	os.WriteData(attach_data, attach_len);
	WriteHeader();
}

CImPduUsersStatusRequest::CImPduUsersStatusRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_request_id;
	is >> m_query_count;
	uint32_t user_id;
	for(uint32_t i=0; i<m_query_count; ++i)
	{
		is >> user_id;
		m_query_list.push_back(user_id);
	}
    m_attach_data = is.ReadData(m_attach_len);
	PARSE_PACKET_ASSERT
}

CImPduUsersStatusRequest::CImPduUsersStatusRequest(uint32_t request_id, list<uint32_t>& query_list,
                                                   uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_USERS_STATUS_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
	uint32_t users_count = query_list.size();
	os << request_id;
	os << users_count;
	for(list<uint32_t>::iterator it=query_list.begin(); it!=query_list.end(); ++it)
	{
		os << *it;
	}
    os.WriteData(attach_data, attach_len);
	WriteHeader();
}

CImPduUserStatusResponse::CImPduUserStatusResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_request_id;
	is >> m_query_id;
	is >> m_status;
	m_attach_data = is.ReadData(m_attach_len);
	PARSE_PACKET_ASSERT
}

CImPduUserStatusResponse::CImPduUserStatusResponse(uint32_t request_id, uint32_t query_id, uint32_t status,
		uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_USER_STATUS_RESPONSE;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << request_id;
	os << query_id;
	os << status;
	os.WriteData(attach_data, attach_len);
	WriteHeader();
}

CImPduUsersStatusResponse::CImPduUsersStatusResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_request_id;
	is >> m_result_count;
	user_stat_t status;
	for(uint32_t i=0; i<m_result_count; ++i)
	{
		is >> (status.user_id);
		is >> (status.status);
		m_result_list.push_back(status);
	}
    m_attach_data = is.ReadData(m_attach_len);
	PARSE_PACKET_ASSERT
}

CImPduUsersStatusResponse::CImPduUsersStatusResponse(uint32_t request_id,
            const list<user_stat_t>& result_list, uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_USERS_STATUS_RESPONSE;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
	uint32_t result_count = result_list.size();
	os << request_id;
	os << result_count;
	for(list<user_stat_t>::const_iterator it=result_list.begin(); it!=result_list.end(); ++it)
	{
		os << (it->user_id);
		os << (it->status);
	}
    os.WriteData(attach_data, attach_len);
	WriteHeader();
}

CImPduServerKickUser::CImPduServerKickUser(uchar_t* buf,uint32_t len)
{
    ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    
	is >> m_user_id;
	is >> m_client_type;
    is >> m_reason;

	PARSE_PACKET_ASSERT
}

CImPduServerKickUser::CImPduServerKickUser(uint32_t user_id, uint32_t client_type, uint32_t reason)
{
    m_pdu_header.command_id = IM_PDU_TYPE_SERVER_KICK_USER;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
	
    os << user_id;
    os << client_type;
    os << reason;
    
	WriteHeader();
}

CImPduUserClientTypeRequest::CImPduUserClientTypeRequest(uchar_t* buf, uint32_t len)
{
    ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    
	is >> m_user_id;
	m_attach_data = is.ReadData(m_attach_len);
    
	PARSE_PACKET_ASSERT
}

CImPduUserClientTypeRequest::CImPduUserClientTypeRequest(uint32_t user_id, uint32_t attach_len,
                                                         uchar_t* attach_data)
{
    m_pdu_header.module_id = SID_OTHER;
    m_pdu_header.command_id = IM_PDU_TYPE_USER_CLIENT_TYPE_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
	
    os << user_id;
    os.WriteData(attach_data, attach_len);
    
	WriteHeader();
}

CImPduUserClientTypeResponse::CImPduUserClientTypeResponse(uchar_t* buf, uint32_t len)
{
    ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    
	is >> m_user_id;
    is >> m_client_type_flag;
	m_attach_data = is.ReadData(m_attach_len);
    
	PARSE_PACKET_ASSERT
}

CImPduUserClientTypeResponse::CImPduUserClientTypeResponse(uint32_t user_id,
                    uint32_t client_type_flag, uint32_t attach_len, uchar_t* attach_data)
{
    m_pdu_header.command_id = IM_PDU_TYPE_USER_CLIENT_TYPE_RESPONSE;
    m_pdu_header.module_id = SID_OTHER;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
	
    os << user_id;
    os << m_client_type_flag;
    os.WriteData(attach_data, attach_len);
    
	WriteHeader();
}

