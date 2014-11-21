/*
 * ImPduGroup.cpp
 *
 *  Created on: 2014-1-2
 *      Author: ziteng@mogujie.com
 */

#include "ImPduGroup.h"

CImPduClientGroupListRequest::CImPduClientGroupListRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
}

CImPduClientGroupListRequest::CImPduClientGroupListRequest(uint16_t cmd_id)
{
	m_pdu_header.module_id = SID_GROUP;
	m_pdu_header.command_id = cmd_id;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	WriteHeader();
}

CImPduClientGroupListResponse::CImPduClientGroupListResponse(uchar_t* buf, uint32_t len)
{
	m_group_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_group_cnt;
	if (m_group_cnt > 0) {
		m_group_list = new client_group_info_t [m_group_cnt];
		ALLOC_FAIL_ASSERT(m_group_list);

		for (uint32_t i = 0; i < m_group_cnt; i++) {
			m_group_list[i].group_id = is.ReadString(m_group_list[i].group_id_len);
			m_group_list[i].group_name = is.ReadString(m_group_list[i].group_name_len);
			m_group_list[i].group_avatar = is.ReadString(m_group_list[i].group_avatar_len);
			m_group_list[i].group_creator_id = is.ReadString(m_group_list[i].group_creator_id_len);
			is >> m_group_list[i].group_type;
			if (m_pdu_header.command_id == CID_GROUP_DIALOG_LIST_RESPONSE) {
				is >> m_group_list[i].group_updated;
			} else {
				m_group_list[i].group_updated = 0;
			}

			uint32_t user_cnt;
			is >> user_cnt;
			for (uint32_t j = 0; j < user_cnt; j++) {
				user_id_t user_id;
				user_id.id_url = is.ReadString(user_id.id_len);
				m_group_list[i].group_memeber_list.push_back(user_id);
			}
		}
	}
	PARSE_PACKET_ASSERT
}

CImPduClientGroupListResponse::CImPduClientGroupListResponse(uint16_t cmd_id,
		uint32_t group_cnt, group_info_t* group_list)
{
	m_group_list = NULL;
	m_pdu_header.module_id = SID_GROUP;
	m_pdu_header.command_id = cmd_id;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	char* id_url = NULL;
	os << group_cnt;
	for (uint32_t i = 0; i < group_cnt; i++) {
		uint32_t group_id = group_list[i].group_id;
		id_url = idtourl(group_id);
		os.WriteString(id_url);
		os.WriteString(group_list[i].group_name, group_list[i].group_name_len);
		os.WriteString(group_list[i].group_avatar, group_list[i].group_avatar_len);
        id_url = idtourl(group_list[i].group_creator_id);
        os.WriteString(id_url);
		
		os << group_list[i].group_type;
		if (cmd_id == CID_GROUP_DIALOG_LIST_RESPONSE) {
			os << group_list[i].group_updated;
		}

		uint32_t user_id;
		uint32_t user_cnt = group_list[i].group_memeber_list.size();
		os << user_cnt;
		for (list<uint32_t>::iterator it = group_list[i].group_memeber_list.begin();
				it != group_list[i].group_memeber_list.end();
				it++) {
			user_id = *it;
			id_url = idtourl(user_id);
			os.WriteString(id_url);
		}
	}
	WriteHeader();
}

CImPduClientGroupListResponse::~CImPduClientGroupListResponse()
{
	if (m_group_list)
		delete [] m_group_list;
}

CImPduClientGroupUserListRequest::CImPduClientGroupUserListRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_group_id = is.ReadString(m_group_id_len);
	PARSE_PACKET_ASSERT
}

CImPduClientGroupUserListRequest::CImPduClientGroupUserListRequest(const char* group_id)
{
	m_pdu_header.module_id = SID_GROUP;
	m_pdu_header.command_id = CID_GROUP_USER_LIST_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(group_id);
	WriteHeader();
}

