 /*
 * imconn.cpp
 *
 *  Created on: 2013-6-5
 *      Author: ziteng@mogujie.com
 */

#include "imconn.h"

#define POLICY_FILE_NAME	"policy.xml"

static char* g_policy_content = NULL;
static uint32_t g_policy_len = 0;
static uint32_t g_send_pkt_cnt = 0;		// 发送数据包总数
static uint32_t g_recv_pkt_cnt = 0;		// 接收数据包总数

static CImConn* FindImConn(ConnMap_t* imconn_map, net_handle_t handle)
{
	CImConn* pConn = NULL;
	ConnMap_t::iterator iter = imconn_map->find(handle);
	if (iter != imconn_map->end())
	{
		pConn = iter->second;
		pConn->AddRef();
	}

	return pConn;
}

void imconn_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	NOTUSED_ARG(handle);
	NOTUSED_ARG(pParam);

	if (!callback_data)
		return;

	ConnMap_t* conn_map = (ConnMap_t*)callback_data;
	CImConn* pConn = FindImConn(conn_map, handle);
	if (!pConn)
		return;

	//log("msg=%d, handle=%d\n", msg, handle);

	switch (msg)
	{
	case NETLIB_MSG_CONFIRM:
		pConn->OnConfirm();
		break;
	case NETLIB_MSG_READ:
		pConn->OnRead();
		break;
	case NETLIB_MSG_WRITE:
		pConn->OnWrite();
		break;
	case NETLIB_MSG_CLOSE:
		pConn->OnClose();
		break;
	default:
		log("!!!imconn_callback error msg: %d\n", msg);
		break;
	}

	pConn->ReleaseRef();
}

//////////////////////////
CImConn::CImConn()
{
	//log("CImConn::CImConn\n");

	m_busy = false;
	m_handle = NETLIB_INVALID_HANDLE;
	m_recv_bytes = 0;
	m_policy_conn = false;

	m_last_send_tick = m_last_recv_tick = get_tick_count();
}

CImConn::~CImConn()
{
	//log("CImConn::~CImConn, handle=%d\n", m_handle);
}

int CImConn::Send(void* data, int len)
{
	m_last_send_tick = get_tick_count();
	++g_send_pkt_cnt;

	if (m_busy)
	{
		m_out_buf.Write(data, len);
		return len;
	}

	int offset = 0;
	int remain = len;
	while (remain > 0) {
		int send_size = remain;
		if (send_size > NETLIB_MAX_SOCKET_BUF_SIZE) {
			send_size = NETLIB_MAX_SOCKET_BUF_SIZE;
		}

		int ret = netlib_send(m_handle, (char*)data + offset , send_size);
		if (ret <= 0) {
			ret = 0;
			break;
		}

		offset += ret;
		remain -= ret;
	}

	if (remain > 0)
	{
		m_out_buf.Write((char*)data + offset, remain);
		m_busy = true;
		log("send busy, remain=%d\n", m_out_buf.GetWriteOffset());
	}

	return len;
}

void CImConn::OnRead()
{
	for (;;)
	{
		uint32_t free_buf_len = m_in_buf.GetAllocSize() - m_in_buf.GetWriteOffset();
		if (free_buf_len < READ_BUF_SIZE)
			m_in_buf.Extend(READ_BUF_SIZE);

		int ret = netlib_recv(m_handle, m_in_buf.GetBuffer() + m_in_buf.GetWriteOffset(), READ_BUF_SIZE);
		if (ret <= 0)
			break;

		m_recv_bytes += ret;
		m_in_buf.IncWriteOffset(ret);

		m_last_recv_tick = get_tick_count();
	}

	if (m_policy_conn) {
		return;
	}

	// no received data is read by ReadPdu(), check if this is a flash security policy request
	if (m_recv_bytes == m_in_buf.GetWriteOffset()) {
		if ( (m_in_buf.GetBuffer()[0] == '<') && (g_policy_content != NULL) ) {
			log("policy request, handle=%d\n", m_handle);
			m_policy_conn = true;
			Send(g_policy_content, g_policy_len);
			return;
		}
	}

	try {
		CImPdu* pPdu = NULL;
		while ( ( pPdu = CImPdu::ReadPdu(m_in_buf.GetBuffer(), m_in_buf.GetWriteOffset()) ) )
		{
			uint32_t pdu_len = pPdu->GetLength();

			HandlePdu(pPdu);

			m_in_buf.Read(NULL, pdu_len);
			delete pPdu;

			++g_recv_pkt_cnt;
		}
	} catch (CPduException& ex) {
		log("!!!catch exception, sid=%u, cid=%u, err_code=%u, err_msg=%s, close the connection\n",
				ex.GetModuleId(), ex.GetCommandId(), ex.GetErrorCode(), ex.GetErrorMsg());
		OnClose();
	}
}

void CImConn::OnWrite()
{
	if (!m_busy)
		return;

	while (m_out_buf.GetWriteOffset() > 0) {
		int send_size = m_out_buf.GetWriteOffset();
		if (send_size > NETLIB_MAX_SOCKET_BUF_SIZE) {
			send_size = NETLIB_MAX_SOCKET_BUF_SIZE;
		}

		int ret = netlib_send(m_handle, m_out_buf.GetBuffer(), send_size);
		if (ret <= 0) {
			ret = 0;
			break;
		}

		m_out_buf.Read(NULL, ret);
	}

	if (m_out_buf.GetWriteOffset() == 0) {
		m_busy = false;
	}

	log("onWrite, remain=%d\n", m_out_buf.GetWriteOffset());
}




