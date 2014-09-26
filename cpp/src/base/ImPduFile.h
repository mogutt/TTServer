/*
 * ImPduFile.h
 *
 *  Created on: 2013-12-9
 *      Author: ziteng@mogujie.com
 */

#ifndef IMPDUFILE_H_
#define IMPDUFILE_H_

#include "ImPduBase.h"


#define PARTIAL_FILE_SIZE		40960

//client & msg_server  trans_mode
enum {
	FILE_TYPE_ONLINE 	= 1,
	FILE_TYPE_OFFLINE	= 2,
};

enum CLIENT_FILE_STATE{
    CLIENT_FILE_PEER_READY = 0,
    CLIENT_FILE_CANCEL,
    CLIENT_FILE_REFUSE,
    CLIENT_FILE_DONE,
};

//client & file_server
enum {
    CLIENT_REALTIME_SENDER = 1,
    CLIENT_REALTIME_RECVER,
    CLIENT_OFFLINE_UPLOAD,
    CLIENT_OFFLINE_DOWNLOAD,
};

// to/from FileServer
#define IM_PDU_TYPE_CLIENT_FILE_LOGIN_REQ				500
#define IM_PDU_TYPE_CLIENT_FILE_LOGIN_RES				501
#define IM_PDU_TYPE_CLIENT_FILE_DATA					502
#define IM_PDU_TYPE_CLIENT_FILE_UPLOAD_FINISH			503
#define IM_PDU_TYPE_CLIENT_FILE_GET_OFFLINE_REQ			504
// yunfan add 2014-8-5
#define IM_PDU_TYPE_CLIENT_FILE_STATE                   532
#define IM_PDU_TYPE_CLIENT_FILE_PULL_DATA_REQ           533
#define IM_PDU_TYPE_CLIENT_FILE_PULL_DATA_RSP           534
// yunfan add end

// to/from MsgServer
#define IM_PDU_TYPE_CLIENT_FILE_REQUEST					510
#define IM_PDU_TYPE_CLIENT_FILE_RESPONSE				511
#define IM_PDU_TYPE_CLIENT_FILE_NOTIFY                  512
#define IM_PDU_TYPE_CLIENT_FILE_HAS_OFFLINE_REQ			516
#define IM_PDU_TYPE_CLIENT_FILE_HAS_OFFLINE_RES			517
#define IM_PDU_TYPE_CLIENT_FILE_ADD_OFFLINE_REQ			518
#define IM_PDU_TYPE_CLIENT_FILE_DEL_OFFLINE_REQ			519
// server pdu
#define IM_PDU_TYPE_FILE_REQUEST					550
#define IM_PDU_TYPE_FILE_RESPONSE					551
#define IM_PDU_TYPE_FILE_NOTIFY                     552
#define IM_PDU_TYPE_FILE_SERVER_IP_REQUEST          553
#define IM_PDU_TYPE_FILE_SERVER_IP_RESPONSE         554

#define IM_PDU_TYPE_MSG_FILE_TRANSFER_REQ           565
#define IM_PDU_TYPE_MSG_FILE_TRANSFER_RSP           566
#define IM_PDU_TYPE_FILE_RECV_READY					552

#define IM_PDU_TYPE_FILE_DOWNLOAD_OFFLINE_NOTIFY	555

// to storage server
#define IM_PDU_TYPE_FILE_HAS_OFFLINE_REQ			561
#define IM_PDU_TYPE_FILE_HAS_OFFLINE_RES			562
#define IM_PDU_TYPE_FILE_ADD_OFFLINE_REQ			563
#define IM_PDU_TYPE_FILE_DEL_OFFLINE_REQ			564

/// yunfan add 2014.8.15
#define IM_PDU_TYPE_FILE_SERVER_IP_REQUEST          553
#define IM_PDU_TYPE_FILE_SERVER_IP_RESPONSE         554

#define IM_PDU_TYPE_MSG_FILE_TRANSFER_REQ           565
#define IM_PDU_TYPE_MSG_FILE_TRANSFER_RSP           566
/// yunfan add end