CImPduClientGroupUserListResponse::CImPduClientGroupUserListResponse(uchar_t* buf, uint32_t len)
{
	m_user_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_group_id = is.ReadString(m_group_id_len);
	is >> m_result;

	if (m_result == 0) {
		m_group_name = is.ReadString(m_group_name_len);
		m_group_avatar = is.ReadString(m_group_avatar_len);
		m_group_creator_id = is.ReadString(m_group_creator_id_len);
		is >> m_group_type;
		is >> m_user_cnt;
		if (m_user_cnt > 0) {
			m_user_list = new user_id_t [m_user_cnt];
			ALLOC_FAIL_ASSERT(m_user_list);
			for (uint32_t i = 0; i < m_user_cnt; i++) {
				m_user_list[i].id_url = is.ReadString(m_user_list[i].id_len);
			}
		}
	} else {
		m_group_name_len = 0;
		m_group_name = NULL;
		m_group_avatar_len = 0;
		m_group_avatar = NULL;
		m_user_cnt = 0;
	}
	PARSE_PACKET_ASSERT
}

CImPduClientGroupUserListResponse::CImPduClientGroupUserListResponse(uint16_t version, const char* group_id,
		uint32_t result, const char* group_name, const char* group_avatar, const char* group_creator_id,
		uint32_t group_type, uint32_t user_cnt, uint32_t* user_list)
{
	m_user_list = NULL;
	m_pdu_header.module_id = SID_GROUP;
	m_pdu_header.command_id = CID_GROUP_USER_LIST_RESPONSE;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(group_id);
	os << result;
	if (result == 0) {
		os.WriteString(group_name);
		os.WriteString(group_avatar);
        os.WriteString(group_creator_id);
		os << group_type;
		os << user_cnt;
		for (uint32_t i = 0; i < user_cnt; i++) {
			uint32_t user_id = user_list[i];
			char* user_id_url = idtourl(user_id);
			os.WriteString(user_id_url);
		}
	}

	WriteHeader();
}

CImPduClientGroupUserListResponse::~CImPduClientGroupUserListResponse()
{
	if (m_user_list)
		delete [] m_user_list;
}


CImPduClientGroupUnreadMsgCntRequest::CImPduClientGroupUnreadMsgCntRequest()
{
	m_pdu_header.module_id = SID_GROUP;
	m_pdu_header.command_id = CID_GROUP_UNREAD_CNT_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
	WriteHeader();
}

CImPduClientGroupUnreadMsgCntResponse::CImPduClientGroupUnreadMsgCntResponse(uchar_t* buf, uint32_t len)
{
	m_group_unread_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_group_unread_cnt;
	if (m_group_unread_cnt > 0) {
		m_group_unread_list = new client_group_unread_cnt_t [m_group_unread_cnt];
		ALLOC_FAIL_ASSERT(m_group_unread_list);
		for (uint32_t i = 0; i < m_group_unread_cnt; i++) {
			m_group_unread_list[i].group_id_url = is.ReadString(m_group_unread_list[i].group_id_len);
			is >> m_group_unread_list[i].unread_msg_cnt;
		}
	}
}

CImPduClientGroupUnreadMsgCntResponse::CImPduClientGroupUnreadMsgCntResponse(uint32_t unread_cnt,
		group_unread_cnt_t* group_unread_list)
{
	m_group_unread_list = NULL;
	m_pdu_header.module_id = SID_GROUP;
	m_pdu_header.command_id = CID_GROUP_UNREAD_CNT_RESPONSE;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << unread_cnt;
	for (uint32_t i = 0; i < unread_cnt; i++) {
		char* group_id_url = idtourl(group_unread_list[i].group_id);
		os.WriteString(group_id_url);
		os << group_unread_list[i].unread_msg_cnt;
	}
	WriteHeader();
}

CImPduClientGroupUnreadMsgCntResponse::~CImPduClientGroupUnreadMsgCntResponse()
{
	if (m_group_unread_list)
		delete [] m_group_unread_list;
}

CImPduClientGroupUnreadMsgRequest::CImPduClientGroupUnreadMsgRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_group_id_url = is.ReadString(m_group_id_len);
	PARSE_PACKET_ASSERT
}

