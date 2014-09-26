/*
 * LoginServConn.h
 *
 *  Created on: 2013-7-8
 *      Author: ziteng@mogujie.com
 */

#ifndef LOGINSERVCONN_H_
#define LOGINSERVCONN_H_

#include "imconn.h"
#include "ServInfo.h"

class CLoginServConn : public CImConn
{
public:
	CLoginServConn();
	virtual ~CLoginServConn();

	bool IsOpen() { return m_bOpen; }

	void Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx);
	virtual void Close();

	virtual void OnConfirm();
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);

	virtual void HandlePdu(CImPdu* pPdu);
private:
	bool 		m_bOpen;
	uint32_t	m_serv_idx;
};

void init_login_serv_conn(serv_info_t* server_list, uint32_t server_count, const char* msg_server_ip_addr1,
		const char* msg_server_ip_addr2, uint16_t msg_server_port, uint32_t max_conn_cnt);
bool is_login_server_available();
void send_to_all_login_server(CImPdu* pPdu);


#endif /* MSGCONN_LS_H_ */