enum {
    FILE_SERVER_ERRNO_OK = 0,
    FILE_SERVER_ERRNO_CREATE_TASK_ID_ERROR = 1,
    FILE_SERVER_ERRNO_CREATE_TASK_ERROR = 2,
    FILE_SERVER_ERRNO_LOGIN_INVALID_TOKEN = 3,
    FILE_SERVER_ERRNO_INVALID_USER_FOR_TASK = 4,
    FILE_SERVER_ERRNO_PULL_DATA_WITH_INVALID_TASK_ID = 5,
    FILE_SERVER_ERRNO_PULL_DATA_ILLIEAGE_USER = 6,
    FILE_SERVER_ERRNO_PULL_DATA_MKDIR_ERROR = 7,
    FILE_SERVER_ERRNO_PULL_DATA_OPEN_FILE_ERROR = 8,
    FILE_SERVER_ERRNO_PULL_DATA_READ_FILE_HEADER_ERROR = 9,
    FILE_SERVER_ERRNO_PULL_DATA_ALLOC_MEM_ERROR = 10,
    FILE_SERVER_ERRNO_PULL_DATA_SEEK_OFFSET_ERROR = 11,
    FILE_SERVER_ERRNO_PULL_DATA_FINISHED = 12,
};

typedef struct {
	uint32_t 	from_id_len;
	char*		from_id_url;
    uint32_t    task_id_len;
    char*       task_id;
    uint32_t    file_name_len;
    char*       file_name;
	uint32_t	file_size;
} client_offline_file_t;

typedef struct {
	uint32_t	from_id;
	uint32_t    task_id_len;
    char*       task_id;
    uint32_t    file_name_len;
    char*       file_name;
	uint32_t	file_size;
} offline_file_t;

//////// to/from FileServer//////
class DLL_MODIFIER CImPduClientFileLoginReq : public CImPdu
{
public:
	CImPduClientFileLoginReq(uchar_t* buf, uint32_t len);
	CImPduClientFileLoginReq(const char* user_id, const char* token, const char* task_id, uint32_t client_mode);
    virtual ~CImPduClientFileLoginReq() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FILE_LOGIN_REQ; }

	uint32_t GetUserIdLen() { return m_user_id_len; }
	char* GetUserId() { return m_user_id; }
	uint32_t GetTokenLen() { return m_token_len; }
	char* GetToken() { return m_token; }
    /// yunfan add 2014.8.6
    char* GetTaskId() { return m_task_id; }
    uint32_t GetTaskIdLen() { return m_task_id_len; }
    uint32_t GetClientMode() { return m_client_mode; }
    /// yunfan add end
    
private:
	uint32_t	m_user_id_len;
	char*		m_user_id;
	uint32_t	m_token_len;
	char*		m_token;
    /// yunfan add 2014.8.6
    char*       m_task_id; // uuid parsed to 32bytes
    uint32_t    m_task_id_len;
    uint32_t    m_client_mode;    /// realtime_sender, realtime_recver, offline_sender, realtime_recver
    /// yunfan add end
};

class DLL_MODIFIER CImPduClientFileLoginRes : public CImPdu
{
public:
	CImPduClientFileLoginRes(uchar_t* buf, uint32_t len);
	CImPduClientFileLoginRes(uint32_t result, const char* task_id);
	virtual ~CImPduClientFileLoginRes() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FILE_LOGIN_RES; }
	uint32_t GetResult() { return m_result; }
	uint32_t GetTaskIdLen() { return m_task_id_len; }
	char* GetTaskId() { return m_task_id; }
private:
	uint32_t	m_result; // 1 failed, 0 succeed
	uint32_t 	m_task_id_len;
	char*		m_task_id;
};

class DLL_MODIFIER CImPduClientFileData : public CImPdu
{
public:
	CImPduClientFileData(uchar_t* buf, uint32_t len);
	CImPduClientFileData(const char* task_id, const char* from_id, const char* to_id, const char* file_name, uint32_t file_size, uint32_t file_offset, uint32_t file_type, uint32_t file_data_len, uchar_t* file_data);
	virtual ~CImPduClientFileData() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FILE_DATA; }

    uint32_t GetTaskIdLen() { return m_task_id_len; }
    char* GetTasKId() { return m_task_id; }
	uint32_t GetFromIdLen() { return m_from_id_len; }
	char* GetFromId() { return m_from_id; }
	uint32_t GetToIdLen() { return m_to_id_len; }
	char* GetToId() { return m_to_id; }
	uint32_t GetFileNameLen() { return m_file_name_len; }
	char* GetFileName() { return m_file_name; }
	uint32_t GetFileSize() { return m_file_size; }
	uint32_t GetFileOffset() { return m_file_offset; }
	uint32_t GetFileType() { return m_file_type; }
	uint32_t GetFileDataLen() { return m_file_data_len; }
	uchar_t* GetFileData() { return m_file_data; }