CImPduClientGroupUnreadMsgRequest::CImPduClientGroupUnreadMsgRequest(const char* group_id_url)
{
	m_pdu_header.module_id = SID_GROUP;
	m_pdu_header.command_id = CID_GROUP_UNREAD_MSG_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(group_id_url);
	WriteHeader();
}

CImPduClientGroupMsgListResponse::CImPduClientGroupMsgListResponse(uchar_t* buf, uint32_t len)
{
	m_msg_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_group_id_url = is.ReadString(m_group_id_len);
	is >> m_msg_cnt;
	if (m_msg_cnt > 0) {
		m_msg_list = new client_group_msg_t [m_msg_cnt];
		ALLOC_FAIL_ASSERT(m_msg_list);
		for (uint32_t i = 0; i < m_msg_cnt; i++) {
			m_msg_list[i].from_user_id_url = is.ReadString(m_msg_list[i].from_user_id_len);
			is >> m_msg_list[i].create_time;
            is >> m_msg_list[i].msg_type;
			m_msg_list[i].msg_content = is.ReadData(m_msg_list[i].msg_len);
		}
	}

	PARSE_PACKET_ASSERT
}

CImPduClientGroupMsgListResponse::CImPduClientGroupMsgListResponse(uint32_t cmd_id,
		const char* group_id_url, uint32_t msg_cnt, group_msg_t* msg_list)
{
	m_msg_list = NULL;
	m_pdu_header.module_id = SID_GROUP;
	m_pdu_header.command_id = cmd_id;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(group_id_url);
	os << msg_cnt;
	for (uint32_t i = 0; i < msg_cnt; i++) {
		char* from_user_id_url = idtourl(msg_list[i].from_user_id);
		os.WriteString(from_user_id_url);
		os << msg_list[i].create_time;
        os << msg_list[i].msg_type;
		os.WriteData(msg_list[i].msg_content, msg_list[i].msg_len);
	}

	WriteHeader();
}

CImPduClientGroupMsgListResponse::~CImPduClientGroupMsgListResponse()
{
	if (m_msg_list)
		delete [] m_msg_list;
}

CImPduClientGroupMsgReadAck::CImPduClientGroupMsgReadAck(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_group_id_url = is.ReadString(m_group_id_len);
	PARSE_PACKET_ASSERT
}

CImPduClientGroupMsgReadAck::CImPduClientGroupMsgReadAck(const char* group_id_url)
{
	m_pdu_header.module_id = SID_GROUP;
	m_pdu_header.command_id = CID_GROUP_MSG_READ_ACK;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(group_id_url);
	WriteHeader();
}

CImPduClientGroupCreateTmpGroupRequest::CImPduClientGroupCreateTmpGroupRequest(uchar_t* buf, uint32_t len)
{
	m_user_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_group_name = is.ReadString(m_group_name_len);
	m_group_avatar = is.ReadString(m_group_avatar_len);
	is >> m_user_cnt;
	if (m_user_cnt > 0) {
		m_user_list = new user_id_t [m_user_cnt];
		for (uint32_t i = 0; i < m_user_cnt; i++) {
			m_user_list[i].id_url = is.ReadString(m_user_list[i].id_len);
		}
	}

	PARSE_PACKET_ASSERT
}

CImPduClientGroupCreateTmpGroupRequest::CImPduClientGroupCreateTmpGroupRequest(const char* group_name,
		const char* group_avatar, list<user_id_t>* user_list)
{
    m_user_list = NULL;
	m_pdu_header.module_id = SID_GROUP;
	m_pdu_header.command_id = CID_GROUP_CREATE_TMP_GROUP_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(group_name);
	os.WriteString(group_avatar);

	if (user_list) {
		os << (uint32_t)user_list->size();
		for (list<user_id_t>::iterator it = user_list->begin(); it != user_list->end(); it++) {
			os.WriteString(it->id_url);
		}
	} else {
		os << (uint32_t)0;
	}

	WriteHeader();
}

CImPduClientGroupCreateTmpGroupRequest::~CImPduClientGroupCreateTmpGroupRequest()
{
	if (m_user_list) {
		delete [] m_user_list;
		m_user_list = NULL;
	}
}

