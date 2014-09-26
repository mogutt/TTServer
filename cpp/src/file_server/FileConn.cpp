/*
 * FileConn.cpp
 *
 *  Created on: 2013-12-9
 *      Author: ziteng@mogujie.com
 */

#include "FileConn.h"
#include <sys/stat.h>

static ConnMap_t g_file_conn_map; // connection with others, on connect insert...
static UserMap_t g_file_user_map; // after user login, insert...
/// yunfan add 2014.8.6
static TaskMap_t g_file_task_map;
static pthread_rwlock_t g_file_task_map_lock = PTHREAD_RWLOCK_INITIALIZER;
/// yunfan add end

static char g_current_save_path[BUFSIZ];

/// yunfan add 2014.8.12
static std::list<svr_ip_addr_t> g_addr;
uint16_t g_listen_port = 0;
uint32_t g_task_timeout = 3600;
#define SEGMENT_SIZE 65536
/// yunfan add end

void file_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	uint64_t cur_time = get_tick_count();
	for (ConnMap_t::iterator it = g_file_conn_map.begin(); it != g_file_conn_map.end(); ) {
		ConnMap_t::iterator it_old = it;
		it++;

		CFileConn* pConn = (CFileConn*)it_old->second;
		pConn->OnTimer(cur_time);
	}
}

void release_task(const char* task_id)
{
    /// should be locked
    if (NULL == task_id) {
        return ;
    }
    
    pthread_rwlock_wrlock(&g_file_task_map_lock);
    TaskMap_t::iterator iter = g_file_task_map.find(task_id);
    if (g_file_task_map.end() == iter) {
        pthread_rwlock_unlock(&g_file_task_map_lock);
        return ;
    }
    transfer_task_t* t = iter->second;
    g_file_task_map.erase(iter);
    pthread_rwlock_unlock(&g_file_task_map_lock);
//    printf("-------ERASE TASK %s----------\n", t->task_id.c_str());
    
    t->lock(__LINE__);
    t->release();
    t->unlock(__LINE__);
    
    delete t;
    t = NULL;
    
    return ;
}

void file_task_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* lparam)
{
    pthread_rwlock_wrlock(&g_file_task_map_lock);
    
    for (TaskMap_t::iterator iter = g_file_task_map.begin(); iter != g_file_task_map.end(); ) {
    	// check if self-destroy == true
    	// then delete task
    	if (true == iter->second->self_destroy) {
    		iter->second->lock(__LINE__);
    		iter->second->release();
    		iter->second->unlock(__LINE__);

            if (iter->second) {
                delete iter->second;
                iter->second = NULL;
            }
    		
    		// remove task from map
    		g_file_task_map.erase(iter++);
    	} else { // self-destroy not true
    		// check if timeout
    		long esp = time(NULL) - iter->second->create_time;
    		if (esp > g_task_timeout) {
    			// set self_destory true
    			// then continue;
    			// next round, it will be deleted
    			iter->second->self_destroy = true;
    		}
    		++iter;
    	}
    }

    pthread_rwlock_unlock(&g_file_task_map_lock);
    
    return ;
}

void init_file_conn(std::list<svr_ip_addr_t>& q, uint32_t timeout)
{
    /// yunfan add 2014.8.12
    g_addr = q;
    g_task_timeout = timeout;
    /// yunfan add end
    
	char work_path[BUFSIZ];
	if(!getcwd(work_path, BUFSIZ)) {
		log("getcwd failed\n");
	} else {
		snprintf(g_current_save_path, BUFSIZ, "%s/offline_file", work_path);
	}

	log("save offline files to %s\n", g_current_save_path);

	int ret = mkdir(g_current_save_path, 0755);
	if ( (ret != 0) && (errno != EEXIST) ) {
		log("!!!mkdir failed to save offline files\n");
	}

	netlib_register_timer(file_conn_timer_callback, NULL, 1000);
    netlib_register_timer(file_task_timer_callback, NULL, 10000);
}