private:
    uint32_t    m_task_id_len;
    char*       m_task_id;
	uint32_t	m_from_id_len;
	char*		m_from_id;
	uint32_t	m_to_id_len;
	char* 		m_to_id;
	uint32_t	m_file_name_len;
	char* 		m_file_name;
	uint32_t	m_file_size;
	uint32_t	m_file_offset;
	uint32_t	m_file_type;
	uint32_t	m_file_data_len;
	uchar_t*	m_file_data;
};

class DLL_MODIFIER CImPduClientFileUploadFinish : public CImPdu
{
public:
	CImPduClientFileUploadFinish(uchar_t* buf, uint32_t len);
	CImPduClientFileUploadFinish(const char* from_id, const char* to_id,
			const char* file_path, const char* save_file_name, uint32_t file_size);
	virtual ~CImPduClientFileUploadFinish() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FILE_UPLOAD_FINISH; }
	uint32_t GetFromIdLen() { return m_from_id_len; }
	char* GetFromId() { return m_from_id; }
	uint32_t GetToIdLen() { return m_to_id_len; }
	char* GetToId() { return m_to_id; }
	uint32_t GetFilePathLen() { return m_file_path_len; }
	char* GetFilePath() { return m_file_path; }
	uint32_t GetSaveFilePathLen() { return m_save_file_path_len; }
	char* GetSaveFilePath() { return m_save_file_path; }
	uint32_t GetFileSize() { return m_file_size; }
private:
	uint32_t	m_from_id_len;
	char*		m_from_id;
	uint32_t	m_to_id_len;
	char* 		m_to_id;
	uint32_t 	m_file_path_len;
	char*		m_file_path;
	uint32_t	m_save_file_path_len;
	char*		m_save_file_path;
	uint32_t	m_file_size;
};

class DLL_MODIFIER CImPduClientFileGetOfflineReq : public CImPdu
{
public:
	CImPduClientFileGetOfflineReq(uchar_t* buf, uint32_t len);
	CImPduClientFileGetOfflineReq(const char* from_id, const char* file_name);
	virtual ~CImPduClientFileGetOfflineReq() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FILE_GET_OFFLINE_REQ; }
	uint32_t GetFromIdLen() { return m_from_id_len; }
	char* GetFromId() { return m_from_id; }
	uint32_t GetFilePathLen() { return m_file_path_len; }
	char* GetFilePath() { return m_file_path; }
private:
	uint32_t	m_from_id_len;
	char*		m_from_id;
	uint32_t 	m_file_path_len;
	char*		m_file_path;
};

/////// to/from MsgServer ////////
class DLL_MODIFIER CImPduClientFileRequest : public CImPdu
{
public:
	CImPduClientFileRequest(uchar_t* buf, uint32_t len);
	CImPduClientFileRequest(const char* from_id, const char* to_id, const char* file_name, uint32_t file_size, uint32_t trans_mode);
	virtual ~CImPduClientFileRequest() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FILE_REQUEST; }

	uint32_t GetFromIdLen() { return m_from_id_len; }
	char* GetFromId() { return m_from_id; }
	uint32_t GetToIdLen() { return m_to_id_len; }
	char* GetToId() { return m_to_id; }
	uint32_t GetFileNameLen() { return m_file_name_len; }
	char* GetFileName() { return m_file_name; }
	uint32_t GetFileSize() { return m_file_size; }
    uint32_t GetTransMode() { return m_trans_mode; }
private:
	uint32_t	m_from_id_len;
	char*		m_from_id;
	uint32_t	m_to_id_len;
	char* 		m_to_id;
	uint32_t	m_file_name_len;
	char* 		m_file_name;
	uint32_t	m_file_size;
    uint32_t    m_trans_mode;
};

