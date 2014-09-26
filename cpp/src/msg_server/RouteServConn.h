/*
 * RouteServConn.h
 *
 *  Created on: 2013-7-8
 *      Author: ziteng@mogujie.com
 */

#ifndef ROUTESERVCONN_H_
#define ROUTESERVCONN_H_

#include "imconn.h"
#include "ServInfo.h"

class CRouteServConn : public CImConn
{
public:
	CRouteServConn();
	virtual ~CRouteServConn();

	bool IsOpen() { return m_bOpen; }
	uint64_t GetConnectTime() { return m_connect_time; }

	void Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx);
	virtual void Close();

	virtual void OnConfirm();
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);

	virtual void HandlePdu(CImPdu* pPdu);
private:
	void _HandleKickUser(CImPduServerKickUser* pPdu);
	void _HandleFriendStatusList(CImPduFriendStatusList* pPdu);
	void _HandleFriendStatusNotify(CImPduFriendStatusNotify* pPdu);
	void _HandleMsgData(CImPduMsgData* pPdu);
	void _HandleP2PMsg(CImPduP2PMsg* pPdu);
    void _HandleGroupP2PMsg(CImPduGroupP2PMessageResponse* pPdu);
    
	void _HandleUserStatusResponse(CImPduUserStatusResponse* pPdu);
	void _HandleUsersStatusResponse(CImPduUsersStatusResponse* pPdu);
    void _HandleUserClientTypeResponse(CImPduUserClientTypeResponse* pPdu);
private:
	bool 		m_bOpen;
	uint32_t	m_serv_idx;
	uint64_t	m_connect_time;
};

void init_route_serv_conn(serv_info_t* server_list, uint32_t server_count);
bool is_route_server_available();
void send_to_all_route_server(CImPdu* pPdu);
CRouteServConn* get_route_serv_conn();


#endif /* ROUTESERVCONN_H_ */