CImPduClientGroupCreateTmpGroupResponse::CImPduClientGroupCreateTmpGroupResponse(uchar_t* buf, uint32_t len)
{
	m_user_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_result;
	m_group_id = is.ReadString(m_group_id_len);
	m_group_name = is.ReadString(m_group_name_len);
	is >> m_user_cnt;
	if (m_user_cnt > 0) {
		m_user_list = new user_id_t [m_user_cnt];
		ALLOC_FAIL_ASSERT(m_user_list);
		for (uint32_t i = 0; i < m_user_cnt; i++) {
			m_user_list[i].id_url = is.ReadString(m_user_list[i].id_len);
		}
	}

	PARSE_PACKET_ASSERT
}

CImPduClientGroupCreateTmpGroupResponse::CImPduClientGroupCreateTmpGroupResponse(uint32_t result,
		const char* group_id, const char* group_name, uint32_t user_cnt, uint32_t* user_list)
{
	m_user_list = NULL;
	m_pdu_header.module_id = SID_GROUP;
	m_pdu_header.command_id = CID_GROUP_CREATE_TMP_GROUP_RESPONSE;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << result;
	os.WriteString(group_id);
	os.WriteString(group_name);
	os << user_cnt;
	for (uint32_t i = 0; i < user_cnt; i++) {
		char* user_id_url = idtourl(user_list[i]);
		os.WriteString(user_id_url);
	}
	WriteHeader();
}

CImPduClientGroupCreateTmpGroupResponse::~CImPduClientGroupCreateTmpGroupResponse()
{
	if (m_user_list) {
		delete [] m_user_list;
	}
}

CImPduClientGroupChangeMemberRequest::CImPduClientGroupChangeMemberRequest(uchar_t* buf, uint32_t len)
{
	m_user_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_group_id = is.ReadString(m_group_id_len);
    is >> m_change_type;
	is >> m_user_cnt;
	if (m_user_cnt > 0) {
		m_user_list = new user_id_t [m_user_cnt];
		for (uint32_t i = 0; i < m_user_cnt; i++) {
			m_user_list[i].id_url = is.ReadString(m_user_list[i].id_len);
		}
	}
	PARSE_PACKET_ASSERT
}

CImPduClientGroupChangeMemberRequest::CImPduClientGroupChangeMemberRequest(
		uint32_t change_type, const char* group_id, list<user_id_t>* user_list)
{
	m_user_list = NULL;
	m_pdu_header.module_id = SID_GROUP;
	m_pdu_header.command_id = CID_GROUP_CHANGE_MEMBER_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(group_id);
    os << change_type;
	if (user_list) {
		os << (uint32_t)user_list->size();
		for (list<user_id_t>::iterator it = user_list->begin(); it != user_list->end(); it++) {
			os.WriteString(it->id_url);
		}
	} else {
		os << (uint32_t)0;
	}

	WriteHeader();
}

CImPduClientGroupChangeMemberRequest::~CImPduClientGroupChangeMemberRequest()
{
	if (m_user_list) {
		delete [] m_user_list;
	}
}

CImPduClientGroupChangeMemberResponse::CImPduClientGroupChangeMemberResponse(uchar_t* buf, uint32_t len)
{
	m_user_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_result;
	m_group_id = is.ReadString(m_group_id_len);
    is >> m_change_type;
	is >> m_user_cnt;
	if (m_user_cnt > 0) {
		m_user_list = new user_id_t [m_user_cnt];
		ALLOC_FAIL_ASSERT(m_user_list);
		for (uint32_t i = 0; i < m_user_cnt; i++) {
			m_user_list[i].id_url = is.ReadString(m_user_list[i].id_len);
		}
	}
	PARSE_PACKET_ASSERT
}