/// yunfan add 2014.8.7
/// offline file upload
/// file-svr will send pull-data-req to sender
/// then wait for sender's rsp
void* _DoUpload(void* lparam)
{
    if (NULL == lparam) {
        return NULL;
    }
    transfer_task_t* t = reinterpret_cast<transfer_task_t*>(lparam);
    
    t->create_time = time(NULL);
    
    // at begin
    // send 10 data-pull-req
    for (uint32_t cnt = 0; cnt < 1; ++cnt) {
        std::map<uint32_t, upload_package_t*>::iterator iter = t->upload_packages.begin();
        if (t->upload_packages.end() != iter) {
            CImPduClientFilePullDataReq pdu(t->task_id.c_str(), idtourl(t->from_user_id), FILE_TYPE_OFFLINE, iter->second->offset, iter->second->size);
            
            /*
            UserMap_t::iterator itor = g_file_user_map.find(t->from_user_id);
            if (g_file_user_map.end() == itor) {
                // error: the sender offline
                log("offline taks %s can not find sender %d\n", t->task_id.c_str(), t->from_user_id);

                // set signal
                t->self_destroy = true;
                return NULL;
            }
            CImConn* pConn = (CImConn*)itor->second;
            */
            CFileConn* pConn = (CFileConn*)t->GetConn(t->from_user_id);
            pConn->SendPdu(&pdu);
            log("Pull Data Req\n");
        }
        ++iter;
    }
    
    // what if there is no rsp?
    // still send req?
    // no!
    // at last, the user will cancel
    // next ver, do change
    while (t->transfered_size != t->upload_packages.size()) {
        if (t->self_destroy) {
        	log("timeout, exit thread, task %s\n", t->task_id.c_str());
        	return NULL;
        }
        sleep(1);
    }
    

    t->lock(__LINE__);
    
    // write head
    if (NULL == t->file_head) {
        t->file_head = new file_header_t;
    }
    if (NULL == t->file_head) {
        log("create file header failed %s\n", t->task_id.c_str());
        // beacuse all data in mem
        // it has to be released
        
        /*
        UserMap_t::iterator ator = g_file_user_map.find(t->from_user_id);
        if (g_file_user_map.end() != ator) {
            CFileConn* pConn = (CFileConn*)ator->second;
            pConn->Close();
        }
         */
        CFileConn* pConn = (CFileConn*)t->GetConn(t->from_user_id);
        pConn->Close();
        
        t->self_destroy = true;
        t->unlock(__LINE__);
        return NULL;
        
    }
    t->file_head->set_create_time(time(NULL));
    t->file_head->set_task_id(t->task_id.c_str());
    t->file_head->set_from_user_id(t->from_user_id);
    t->file_head->set_to_user_id(t->to_user_id);
    t->file_head->set_file_name("");
    t->file_head->set_file_size(t->file_size);
    fwrite(t->file_head, 1, sizeof(file_header_t), t->fp);
    
    std::map<uint32_t, upload_package_t*>::iterator itor = t->upload_packages.begin();
    for ( ; itor != t->upload_packages.end(); ++itor) {
        fwrite(itor->second->data, 1, itor->second->size, t->fp);
    }
    
    fflush(t->fp);
    if (t->fp) {
    	fclose(t->fp);
        t->fp = NULL;
    }

    t->unlock(__LINE__);
    return NULL;
}

int generate_id(char* id)
{
    if (NULL == id) {
        return  -1; // invalid param
    }
    
    uuid_t uid = {0};
    uuid_generate(uid);
    if (uuid_is_null(uid)) {
        id = NULL;
        return -2; // uuid generate failed
    }
    uuid_unparse(uid, id);
    
    return 0;
}
/// yunfan add end

CFileConn::CFileConn()
{
	//log("CFileConn\n");
	m_bAuth = false;
	m_user_id = 0;
}

CFileConn::~CFileConn()
{
	log("~CFileConn, user_id=%u\n", m_user_id);

	for (FileMap_t::iterator it = m_save_file_map.begin(); it != m_save_file_map.end(); it++) {
		file_stat_t* file = it->second;
		fclose(file->fp);
		delete file;
	}
	m_save_file_map.clear();
}

void CFileConn::Close()
{
    log("close client, handle %d\n", m_handle);
    
    m_bAuth = false;
    
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_file_conn_map.erase(m_handle);
	}

	if (m_user_id > 0) {
		g_file_user_map.erase(m_user_id);
        m_user_id = 0;
	}

	ReleaseRef();
}

void CFileConn::OnConnect(net_handle_t handle)
{
	/// yunfan modify 2014.8.7
    m_handle = handle;

	g_file_conn_map.insert(make_pair(handle, this));    
	netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void*)imconn_callback);
	netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void*)&g_file_conn_map);

	uint32_t socket_buf_size = NETLIB_MAX_SOCKET_BUF_SIZE;
	netlib_option(handle, NETLIB_OPT_SET_SEND_BUF_SIZE, &socket_buf_size);
	netlib_option(handle, NETLIB_OPT_SET_RECV_BUF_SIZE, &socket_buf_size);
    /// yunfan modify end
}

void CFileConn::OnClose()
{
	log("client onclose: handle=%d\n", m_handle);
	Close();
}

void CFileConn::OnTimer(uint64_t curr_tick)
{
	if (curr_tick > m_last_send_tick + CLIENT_HEARTBEAT_INTERVAL) {
		CImPduHeartbeat pdu;
		SendPdu(&pdu);
	}
    
	if (curr_tick > m_last_recv_tick + CLIENT_TIMEOUT) {
		log("client timeout, user_id=%u\n", m_user_id);
//		Close();
	}
}

void CFileConn::OnWrite()
{
	CImConn::OnWrite();
}

