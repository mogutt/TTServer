/*
 * ImPduClient.cpp
 * Interactive Packet with client
 *
 *  Created on: 2013-8-27
 *      Author: ziteng@mogujie.com
 */

#include "ImPduClient.h"
#include <stdlib.h>
#include <ctime>

CImPduHeartbeat::CImPduHeartbeat()
{
	m_pdu_header.command_id = IM_PDU_TYPE_HEARTBEAT;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	WriteHeader();
}

CImPduMsgServRequest::CImPduMsgServRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	PARSE_PACKET_ASSERT
}

CImPduMsgServRequest::CImPduMsgServRequest()
{
	m_pdu_header.module_id = SID_LOGIN;
	m_pdu_header.command_id = CID_LOGIN_REQ_MSGSERVER;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
	WriteHeader();
}

/////////////////////// 锟斤拷陆锟斤拷锟� ////////////////////
CImPduMsgServResponse::CImPduMsgServResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_result;
	if (!m_result) {
		m_ip1_addr = is.ReadString(m_ip1_len);
		m_ip2_addr = is.ReadString(m_ip2_len);
		is >> m_port;
	}

	PARSE_PACKET_ASSERT
}

CImPduMsgServResponse::CImPduMsgServResponse(uint32_t result, const char* ip1_addr, const char* ip2_addr, uint16_t port)
{
	m_pdu_header.module_id = SID_LOGIN;
	m_pdu_header.command_id = CID_LOGIN_RES_MSGSERVER;
	//m_domain_name = NULL;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << result;
	if (!result) {
		os.WriteString(ip1_addr);
		os.WriteString(ip2_addr);
		os << port;
	}
	WriteHeader();
}

CImPduLoginRequest::CImPduLoginRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_user_name = NULL;
	m_password = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_user_name = is.ReadString(m_user_name_len);
	m_password = is.ReadString(m_password_len);
	is >> m_online_status;
    is >> m_client_type;
    m_client_version = is.ReadString(m_client_version_len);

	PARSE_PACKET_ASSERT
}

CImPduLoginRequest::CImPduLoginRequest(const char* user_name, const char* password, uint32_t online_status,
        uint32_t client_type, const char* client_version)
{
	m_pdu_header.module_id = SID_LOGIN;
	m_pdu_header.command_id = CID_LOGIN_REQ_USERLOGIN;
	m_user_name = NULL;
	m_password = NULL;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(user_name);
	os.WriteString(password);
	os << online_status;
    os << client_type;
	os.WriteString(client_version);

	WriteHeader();
}

CImPduLoginResponse::CImPduLoginResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_server_time;
	is >> m_result;
	if (m_result == 0) {
		is >> m_online_status;
		m_user_id_url = is.ReadString(m_user_id_url_len);
		m_nickname = is.ReadString(m_nickname_len);
		m_avatar_url = is.ReadString(m_avatar_len);
        m_title = is.ReadString(m_title_len);
        m_position = is.ReadString(m_position_len);
        is >> m_role_status;
        is >> m_sex;
        m_depart_id_url = is.ReadString(m_depart_id_url_len);
        is >> m_job_num;
        m_telphone = is.ReadString(m_telphone_len);
        m_email = is.ReadString(m_email_len);
        m_token = is.ReadString(m_token_len);
	}
	PARSE_PACKET_ASSERT
}

CImPduLoginResponse::CImPduLoginResponse(uint32_t result, uint32_t online_status, user_info_t* user, char* token)
{
	m_pdu_header.module_id = SID_LOGIN;
	m_pdu_header.command_id = CID_LOGIN_RES_USERLOGIN;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << (uint32_t)time(NULL);
	os << result;
	if (result == 0) {
		os << online_status;
		os.WriteString(idtourl(user->user_id));
		os.WriteString(user->nick_name, user->nick_name_len);
		os.WriteString(user->avatar_url, user->avatar_len);
        os.WriteString(user->title, user->title_len);
        os.WriteString(user->position, user->position_len);
        os << user->role_status;
        os << user->sex;
        os.WriteString(idtourl(user->depart_id));
        os << user->job_num;
        os.WriteString(user->telphone, user->telphone_len);
        os.WriteString(user->email, user->email_len);
        os.WriteString(token);
	}

	WriteHeader();
}