class DLL_MODIFIER CImPduClientFileResponse : public CImPdu
{
public:
	CImPduClientFileResponse(uchar_t* buf, uint32_t len);
	CImPduClientFileResponse(uint32_t result, const char* from_id, const char* to_id,
                             const char* file_name, const char* task_id,
                             const list<svr_ip_addr_t>* ip_addr_list, uint32_t trans_mode);
	virtual ~CImPduClientFileResponse() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FILE_RESPONSE; }

    uint32_t GetResult() { return m_result; }
	uint32_t GetFromIdLen() { return m_from_id_len; }
	char* GetFromId() { return m_from_id; }
	uint32_t GetToIdLen() { return m_to_id_len; }
	char* GetToId() { return m_to_id; }
	uint32_t GetFileNameLen() { return m_file_name_len; }
	char* GetFileName() { return m_file_name; }
    uint32_t GetTaskIdLen() { return m_task_id_len; }
    char* GetTaskId() { return m_task_id; }
	uint32_t GetIPAddrCnt() { return m_ip_addr_cnt; }
    ip_addr_t* GetIPAddrList() { return m_ip_addr_list; }
    uint32_t GetTransMode() { return m_trans_mode; }
private:
    uint32_t    m_result;
	uint32_t	m_from_id_len;
	char*		m_from_id;
	uint32_t	m_to_id_len;
	char* 		m_to_id;
	uint32_t	m_file_name_len;
	char* 		m_file_name;
    uint32_t    m_task_id_len;
    char*       m_task_id;
	uint32_t    m_ip_addr_cnt;
    ip_addr_t*  m_ip_addr_list;
    uint32_t    m_trans_mode;
};

class DLL_MODIFIER CImPduClientFileNotify : public CImPdu
{
public:
    CImPduClientFileNotify(uchar_t* buf, uint32_t len);
    CImPduClientFileNotify(const char* from_id, const char* to_id, const char* file_name,
            uint32_t file_size, const char* task_id, const list<svr_ip_addr_t>* ip_addr_list,
            uint32_t trans_mode, uint32_t offline_ready);
    virtual ~CImPduClientFileNotify() {}
    
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FILE_NOTIFY; }
    uint32_t GetFromIdLen() { return m_from_id_len; }
	char* GetFromId() { return m_from_id; }
	uint32_t GetToIdLen() { return m_to_id_len; }
	char* GetToId() { return m_to_id; }
	uint32_t GetFileNameLen() { return m_file_name_len; }
	char* GetFileName() { return m_file_name; }
    uint32_t GetFileSize() { return m_file_size; }
    uint32_t GetTaskIdLen() { return m_task_id_len; }
    char* GetTaskId() { return m_task_id; }
	uint32_t GetIPAddrCnt() { return m_ip_addr_cnt; }
    ip_addr_t* GetIPAddrList() { return m_ip_addr_list; }
    uint32_t GetTransMode() { return m_trans_mode; }
    uint32_t GetOfflineReady() { return m_offline_ready; }
private:
    uint32_t	m_from_id_len;
	char*		m_from_id;
	uint32_t	m_to_id_len;
	char* 		m_to_id;
	uint32_t	m_file_name_len;
	char* 		m_file_name;
    uint32_t    m_file_size;
    uint32_t    m_task_id_len;
    char*       m_task_id;
	uint32_t    m_ip_addr_cnt;
    ip_addr_t*  m_ip_addr_list;
    uint32_t    m_trans_mode;
    uint32_t    m_offline_ready;            //1: True 0: False
};

class DLL_MODIFIER CImPduClientFileHasOfflineReq : public CImPdu
{
public:
	CImPduClientFileHasOfflineReq(uchar_t* buf, uint32_t len);
	CImPduClientFileHasOfflineReq();
	virtual ~CImPduClientFileHasOfflineReq() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FILE_HAS_OFFLINE_REQ; }
};

class DLL_MODIFIER CImPduClientFileHasOfflineRes : public CImPdu
{
public:
	CImPduClientFileHasOfflineRes(uchar_t* buf, uint32_t len);
	CImPduClientFileHasOfflineRes(uint32_t file_cnt = 0, offline_file_t* serv_file_list = NULL,
                                   const list<svr_ip_addr_t>* ip_addr_list = NULL);
	virtual ~CImPduClientFileHasOfflineRes();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FILE_HAS_OFFLINE_RES; }
	uint32_t GetFileCnt() { return m_file_cnt; }
	client_offline_file_t* GetFileList() { return m_file_list; }
    uint32_t GetIPAddrCnt() { return m_ip_addr_cnt; }
    ip_addr_t* GetIPAddrList() { return m_ip_addr_list; }
private:
	uint32_t 				m_file_cnt;
	client_offline_file_t* 	m_file_list;
    uint32_t                m_ip_addr_cnt;
    ip_addr_t*              m_ip_addr_list;
};

