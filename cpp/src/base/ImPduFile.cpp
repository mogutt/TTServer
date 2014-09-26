/*
 * ImPduFile.cpp
 *
 *  Created on: 2013-12-9
 *      Author: ziteng@mogujie.com
 */

#include "ImPduFile.h"

CImPduClientFileLoginReq::CImPduClientFileLoginReq(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_user_id = is.ReadString(m_user_id_len);
	m_token = is.ReadString(m_token_len);
    m_task_id = is.ReadString(m_task_id_len);
    is >> m_client_mode;
	PARSE_PACKET_ASSERT
}

CImPduClientFileLoginReq::CImPduClientFileLoginReq(const char* user_id, const char* token, const char* task_id, uint32_t client_mode)
{
	m_pdu_header.module_id = SID_FILE;
	m_pdu_header.command_id = CID_FILE_LOGIN_REQ;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
	os.WriteString(user_id);
	os.WriteString(token);
    os.WriteString(task_id);
    os << client_mode;
	WriteHeader();
}

CImPduClientFileLoginRes::CImPduClientFileLoginRes(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_result;
	m_task_id = is.ReadString(m_task_id_len);
	PARSE_PACKET_ASSERT
}

CImPduClientFileLoginRes::CImPduClientFileLoginRes(uint32_t result, const char* task_id)
{
	m_pdu_header.module_id = SID_FILE;
	m_pdu_header.command_id = CID_FILE_LOGIN_RES;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << result;
	os.WriteString(task_id);
	WriteHeader();
}

CImPduClientFileRequest::CImPduClientFileRequest(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_from_id = is.ReadString(m_from_id_len);
	m_to_id = is.ReadString(m_to_id_len);
	m_file_name = is.ReadString(m_file_name_len);
	is >> m_file_size;
    is >> m_trans_mode;
	PARSE_PACKET_ASSERT
}

CImPduClientFileRequest::CImPduClientFileRequest(const char* from_id, const char* to_id,
		const char* file_name, uint32_t file_size, uint32_t trans_mode)
{
	m_pdu_header.module_id = SID_FILE;
	m_pdu_header.command_id = CID_FILE_REQUEST;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(from_id);
	os.WriteString(to_id);
	os.WriteString(file_name);
	os << file_size;
    os << trans_mode;
	WriteHeader();
}

CImPduClientFileResponse::CImPduClientFileResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

    is >> m_result;
	m_from_id = is.ReadString(m_from_id_len);
	m_to_id = is.ReadString(m_to_id_len);
	m_file_name = is.ReadString(m_file_name_len);
    m_task_id = is.ReadString(m_task_id_len);
	
    is >> m_ip_addr_cnt;
    
    if (m_ip_addr_cnt > 0)
    {
        m_ip_addr_list = new ip_addr_t [m_ip_addr_cnt];
		ALLOC_FAIL_ASSERT(m_ip_addr_list);
        
		for (uint32_t i = 0; i < m_ip_addr_cnt; i++) {
            m_ip_addr_list[i].ip = is.ReadString(m_ip_addr_list[i].ip_len);
            is >> m_ip_addr_list[i].port;
		}
    }
    
    is >> m_trans_mode;
	PARSE_PACKET_ASSERT
}

CImPduClientFileResponse::CImPduClientFileResponse(uint32_t result, const char* from_id,
                const char* to_id, const char* file_name, const char* task_id,
                const list<svr_ip_addr_t>* ip_addr_list , uint32_t trans_mode)
{
	m_pdu_header.module_id = SID_FILE;
	m_pdu_header.command_id = CID_FILE_RESPONSE;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    os << result;
	os.WriteString(from_id);
	os.WriteString(to_id);
	os.WriteString(file_name);
    os.WriteString(task_id);
	
    uint32_t ip_addr_cnt = 0;
    
    if (ip_addr_list)
    {
        ip_addr_cnt = ip_addr_list->size();
        os << ip_addr_cnt;
        for (list<svr_ip_addr_t>::const_iterator it = ip_addr_list->begin(); it != ip_addr_list->end(); it++)
        {
            const svr_ip_addr_t& ip_addr = *it;
            os.WriteString(ip_addr.ip.c_str(), ip_addr.ip.length());
            os << ip_addr.port;
        }
    }
    else
    {
        os << ip_addr_cnt;
    }
    
    
    os << trans_mode;
	WriteHeader();
}