CImPduKickUser::CImPduKickUser(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_user_id;
	is >> m_reason;

	PARSE_PACKET_ASSERT
}

CImPduKickUser::CImPduKickUser(uint32_t user_id, uint32_t reason)
{
	m_pdu_header.module_id = SID_LOGIN;
	m_pdu_header.command_id = CID_LOGIN_KICK_USER;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << user_id;
	os << reason;
	WriteHeader();
}

/////////////////////// 锟斤拷锟斤拷锟叫憋拷锟斤拷锟� ////////////////////
CImPduClientBuddyListRequest::CImPduClientBuddyListRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
	is >> m_req_type;
	PARSE_PACKET_ASSERT
}

CImPduClientBuddyListRequest::CImPduClientBuddyListRequest(uint32_t req_type)
{
	m_pdu_header.module_id = SID_BUDDY_LIST;
	m_pdu_header.command_id = CID_BUDDY_LIST_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << req_type;
	WriteHeader();
}

CImPduClientFriendList::CImPduClientFriendList(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_friend_list = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_friend_cnt;

	if (m_friend_cnt > 0) {
		m_friend_list = new client_user_info_t[m_friend_cnt]();
		ALLOC_FAIL_ASSERT(m_friend_list)

		for (uint32_t i = 0; i < m_friend_cnt; i++) {
			m_friend_list[i].id_url = is.ReadString(m_friend_list[i].id_len);
            is >> m_friend_list[i].user_updated;
		}
	}

	PARSE_PACKET_ASSERT
}

CImPduClientFriendList::CImPduClientFriendList(uint32_t friend_cnt, user_info_t* friend_list)
{
	m_pdu_header.module_id = SID_BUDDY_LIST;
	m_pdu_header.command_id = CID_BUDDY_LIST_FRIEND_LIST;
	m_friend_list = NULL;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	char* id_url = NULL;
	os << friend_cnt;
	for (uint32_t i = 0; i < friend_cnt; i++) {
		id_url = idtourl(friend_list[i].user_id);
		os.WriteString(id_url);
        os << friend_list[i].user_updated;
	}

	WriteHeader();
}

CImPduClientFriendList::~CImPduClientFriendList()
{
	if (m_friend_list) {
		delete [] m_friend_list;
		m_friend_list = NULL;
	}
}

CImPduClientOnlineFriendList::CImPduClientOnlineFriendList(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_friend_stat_list = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_list_type;
	is >> m_friend_cnt;
	if (m_friend_cnt > 0) {
		m_friend_stat_list = new client_stat_t [m_friend_cnt];
		ALLOC_FAIL_ASSERT(m_friend_stat_list)

		for (uint32_t i = 0; i < m_friend_cnt; i++) {
			m_friend_stat_list[i].id_url = is.ReadString(m_friend_stat_list[i].id_len);
			is >> m_friend_stat_list[i].status;
		}
	}

	PARSE_PACKET_ASSERT
}

CImPduClientOnlineFriendList::CImPduClientOnlineFriendList(uint16_t list_type, uint32_t friend_cnt, user_stat_t* friend_stat_list)
{
	m_friend_stat_list = NULL;
	m_pdu_header.module_id = SID_BUDDY_LIST;
	m_pdu_header.command_id = CID_BUDDY_LIST_ONLINE_FRIEND_LIST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	char* id_url = NULL;
	os << list_type;
	os << friend_cnt;
	for (uint32_t i = 0; i < friend_cnt; i++) {
		id_url = idtourl(friend_stat_list[i].user_id);
		os.WriteString(id_url);
		os << friend_stat_list[i].status;
	}
	WriteHeader();
}

CImPduClientOnlineFriendList::~CImPduClientOnlineFriendList()
{
	if (m_friend_stat_list) {
		delete [] m_friend_stat_list;
		m_friend_stat_list = NULL;
	}
}

CImPduClientFriendNotify::CImPduClientFriendNotify(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_id_url = is.ReadString(m_id_len);
	is >> m_status;
	PARSE_PACKET_ASSERT
}

CImPduClientFriendNotify::CImPduClientFriendNotify(const char* id_url, uint32_t status)
{
	m_id_url = NULL;
	m_pdu_header.module_id = SID_BUDDY_LIST;
	m_pdu_header.command_id = CID_BUDDY_LIST_STATUS_NOTIFY;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(id_url);
	os << status;
	WriteHeader();
}