void CFileConn::HandlePdu(CImPdu* pPdu)
{
	switch (pPdu->GetPduType()) {
	case IM_PDU_TYPE_HEARTBEAT:
		break;
	case IM_PDU_TYPE_CLIENT_FILE_LOGIN_REQ:
		_HandleClientFileLoginReq((CImPduClientFileLoginReq*) pPdu);
		break;
    /// yunfan add 2014.8.6
    case IM_PDU_TYPE_MSG_FILE_TRANSFER_REQ:
        _HandleMsgFileTransferReq((CImPduMsgFileTransferReq*)pPdu);
        break ;
    case IM_PDU_TYPE_CLIENT_FILE_STATE:
        _HandleClientFileStates((CImPduClientFileState*)pPdu);
        break ;
    case IM_PDU_TYPE_CLIENT_FILE_PULL_DATA_REQ:
        _HandleClientFilePullFileReq((CImPduClientFilePullDataReq*)pPdu);
        break ;
    case IM_PDU_TYPE_CLIENT_FILE_PULL_DATA_RSP:
        _HandleClientFilePullFileRsp((CImPduClientFilePullDataRsp*) pPdu);
        break ;

    case IM_PDU_TYPE_FILE_SERVER_IP_REQUEST:
    	_HandleGetServerAddressReq((CImPduFileServerIPReq*)pPdu);
    /// yunfan add end
            
	default: 
		log("no such pdu_type: %u\n", pPdu->GetPduType());
		break;
	}
}

int CFileConn::_HandleClientFileLoginReq(CImPduClientFileLoginReq* pPdu)
{
    // if can not find uuid
    // return invalid uuid
    // if invalid user_id
    // return invalid user
    
    // if ready_to_recv or offline / mobile task
    //   return can_send
    // return ok
    
	string user_id_url(pPdu->GetUserId(), pPdu->GetUserIdLen());
	string token(pPdu->GetToken(), pPdu->GetTokenLen());
    uint32_t mode = pPdu->GetClientMode();

	m_user_id = urltoid(user_id_url.c_str());
	m_user_id_url = user_id_url;
	log("client login, user_id=%u, token=%s handle %d\n", m_user_id, token.c_str(), m_handle);
/*
    if (!IsTokenValid(m_user_id, token.c_str())) {
		log("token not valid, close the connection\n");
		CImPduClientFileLoginRes pdu(1);
        pdu.SetReserved(pPdu->GetReserved());
		SendPdu(&pdu);
		Close();
		return;
	}
  */  
    // auth done
    m_bAuth = true;
    
    /// yunfan add 2014.8.6
    // after auth done
    string str_task_id(pPdu->GetTaskId(), pPdu->GetTaskIdLen());
    
    // create task for offline download
    if (CLIENT_OFFLINE_DOWNLOAD == mode) {
        // create a thread // insert into a queue, multi-threads handle the queue
        // find file
        // send file
        transfer_task_t* t = new transfer_task_t;
        if (NULL == t) {
            CImPduClientFileLoginRes pdu(1, str_task_id.c_str());
            pdu.SetReserved(pPdu->GetReserved());
            SendPdu(&pdu);
            Close();
            log("create task failed for task id %s, user %d\n", str_task_id.c_str(), m_user_id);
            return -1; // create task failed
        }
        t->task_id = str_task_id;
        t->to_user_id = m_user_id;
        t->create_time = time(NULL);
        pthread_rwlock_wrlock(&g_file_task_map_lock);
        g_file_task_map.insert(std::make_pair(str_task_id.c_str(), t));
        pthread_rwlock_unlock(&g_file_task_map_lock);
//        printf("---------INSERT TASK %s----------\n", t->task_id.c_str());
    }
    
    // check task
    pthread_rwlock_wrlock(&g_file_task_map_lock);
    TaskMap_t::iterator iter = g_file_task_map.find((char*)str_task_id.c_str());
    if (g_file_task_map.end() == iter) {
        // failed to find task
        // return invaild task id
        pthread_rwlock_unlock(&g_file_task_map_lock);
        CImPduClientFileLoginRes pdu(1, str_task_id.c_str()); // invalid task id errno undefined
        pdu.SetReserved(pPdu->GetReserved());
        SendPdu(&pdu);
        Close(); // invalid user for task
        log("check task id failed, user_id = %u, request taks id %s\n", m_user_id, str_task_id.c_str());
        return -2;
    }
    transfer_task_t* t = iter->second;
    pthread_rwlock_unlock(&g_file_task_map_lock);
    
    // check user
    if (t->from_user_id != m_user_id && t->to_user_id != m_user_id) {
        // invalid user
        // return error
        CImPduClientFileLoginRes pdu(1, str_task_id.c_str());
        pdu.SetReserved(pPdu->GetReserved());
        SendPdu(&pdu);
        Close();
        log("invalid user %u for task %s\n", m_user_id, str_task_id.c_str());
        return -3;
    }
    
    // prepare for offline upload
    if (CLIENT_OFFLINE_UPLOAD == mode) {
        int iret = _PreUpload(str_task_id.c_str());
        if (0 > iret) {
            CImPduClientFileLoginRes pdu(1, str_task_id.c_str());
            pdu.SetReserved(pPdu->GetReserved());
            SendPdu(&pdu);
            Close();
            log("preload faild for task %s, err %d\n", str_task_id.c_str(), iret);
            return -4;
        }
    }
    
    /*
	UserMap_t::iterator it = g_file_user_map.find(m_user_id);
	if (it != g_file_user_map.end()) {
		CFileConn* pConn = (CFileConn*)it->second;
		if (pConn == this) {
			log("find user to update user map failed\n");
			return -5;
		}
		pConn->Close();
	}
    
    // insert into user map, user_id & CFileConn *this
	g_file_user_map.insert(make_pair(m_user_id, this));
    */
    
    if (t->from_user_id == m_user_id) {
        t->from_conn = this;
    }
    if (t->to_user_id == m_user_id) {
        t->to_conn = this;
    }
    
    // send result
	CImPduClientFileLoginRes pdu(0, str_task_id.c_str());
    pdu.SetReserved(pPdu->GetReserved());
	SendPdu(&pdu); // login succeed
    
    /// yunfan add 2014.8.12
    // 2014.8.14
    // record state
    if (m_user_id == t->from_user_id) {
        t->ready_to_send = true;
    }
    if (m_user_id == t->to_user_id) {
        t->ready_to_recv = true;
    }
    // notify that the peet is ready
    if ( (m_user_id == t->to_user_id && \
          t->ready_to_send) || \
        (m_user_id == t->from_user_id && \
         t->ready_to_recv)){
        // send peer-ready state to recver
        _StatesNotify(CLIENT_FILE_PEER_READY, str_task_id.c_str(), m_user_id, t->GetConn(m_user_id));
        log("nofity recver %d task %s can recv\n", m_user_id, str_task_id.c_str());
    }
    
    // create a thread // insert into a queue, multi-threads handle the queue
    // send to client PULLDATA msg
    // recv and write file
    if (CLIENT_OFFLINE_UPLOAD == mode) {
        // check thread id
        pthread_create(&t->worker, NULL, _DoUpload, t);
        log("create thread for offline upload task %s user %d thread id %d\n", str_task_id.c_str(), m_user_id, t->worker);
    }
    /// yunfan add end

    return 0;
}