CImPduClientGroupChangeMemberResponse::CImPduClientGroupChangeMemberResponse(uint32_t change_type, uint32_t result,
		const char* group_id, uint32_t user_cnt, uint32_t* user_list)
{
	m_user_list = NULL;
	m_pdu_header.module_id = SID_GROUP;
	m_pdu_header.command_id = CID_GROUP_CHANGE_MEMBER_RESPONSE;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << result;
	os.WriteString(group_id);
    os << change_type;
	os << user_cnt;
	for (uint32_t i = 0; i < user_cnt; i++) {
		char* user_id_url = idtourl(user_list[i]);
		os.WriteString(user_id_url);
	}

	WriteHeader();
}

CImPduClientGroupChangeMemberResponse::~CImPduClientGroupChangeMemberResponse()
{
	if (m_user_list) {
		delete [] m_user_list;
	}
}

CImPduClientGroupCreateNormalGroupNotify::CImPduClientGroupCreateNormalGroupNotify(uchar_t*buf,
                                                                                   uint32_t len)
{
    m_user_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    
	m_group_id_url = is.ReadString(m_group_id_url_len);
    m_group_name = is.ReadString(m_group_name_len);
    m_group_avatar = is.ReadString(m_group_avatar_len);
	is >> m_user_cnt;
	if (m_user_cnt > 0) {
		m_user_list = new user_id_t [m_user_cnt];
		ALLOC_FAIL_ASSERT(m_user_list);
		for (uint32_t i = 0; i < m_user_cnt; i++) {
			m_user_list[i].id_url = is.ReadString(m_user_list[i].id_len);
		}
	}
	PARSE_PACKET_ASSERT
}

CImPduClientGroupCreateNormalGroupNotify::CImPduClientGroupCreateNormalGroupNotify(
    const char* group_id_url, const char* group_name, const char* group_avatar,
                                    uint32_t user_cnt, uint32_t* user_list)
{
    m_user_list = NULL;
	m_pdu_header.module_id = SID_GROUP;
	m_pdu_header.command_id = CID_GROUP_CREATE_NORMAL_GROUP_NOTIFY;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
	os.WriteString(group_id_url);
    os.WriteString(group_name);
    os.WriteString(group_avatar);
	os << user_cnt;
	for (uint32_t i = 0; i < user_cnt; i++) {
		char* user_id_url = idtourl(user_list[i]);
		os.WriteString(user_id_url);
	}
    
	WriteHeader();
}

CImPduClientGroupCreateNormalGroupNotify::~CImPduClientGroupCreateNormalGroupNotify()
{
    if (m_user_list != NULL) {
        delete m_user_list;
        m_user_list = NULL;
    }
}

CImPduClientGroupChangeMemberNotify::CImPduClientGroupChangeMemberNotify(uchar_t*buf,
                                                                         uint32_t len)
{
    m_user_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    
	m_group_id_url = is.ReadString(m_group_id_url_len);
	is >> m_user_cnt;
	if (m_user_cnt > 0) {
		m_user_list = new user_id_t [m_user_cnt];
		ALLOC_FAIL_ASSERT(m_user_list);
		for (uint32_t i = 0; i < m_user_cnt; i++) {
			m_user_list[i].id_url = is.ReadString(m_user_list[i].id_len);
		}
	}
	PARSE_PACKET_ASSERT
}

CImPduClientGroupChangeMemberNotify::CImPduClientGroupChangeMemberNotify(const char* group_id_url,
                                                    uint32_t user_cnt, uint32_t* user_list)
{
    m_user_list = NULL;
	m_pdu_header.module_id = SID_GROUP;
	m_pdu_header.command_id = CID_GROUP_CHANGE_MEMEBER_NOTIFY;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
	os.WriteString(group_id_url);
	os << user_cnt;
	for (uint32_t i = 0; i < user_cnt; i++) {
		char* user_id_url = idtourl(user_list[i]);
		os.WriteString(user_id_url);
	}
    
	WriteHeader();
}

CImPduClientGroupChangeMemberNotify::~CImPduClientGroupChangeMemberNotify()
{
    if (m_user_list != NULL) {
        delete []m_user_list;
        m_user_list = NULL;
    }
}

