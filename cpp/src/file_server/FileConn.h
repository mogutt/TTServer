/*
 * FileConn.h
 *
 *  Created on: 2013-12-9
 *      Author: ziteng@mogujie.com
 */

#ifndef FILECONN_H_
#define FILECONN_H_

#include "imconn.h"
#include "impdu.h"
#include <deque>
#include <pthread.h>
#include <deque>
#include "file_server_util.h"


typedef struct {
	FILE* 		fp;
	string		file_path;
	string		save_path;
	string 		peer_user_id;
	uint32_t	file_size;
	uint32_t	transfer_size;
} file_stat_t;


typedef map<string, file_stat_t*> FileMap_t;

/// yunfan add 2014.8.6


typedef map<std::string, transfer_task_t*> TaskMap_t; // on client connect
/// yunfan add end

class CFileConn : public CImConn
{
public:
	CFileConn();
	virtual ~CFileConn();

	virtual void Close();

	void OnConnect(net_handle_t handle);
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);

	virtual void OnWrite();
	virtual void HandlePdu(CImPdu* pPdu);
    
private:
	int _HandleClientFileLoginReq(CImPduClientFileLoginReq* pPdu);
	
    /// yunfan add 2014.8.6
	int _HandleMsgFileTransferReq(CImPduMsgFileTransferReq* pPdu);
	int _HandleClientFileStates(CImPduClientFileState* pPdu);
	int _HandleClientFilePullFileReq(CImPduClientFilePullDataReq* pPdu);
	int _HandleClientFilePullFileRsp(CImPduClientFilePullDataRsp *pPdu);
	int _StatesNotify(int state, const char* task_id, uint32_t user_id, CImConn* pConn);
	int _HandleGetServerAddressReq(CImPduFileServerIPReq* pPdu);
    /// yunfan add end
    
	bool _IsAuth() { return m_bAuth; }
    
    /// yunfan add 2014.8.18
private:
    int _PreUpload(const char* task_id);
//  int _DoUpload(const char* task_id);
    /// yunan add end
    

private:
	bool		m_bAuth;
	uint32_t	m_user_id;
	string 		m_user_id_url;
	FileMap_t	m_save_file_map;
    TaskMap_t   m_file_task_map;
	list<file_stat_t*>	m_send_file_list;
    
    /// yunfan add 2014.8.7
    std::deque<CImPduClientFileData*> m_filedatad_queue;
    /// yunfan add end
};

void init_file_conn(std::list<svr_ip_addr_t>&, uint32_t timeout);

#endif /* FILECONN_H_ */