/// yunfan add 2014.8.6
int CFileConn::_HandleMsgFileTransferReq(CImPduMsgFileTransferReq* pPdu)
{
    // if realtime transfer
    // new realtime_task
    // generate uuid
    // copy userid_1 userid_2
    // time_t = time(null);
    // return uuid
    // else new offline_task
    // generate uuid
    // copy user_1
    // copy file_size
    // time_t = time(null)
    // return uuid
    
    // create task for:
    // realtime transfer and offline upload
    
    uint32_t from_id = pPdu->GetFromId();
    uint32_t to_id = pPdu->GetToId();
    
    char task_id[64] = {0};
    int iret = generate_id(task_id);
    if (0 > iret || NULL == task_id) {
        CImPduMsgFileTransferRsp pdu(FILE_SERVER_ERRNO_CREATE_TASK_ID_ERROR, from_id, to_id, pPdu->GetAttachLen(), pPdu->GetAttachData());
        pdu.SetReserved(pPdu->GetReserved());
        SendPdu(&pdu);
        log("create task id failed\n");
        return -1; // errno create task id failed 1
    }
    
    // new task and add to task_map
    transfer_task_t* task = new transfer_task_t;
    if (NULL == task) {
        // log new failed
        // return error
        CImPduMsgFileTransferRsp pdu(FILE_SERVER_ERRNO_CREATE_TASK_ERROR, from_id, to_id, pPdu->GetAttachLen(), pPdu->GetAttachData()); // failed
        pdu.SetReserved(pPdu->GetReserved());
        SendPdu(&pdu);
        Close(); // close connection with msg svr
        log("create task failed\n");
        return -2; // create task failed
    }
    
    task->transfer_mode = pPdu->GetTransferMode();
    task->task_id = task_id;
    task->from_user_id = from_id;
    task->to_user_id = to_id;
    task->file_size = pPdu->GetFileLength();
    task->create_time = time_t(NULL);
/*
    // if offline, ready_to_recv is true
    switch (task->transfer_mode) {
        case FILE_TYPE_OFFLINE:
            task->ready_to_recv = true; // offline default can send
            break;
        default:
            task->ready_to_recv = false;
            break;
    }
*/
    // read cfg file
    std::string file_name(pPdu->GetFileName(), pPdu->GetFileNameLen());
    	CImPduMsgFileTransferRsp pdu(from_id, to_id, file_name.c_str(), pPdu->GetFileLength(), task_id, pPdu->GetTransferMode(), pPdu->GetAttachLen(), pPdu->GetAttachData());
	pdu.SetReserved(pPdu->GetReserved());
	SendPdu(&pdu);
    
    task->create_time = time(NULL);
    pthread_rwlock_wrlock(&g_file_task_map_lock);
    g_file_task_map.insert(make_pair((char*)task->task_id.c_str(), task));
    pthread_rwlock_unlock(&g_file_task_map_lock);
//    printf("----------INSERT TASK %s----------\n", task->task_id.c_str());
    
    log("create task succeed, task id %s, task type %d, from user %d, to user %d\n", task->task_id.c_str(), task->transfer_mode, task->from_user_id, task->to_user_id);
    
    return 0;
}