CImPduClientFileNotify::CImPduClientFileNotify(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    
	m_from_id = is.ReadString(m_from_id_len);
	m_to_id = is.ReadString(m_to_id_len);
	m_file_name = is.ReadString(m_file_name_len);
    is >> m_file_size;
    m_task_id = is.ReadString(m_task_id_len);
	
    is >> m_ip_addr_cnt;
    
    if (m_ip_addr_cnt > 0)
    {
        m_ip_addr_list = new ip_addr_t [m_ip_addr_cnt];
		ALLOC_FAIL_ASSERT(m_ip_addr_list);
        
		for (uint32_t i = 0; i < m_ip_addr_cnt; i++) {
            m_ip_addr_list[i].ip = is.ReadString(m_ip_addr_list[i].ip_len);
            is >> m_ip_addr_list[i].port;
		}
    }

    is >> m_trans_mode;
    is >> m_offline_ready;
	PARSE_PACKET_ASSERT
}

CImPduClientFileNotify::CImPduClientFileNotify(const char* from_id, const char* to_id,
            const char* file_name, uint32_t file_size, const char* task_id,
            const list<svr_ip_addr_t>* ip_addr_list, uint32_t trans_mode, uint32_t offline_ready)
{
	m_pdu_header.module_id = SID_FILE;
	m_pdu_header.command_id = CID_FILE_NOTIFY;
    
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
	os.WriteString(from_id);
	os.WriteString(to_id);
	os.WriteString(file_name);
    os << file_size;
    os.WriteString(task_id);
    
	uint32_t ip_addr_cnt = 0;
    
    if (ip_addr_list)
    {
        ip_addr_cnt = ip_addr_list->size();
        os << ip_addr_cnt;
        for (list<svr_ip_addr_t>::const_iterator it = ip_addr_list->begin(); it != ip_addr_list->end(); it++)
        {
            const svr_ip_addr_t& ip_addr = *it;
            os.WriteString(ip_addr.ip.c_str(), ip_addr.ip.length());
            os << ip_addr.port;
        }
    }
    else
    {
        os << ip_addr_cnt;
    }
    
    os << trans_mode;
    os << offline_ready;
	WriteHeader();
}


CImPduClientFileHasOfflineReq::CImPduClientFileHasOfflineReq(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
}

CImPduClientFileHasOfflineReq::CImPduClientFileHasOfflineReq()
{
	m_pdu_header.module_id = SID_FILE;
	m_pdu_header.command_id = CID_FILE_HAS_OFFLINE_REQ;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	WriteHeader();
}

CImPduClientFileHasOfflineRes::CImPduClientFileHasOfflineRes(uchar_t* buf, uint32_t len)
{
	m_file_list = NULL;
    m_ip_addr_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_file_cnt;
	if (m_file_cnt > 0) {
		m_file_list = new client_offline_file_t [m_file_cnt];
		ALLOC_FAIL_ASSERT(m_file_list);

		for (uint32_t i = 0; i < m_file_cnt; i++) {
            m_file_list[i].from_id_url = is.ReadString(m_file_list[i].from_id_len);
            m_file_list[i].task_id = is.ReadString(m_file_list[i].task_id_len);
            m_file_list[i].file_name = is.ReadString(m_file_list[i].file_name_len);
            is >> m_file_list[i].file_size;
		}
	}
    
    is >> m_ip_addr_cnt;
    
    if (m_ip_addr_cnt > 0)
    {
        m_ip_addr_list = new ip_addr_t [m_ip_addr_cnt];
		ALLOC_FAIL_ASSERT(m_ip_addr_list);
        
		for (uint32_t i = 0; i < m_ip_addr_cnt; i++) {
            m_ip_addr_list[i].ip = is.ReadString(m_ip_addr_list[i].ip_len);
            is >> m_ip_addr_list[i].port;
		}
    }

	PARSE_PACKET_ASSERT
}

