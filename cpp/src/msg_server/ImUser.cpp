/*
 * ImUser.cpp
 *
 *  Created on: 2014年4月16日
 *      Author: ziteng
 *  Brief:
 *  	a map from user_id to userInfo and connection list
 */

#include "ImUser.h"
#include "MsgConn.h"
#include "RouteServConn.h"

CImUser::CImUser(string user_name)
{
    //log("ImUser, userId=%u\n", user_id);
    m_name = user_name;
    m_bValidate = false;
    m_user_id = 0;
	m_user_updated = false;
}

CImUser::~CImUser()
{
	//log("~ImUser, userId=%u\n", m_user_id);
}

CMsgConn* CImUser::GetUnValidateMsgConn(uint32_t handle)
{
	for (set<CMsgConn*>::iterator it = m_unvalidate_conn_set.begin(); it != m_unvalidate_conn_set.end(); it++)
    {
		CMsgConn* pConn = *it;
		if (pConn->GetHandle() == handle) {
			return pConn;
		}
	}

	return NULL;
}

CMsgConn* CImUser::GetMsgConn(string token)
{
    CMsgConn* pMsgConn = NULL;
    map<string, CMsgConn*>::iterator it = m_conn_map.find(token);
    if (it != m_conn_map.end()) {
        pMsgConn = it->second;
    }
    return pMsgConn;
}

CMsgConn* CImUser::GetMsgConnByHandle(uint32_t handle)
{
    CMsgConn* pMsgConn = NULL;
    map<string, CMsgConn*>::iterator it = m_conn_map.begin();
    for (; it != m_conn_map.end(); it++) {
        if (it->second->GetHandle() == handle) {
            pMsgConn = it->second;
        }
    }
    return pMsgConn;
}

void CImUser::ValidateMsgConn(string token, CMsgConn* pMsgConn)
{
    AddMsgConn(token, pMsgConn);
    DelUnValidateMsgConn(pMsgConn);
}


user_conn_t CImUser::GetUserConn()
{
	uint32_t conn_cnt = 0;
	for (map<string, CMsgConn*>::iterator it = m_conn_map.begin(); it != m_conn_map.end(); it++) {
		CMsgConn* pConn = it->second;
		if (pConn->IsOpen()) {
			conn_cnt++;
		}
	}

	user_conn_t user_cnt = {m_user_id, conn_cnt};
	return user_cnt;
}


void CImUser::SetUser(user_info_t* user)
{
	if (!m_user_updated) {
        m_user_id = user->user_id;
        m_user_id_url = string(idtourl(m_user_id));
        m_user_updated = true;
	}
}

void CImUser::BroadcastPdu(CImPdu* pPdu, CMsgConn* pFromConn)
{
	for (map<string, CMsgConn*>::iterator it = m_conn_map.begin(); it != m_conn_map.end(); it++) {
		CMsgConn* pConn = it->second;
		if (pConn != pFromConn) {
			pConn->SendPdu(pPdu);
		}
	}
}

void CImUser::BroadcastPduWithOutMobile(CImPdu *pPdu, CMsgConn* pFromConn)
{
    for (map<string, CMsgConn*>::iterator it = m_conn_map.begin(); it != m_conn_map.end(); it++)
    {
        CMsgConn* pConn = it->second;
        if (pConn != pFromConn && CHECK_CLIENT_TYPE_PC(pConn->GetClientType())) {
            pConn->SendPdu(pPdu);
        }
    }
}

void CImUser::BroadcastClientMsgData(CImPduClientMsgData* pPdu, CMsgConn* pFromConn, uint32_t from_id)
{
	for (map<string, CMsgConn*>::iterator it = m_conn_map.begin(); it != m_conn_map.end(); it++) {
		CMsgConn* pConn = it->second;
		if (pConn != pFromConn) {
			pConn->SendPdu(pPdu);
			pConn->AddToSendList(pPdu->GetSeqNo(), from_id);
		}
	}
}

void CImUser::BroadcastData(void *buff, uint32_t len, CMsgConn* pFromConn)
{
    if(!buff)
        return;
    for (map<string, CMsgConn*>::iterator it = m_conn_map.begin(); it != m_conn_map.end(); it++) {
		CMsgConn* pConn = it->second;
        
        if(pConn == NULL)
            continue;
        
		if (pConn != pFromConn) {
			pConn->Send(buff, len);
		}
	}
}

void CImUser::SendStatusChangeToFriend(uint32_t online_status)
{
	if (!m_bValidate || m_online_friend_set.empty()) {
		return;
	}

	list<uint32_t> friend_id_list;
	CImPduClientFriendNotify pduCFN(m_user_id_url.c_str(), online_status);
	for (set<uint32_t>::iterator it = m_online_friend_set.begin(); it != m_online_friend_set.end(); it++) {
		uint32_t online_friend_id = *it;
		friend_id_list.push_back(online_friend_id);
	}

	CRouteServConn* pRouteConn = get_route_serv_conn();
	if (pRouteConn) {
		CImPduFriendStatusNotify pduFSN(m_user_id, online_status, &friend_id_list);
		pRouteConn->SendPdu(&pduFSN);
	}
}

void CImUser::HandleKickUser(CMsgConn* pConn)
{
    map<string, CMsgConn*>::iterator it = m_conn_map.find(pConn->GetToken());
    if (it != m_conn_map.end()) {
        CMsgConn* pConn = it->second;
		if(pConn) {
			log("kick service user, user_id=%u, token = %s.\n", m_user_id, pConn->GetToken().c_str());
			CImPduKickUser pdu(m_user_id, KICK_REASON_DUPLICATE_USER);
			pConn->SendPdu(&pdu);
			//pConn->Close();
		}
    }
}