int CFileConn::_HandleClientFileStates(CImPduClientFileState* pPdu)
{
    // switch state
    // case ready_to_recv
    //     if sender_on
    //       tell sender can_send
    //     else update state
    // case cancel
    //      notify ohter node cancel
    //      close socket
    // case dnoe
    //      notify recver done
    //      close socket

    if (!_IsAuth()) {
		return -1;
	}
    
    string str_task_id = pPdu->GetTaskId();
    std::string str_user_id(pPdu->GetUserId(), pPdu->GetUserIdLen());
    uint32_t user_id = urltoid(str_user_id.c_str());
    
    pthread_rwlock_wrlock(&g_file_task_map_lock);
    TaskMap_t::iterator iter = g_file_task_map.find((char*)str_task_id.c_str());
    if (g_file_task_map.end() == iter) {
        pthread_rwlock_unlock(&g_file_task_map_lock);
        return -2; // invalid task id
    }
    transfer_task_t* t = iter->second;
    pthread_rwlock_unlock(&g_file_task_map_lock);
    
    t->lock(__LINE__);
    if (t->from_user_id != user_id && t->to_user_id != user_id) {
        log("invalid user_id %d for task %s", pPdu->GetUserId(), str_task_id.c_str());
        t->unlock(__LINE__);
        return -3;
    }
    t->unlock(__LINE__);
    
    switch (pPdu->GetState()) {
        case CLIENT_FILE_CANCEL:
        case CLIENT_FILE_DONE:
        case CLIENT_FILE_REFUSE:
        {
            // notify other client
            /*
            UserMap_t::iterator it = g_file_user_map.find(t->GetOpponent(user_id));
            if (it == g_file_user_map.end()) {
                log("no such user, user_id=%u\n", user_id);
            }
            
            CFileConn* pConn = (CFileConn*)it->second;
            pConn->SendPdu(pPdu);
            */
            CFileConn* pConn = (CFileConn*)t->GetOpponentConn(user_id);
            pConn->SendPdu(pPdu);
            
            // release
            /*
            CFileConn* pConnThis = (CFileConn*)t->GetConn(); // close the client
            pConnThis->Close();
            pConn->Close(); // close the opponent client
            */
            log("task %s %d by user_id %d notify %d, erased\n", str_task_id.c_str(), pPdu->GetState(), user_id, t->GetOpponent(user_id));
            
            t->self_destroy = true;
            break;
        }
            
        default:
            break;
    }
    
    return 0;
}

// data handler async
// if uuid not found
// return invalid uuid and close socket
// if offline or mobile task
// check if file size too large, write data and ++size
// if realtime task
// if transfer data