CImPduClientFileHasOfflineRes::CImPduClientFileHasOfflineRes(uint32_t file_cnt, offline_file_t* serv_file_list, const list<svr_ip_addr_t>* ip_addr_list)
{
	m_file_list = NULL;
    m_ip_addr_list = NULL;
	m_pdu_header.module_id = SID_FILE;
	m_pdu_header.command_id = CID_FILE_HAS_OFFLINE_RES;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
	os << file_cnt;
	for (uint32_t i = 0; i < file_cnt; i++) {
        char* user_id_url = idtourl(serv_file_list[i].from_id);
        os.WriteString(user_id_url);
        os.WriteString(serv_file_list[i].task_id, serv_file_list[i].task_id_len);
        os.WriteString(serv_file_list[i].file_name, serv_file_list[i].file_name_len);
        os << serv_file_list[i].file_size;
    }

    uint32_t ip_addr_cnt = 0;

    if (ip_addr_list)
    {
        ip_addr_cnt = ip_addr_list->size();
        os << ip_addr_cnt;
        for (list<svr_ip_addr_t>::const_iterator it = ip_addr_list->begin(); it != ip_addr_list->end(); it++)
        {
            const svr_ip_addr_t& ip_addr = *it;
            os.WriteString(ip_addr.ip.c_str(), ip_addr.ip.length());
            os << ip_addr.port;
        }
    }
    else
    {
        os << ip_addr_cnt;
    }

    WriteHeader();
}

CImPduClientFileHasOfflineRes::~CImPduClientFileHasOfflineRes()
{
    if (m_file_list) {
        delete [] m_file_list;
        m_file_list = NULL;
    }
    if (m_ip_addr_list) {
        delete []m_ip_addr_list;
        m_ip_addr_list= NULL;
    }
}

CImPduClientFileAddOfflineReq::CImPduClientFileAddOfflineReq(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_from_id = is.ReadString(m_from_id_len);
	m_to_id = is.ReadString(m_to_id_len);
    m_task_id = is.ReadString(m_task_id_len);
    m_file_name = is.ReadString(m_file_name_len);
	is >> m_file_size;
	PARSE_PACKET_ASSERT
}

CImPduClientFileAddOfflineReq::CImPduClientFileAddOfflineReq(const char* from_id, const char* to_id,
		const char* task_id, const char* file_name, uint32_t file_size)
{
	m_pdu_header.module_id = SID_FILE;
	m_pdu_header.command_id = CID_FILE_ADD_OFFLINE_REQ;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

    os.WriteString(from_id);
    os.WriteString(to_id);
    os.WriteString(task_id);
    os.WriteString(file_name);
    os << file_size;
    WriteHeader();
}

CImPduClientFileDelOfflineReq::CImPduClientFileDelOfflineReq(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	m_from_id = is.ReadString(m_from_id_len);
	m_to_id = is.ReadString(m_to_id_len);
    m_task_id = is.ReadString(m_task_id_len);
	PARSE_PACKET_ASSERT
}

CImPduClientFileDelOfflineReq::CImPduClientFileDelOfflineReq(const char* from_id, const char* to_id, const char* task_id)
{
	m_pdu_header.module_id = SID_FILE;
	m_pdu_header.command_id = CID_FILE_DEL_OFFLINE_REQ;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(from_id);
	os.WriteString(to_id);
    os.WriteString(task_id);
	WriteHeader();
}


/// yunfan add 2014.8.6
CImPduMsgFileTransferReq::CImPduMsgFileTransferReq(uchar_t* buf, uint32_t len)
{
    ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    
    is >> m_from_id;
    is >> m_to_id;
    m_file_name = is.ReadString(m_file_name_len);
	is >> m_file_length;
    is >> m_transfer_mode;
    m_attach_data = is.ReadData(m_attach_len);
    
	PARSE_PACKET_ASSERT
}