// for server use
#ifndef WIN32
CImPduGroupListRequest::CImPduGroupListRequest(uint16_t cmd_id, uint32_t user_id, uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = cmd_id;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << user_id;
	os.WriteData(attach_data, attach_len);
	WriteHeader();
}

CImPduGroupListResponse::CImPduGroupListResponse(uchar_t* buf, uint32_t len)
{
	m_group_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_req_user_id;
	is >> m_group_cnt;
	if (m_group_cnt > 0) {
		m_group_list = new group_info_t [m_group_cnt];
		ALLOC_FAIL_ASSERT(m_group_list);
		for (uint32_t i = 0; i < m_group_cnt; i++) {
			is >> m_group_list[i].group_id;
			m_group_list[i].group_name = is.ReadString(m_group_list[i].group_name_len);
			m_group_list[i].group_avatar = is.ReadString(m_group_list[i].group_avatar_len);
			is >> m_group_list[i].group_type;
			is >> m_group_list[i].group_creator_id;
			if (m_pdu_header.command_id == IM_PDU_TYPE_GROUP_DIALOG_LIST_RESPONSE) {
				is >> m_group_list[i].group_updated;
			} else {
				m_group_list[i].group_updated = 0;
			}

			uint32_t user_cnt;
			uint32_t user_id;
			is >> user_cnt;
			for (uint32_t j = 0; j < user_cnt; j++) {
				is >> user_id;
				m_group_list[i].group_memeber_list.push_back(user_id);
			}
		}
	}
	m_attach_data = is.ReadData(m_attach_len);
	PARSE_PACKET_ASSERT
}

CImPduGroupListResponse::~CImPduGroupListResponse()
{
	if (m_group_list)
		delete [] m_group_list;
}

CImPduGroupUserListRequest::CImPduGroupUserListRequest(uint32_t req_user_id, uint32_t group_id,
		uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_GROUP_USER_LIST_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << req_user_id;
	os << group_id;
	os.WriteData(attach_data, attach_len);

	WriteHeader();
}

CImPduGroupUserListResponse::CImPduGroupUserListResponse(uchar_t* buf, uint32_t len)
{
	m_user_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_req_user_id;
	is >> m_group_id;
	is >> m_result;
	if (m_result == 0) {
		m_group_name = is.ReadString(m_group_name_len);
		m_group_avatar = is.ReadString(m_group_avatar_len);
		is >> m_group_type;
		is >> m_group_creator_id;
		is >> m_user_cnt;
		if (m_user_cnt > 0) {
			m_user_list = new uint32_t [m_user_cnt];
			ALLOC_FAIL_ASSERT(m_user_list);
			for (uint32_t i = 0; i < m_user_cnt; i++) {
				is >> m_user_list[i];
			}
		}
	} else {
		m_group_name_len = 0;
		m_group_name = NULL;
		m_group_avatar_len = 0;
		m_group_avatar = NULL;
		m_user_cnt = 0;
	}
	m_attach_data = is.ReadData(m_attach_len);

	PARSE_PACKET_ASSERT
}

CImPduGroupUserListResponse::~CImPduGroupUserListResponse()
{
	if (m_user_list)
		delete [] m_user_list;
}

CImPduGroupUnreadMsgCntRequest::CImPduGroupUnreadMsgCntRequest(uint32_t req_user_id,
        uint32_t client_type, uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_GROUP_UNREAD_MSG_CNT_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << req_user_id;
    os << client_type;
	os.WriteData(attach_data, attach_len);
	WriteHeader();
}

CImPduGroupUnreadMsgCntResponse::CImPduGroupUnreadMsgCntResponse(uchar_t* buf, uint32_t len)
{
	m_group_unread_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_req_user_id;
	is >> m_group_unread_cnt;
	if (m_group_unread_cnt > 0) {
		m_group_unread_list = new group_unread_cnt_t [m_group_unread_cnt];
		ALLOC_FAIL_ASSERT(m_group_unread_list);
		for (uint32_t i = 0; i < m_group_unread_cnt; i++) {
			is >> m_group_unread_list[i].group_id;
			is >> m_group_unread_list[i].unread_msg_cnt;
		}
	}

	m_attach_data = is.ReadData(m_attach_len);
	PARSE_PACKET_ASSERT
}

