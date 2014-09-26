/*
 * RouteConn.h
 *
 *  Created on: 2013-7-4
 *      Author: ziteng@mogujie.com
 */

#ifndef ROUTECONN_H_
#define ROUTECONN_H_

#include "imconn.h"

class CRouteConn : public CImConn
{
public:
	CRouteConn();
	virtual ~CRouteConn();

	virtual void Close();

	virtual void OnConnect(net_handle_t handle);
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);

	virtual void HandlePdu(CImPdu* pPdu);

private:
	void _HandleOnlineUserInfo(CImPduOnlineUserInfo* pPdu);
	void _HandleUserStatusUpdate(CImPduUserStatusUpdate* pPdu);
	void _HandleFriendStatusQuery(CImPduFriendStatusQuery* pPdu);
	void _HandleFriendStatusNotify(CImPduFriendStatusNotify* pPdu);
	void _HandleMsgData(CImPduMsgData* pPdu);
	void _HandleP2PMsg(CImPduP2PMsg* pPdu);
	void _HandleRoleSet(CImPduRoleSet* pPdu);
	void _HandleUserStatusRequest(CImPduUserStatusRequest* pPdu);
	void _HandleUsersStatusRequest(CImPduUsersStatusRequest* pPdu);
    void _HandleKickOut(CImPduServerKickUser* pPdu);
    void _HandleUserClientType(CImPduUserClientTypeRequest * pPdu);
    void _HandleFileNotify(CImPduFileNotify* pPdu);

	void _UpdateUserStatus(uint32_t user_id, uint32_t status, uint32_t client_type_flag);
	void _BroadcastMsg(CImPdu* pPdu);
    void _DeliverMsgData(uint32_t from_id, uint32_t to_id, CImPdu* pPdu);
private:
	list<uint32_t>	m_notify_user_list;
	bool			m_bMaster;
};

void init_routeconn_timer_callback();

#endif /* ROUTECONN_H_ */