int CFileConn::_HandleClientFilePullFileReq(CImPduClientFilePullDataReq *pPdu)
{
    if (!_IsAuth()) {
		return -1;
	}
    
    string str_user_id(pPdu->GetUserId(), pPdu->GetUserIdLen()); /// recver's user id
	uint32_t user_id = urltoid(str_user_id.c_str());
    string str_task_id(pPdu->GetTaskId(), pPdu->GetTaskIdLen());
    uint32_t mode = pPdu->GetMode();
	uint32_t offset = pPdu->GetOffset();
	uint32_t datasize = pPdu->GetDataSize();
    
    // since the task had been created when the recver logged-in
    // we can find task in g_file_task_map here
    pthread_rwlock_wrlock(&g_file_task_map_lock);
    TaskMap_t::iterator iter = g_file_task_map.find(str_task_id.c_str());
    if (g_file_task_map.end() == iter) {
        // invalid task id
        pthread_rwlock_unlock(&g_file_task_map_lock);
        log("invalid task id %s \n", str_task_id.c_str());
        CImPduClientFilePullDataRsp rsp(FILE_SERVER_ERRNO_PULL_DATA_WITH_INVALID_TASK_ID);
        rsp.SetReserved(pPdu->GetReserved());
        SendPdu(&rsp);
        return -2;
    }
    transfer_task_t* t = iter->second;
    pthread_rwlock_unlock(&g_file_task_map_lock);
    
    t->lock(__LINE__);
    
    t->create_time = time(NULL);
    
    if (t->from_user_id != user_id /*for the realtime recver*/ && t->to_user_id != user_id /*for the offline download*/) {
        // invalid user
        log("illieage user %d for task %s\n", user_id, str_task_id.c_str());
        CImPduClientFilePullDataRsp rsp(FILE_SERVER_ERRNO_PULL_DATA_ILLIEAGE_USER);
        rsp.SetReserved(pPdu->GetReserved());
        SendPdu(&rsp);
        
        t->unlock(__LINE__);
        return -3;
    }
    
    switch (mode) {
        case FILE_TYPE_ONLINE: // transfer request to sender
        {
            string str_to_user = idtourl(t->GetOpponent(user_id));
            CImPduClientFilePullDataReq pdu(str_task_id.c_str(), str_to_user.c_str(), mode, offset, datasize);
            
            /*
            UserMap_t::iterator it = g_file_user_map.find(t->GetOpponent(user_id));
            if (it == g_file_user_map.end()) {
                log("ERROR no such user, user_id=%u\n", t->GetOpponent(user_id));
                
                t->unlock(__LINE__);
                return -4;
            }
            CFileConn* pConn = (CFileConn*)it->second;
            pConn->SendPdu(pPdu);
            */
            
            CFileConn* pConn = (CFileConn*)t->GetOpponentConn(user_id);
            pConn->SendPdu(pPdu);
            break;
        }
        case FILE_TYPE_OFFLINE: // for the offline download
        {
            // find file use task id
            // send header info to user
            // send data
            // save path manager not used
            
            // save transfered info into task
            // like FILE*
            // transfered size
            
            
            // haven't been opened
            size_t size = 0;
            if (NULL == t->fp) {
                char save_path[BUFSIZ] = {0};
                snprintf(save_path, BUFSIZ, "%s/%s", g_current_save_path, str_user_id.c_str()); // those who can only get files under their user_id-dir
                
                int ret = mkdir(save_path, 0755);
                if ( (ret != 0) && (errno != EEXIST) ) {
                    log("mkdir failed for path: %s\n", save_path);
                    CImPduClientFilePullDataRsp rsp(FILE_SERVER_ERRNO_PULL_DATA_MKDIR_ERROR);
                    rsp.SetReserved(pPdu->GetReserved());
                    SendPdu(&rsp);
                    
                    t->unlock(__LINE__);
                    return -5;
                }
                
                strncat(save_path, "/", BUFSIZ);
                strncat(save_path, str_task_id.c_str(), BUFSIZ); // use task_id as file name, in case of same-name file
                
                // open at first time
                t->fp = fopen(save_path, "rb");  // save fp
                if (!t->fp) {
                    log("can not open file\n");
                    CImPduClientFilePullDataRsp rsp(FILE_SERVER_ERRNO_PULL_DATA_OPEN_FILE_ERROR);
                    rsp.SetReserved(pPdu->GetReserved());
                    SendPdu(&rsp);
                    
                    t->unlock(__LINE__);
                    return -5;
                }
                
                // read head at open
                if (NULL == t->file_head) {
                    t->file_head = new file_header_t;
                    if (NULL == t->file_head) {
                        // close to ensure next time will new file-header again
                        fclose(t->fp);
                        CImPduClientFilePullDataRsp rsp(FILE_SERVER_ERRNO_PULL_DATA_READ_FILE_HEADER_ERROR);
                        rsp.SetReserved(pPdu->GetReserved());
                        SendPdu(&rsp);
                        
                        t->unlock(__LINE__);
                        return -6;
                    }
                }
                
                size = fread(t->file_head, 1, sizeof(file_header_t), t->fp); // read header
                if (sizeof(file_header_t) > size) {
                    // close to ensure next time will read again
                    fclose(t->fp); // error to get header
                    CImPduClientFilePullDataRsp rsp(FILE_SERVER_ERRNO_PULL_DATA_READ_FILE_HEADER_ERROR);
                    rsp.SetReserved(pPdu->GetReserved());
                    SendPdu(&rsp);
                    
                    t->unlock(__LINE__);
                    return -7;
                } // the header won't be sent to recver, because the msg svr had already notified it.
                // if the recver needs to check it, it could be helpful
                // or sometime later, the recver needs it in some way.
            }
            
            // read data and send based on offset and datasize.
            char* tmpbuf = new char[pPdu->GetDataSize()];
            if (NULL == tmpbuf) {
                // alloc mem failed
                CImPduClientFilePullDataRsp rsp(FILE_SERVER_ERRNO_PULL_DATA_ALLOC_MEM_ERROR);
                rsp.SetReserved(pPdu->GetReserved());
                SendPdu(&rsp);
                
                t->unlock(__LINE__);
                return -8;
            }
            memset(tmpbuf, 0, pPdu->GetDataSize());
            
            // offset file_header_t
            int iret = fseek(t->fp, sizeof(file_header_t) + pPdu->GetOffset(), SEEK_SET); // read after file_header_t
            if (0 != iret) {
                CImPduClientFilePullDataRsp rsp(FILE_SERVER_ERRNO_PULL_DATA_SEEK_OFFSET_ERROR);
                rsp.SetReserved(pPdu->GetReserved());
                SendPdu(&rsp);
                delete[] tmpbuf;
                
                t->unlock(__LINE__);
                return -9;
                // offset failed
            }
            size = fread(tmpbuf, 1, pPdu->GetDataSize(), t->fp);
            CImPduClientFilePullDataRsp pdu((char*)str_task_id.c_str(), t->file_head->from_user_id, pPdu->GetOffset(), (uint32_t)size, (uchar_t*)tmpbuf);
            pdu.SetReserved(pPdu->GetReserved());
            SendPdu(&pdu);
            delete[] tmpbuf;
            
            t->transfered_size += size; // record transfered size for next time offset
            if (0 == size) {
//                Close(); // read EOF
                fclose(t->fp);
                t->fp = NULL;
                
                _StatesNotify(CLIENT_FILE_DONE, str_task_id.c_str(), user_id, this);
                
                //CImPduClientFilePullDataRsp rsp(FILE_SERVER_ERRNO_PULL_DATA_FINISHED);
                // finished
                //rsp.SetReserved(pPdu->GetReserved());
                //SendPdu(&rsp);
                Close();
                
                t->self_destroy = true;
                t->unlock(__LINE__);
                return -10;
            }
            
            break;
        }
        default:
            break;
    }
  
    t->unlock(__LINE__);
    return 0;
}