class DLL_MODIFIER CImPduClientFileAddOfflineReq : public CImPdu
{
public:
	CImPduClientFileAddOfflineReq(uchar_t* buf, uint32_t len);
	CImPduClientFileAddOfflineReq(const char* from_id, const char* to_id, const char* task_id, const char* file_name, uint32_t file_size);
	virtual ~CImPduClientFileAddOfflineReq() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FILE_ADD_OFFLINE_REQ; }
	uint32_t GetFromIdLen() { return m_from_id_len; }
	char* GetFromId() { return m_from_id; }
	uint32_t GetToIdLen() { return m_to_id_len; }
	char* GetToId() { return m_to_id; }
    uint32_t GetTaskIdLen() { return m_task_id_len; }
    char*   GetTaskId() { return m_task_id; }
	uint32_t GetFileNameLen() { return m_file_name_len; }
	char* GetFileName() { return m_file_name; }
	uint32_t GetFileSize() { return m_file_size; }
private:
	uint32_t	m_from_id_len;
	char* 		m_from_id;
	uint32_t	m_to_id_len;
	char* 		m_to_id;
    uint32_t    m_task_id_len;
    char*       m_task_id;
    uint32_t    m_file_name_len;
    char*       m_file_name;
    uint32_t	m_file_size;
    
};

class DLL_MODIFIER CImPduClientFileDelOfflineReq : public CImPdu
{
public:
	CImPduClientFileDelOfflineReq(uchar_t* buf, uint32_t len);
	CImPduClientFileDelOfflineReq(const char* from_id, const char* to_id, const char* task_id);
	virtual ~CImPduClientFileDelOfflineReq() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FILE_DEL_OFFLINE_REQ; }
	uint32_t GetFromIdLen() { return m_from_id_len; }
	char* GetFromId() { return m_from_id; }
	uint32_t GetToIdLen() { return m_to_id_len; }
	char* GetToId() { return m_to_id; }
	uint32_t GetTaskIdLen() { return m_task_id_len; }
    char* GetTaskId() { return m_task_id; }
private:
	uint32_t	m_from_id_len;
	char* 		m_from_id;
	uint32_t	m_to_id_len;
	char* 		m_to_id;
    uint32_t    m_task_id_len;
    char*       m_task_id;
};

/// yunfan add 2014.8.6
/// from message server to file server
/// tell file server sb. wants to transfer file to someone else
/// params transfer type: realtime / offline or mobile
/// params src userid and dst userid
/// params file info: size type....
class CImPduMsgFileTransferReq : public CImPdu
{
public:
    CImPduMsgFileTransferReq(uchar_t* buf, uint32_t len);
    CImPduMsgFileTransferReq(uint32_t from_id, uint32_t to_id, const char* file_name, uint32_t file_length, uint32_t mode, uint32_t attach_len, uchar_t* attach_data);
    virtual ~CImPduMsgFileTransferReq();
    
public:
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_MSG_FILE_TRANSFER_REQ; }
	uint32_t GetFromId() { return m_from_id; }
	uint32_t GetToId() { return m_to_id; }
    uint32_t GetFileNameLen() { return m_file_name_len; }
    char* GetFileName() { return m_file_name; }
	uint32_t GetFileLength() { return m_file_length; }
    uint32_t GetTransferMode() { return m_transfer_mode; }
    uint32_t GetAttachLen() { return m_attach_len; }
    uchar_t* GetAttachData() { return m_attach_data; }
private:
    uint32_t    m_from_id;
	uint32_t 	m_to_id;
    uint32_t    m_file_name_len;
    char*       m_file_name;
    uint32_t    m_file_length;
    uint32_t    m_transfer_mode;
    uint32_t    m_attach_len;
    uchar_t*    m_attach_data;
};

class CImPduMsgFileTransferRsp : public CImPdu
{
public:
    CImPduMsgFileTransferRsp(uchar_t* buf, uint32_t len);
    CImPduMsgFileTransferRsp(uint32_t result, uint32_t from_id, uint32_t to_id, uint32_t attach_len, uchar_t* attach_data);
    CImPduMsgFileTransferRsp(uint32_t from_id, uint32_t to_id, const char* file_name, uint32_t file_len, const char* task_id,
                             uint32_t trans_mode, uint32_t attach_len, uchar_t* attach_data);
    virtual ~CImPduMsgFileTransferRsp() {}
    
public:
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_MSG_FILE_TRANSFER_RSP; }
    uint32_t GetResult() { return m_result; }
	char* GetTaskId() { return m_task_id; }
    uint32_t GetTaskIdLen() { return m_task_id_len; }
    uint32_t GetFromId() { return m_from_id; }
    uint32_t GetToId() { return m_to_id; }
    
    uint32_t GetFileNameLen() { return m_file_name_len; }
    char* GetFileName() { return m_file_name; }
    uint32_t GetFileLen() { return m_file_length; }
    uint32_t GetTransMode() { return m_trans_mode; }
    uint32_t GetAttachLen() { return m_attach_len; }
    uchar_t* GetAttachData() { return m_attach_data; }