CImPduClientUserStatusRequest::CImPduClientUserStatusRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_user_id = is.ReadString(m_user_id_len);
	PARSE_PACKET_ASSERT
}

CImPduClientUserStatusRequest::CImPduClientUserStatusRequest(const char* user_id)
{
	m_pdu_header.module_id = SID_BUDDY_LIST;
	m_pdu_header.command_id = CID_BUDDY_LIST_USER_STATUS_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(user_id);
	WriteHeader();
}

CImPduClientUserStatusResponse::CImPduClientUserStatusResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_user_id = is.ReadString(m_user_id_len);
	is >> m_status;
	PARSE_PACKET_ASSERT
}

CImPduClientUserStatusResponse::CImPduClientUserStatusResponse(const char* user_id, uint32_t status)
{
	m_pdu_header.module_id = SID_BUDDY_LIST;
	m_pdu_header.command_id = CID_BUDDY_LIST_USER_STATUS_RESPONSE;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(user_id);
	os << status;
	WriteHeader();
}

CImPduClientUsersStatusRequest::CImPduClientUsersStatusRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_user_count;
	uint32_t id_len;
	for(uint32_t i=0; i< m_user_count; ++i)
	{
		char* user_id = is.ReadString(id_len);
		if (id_len > 32) {
			// fix id_len 很大造成的crash，一般都不会大于10
			break;
		}
		string user_id_url(user_id, id_len);
		m_users.push_back(user_id_url);
	}
	PARSE_PACKET_ASSERT
}

CImPduClientUsersStatusRequest::CImPduClientUsersStatusRequest(list<string>& users)
{
	m_pdu_header.module_id = SID_BUDDY_LIST;
	m_pdu_header.command_id = CID_BUDDY_LIST_USERS_STATUS_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
	uint32_t users_count = users.size();
	os << users_count;
	for(list<string>::iterator it=users.begin(); it!=users.end(); ++it)
	{
		os.WriteString(it->c_str());
	}
	WriteHeader();
}

CImPduClientUsersStatusResponse::CImPduClientUsersStatusResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_user_status_list = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_user_count;
	if (m_user_count > 0) {
		m_user_status_list = new client_user_stat_t [ m_user_count];
		ALLOC_FAIL_ASSERT(m_user_status_list)

		for (uint32_t i = 0; i < m_user_count; i++) {
			m_user_status_list[i].id_url = is.ReadString(m_user_status_list[i].id_len);
			is >> m_user_status_list[i].status;
		}
	}

	PARSE_PACKET_ASSERT
}

CImPduClientUsersStatusResponse::CImPduClientUsersStatusResponse(const list<user_stat_t>& user_status_list)
{

	m_pdu_header.module_id = SID_BUDDY_LIST;
	m_pdu_header.command_id = CID_BUDDY_LIST_USERS_STATUS_RESPONSE;
	m_user_status_list = NULL;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
	char* id_url = NULL;
	uint32_t users_count = user_status_list.size();
	os << users_count;
	for (list<user_stat_t>::const_iterator it = user_status_list.begin(); it != user_status_list.end(); it++) {
		id_url = idtourl(it->user_id);
		os.WriteString(id_url);
		os << it->status;
	}

	WriteHeader();
}

CImPduClientUsersStatusResponse::~CImPduClientUsersStatusResponse()
{
	if (m_user_status_list) {
		delete [] m_user_status_list;
	}
}

/////////////////////// 锟斤拷息锟斤拷锟� ////////////////////
CImPduClientMsgData::CImPduClientMsgData(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_from_id_url = m_to_id_url = NULL;
	m_msg_data = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_seq_no;
	m_from_id_url = is.ReadString(m_from_id_len);
	m_to_id_url = is.ReadString(m_to_id_len);
	is >> m_create_time;
	is >> m_msg_type;
	m_msg_data = is.ReadData(m_msg_len);
	m_attach_data = is.ReadString(m_attach_len);

	PARSE_PACKET_ASSERT
}