int CFileConn::_HandleClientFilePullFileRsp(CImPduClientFilePullDataRsp *pPdu)
{
    if (!_IsAuth()) {
		return -1;
	}
    
    string str_user_id(pPdu->GetUserId(), pPdu->GetUserIdLen()); /// sender's user id
	uint32_t user_id = urltoid(str_user_id.c_str());
    string str_task_id(pPdu->GetTaskId(), pPdu->GetTaskIdLen());
	uint32_t offset = pPdu->GetOffset();
	uint32_t datasize = pPdu->GetDataSize();
    
    pthread_rwlock_wrlock(&g_file_task_map_lock);
    TaskMap_t::iterator iter = g_file_task_map.find(str_task_id.c_str());
    if (g_file_task_map.end() == iter) {
        // invalid task id
        pthread_rwlock_unlock(&g_file_task_map_lock);
        return -2;
    }
    transfer_task_t* t = iter->second;
    pthread_rwlock_unlock(&g_file_task_map_lock);
    
    t->lock(__LINE__);
    
    t->create_time = time(NULL);
    
    if (t->from_user_id != user_id && t->to_user_id != user_id) {
        // invalid user
        t->unlock(__LINE__);
        return -3;
    }
    
    switch (t->transfer_mode) {
        case FILE_TYPE_ONLINE: // transfer request to sender
        {
            string str_to_user = idtourl(t->GetOpponent(user_id));
            uchar_t* pData = new uchar_t[datasize];
            if (NULL == pData) {

                t->unlock(__LINE__);
                return -4;
            }
            memcpy((void*)pData, (void*)pPdu->GetData(), datasize);
            
            CImPduClientFilePullDataRsp pdu(str_task_id.c_str(), str_to_user.c_str(), offset, datasize, pData);
            
            /*
            UserMap_t::iterator it = g_file_user_map.find(t->GetOpponent(user_id));
            if (it == g_file_user_map.end()) {
                log("FATAL no such user, user_id=%u\n", t->GetOpponent(user_id));
                delete [] pData;
                t->unlock(__LINE__);
                return -5;
            }
            CFileConn* pConn = (CFileConn*)it->second;
            pConn->SendPdu(pPdu); /// send to recver
             */
            CFileConn* pConn = (CFileConn*)t->GetOpponentConn(user_id);
            pConn->SendPdu(pPdu); /// send to recver
            
            delete [] pData;
            break;
        }
        case FILE_TYPE_OFFLINE: /// this is the response to the server pull-data-req
        {
            if (t->upload_packages.size() <= 0) {
                log("FATAL ERROR");
                t->unlock(__LINE__);
                return -6;
            }
            
            // check if data size ok
            std::map<uint32_t, upload_package_t*>::iterator itPacks = t->upload_packages.find(pPdu->GetOffset());
            if (t->upload_packages.end() != itPacks) { // offset right
                
                // check if data size ok
                if (pPdu->GetDataSize() != itPacks->second->size) {
                    // the rsp's data size is different from req's
                    // refuse it or dynamic adjust
                    // do not adjust now, maybe later
                    uint32_t offset = itPacks->second->offset;
                    uint32_t size = itPacks->second->size;
                    // resend req
                    CImPduClientFilePullDataReq req(str_task_id.c_str(), str_user_id.c_str(), t->transfer_mode, offset, size);
                    SendPdu(&req);
                    log("size not match");

                    t->unlock(__LINE__);
                    return -7;
                }
                
                // check if data-ptr OK
                if (NULL == itPacks->second->data) {
                    itPacks->second->data = new char[itPacks->second->size];
                    if (NULL == itPacks->second->data) {
                        uint32_t offset = itPacks->second->offset;
                        uint32_t size = itPacks->second->size;
                        
                        log("alloc mem failed");
                        // resend req
                        CImPduClientFilePullDataReq req(str_task_id.c_str(), str_user_id.c_str(), t->transfer_mode, offset, size);
                        SendPdu(&req);

                        t->unlock(__LINE__);
                        return -8;
                    }
                }
                
                // copy data
                memset(itPacks->second->data, 0,  itPacks->second->size);
                memcpy(itPacks->second->data, pPdu->GetData(), pPdu->GetDataSize());
                ++t->transfered_size;
            }
            
            // find which segment hasn't got data yet
            bool bFound = false;
            std::map<uint32_t, upload_package_t*>::iterator itor = t->upload_packages.begin();
            for ( ; itor != t->upload_packages.end(); ++itor) {
                if (NULL == itor->second->data) {
                    bFound = true;
                    break;
                }
            }
            if (!bFound) {
                // all packages recved
                _StatesNotify(CLIENT_FILE_DONE, str_task_id.c_str(), user_id, t->GetConn(user_id));
                Close();

                t->unlock(__LINE__);
                return -9;
            }
            // prepare to send req for this segment
            uint32_t next_offset = itor->second->offset;
            uint32_t next_size = itor->second->size;
            
            // send pull-data-req
            CImPduClientFilePullDataReq req(str_task_id.c_str(), str_user_id.c_str(), t->transfer_mode, next_offset, next_size);
            SendPdu(&req);
            break;
        }
        default:
            break;
    }
    
    t->unlock(__LINE__);
    return 0;
}