CImPduGroupUnreadMsgCntResponse::~CImPduGroupUnreadMsgCntResponse()
{
	if (m_group_unread_list)
		delete [] m_group_unread_list;
}

CImPduGroupUnreadMsgRequest::CImPduGroupUnreadMsgRequest(uint32_t req_user_id, uint32_t group_id,
			uint32_t client_type, uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_GROUP_UNREAD_MSG_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << req_user_id;
	os << group_id;
    os << client_type;
	os.WriteData(attach_data, attach_len);
	WriteHeader();
}

CImPduGroupMsgListResponse::CImPduGroupMsgListResponse(uchar_t* buf, uint32_t len)
{
	m_msg_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_req_user_id;
	is >> m_req_cmd_id;
	is >> m_group_id;
	is >> m_msg_cnt;
	if (m_msg_cnt > 0) {
		m_msg_list = new group_msg_t [m_msg_cnt];
		ALLOC_FAIL_ASSERT(m_msg_list);
		for (uint32_t i = 0; i < m_msg_cnt; i++) {
			is >> m_msg_list[i].from_user_id;
			is >> m_msg_list[i].create_time;
            is >> m_msg_list[i].msg_type;
			m_msg_list[i].msg_content = is.ReadData(m_msg_list[i].msg_len);
		}
	}
	m_attach_data = is.ReadData(m_attach_len);

	PARSE_PACKET_ASSERT
}

CImPduGroupMsgListResponse::~CImPduGroupMsgListResponse()
{
	if (m_msg_list)
		delete [] m_msg_list;
}

CImPduGroupMsgReadAck::CImPduGroupMsgReadAck(uint32_t req_user_id, uint32_t group_id,
                                             uint32_t client_type)
{
	m_pdu_header.command_id = IM_PDU_TYPE_GROUP_MSG_READ_ACK;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << req_user_id;
	os << group_id;
    os << client_type;
	WriteHeader();
}

CImPduGroupCreateTmpGroupRequest::CImPduGroupCreateTmpGroupRequest(uint32_t req_user_id, const char* group_name,
	const char* group_avatar, list<uint32_t>* user_list, uint32_t attach_len, uchar_t* attach_data)
{
    m_pdu_header.command_id = IM_PDU_TYPE_GROUP_CREATE_TMP_GROUP_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << req_user_id;
	os.WriteString(group_name);
	os.WriteString(group_avatar);
	os << (uint32_t)user_list->size();
	for (list<uint32_t>::iterator it = user_list->begin(); it != user_list->end(); it++) {
		uint32_t user_id = *it;
		os << user_id;
	}
	os.WriteData(attach_data, attach_len);
	WriteHeader();
}

CImPduGroupCreateTmpGroupResponse::CImPduGroupCreateTmpGroupResponse(uchar_t* buf, uint32_t len)
{
	m_user_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_req_user_id;
	is >> m_result;
	is >> m_group_id;
	m_group_name = is.ReadString(m_group_name_len);
	is >> m_user_cnt;
	if (m_user_cnt > 0) {
		m_user_list = new uint32_t [m_user_cnt];
		for (uint32_t i = 0; i < m_user_cnt; i++) {
			is >> m_user_list[i];
		}
	}
	m_attach_data = is.ReadData(m_attach_len);

	PARSE_PACKET_ASSERT
}

CImPduGroupCreateTmpGroupResponse::~CImPduGroupCreateTmpGroupResponse()
{
	if (m_user_list) {
		delete [] m_user_list;
		m_user_list = NULL;
	}
}

CImPduGroupChangeMemberRequest::CImPduGroupChangeMemberRequest(uint32_t change_type, uint32_t req_user_id,
		uint32_t group_id, list<uint32_t>* user_list, uint32_t attach_len, uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_GROUP_CHANGE_MEMBER_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << req_user_id;
	os << group_id;
    os << change_type;
	os << (uint32_t)user_list->size();
	for (list<uint32_t>::iterator it = user_list->begin(); it != user_list->end(); it++) {
		uint32_t user_id = *it;
		os << user_id;
	}

	os.WriteData(attach_data, attach_len);
	WriteHeader();
}