// 只支持一个WINDOWS/MAC客户端登陆,或者一个ios/android登录
bool CImUser::KickOutSameClientType(uint32_t client_type, CMsgConn* pFromConn)
{
    for (map<string, CMsgConn*>::iterator it = m_conn_map.begin(); it != m_conn_map.end(); it++)
    {
        CMsgConn* pMsgConn = it->second;
        
        //16进制位移计算
        if ((((pMsgConn->GetClientType() ^ client_type) >> 4) == 0) && (pMsgConn != pFromConn)) {
            HandleKickUser(pMsgConn);
            break;
        }
    }
    return true;
}

uint32_t CImUser::GetClientTypeFlag()
{
    uint32_t client_type_flag = 0x00;
    map<string, CMsgConn*>::iterator it = m_conn_map.begin();
    for (; it != m_conn_map.end(); it++)
    {
        CMsgConn* pConn = it->second;
        uint32_t client_type = pConn->GetClientType();
        if (CHECK_CLIENT_TYPE_PC(client_type))
        {
            client_type_flag |= CLIENT_TYPE_FLAG_PC;
        }
        else if (CHECK_CLIENT_TYPE_MOBILE(client_type))
        {
            client_type_flag |= CLIENT_TYPE_FLAG_MOBILE;
        }
    }
    return client_type_flag;
}

CImUserManager::~CImUserManager()
{
    RemoveAll();
}

CImUserManager* CImUserManager::GetInstance()
{
    static CImUserManager s_manager;
    return &s_manager;
}


CImUser* CImUserManager::GetImUserByName(string user_name)
{
    CImUser* pUser = NULL;
    ImUserMapByName_t::iterator it = m_im_user_map_by_name.find(user_name);
    if (it != m_im_user_map_by_name.end()) {
        pUser = it->second;
    }
    return pUser;
}

CImUser* CImUserManager::GetImUserById(uint32_t user_id)
{
    CImUser* pUser = NULL;
    ImUserMap_t::iterator it = m_im_user_map.find(user_id);
    if (it != m_im_user_map.end()) {
        pUser = it->second;
    }
    return pUser;
}

CMsgConn* CImUserManager::GetMsgConnByHandle(uint32_t user_id, uint32_t handle)
{
    CMsgConn* pMsgConn = NULL;
    CImUser* pImUser = GetImUserById(user_id);
    if (pImUser) {
        pMsgConn = pImUser->GetMsgConnByHandle(handle);
    }
    return pMsgConn;
}

bool CImUserManager::AddImUserByName(string user_name, CImUser *pUser)
{
    bool bRet = false;
    if (GetImUserByName(user_name) == NULL) {
        m_im_user_map_by_name[user_name] = pUser;
        bRet = true;
    }
    return bRet;
}

void CImUserManager::RemoveImUserByName(string user_name)
{
    m_im_user_map_by_name.erase(user_name);
}

bool CImUserManager::AddImUserById(uint32_t user_id, CImUser *pUser)
{
    bool bRet = false;
    if (GetImUserById(user_id) == NULL) {
        m_im_user_map[user_id] = pUser;
        bRet = true;
    }
    return bRet;
}

void CImUserManager::RemoveImUserById(uint32_t user_id)
{
    m_im_user_map.erase(user_id);
}

void CImUserManager::RemoveImUser(CImUser *pUser)
{
    if (pUser != NULL) {
        RemoveImUserById(pUser->GetUserId());
        RemoveImUserByName(pUser->GetUserName());
        delete pUser;
        pUser = NULL;
    }
}

void CImUserManager::RemoveAll()
{
    for (ImUserMapByName_t::iterator it = m_im_user_map_by_name.begin(); it != m_im_user_map_by_name.end();
         it++)
    {
        CImUser* pUser = it->second;
        if (pUser != NULL) {
            delete pUser;
            pUser = NULL;
        }
    }
    m_im_user_map_by_name.clear();
    m_im_user_map.clear();
}

void CImUserManager::GetOnlineUserInfo(list<user_conn_stat_t>* online_user_info)
{
	user_conn_stat_t status;
	CImUser* pImUser = NULL;
	for (ImUserMap_t::iterator it = m_im_user_map.begin(); it != m_im_user_map.end(); it++) {
		pImUser = (CImUser*)it->second;
		if (pImUser->IsValidate())
        {
			status.user_id = pImUser->GetUserId();
			status.status = pImUser->GetIMOnlineStatus();
           status.client_type_flag = pImUser->GetClientTypeFlag();
			online_user_info->push_back(status);
		}
	}
    
}

void CImUserManager::GetUserConnCnt(list<user_conn_t>* user_conn_list, uint32_t& total_conn_cnt)
 {
     total_conn_cnt = 0;
     CImUser* pImUser = NULL;
     for (ImUserMap_t::iterator it = m_im_user_map.begin(); it != m_im_user_map.end(); it++)
     {
         pImUser = (CImUser*)it->second;
         if (pImUser->IsValidate())
         {
             user_conn_t user_conn_cnt = pImUser->GetUserConn();
             user_conn_list->push_back(user_conn_cnt);
             total_conn_cnt += user_conn_cnt.conn_cnt;
         }
     }
 }

void CImUserManager::BroadcastPdu(CImPdu* pdu)
{
    CImUser* pImUser = NULL;
    for (ImUserMap_t::iterator it = m_im_user_map.begin(); it != m_im_user_map.end(); it++)
    {
        pImUser = (CImUser*)it->second;
        if (pImUser->IsValidate())
        {
            pImUser->BroadcastPdu(pdu);
        }
    }
}