CImPduClientMsgData::CImPduClientMsgData(uint32_t seq_no, const char* from_id_url, const char* to_id_url,
		uint32_t create_time, uint8_t msg_type, uint32_t msg_len, uchar_t* msg_data,
		uint32_t attach_len, char* attach_data)
{
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = CID_MSG_DATA;
	m_from_id_url = m_to_id_url = NULL;
	m_msg_data = NULL;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << seq_no;
	os.WriteString(from_id_url);
	os.WriteString(to_id_url);
	os << create_time;
	os << msg_type;
	os.WriteData(msg_data, msg_len);
	os.WriteString(attach_data, attach_len);

	WriteHeader();
}

void CImPduClientMsgData::AddTime()
{
	uchar_t* buf = GetBuffer() + IM_PDU_HEADER_LEN + 4;
	uint32_t from_id_len = CByteStream::ReadUint32(buf);
	uint32_t to_id_len = CByteStream::ReadUint32(buf + 4 + from_id_len);
	CByteStream::WriteUint32(buf + 8 + from_id_len + to_id_len, (uint32_t)time(NULL));
}

CImPduClientMsgDataAck::CImPduClientMsgDataAck(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_seq_no;
	m_from_id_url = is.ReadString(m_from_id_len);
	PARSE_PACKET_ASSERT
}

CImPduClientMsgDataAck::CImPduClientMsgDataAck(uint32_t seq_no, uint32_t from_id_len, const char* from_id_url)
{
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = CID_MSG_DATA_ACK;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << seq_no;
	os.WriteString(from_id_url, from_id_len);
	WriteHeader();
}

CImPduClientMsgReadAck::CImPduClientMsgReadAck(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_from_id_url = is.ReadString(m_from_id_len);
	PARSE_PACKET_ASSERT
}

CImPduClientMsgReadAck::CImPduClientMsgReadAck(const char* from_id_url)
{
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = CID_MSG_READ_ACK;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(from_id_url);
	WriteHeader();
}

CImPduClientP2PCmdMsg::CImPduClientP2PCmdMsg(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_seq_no;
	m_from_id = is.ReadString(m_from_id_len);
	m_to_id = is.ReadString(m_to_id_len);
	m_cmd_msg_data = is.ReadData(m_cmd_msg_len);

	PARSE_PACKET_ASSERT
}

CImPduClientP2PCmdMsg::CImPduClientP2PCmdMsg(uint32_t seq_no, const char* from_id, const char* to_id,
		uint32_t cmd_msg_len, uchar_t* cmd_msg_data)
{
	m_pdu_header.module_id = SID_SWITCH_SERVICE;
	m_pdu_header.command_id = CID_SWITCH_P2P_CMD;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << seq_no;
	os.WriteString(from_id);
	os.WriteString(to_id);
	os.WriteData(cmd_msg_data, cmd_msg_len);

	WriteHeader();
}

CImPduClientTimeRequest::CImPduClientTimeRequest()
{
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = CID_MSG_TIME_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
	WriteHeader();
}

CImPduClientTimeResponse::CImPduClientTimeResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_server_time;
	PARSE_PACKET_ASSERT
}

CImPduClientTimeResponse::CImPduClientTimeResponse()
{
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = CID_MSG_TIME_RESPONSE;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << (uint32_t)time(NULL);
	WriteHeader();
}

CImPduClientUnreadMsgCntRequest::CImPduClientUnreadMsgCntRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
}

CImPduClientUnreadMsgCntRequest::CImPduClientUnreadMsgCntRequest()
{
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = CID_MSG_UNREAD_CNT_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	WriteHeader();
}

CImPduClientUnreadMsgCntResponse::CImPduClientUnreadMsgCntResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_unread_list = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_unread_cnt;
	if (m_unread_cnt > 0) {
		m_unread_list = (unread_info_t*)malloc(m_unread_cnt * sizeof(unread_info_t));
		ALLOC_FAIL_ASSERT(m_unread_list)

		for (uint32_t i = 0; i < m_unread_cnt; i++) {
			m_unread_list[i].id_url = is.ReadString(m_unread_list[i].id_len);
			is >> m_unread_list[i].unread_cnt;
		}
	}

	PARSE_PACKET_ASSERT
}

CImPduClientUnreadMsgCntResponse::CImPduClientUnreadMsgCntResponse(uint32_t unread_cnt, UserUnreadMsgCnt_t* unread_list)
{
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = CID_MSG_UNREAD_CNT_RESPONSE;
	m_unread_list = NULL;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	char* id_url = NULL;
	os << unread_cnt;
	for (uint32_t i = 0; i < unread_cnt; i++) {
		id_url = idtourl(unread_list[i].from_user_id);
		os.WriteString(id_url);
		os << unread_list[i].unread_msg_cnt;
	}

	WriteHeader();
}