CImPduMsgFileTransferReq::CImPduMsgFileTransferReq(uint32_t from_id, uint32_t to_id, const char* file_name, uint32_t file_length, uint32_t mode, uint32_t attach_len, uchar_t* attach_data)
{
    m_pdu_header.module_id = SID_FILE;
	m_pdu_header.command_id = IM_PDU_TYPE_MSG_FILE_TRANSFER_REQ;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
    
    os << from_id;
    os << to_id;
    os.WriteString(file_name);
    os << file_length;
    os << mode;
    os.WriteData(attach_data, attach_len);
	WriteHeader();
}

CImPduMsgFileTransferReq::~CImPduMsgFileTransferReq()
{
    return ;
}

CImPduMsgFileTransferRsp::CImPduMsgFileTransferRsp(uchar_t* buf, uint32_t len)
{
    ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    
    is >> m_result;
    is >> m_from_id;
    is >> m_to_id;
    m_file_name = is.ReadString(m_file_name_len);
    is >> m_file_length;
    m_task_id = is.ReadString(m_task_id_len);
    is >> m_trans_mode;
    m_attach_data = is.ReadData(m_attach_len);
    
    PARSE_PACKET_ASSERT
}

CImPduMsgFileTransferRsp::CImPduMsgFileTransferRsp(uint32_t result, uint32_t from_id, uint32_t to_id, uint32_t attach_len, uchar_t* attach_data)
{
    m_pdu_header.module_id = SID_OTHER;
	m_pdu_header.command_id = IM_PDU_TYPE_MSG_FILE_TRANSFER_RSP;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
    os << result;
    os << from_id;
    os << to_id;
    os.WriteString("");
    os << 0;
    os.WriteString("");
    os << 0;
    os.WriteData(attach_data, attach_len);
	WriteHeader();
}

CImPduMsgFileTransferRsp::CImPduMsgFileTransferRsp(uint32_t from_id, uint32_t to_id,
        const char* file_name, uint32_t file_len, const char* task_id, uint32_t trans_mode,
        uint32_t attach_len, uchar_t* attach_data)
{
    m_pdu_header.module_id = SID_OTHER;
	m_pdu_header.command_id = IM_PDU_TYPE_MSG_FILE_TRANSFER_RSP;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
    os << 0;
    os << from_id;
    os << to_id;
    os.WriteString(file_name);
    os << file_len;
    os.WriteString(task_id);
    os << trans_mode;

    os.WriteData(attach_data, attach_len);
	WriteHeader();
    
    return ;
}


CImPduClientFileState::CImPduClientFileState(uchar_t* buf, uint32_t len)
{
    ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    
    is >> m_state;
    m_task_id = is.ReadString(m_task_id_len);
    m_user_id = is.ReadString(m_user_id_len);
    
    PARSE_PACKET_ASSERT
}

CImPduClientFileState::CImPduClientFileState(uint32_t state, const char* task_id, const char* user_id)
{
    m_pdu_header.module_id = SID_FILE;
	m_pdu_header.command_id = CID_FILE_STATE;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
    os << state;
    os.WriteString(task_id);
    os.WriteString(user_id);
	WriteHeader();
    
    return ;
}

CImPduClientFileState::~CImPduClientFileState()
{
    return ;
}

CImPduClientFilePullDataReq::CImPduClientFilePullDataReq(uchar_t* buf, uint32_t len)
{
    ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    
	m_task_id = is.ReadString(m_task_id_len);
	m_user_id = is.ReadString(m_user_id_len);
    is >> m_mode;
    is >> m_offset;
	is >> m_datasize;
	PARSE_PACKET_ASSERT
}

CImPduClientFilePullDataReq::CImPduClientFilePullDataReq(const char* task_id, const char* user_id, uint32_t mode, uint32_t offset, uint32_t data_size)
{
    m_pdu_header.module_id = SID_FILE;
	m_pdu_header.command_id = CID_FILE_PULL_DATA_REQ;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
    os.WriteString(task_id);
    os.WriteString(user_id);
    os << mode;
    os << offset;
    os << data_size;
	WriteHeader();

    return ;
}

CImPduClientFilePullDataReq::~CImPduClientFilePullDataReq()
{
    return ;
}

