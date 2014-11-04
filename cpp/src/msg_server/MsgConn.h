/*
 * MsgConn.h
 *
 *  Created on: 2013-7-5
 *      Author: ziteng@mogujie.com
 */

#ifndef MSGCONN_H_
#define MSGCONN_H_

#include "imconn.h"

#define KICK_FROM_ROUTE_SERVER 		1
#define MAX_ONLINE_FRIEND_CNT		100	//通知好友状态通知的最多个数

typedef struct {
	uint32_t seq_no;
	uint32_t from_id;
	uint64_t timestamp;
} msg_ack_t;

class CMsgConn : public CImConn
{
public:
	CMsgConn();
	virtual ~CMsgConn();

    string GetUserName() { return m_user_name; }
    uint32_t GetUserId();
    uint32_t GetHandle() { return m_handle; }
	uint16_t GetPduVersion() { return m_pdu_version; }
    uint32_t GetClientType() { return m_client_type; }
    uint32_t GetClientTypeFlag();
    void SetToken(string token) { m_token = token; }
    string GetToken() { return m_token; }
	void SetOpen() { m_bOpen = true; }
	bool IsOpen() { return m_bOpen; }
	void SendUserStatusUpdate(uint32_t user_status);

	virtual void Close(bool kick_user = false);

	virtual void OnConnect(net_handle_t handle);
	virtual void OnClose();
	virtual inline void OnTimer(uint64_t curr_tick);

	virtual void HandlePdu(CImPdu* pPdu);

	void AddToSendList(uint32_t seq_no, uint32_t from_id);
	void DelFromSendList(uint32_t seq_no, uint32_t from_id);
	void SendUserActionLog(uint32_t action_type);
private:
    void _HandleHeartBeat(CImPduHeartbeat* pPdu);
	void _HandleLoginRequest(CImPduLoginRequest* pPdu);
	void _HandleBuddyListRequest(CImPduClientBuddyListRequest* pPdu);
	void _HandleClientMsgData(CImPduClientMsgData* pPdu);
	void _HandleClientMsgDataAck(CImPduClientMsgDataAck* pPdu);
	void _HandleClientTimeRequest(CImPduClientTimeRequest* pPdu);
	void _HandleClientUnreadMsgCntRequest(CImPduClientUnreadMsgCntRequest* pPdu);
	void _HandleClientUnreadMsgRequest(CImPduClientUnreadMsgRequest* pPdu);
	void _HandleClientMsgReadAck(CImPduClientMsgReadAck* pPdu);
	void _HandleClientP2PCmdMsg(CImPduClientP2PCmdMsg* pPdu);
	void _HandleClientUserInfoRequest(CImPduClientUserInfoRequest* pPdu);
	void _HandleClientDBQueryRequest(CImPduClientDBQueryRequest* pPdu);

	void _HandleClientServiceSetting(CImPduClientServiceSetting* pPdu);
	void _HandleClientUserStatusRequest(CImPduClientUserStatusRequest* pPdu);
	void _HandleClientUsersStatusRequest(CImPduClientUsersStatusRequest* pPdu);

	void _HandleClientRemoveSessionRequest(CImPduClientRemoveSessionRequest* pPdu);
	void _HandleClientAllUserRequest(CImPduClientAllUserRequest* pPdu);
    
    void _HandleClientDepartmentRequest(CImPduClientDepartmentRequest* pPdu);
private:
    string          m_user_name;
    string          m_token;
	bool			m_bOpen;	// only DB validate passed will be set to true;
	uint64_t		m_login_time;

    uint32_t		m_last_seq_no;

	uint16_t		m_pdu_version;
    
    string 			m_client_version;	// e.g MAC/2.2, or WIN/2.2

	list<msg_ack_t>	m_send_msg_list;

	uint32_t		m_msg_cnt_per_sec;
    
    uint32_t        m_client_type;        //客户端登录方式
};

void init_msg_conn();

#endif /* MSGCONN_H_ */