CImPduClientUnreadMsgCntResponse::~CImPduClientUnreadMsgCntResponse()
{
	if (m_unread_list) {
		free(m_unread_list);
		m_unread_list = NULL;
	}
}

CImPduClientUnreadMsgRequest::CImPduClientUnreadMsgRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_from_id_url = is.ReadString(m_from_id_len);
	PARSE_PACKET_ASSERT
}

CImPduClientUnreadMsgRequest::CImPduClientUnreadMsgRequest(const char* from_id_url)
{
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = CID_MSG_UNREAD_MSG_REUQEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(from_id_url);
	WriteHeader();
}

CImPduClientMsgListResponse::CImPduClientMsgListResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_msg_list = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_display_id = is.ReadString(m_display_id_len);
	is >> m_msg_cnt;
	if (m_msg_cnt > 0) {
		m_msg_list = (client_msg_t*)malloc(m_msg_cnt * sizeof(client_msg_t));
		ALLOC_FAIL_ASSERT(m_msg_list)

		for (uint32_t i = 0; i < m_msg_cnt; i++) {
			m_msg_list[i].from_id_url = is.ReadString(m_msg_list[i].from_id_len);
			m_msg_list[i].from_name = is.ReadString(m_msg_list[i].from_name_len);
			m_msg_list[i].from_nick_name = is.ReadString(m_msg_list[i].from_nick_name_len);
			m_msg_list[i].from_avatar_url = is.ReadString(m_msg_list[i].from_avatar_len);

			is >> m_msg_list[i].create_time;
			is >> m_msg_list[i].msg_type;
			m_msg_list[i].msg_content = is.ReadData(m_msg_list[i].msg_len);
		}
	}

	PARSE_PACKET_ASSERT
}

CImPduClientMsgListResponse::CImPduClientMsgListResponse(uint16_t cmd_id, const char* display_id_url,
		uint32_t msg_cnt, server_msg_t* msg_list)
{
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = cmd_id;
	m_msg_list = NULL;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	char* id_url = NULL;
	os.WriteString(display_id_url);
	os << msg_cnt;
	for (uint32_t i = 0; i < msg_cnt; i++) {
		id_url = idtourl(msg_list[i].from_user_id);
		os.WriteString(id_url);
		os.WriteString(msg_list[i].from_name, msg_list[i].from_name_len);
		os.WriteString(msg_list[i].from_nick_name, msg_list[i].from_nick_name_len);
		os.WriteString(msg_list[i].from_avatar_url, msg_list[i].from_avatar_len);

		os << msg_list[i].create_time;
		os << msg_list[i].msg_type;
		os.WriteData(msg_list[i].msg_data, msg_list[i].msg_len);
	}

	WriteHeader();
}

CImPduClientMsgListResponse::~CImPduClientMsgListResponse()
{
	if (m_msg_list) {
		free(m_msg_list);
		m_msg_list = NULL;
	}
}


/////////////////////// relate to user information ////////////////////
CImPduClientAllUserRequest::CImPduClientAllUserRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	PARSE_PACKET_ASSERT
}

CImPduClientAllUserRequest::CImPduClientAllUserRequest()
{
	m_pdu_header.module_id = SID_BUDDY_LIST;
	m_pdu_header.command_id = CID_BUDDY_LIST_ALL_USER_REQUEST;

	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    WriteHeader();
}

CImPduClientUserInfoRequest::CImPduClientUserInfoRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	m_client_id_list = NULL;
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_user_cnt;
	if (m_user_cnt > 0) {
		m_client_id_list = (client_id_t*)malloc(m_user_cnt * sizeof(client_id_t));
		ALLOC_FAIL_ASSERT(m_client_id_list)

		for (uint32_t i = 0; i < m_user_cnt; i++) {
			m_client_id_list[i].id_url = is.ReadString(m_client_id_list[i].id_len);
		}
	}

	PARSE_PACKET_ASSERT
}