CImPduClientFilePullDataRsp::CImPduClientFilePullDataRsp(uchar_t* buf, uint32_t len)
{
    ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    
    is >> m_result;
	m_task_id = is.ReadString(m_task_id_len);
    m_user_id = is.ReadString(m_user_id_len);
	is >> m_offset;
    m_data = is.ReadData(m_datasize);
	PARSE_PACKET_ASSERT
}

CImPduClientFilePullDataRsp::CImPduClientFilePullDataRsp(uint32_t result)
{
    m_pdu_header.module_id = SID_FILE;
	m_pdu_header.command_id = CID_FILE_PULL_DATA_RSP;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
    os << result;
    os.WriteString("");
    os.WriteString("");
    os << 0;
    os.WriteData((uchar_t*)"", 0);
	WriteHeader();

}

CImPduClientFilePullDataRsp::CImPduClientFilePullDataRsp(const char* task_id, const char* user_id, uint32_t offset, uint32_t data_size, uchar_t* data)
{
    m_pdu_header.module_id = SID_FILE;
	m_pdu_header.command_id = CID_FILE_PULL_DATA_RSP;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
    os << 0;
    os.WriteString(task_id);
    os.WriteString(user_id);
    os << offset;
    os.WriteData(data, data_size);
	WriteHeader();
   	
    return ;
}

CImPduClientFilePullDataRsp::~CImPduClientFilePullDataRsp()
{
    return ;
}
/// yunfan add end


// file pdu for server
#ifndef WIN32
CImPduFileServerIPReq::CImPduFileServerIPReq(uchar_t* buf, uint32_t len)
{
    ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
	PARSE_PACKET_ASSERT
}

CImPduFileServerIPReq::CImPduFileServerIPReq()
{
    m_pdu_header.module_id = SID_OTHER;
	m_pdu_header.command_id = IM_PDU_TYPE_FILE_SERVER_IP_REQUEST;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	WriteHeader();
}

CImPduFileServerIPRsp::CImPduFileServerIPRsp(uchar_t* buf, uint32_t len)
{
    m_ip_list = NULL;
    ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    
    is >> m_ip_cnt;
    if (m_ip_cnt > 0)
    {
        m_ip_list = new ip_addr_t [m_ip_cnt];
		ALLOC_FAIL_ASSERT(m_ip_list);
        
		for (uint32_t i = 0; i < m_ip_cnt; i++) {
            m_ip_list[i].ip = is.ReadString(m_ip_list[i].ip_len);
            is >> m_ip_list[i].port;
		}
    }
    
    PARSE_PACKET_ASSERT
}

CImPduFileServerIPRsp::CImPduFileServerIPRsp(list<svr_ip_addr_t>* ip_list)
{
    m_ip_list = NULL;
    m_pdu_header.module_id = SID_OTHER;
	m_pdu_header.command_id = IM_PDU_TYPE_FILE_SERVER_IP_RESPONSE;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);
    
    if (ip_list != NULL) {
        uint32_t ip_list_cnt = ip_list->size();
        os << ip_list_cnt;
        if (ip_list_cnt > 0) {
            for (list<svr_ip_addr_t>::iterator it = ip_list->begin(); it != ip_list->end(); it++)
            {
                svr_ip_addr_t& ip_addr = *it;
                os.WriteString(ip_addr.ip.c_str(), ip_addr.ip.length());
                os << (*it).port;
            }
        }
    }
    else
    {
        os << 0;
    }
    
    
	WriteHeader();
}

CImPduFileServerIPRsp::~CImPduFileServerIPRsp()
{
    if (m_ip_list) {
        delete []m_ip_list;
        m_ip_list = NULL;
    }
}