private:
    uint32_t    m_result; // 0 OK
    uint32_t    m_from_id;
    uint32_t    m_to_id;
    uint32_t    m_file_name_len;
    char*       m_file_name;
    uint32_t    m_file_length;
    uint32_t    m_task_id_len;        // 0 failed
    char*       m_task_id;            // null failed
    uint32_t    m_trans_mode;         // 0 failed
    uint32_t    m_attach_len;         // 0 failed
    uchar_t*    m_attach_data;        // null failed
};


class DLL_MODIFIER CImPduClientFileState : public CImPdu
{
public:
    CImPduClientFileState(uchar_t* buf, uint32_t len);
    CImPduClientFileState(uint32_t state, const char* task_id, const char* user_id);
    virtual ~CImPduClientFileState();
    
public:
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FILE_STATE; }
    uint32_t GetState() { return m_state; }
    uint32_t GetTaskIdLen() { return m_task_id_len; }
    char* GetTaskId() { return m_task_id; }
    uint32_t GetUserIdLen() { return m_user_id_len; }
    char* GetUserId() { return m_user_id; }
    
private:
    uint32_t            m_state;
    uint32_t            m_task_id_len;
    char*               m_task_id;
    uint32_t            m_user_id_len;
    char*               m_user_id;
};

class DLL_MODIFIER CImPduClientFilePullDataReq : public CImPdu
{
public:
    CImPduClientFilePullDataReq(uchar_t* buf, uint32_t len);
    CImPduClientFilePullDataReq(const char* task_id, const char* user_id, uint32_t mode, uint32_t offset, uint32_t data_size);
    virtual ~CImPduClientFilePullDataReq();
    
public:
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FILE_PULL_DATA_REQ; }
    uint32_t GetTaskIdLen() { return m_task_id_len; }
    char* GetTaskId() { return m_task_id; }
    uint32_t GetUserIdLen() { return m_user_id_len; }
    char* GetUserId() { return m_user_id; }
    uint32_t GetDataSize() { return m_datasize; }
    uint32_t GetOffset() { return m_offset; }
    uint32_t GetMode() { return m_mode; }
    
private:
    uint32_t m_task_id_len;
    char* m_task_id;
    uint32_t m_user_id_len;
    char*    m_user_id;
    uint32_t m_mode;
    uint32_t m_offset;
    uint32_t m_datasize;
};

class DLL_MODIFIER CImPduClientFilePullDataRsp : public CImPdu
{
public:
    CImPduClientFilePullDataRsp(uchar_t* buf, uint32_t len);
    CImPduClientFilePullDataRsp(uint32_t result);
    CImPduClientFilePullDataRsp(const char* task_id, const char* user_id, uint32_t offset, uint32_t data_size, uchar_t* data);
    virtual ~CImPduClientFilePullDataRsp();
    
public:
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_CLIENT_FILE_PULL_DATA_RSP; }
    uint32_t GetResult() { return m_result; }
    uint32_t GetTaskIdLen() { return m_task_id_len; }
    char* GetTaskId() { return m_task_id; }
    uint32_t GetUserIdLen() { return m_user_id_len; }
    char* GetUserId() { return m_user_id; }
    uint32_t GetOffset() { return m_offset; }
    uint32_t GetDataSize() { return m_datasize; }
    void* GetData() { return m_data; }
    
private:
    uint32_t m_result; // 0 OK
    uint32_t m_task_id_len;
    char* m_task_id;
    uint32_t m_user_id_len;
    char* m_user_id;
    uint32_t m_offset;
    uint32_t m_datasize;
    uchar_t* m_data;
};

class CImPduFileServerIPReq : public CImPdu
{
public:
    CImPduFileServerIPReq(uchar_t* buf, uint32_t len);
    CImPduFileServerIPReq();
    virtual ~CImPduFileServerIPReq(){}