CImPduClientUserInfoRequest::CImPduClientUserInfoRequest(uint32_t user_cnt, client_id_t* client_id_list)
{
	m_pdu_header.module_id = SID_BUDDY_LIST;
	m_pdu_header.command_id = CID_BUDDY_LIST_USER_INFO_REQUEST;
	m_client_id_list = NULL;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << user_cnt;
	for (uint32_t i = 0; i < user_cnt; i++) {
		os.WriteString(client_id_list[i].id_url);
	}

	WriteHeader();
}

CImPduClientUserInfoRequest::~CImPduClientUserInfoRequest()
{
	if (m_client_id_list) {
		free(m_client_id_list);
		m_client_id_list = NULL;
	}
}

CImPduClientUserInfoResponse::CImPduClientUserInfoResponse(uchar_t* buf, uint32_t len)
{
	m_user_info_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_user_cnt;
	if (m_user_cnt > 0) {
		m_user_info_list = (client_user_info_t*)malloc(m_user_cnt * sizeof(client_user_info_t));
		ALLOC_FAIL_ASSERT(m_user_info_list)

		for (uint32_t i = 0; i < m_user_cnt; i++) {
			m_user_info_list[i].id_url = is.ReadString(m_user_info_list[i].id_len);
			m_user_info_list[i].name = is.ReadString(m_user_info_list[i].name_len);
			m_user_info_list[i].nick_name = is.ReadString(m_user_info_list[i].nick_name_len);
			m_user_info_list[i].avatar_url = is.ReadString(m_user_info_list[i].avatar_len);
            m_user_info_list[i].title = is.ReadString(m_user_info_list[i].title_len);
            m_user_info_list[i].position = is.ReadString(m_user_info_list[i].position_len);
            is >> m_user_info_list[i].role_status;
            is >> m_user_info_list[i].sex;
            m_user_info_list[i].depart_id_url = is.ReadString(m_user_info_list[i].depart_id_len);
            is >> m_user_info_list[i].job_num;
            m_user_info_list[i].telphone = is.ReadString(m_user_info_list[i].telphone_len);
            m_user_info_list[i].email = is.ReadString(m_user_info_list[i].email_len);
		}
	}

	PARSE_PACKET_ASSERT
}

CImPduClientUserInfoResponse::CImPduClientUserInfoResponse(uint16_t cmd_id, uint32_t user_cnt, user_info_t* user_info_list)
{
	m_pdu_header.module_id = SID_BUDDY_LIST;
	m_pdu_header.command_id = cmd_id;
	m_user_info_list = NULL;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	char* id_url = NULL;
	os << user_cnt;
	for (uint32_t i = 0; i < user_cnt; i++) {
		id_url = idtourl(user_info_list[i].user_id);
		os.WriteString(id_url);
		os.WriteString(user_info_list[i].name, user_info_list[i].name_len);
		os.WriteString(user_info_list[i].nick_name, user_info_list[i].nick_name_len);
		os.WriteString(user_info_list[i].avatar_url, user_info_list[i].avatar_len);
		os.WriteString(user_info_list[i].title, user_info_list[i].title_len);
		os.WriteString(user_info_list[i].position, user_info_list[i].position_len);
        os << user_info_list[i].role_status;
        os << user_info_list[i].sex;
        os.WriteString(idtourl(user_info_list[i].depart_id));
        os << user_info_list[i].job_num;
		os.WriteString(user_info_list[i].telphone, user_info_list[i].telphone_len);
		os.WriteString(user_info_list[i].email, user_info_list[i].email_len);
	}

	WriteHeader();
}

CImPduClientUserInfoResponse::~CImPduClientUserInfoResponse()
{
	if (m_user_info_list) {
		free(m_user_info_list);
		m_user_info_list = NULL;
	}
}

CImPduClientDBQueryRequest::CImPduClientDBQueryRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_seq_no;
	m_query_data = is.ReadData(m_query_len);
	PARSE_PACKET_ASSERT
}

CImPduClientDBQueryRequest::CImPduClientDBQueryRequest(uint32_t seq_no, uint32_t query_len, uchar_t* query_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_CLIENT_DB_QUERY_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << seq_no;
	os.WriteData(query_data, query_len);
	WriteHeader();
}

CImPduClientDBQueryResponse::CImPduClientDBQueryResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_seq_no;
	m_result_data = is.ReadData(m_result_len);
	PARSE_PACKET_ASSERT
}

CImPduClientDBQueryResponse::CImPduClientDBQueryResponse(uint32_t seq_no, uint32_t query_len, uchar_t* query_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_CLIENT_DB_QUERY_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << seq_no;
	os.WriteData(query_data, query_len);
	WriteHeader();
}

