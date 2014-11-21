/*
 * ImUser.h
 *
 *  Created on: 2014年4月16日
 *      Author: ziteng
 */

#ifndef IMUSER_H_
#define IMUSER_H_

#include "imconn.h"

#define MAX_ONLINE_FRIEND_CNT		100	//通知好友状态通知的最多个数

class CMsgConn;

class CImUser
{
public:
	CImUser(string user_name);
	~CImUser();

	uint32_t GetUserId() { return m_user_id; }
    string GetUserName() { return m_name; }
	bool IsValidate() { return m_bValidate; }
	void SetValidated() { m_bValidate = true; }
	uint32_t GetIMOnlineStatus() { return m_online_status; }

	void AddOnlineFriend(uint32_t friend_id) {
		if (m_online_friend_set.size() < MAX_ONLINE_FRIEND_CNT)
			m_online_friend_set.insert(friend_id);
	}

	void RemoveOnlineFriend(uint32_t friend_id) {
		m_online_friend_set.erase(friend_id);
	}

	bool IsMsgConnEmpty() { return m_conn_map.empty(); }
	void AddMsgConn(string token, CMsgConn* pMsgConn) { m_conn_map[token] = pMsgConn; }
	void DelMsgConn(string token) { m_conn_map.erase(token); }
    CMsgConn* GetMsgConn(string token);
    CMsgConn* GetMsgConnByHandle(uint32_t handle);
    void ValidateMsgConn(string token, CMsgConn*);
    
    void AddUnValidateMsgConn(CMsgConn* pMsgConn) { m_unvalidate_conn_set.insert(pMsgConn); }
    void DelUnValidateMsgConn(CMsgConn* pMsgConn) { m_unvalidate_conn_set.erase(pMsgConn); }
    CMsgConn* GetUnValidateMsgConn(uint32_t handle);
    

	user_conn_t GetUserConn();
	void SetIMOnlineStatus(uint32_t status) { m_online_status = status; }
	void SetUser(user_info_t* user);
	void BroadcastPdu(CImPdu* pPdu, CMsgConn* pFromConn = NULL);
    void BroadcastPduWithOutMobile(CImPdu* pPdu, CMsgConn* pFromConn = NULL);
	void BroadcastClientMsgData(CImPduClientMsgData* pPdu, CMsgConn* pFromConn, uint32_t from_id);
    void BroadcastData(void* buff, uint32_t len, CMsgConn* pFromConn = NULL);
    
	void SendStatusChangeToFriend(uint32_t online_status);
    
    void HandleKickUser(CMsgConn* pConn);
    
    bool KickOutSameClientType(uint32_t client_type, CMsgConn* pFromConn = NULL);
    
    uint32_t GetClientTypeFlag();
private:
	uint32_t		m_user_id;
    string			m_name;
	string 			m_user_id_url;
	bool 			m_user_updated;
	uint32_t		m_online_status;	// 1-online, 2-off-line, 3-leave

    bool 			m_bValidate;

	set<uint32_t>	m_online_friend_set;
	map<string /* token */, CMsgConn*>	m_conn_map;
    set<CMsgConn*> m_unvalidate_conn_set;
    //map<string, >
};

typedef map<uint32_t /* user_id */, CImUser*> ImUserMap_t;
typedef map<string, CImUser*> ImUserMapByName_t;

class CImUserManager
{
public:
    CImUserManager() {}
    ~CImUserManager();
    
    static CImUserManager* GetInstance();
    CImUser* GetImUserById(uint32_t user_id);
    CImUser* GetImUserByName(string user_name);
    
    CMsgConn* GetMsgConnByHandle(uint32_t user_id, uint32_t handle);
    bool AddImUserByName(string user_name, CImUser* pUser);
    void RemoveImUserByName(string user_name);
    
    bool AddImUserById(uint32_t user_id, CImUser* pUser);
    void RemoveImUserById(uint32_t user_id);
    
    void RemoveImUser(CImUser* pUser);
    
    void RemoveAll();
    void GetOnlineUserInfo(list<user_conn_stat_t>* online_user_info);
    void GetUserConnCnt(list<user_conn_t>* user_conn_list, uint32_t& total_conn_cnt);

    void BroadcastPdu(CImPdu* pdu);
private:
    ImUserMap_t m_im_user_map;
    ImUserMapByName_t m_im_user_map_by_name;
};

#endif /* IMUSER_H_ */