    virtual uint16_t GetPduType() { return IM_PDU_TYPE_FILE_SERVER_IP_REQUEST; }
};

class CImPduFileServerIPRsp : public CImPdu
{
public:
    CImPduFileServerIPRsp(uchar_t* buf, uint32_t len);
    CImPduFileServerIPRsp(list<svr_ip_addr_t>* ip_list);
    virtual ~CImPduFileServerIPRsp();

    virtual uint16_t GetPduType() { return IM_PDU_TYPE_FILE_SERVER_IP_RESPONSE; }

    uint32_t GetIPCnt() { return m_ip_cnt; }
    ip_addr_t* GetIPList() { return m_ip_list; }
private:
    uint32_t    m_ip_cnt;
    ip_addr_t*  m_ip_list;
};
/// yunfan add end

////// for server pdu ////
#ifndef WIN32
class CImPduFileNotify : public CImPdu
{
public:
    CImPduFileNotify(uchar_t* buf, uint32_t len);
    CImPduFileNotify(uint32_t from_id, uint32_t to_id, const char* file_name,
            uint32_t file_size, const char* task_id, const list<svr_ip_addr_t>* ip_addr_list,
            uint32_t trans_mode, uint32_t offline_ready);
    virtual ~CImPduFileNotify() {}
    
    virtual uint16_t GetPduType() { return IM_PDU_TYPE_FILE_NOTIFY; }
	uint32_t GetFromId() { return m_from_id; }
	uint32_t GetToId() { return m_to_id; }
	uint32_t GetFileNameLen() { return m_file_name_len; }
	char* GetFileName() { return m_file_name; }
    uint32_t GetFileSize() { return m_file_size; }
    uint32_t GetTaskIdLen() { return m_task_id_len; }
    char* GetTaskId() { return m_task_id; }
	uint32_t GetIPAddrCnt() { return m_ip_addr_cnt; }
    ip_addr_t* GetIPAddrList() { return m_ip_addr_list; }
    uint32_t GetTransMode() { return m_trans_mode; }
    uint32_t GetOfflineReady() { return m_offline_ready; }
private:
    uint32_t	m_from_id;
	uint32_t	m_to_id;
	uint32_t	m_file_name_len;
	char* 		m_file_name;
    uint32_t    m_file_size;
    uint32_t    m_task_id_len;
    char*       m_task_id;
	uint32_t    m_ip_addr_cnt;
    ip_addr_t*  m_ip_addr_list;
    uint32_t    m_trans_mode;
    uint32_t    m_offline_ready;
};


// to storage server
class CImPduFileHasOfflineReq : public CImPdu
{
public:
	CImPduFileHasOfflineReq(uint32_t req_user_id, uint32_t attach_len, uchar_t* attach_data);
	virtual ~CImPduFileHasOfflineReq() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_FILE_HAS_OFFLINE_REQ; }
};

class CImPduFileHasOfflineRes : public CImPdu
{
public:
	CImPduFileHasOfflineRes(uchar_t* buf, uint32_t len);
	virtual ~CImPduFileHasOfflineRes();

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_FILE_HAS_OFFLINE_RES; }
	uint32_t GetReqUserId() { return m_req_user_id; }
	uint32_t GetFileCnt() { return m_file_cnt; }
	offline_file_t* GetFileList() { return m_file_list; }
    uint32_t GetAttachLen() { return m_attach_len; }
    uchar_t* GetAttachData() { return m_attach_data; }
private:
	uint32_t			m_req_user_id;
	uint32_t 			m_file_cnt;
	offline_file_t* 	m_file_list;
    uint32_t            m_attach_len;
    uchar_t*            m_attach_data;
};

class CImPduFileAddOfflineReq : public CImPdu
{
public:
	CImPduFileAddOfflineReq(uint32_t req_user_id, uint32_t from_id, uint32_t to_id,
                        const char* task_id, const char* file_name, uint32_t file_size);
	virtual ~CImPduFileAddOfflineReq() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_FILE_ADD_OFFLINE_REQ; }
};

class CImPduFileDelOfflineReq : public CImPdu
{
public:
	CImPduFileDelOfflineReq(uint32_t req_user_id, uint32_t from_id, uint32_t to_id, const char* task_id);
	virtual ~CImPduFileDelOfflineReq() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_FILE_DEL_OFFLINE_REQ; }
};



#endif	// WIN32

#endif /* IMPDUFILE_H_ */