CImPduClientRemoveSessionRequest::CImPduClientRemoveSessionRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_session_type;
	m_session_id = is.ReadString(m_session_id_len);
	PARSE_PACKET_ASSERT
}

CImPduClientRemoveSessionRequest::CImPduClientRemoveSessionRequest(uint32_t session_type, const char* session_id)
{
	m_pdu_header.command_id = CID_BUDDY_LIST_REMOVE_SESSION_REQ;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << session_type;
	os.WriteString(session_id);
	WriteHeader();
}

CImPduClientRemoveSessionResponse::CImPduClientRemoveSessionResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_result;
	is >> m_session_type;
	m_session_id = is.ReadString(m_session_id_len);
	PARSE_PACKET_ASSERT
}

CImPduClientRemoveSessionResponse::CImPduClientRemoveSessionResponse(uint32_t result, uint32_t session_type,
		const char* session_id)
{
	m_pdu_header.module_id = SID_BUDDY_LIST;
	m_pdu_header.command_id = CID_BUDDY_LIST_REMOVE_SESSION_RES;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << result;
	os << session_type;
	os.WriteString(session_id);
	WriteHeader();
}

/////////////////////// set online status ////////////////////
CImPduClientServiceSetting::CImPduClientServiceSetting(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_key;
	is >> m_value;
	PARSE_PACKET_ASSERT
}

CImPduClientServiceSetting::CImPduClientServiceSetting(uint32_t key, uint32_t value)
{
	m_pdu_header.command_id = IM_PDU_TYPE_CLIENT_SERVICE_SETTING;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << key;
	os << value;
	WriteHeader();
}

CImPduClientDepartmentRequest::CImPduClientDepartmentRequest(uchar_t* buf, uint32_t len)
{
    ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    
	PARSE_PACKET_ASSERT
}

CImPduClientDepartmentRequest::CImPduClientDepartmentRequest()
{
    m_pdu_header.module_id = SID_BUDDY_LIST;
    m_pdu_header.command_id = CID_BUDDY_LIST_DEPARTMENT_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
	WriteHeader();
}

CImPduClientDepartmentResponse::CImPduClientDepartmentResponse(uchar_t* buf, uint32_t len)
{
    ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    m_depart_list = NULL;
    is >> m_depart_cnt;
    if (m_depart_cnt > 0) {
        m_depart_list = new client_department_info_t[m_depart_cnt];
        ALLOC_FAIL_ASSERT(m_depart_list)

        for (uint32_t i = 0; i < m_depart_cnt; i++) {
            m_depart_list[i].depart_id_url = is.ReadString(m_depart_list[i].depart_id_url_len);
            m_depart_list[i].title = is.ReadString(m_depart_list[i].title_len);
            m_depart_list[i].description = is.ReadString(m_depart_list[i].description_len);
            m_depart_list[i].parent_depart_id_url = is.ReadString(m_depart_list[i].parent_depart_id_url_len);
            m_depart_list[i].leader = is.ReadString(m_depart_list[i].leader_len);
            is >> m_depart_list[i].status;
        }
    }
    
	PARSE_PACKET_ASSERT
}

CImPduClientDepartmentResponse::CImPduClientDepartmentResponse(uint32_t depart_cnt,
                                                               department_info_t* depart_list)
{
    m_pdu_header.module_id = SID_BUDDY_LIST;
    m_pdu_header.command_id = CID_BUDDY_LIST_DEPARTMENT_RESPONSE;
    m_depart_list = NULL;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
    os << depart_cnt;
    for (uint32_t i = 0; i < depart_cnt; i++) {
        os.WriteString(idtourl(depart_list[i].depart_id));
        os.WriteString(depart_list[i].title, depart_list[i].title_len);
        os.WriteString(depart_list[i].description, depart_list[i].description_len);
        os.WriteString(idtourl(depart_list[i].parent_depart_id));
        os.WriteString(idtourl(depart_list[i].leader));
        os << depart_list[i].status;
    }
	WriteHeader();
}

CImPduClientDepartmentResponse::~CImPduClientDepartmentResponse()
{
    if (m_depart_list != NULL) {
        delete []m_depart_list;
        m_depart_list = NULL;
    }
}