int CFileConn::_StatesNotify(int state, const char* task_id, uint32_t user_id, CImConn* conn)
{
    /*
    UserMap_t::iterator iter = g_file_user_map.find(user_id);
    if (g_file_user_map.end() == iter) {
    	log("failed to get user %d\n", user_id);
        return -1;
    }
    CFileConn* pConn = (CFileConn*)iter->second;
     */
    CFileConn* pConn = (CFileConn*)conn;
    
    CImPduClientFileState pdu(state, task_id, idtourl(user_id));
    
    pConn->SendPdu(&pdu);
    log("notify to user %d state %d task %s\n", user_id, state, task_id);
    return 0;
}

int CFileConn::_PreUpload(const char* task_id)
{
    pthread_rwlock_wrlock(&g_file_task_map_lock);
    TaskMap_t::iterator iter = g_file_task_map.find(task_id);
    if (g_file_task_map.end() == iter) {
        pthread_rwlock_unlock(&g_file_task_map_lock);
        log("failed to find task %s in task map\n", task_id);
        return -1;
    }
    transfer_task_t* t = iter->second;
    pthread_rwlock_unlock(&g_file_task_map_lock);
    
    char save_path[BUFSIZ];
    std::string str_user_id = idtourl(t->to_user_id);
    snprintf(save_path, BUFSIZ, "%s/%s", g_current_save_path, str_user_id.c_str());
    int ret = mkdir(save_path, 0755);
    if ( (ret != 0) && (errno != EEXIST) ) {
        log("mkdir failed for path: %s\n", save_path);
        
        t->self_destroy = true;
        return -2;
    }
    
    // save as g_current_save_path/to_id_url/task_id
    strncat(save_path, "/", BUFSIZ);
    strncat(save_path, t->task_id.c_str(), BUFSIZ);
    
    t->fp = fopen(save_path, "ab+");
    if (!t->fp) {
        log("open file for write failed\n");
        
        t->self_destroy = true;
        return -3;
    }
    
    uint32_t total_packages = t->file_size / SEGMENT_SIZE;
    for (uint32_t cnt = 0; cnt < total_packages; ++cnt) {
        upload_package_t* package = new upload_package_t(cnt, cnt * SEGMENT_SIZE, SEGMENT_SIZE);
        if (NULL == t) {
            log("create upload packages failed\n");
            
            t->self_destroy = true;
            return -4;
        }
        t->upload_packages.insert(std::make_pair(package->offset, package));
    }
    
    uint32_t last_piece = t->file_size % SEGMENT_SIZE;
    if (last_piece) {
        total_packages += 1;
        upload_package_t* package = new upload_package_t(t->upload_packages.size(), t->file_size - last_piece, last_piece);
        if (NULL == package) {
            log("create upload package failed\n");
            
            t->self_destroy = true;
            return -5;
        }
        t->upload_packages.insert(std::make_pair(package->offset, package));
    }

    return 0;
}

int CFileConn::_HandleGetServerAddressReq(CImPduFileServerIPReq* pPdu)
{
	CImPduFileServerIPRsp rsp(&g_addr);
	SendPdu(&rsp);

	return 0;
}
/// yunfan add end
