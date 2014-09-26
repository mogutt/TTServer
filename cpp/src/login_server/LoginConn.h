/*
 * LoginConn.h
 *
 *  Created on: 2013-6-21
 *      Author: jianqingdu
 */

#ifndef LOGINCONN_H_
#define LOGINCONN_H_

#include "imconn.h"

enum {
	LOGIN_CONN_TYPE_CLIENT = 1,
	LOGIN_CONN_TYPE_MSG_SERV
};

class CLoginConn : public CImConn
{
public:
	CLoginConn();
	virtual ~CLoginConn();

	virtual void Close();

	void OnConnect2(net_handle_t handle, int conn_type);
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);

	virtual void HandlePdu(CImPdu* pPdu);
private:
	void _HandleMsgServInfo(CImPduMsgServInfo* pPdu);
	void _HandleUserCntUpdate(CImPduUserCntUpdate* pPdu);
	void _HandleMsgServRequest(CImPduMsgServRequest* pPdu);
    void _HandleUserConnInfo(CImPduUserConnInfo* pPdu);
private:
	int	m_conn_type;
};

void init_login_conn();

#endif /* LOGINCONN_H_ */