CImPduGroupChangeMemberResponse::CImPduGroupChangeMemberResponse(uchar_t* buf, uint32_t len)
{
    m_user_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_req_user_id;
	is >> m_result;
	is >> m_group_id;
    is >> m_change_type;
	is >> m_user_cnt;
	if (m_user_cnt > 0) {
		m_user_list = new uint32_t [m_user_cnt];
		ALLOC_FAIL_ASSERT(m_user_list);
		for (uint32_t i = 0; i < m_user_cnt; i++) {
			is >> m_user_list[i];
		}
	}
	m_attach_data = is.ReadData(m_attach_len);

	PARSE_PACKET_ASSERT
}

CImPduGroupChangeMemberResponse::~CImPduGroupChangeMemberResponse()
{
	if (m_user_list) {
		delete [] m_user_list;
		m_user_list = NULL;
	}
}

CImPduGroupCreateNormalGroupNotify::CImPduGroupCreateNormalGroupNotify(uchar_t* buf, uint32_t len)
{
    ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    m_user_list = NULL;
	is >> m_group_id;
    m_group_name = is.ReadString(m_group_name_len);
    m_group_avatar = is.ReadString(m_group_avatar_len);
	is >> m_user_cnt;
	if (m_user_cnt > 0) {
		m_user_list = new uint32_t [m_user_cnt];
		ALLOC_FAIL_ASSERT(m_user_list);
		for (uint32_t i = 0; i < m_user_cnt; i++) {
			is >> m_user_list[i];
		}
	}
    
	PARSE_PACKET_ASSERT
}

CImPduGroupCreateNormalGroupNotify::CImPduGroupCreateNormalGroupNotify(uint32_t group_id,
            const char* group_name, const char* group_avatar, uint32_t user_cnt, uint32_t* user_list)
{
    m_pdu_header.module_id = SID_OTHER;
    m_pdu_header.command_id = IM_PDU_TYPE_GROUP_CREATE_NORMAL_GROUP_NOTIFY;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
    m_user_list = NULL;
	os << group_id;
    os.WriteString(group_name);
    os.WriteString(group_avatar);
	os << user_cnt;
	for (uint32_t i = 0; i < user_cnt; i++) {
		uint32_t user_id = user_list[i];
		os << user_id;
	}
	WriteHeader();
}

CImPduGroupCreateNormalGroupNotify::~CImPduGroupCreateNormalGroupNotify()
{
    if (m_user_list != NULL) {
        delete m_user_list;
        m_user_list = NULL;
    }
}

CImPduGroupChangeMemberNotify::CImPduGroupChangeMemberNotify(uchar_t*buf, uint32_t len)
{
    ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    m_user_list = NULL;
	is >> m_group_id;
	is >> m_user_cnt;
	if (m_user_cnt > 0) {
		m_user_list = new uint32_t [m_user_cnt];
		ALLOC_FAIL_ASSERT(m_user_list);
		for (uint32_t i = 0; i < m_user_cnt; i++) {
			is >> m_user_list[i];
		}
	}
    
	PARSE_PACKET_ASSERT
}

CImPduGroupChangeMemberNotify::CImPduGroupChangeMemberNotify(uint32_t group_id,
                                                             uint32_t user_cnt, uint32_t* user_list)
{
    m_pdu_header.module_id = SID_OTHER;
    m_pdu_header.command_id = IM_PDU_TYPE_GROUP_CHANGE_MEMBER_NOTIFY;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    m_user_list = NULL;
    
	os << group_id;
	os << user_cnt;
	for (uint32_t i = 0; i < user_cnt; i++) {
		uint32_t user_id = user_list[i];
		os << user_id;
	}
	WriteHeader();
}

CImPduGroupChangeMemberNotify::~CImPduGroupChangeMemberNotify()
{
    if (m_user_list != NULL) {
        delete m_user_list;
        m_user_list = NULL;
    }
}

#endif