CImPduFileNotify::CImPduFileNotify(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
    is >> m_from_id;
    is >> m_to_id;
	m_file_name = is.ReadString(m_file_name_len);
    is >> m_file_size;
    m_task_id = is.ReadString(m_task_id_len);
	is >> m_ip_addr_cnt;
    
    if (m_ip_addr_cnt > 0)
    {
        m_ip_addr_list = new ip_addr_t [m_ip_addr_cnt];
		ALLOC_FAIL_ASSERT(m_ip_addr_list);
        
		for (uint32_t i = 0; i < m_ip_addr_cnt; i++) {
            m_ip_addr_list[i].ip = is.ReadString(m_ip_addr_list[i].ip_len);
            is >> m_ip_addr_list[i].port;
		}
    }
    
    is >> m_trans_mode;
    is >> m_offline_ready;
	PARSE_PACKET_ASSERT
}

CImPduFileNotify::CImPduFileNotify(uint32_t from_id, uint32_t to_id, const char* file_name,
        uint32_t file_size, const char* task_id, const list<svr_ip_addr_t>* ip_addr_list,
                                   uint32_t trans_mode, uint32_t offline_ready)
{
	m_pdu_header.module_id = SID_OTHER;
	m_pdu_header.command_id = IM_PDU_TYPE_FILE_NOTIFY;
    
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

    os << from_id;
    os << to_id;
	os.WriteString(file_name);
    os << file_size;
    os.WriteString(task_id);
	
    uint32_t ip_addr_cnt = 0;
    
    if (ip_addr_list)
    {
        ip_addr_cnt = ip_addr_list->size();
        os << ip_addr_cnt;
        for (list<svr_ip_addr_t>::const_iterator it = ip_addr_list->begin(); it != ip_addr_list->end(); it++)
        {
            const svr_ip_addr_t& ip_addr = *it;
            os.WriteString(ip_addr.ip.c_str(), ip_addr.ip.length());
            os << ip_addr.port;
        }
    }
    else
    {
        os << ip_addr_cnt;
    }
    
    os << trans_mode;
    os << offline_ready;
	WriteHeader();
}

CImPduFileHasOfflineReq::CImPduFileHasOfflineReq(uint32_t req_user_id, uint32_t attach_len,
                                                 uchar_t* attach_data)
{
	m_pdu_header.command_id = IM_PDU_TYPE_FILE_HAS_OFFLINE_REQ;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << req_user_id;
    os.WriteData(attach_data, attach_len);
	WriteHeader();
}

CImPduFileHasOfflineRes::CImPduFileHasOfflineRes(uchar_t* buf, uint32_t len)
{
	m_file_list = NULL;
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	is >> m_req_user_id;
	is >> m_file_cnt;
	if (m_file_cnt > 0) {
		m_file_list = new offline_file_t [m_file_cnt];
		ALLOC_FAIL_ASSERT(m_file_list);

        for (uint32_t i = 0; i < m_file_cnt; i++)
        {
            is >> m_file_list[i].from_id;
            m_file_list[i].task_id = is.ReadString(m_file_list[i].task_id_len);
            m_file_list[i].file_name = is.ReadString(m_file_list[i].file_name_len);
            is >> m_file_list[i].file_size;
        }
	}
    m_attach_data = is.ReadData(m_attach_len);

	PARSE_PACKET_ASSERT
}

CImPduFileHasOfflineRes::~CImPduFileHasOfflineRes()
{
	if (m_file_list) {
		delete [] m_file_list;
		m_file_list = NULL;
	}
}

CImPduFileAddOfflineReq::CImPduFileAddOfflineReq(uint32_t req_user_id, uint32_t from_id,
        uint32_t to_id, const char* task_id, const char* file_name, uint32_t file_size)
{
	m_pdu_header.command_id = IM_PDU_TYPE_FILE_ADD_OFFLINE_REQ;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << req_user_id;
	os << from_id;
	os << to_id;
	os.WriteString(task_id);
    os.WriteString(file_name);
	os << file_size;
	WriteHeader();
}

CImPduFileDelOfflineReq::CImPduFileDelOfflineReq(uint32_t req_user_id,
                    uint32_t from_id, uint32_t to_id, const char* task_id)
{
	m_pdu_header.command_id = IM_PDU_TYPE_FILE_DEL_OFFLINE_REQ;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << req_user_id;
	os << from_id;
	os << to_id;
    os.WriteString(task_id);
	WriteHeader();
}



#endif // WIN32
