/*
 * ImPduBase.cpp
 *
 *  Created on: 2013-8-27
 *      Author: ziteng@mogujie.com
 */

#include "ImPduBase.h"
#include "ImPduClient.h"

#include "ImPduGroup.h"
#include "ImPduFile.h"

#ifndef WIN32
#include "ImPduServer.h"
#endif

CImPdu::CImPdu()
{
	m_incoming_buf = NULL;
	m_incoming_len = 0;

	m_pdu_header.module_id = SID_OTHER;
	m_pdu_header.command_id = 0;
    m_pdu_header.version = IM_PDU_VERSION;
	m_pdu_header.reserved = 0;
}

uchar_t* CImPdu::GetBuffer()
{
	if (m_incoming_buf)
		return m_incoming_buf;
	else
		return m_buf.GetBuffer();
}

uint32_t CImPdu::GetLength()
{
	if (m_incoming_buf)
		return m_incoming_len;
	else
		return m_buf.GetWriteOffset();
}

void CImPdu::WriteHeader()
{
	uchar_t* buf = GetBuffer();

	CByteStream::WriteInt32(buf, GetLength());
	CByteStream::WriteUint16(buf + 4, m_pdu_header.module_id);
	CByteStream::WriteUint16(buf + 6, m_pdu_header.command_id);
    CByteStream::WriteUint16(buf + 8, m_pdu_header.version);
	CByteStream::WriteUint16(buf + 10, m_pdu_header.reserved);
}

void CImPdu::SetVersion(uint16_t version)
{
	uchar_t* buf = GetBuffer();
	CByteStream::WriteUint16(buf + 8, version);
}

void CImPdu::SetReserved(uint16_t reserved)
{
	uchar_t* buf = GetBuffer();
	CByteStream::WriteUint16(buf + 10, reserved);
}

int CImPdu::ReadPduHeader(uchar_t* buf, uint32_t len, PduHeader_t* header)
{
	int ret = -1;
	if (len >= IM_PDU_HEADER_LEN && buf && header) {
		CByteStream is(buf, len);

		is >> header->length;
		is >> header->module_id;
		is >> header->command_id;
        is >> header->version;
		is >> header->reserved;

		ret = 0;
	}

	return ret;
}

CImPdu* CImPdu::ReadPdu(uchar_t *buf, uint32_t len)
{
	uint32_t pdu_len = 0;
	if (!_IsPduAvailable(buf, len, pdu_len))
		return NULL;

	uint16_t service_id = CByteStream::ReadUint16(buf + 4);
	uint16_t command_id = CByteStream::ReadUint16(buf + 6);
	CImPdu* pPdu = NULL;

	switch (service_id)
	{
	case SID_LOGIN:
		pPdu = ReadPduLogin(command_id, buf, pdu_len);
		break;
	case SID_BUDDY_LIST:
		pPdu = ReadPduBuddyList(command_id, buf, pdu_len);
		break;
	case SID_GROUP:
		pPdu = ReadPduGroup(command_id, buf, pdu_len);
		break;
	case SID_MSG:
		pPdu = ReadPduMsg(command_id, buf, pdu_len);
		break;
	case SID_OTHER:
		pPdu = ReadPduOther(command_id, buf, pdu_len);
		break;
	case SID_SWITCH_SERVICE:
		pPdu = ReadPduSwitchService(command_id, buf, pdu_len);
		break;
	case SID_FILE:
		pPdu = ReadPduFile(command_id, buf, pdu_len);
		break;
	default:
		throw CPduException(service_id, command_id, ERROR_CODE_WRONG_SERVICE_ID, "wrong service id");
	}

	pPdu->_SetIncomingLen(pdu_len);
	pPdu->_SetIncomingBuf(buf);
	return pPdu;
}

CImPdu* CImPdu::ReadPduLogin(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len)
{
	CImPdu* pPdu = NULL;
	switch (command_id) {
	case CID_LOGIN_REQ_MSGSERVER:
		pPdu = new CImPduMsgServRequest(pdu_buf, pdu_len);
		break;
	case CID_LOGIN_RES_MSGSERVER:
		pPdu = new CImPduMsgServResponse(pdu_buf, pdu_len);
		break;
	case CID_LOGIN_REQ_USERLOGIN:
		pPdu = new CImPduLoginRequest(pdu_buf, pdu_len);
		break;
	case CID_LOGIN_RES_USERLOGIN:
		pPdu = new CImPduLoginResponse(pdu_buf, pdu_len);
		break;
	case CID_LOGIN_KICK_USER:
		pPdu = new CImPduKickUser(pdu_buf, pdu_len);
		break;
	default:
		throw CPduException(SID_LOGIN, command_id, ERROR_CODE_WRONG_COMMAND_ID, "wrong command id");
	}

	return pPdu;
}

CImPdu* CImPdu::ReadPduBuddyList(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len)
{
	CImPdu* pPdu = NULL;
	switch (command_id) {
	case CID_BUDDY_LIST_REQUEST:
		pPdu = new CImPduClientBuddyListRequest(pdu_buf, pdu_len);
		break;
	case CID_BUDDY_LIST_FRIEND_LIST:
		pPdu = new CImPduClientFriendList(pdu_buf, pdu_len);
		break;
	case CID_BUDDY_LIST_ONLINE_FRIEND_LIST:
		pPdu = new CImPduClientOnlineFriendList(pdu_buf, pdu_len);
		break;
	case CID_BUDDY_LIST_STATUS_NOTIFY:
		pPdu = new CImPduClientFriendNotify(pdu_buf, pdu_len);
		break;
	case CID_BUDDY_LIST_USER_STATUS_REQUEST:
		pPdu = new CImPduClientUserStatusRequest(pdu_buf, pdu_len);
		break;
	case CID_BUDDY_LIST_USER_STATUS_RESPONSE:
		pPdu = new CImPduClientUserStatusResponse(pdu_buf, pdu_len);
		break;
	case CID_BUDDY_LIST_USER_INFO_RESPONSE:
	case CID_BUDDY_LIST_ALL_USER_RESPONSE:
		pPdu = new CImPduClientUserInfoResponse(pdu_buf, pdu_len);
		break;
	case CID_BUDDY_LIST_USER_INFO_REQUEST:
		pPdu = new CImPduClientUserInfoRequest(pdu_buf, pdu_len);
		break;
	case CID_BUDDY_LIST_REMOVE_SESSION_REQ:
		pPdu = new CImPduClientRemoveSessionRequest(pdu_buf, pdu_len);
		break;
	case CID_BUDDY_LIST_REMOVE_SESSION_RES:
		pPdu = new CImPduClientRemoveSessionResponse(pdu_buf, pdu_len);
		break;
	case CID_BUDDY_LIST_ALL_USER_REQUEST:
		pPdu = new CImPduClientAllUserRequest(pdu_buf, pdu_len);
		break;
	case CID_BUDDY_LIST_USERS_STATUS_REQUEST:
		pPdu = new CImPduClientUsersStatusRequest(pdu_buf, pdu_len);
		break;
	case CID_BUDDY_LIST_USERS_STATUS_RESPONSE:
		pPdu = new CImPduClientUsersStatusResponse(pdu_buf, pdu_len);
		break;
    case CID_BUDDY_LIST_DEPARTMENT_REQUEST:
        pPdu = new CImPduClientDepartmentRequest(pdu_buf, pdu_len);
        break;
    case CID_BUDDY_LIST_DEPARTMENT_RESPONSE:
        pPdu = new CImPduClientDepartmentResponse(pdu_buf, pdu_len);
        break;
	default:
		throw CPduException(SID_BUDDY_LIST, command_id, ERROR_CODE_WRONG_COMMAND_ID, "wrong command id");
	}

	return pPdu;
}

CImPdu* CImPdu::ReadPduGroup(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len)
{
	CImPdu* pPdu = NULL;

	switch (command_id) {
	case CID_GROUP_LIST_REQUEST:
	case CID_GROUP_DIALOG_LIST_REQUEST:
		pPdu = new CImPduClientGroupListRequest(pdu_buf, pdu_len);
		break;
	case CID_GROUP_LIST_RESPONSE:
	case CID_GROUP_DIALOG_LIST_RESPONSE:
		pPdu = new CImPduClientGroupListResponse(pdu_buf, pdu_len);
		break;
	case CID_GROUP_USER_LIST_REQUEST:
		pPdu = new CImPduClientGroupUserListRequest(pdu_buf, pdu_len);
		break;
	case CID_GROUP_USER_LIST_RESPONSE:
		pPdu = new CImPduClientGroupUserListResponse(pdu_buf, pdu_len);
		break;
	case CID_GROUP_UNREAD_CNT_REQUEST:
		pPdu = new CImPduClientGroupUnreadMsgCntRequest(pdu_buf, pdu_len);
		break;
	case CID_GROUP_UNREAD_CNT_RESPONSE:
		pPdu = new CImPduClientGroupUnreadMsgCntResponse(pdu_buf, pdu_len);
		break;
	case CID_GROUP_UNREAD_MSG_REQUEST:
		pPdu = new CImPduClientGroupUnreadMsgRequest(pdu_buf, pdu_len);
		break;
	case CID_GROUP_UNREAD_MSG_RESPONSE:
		pPdu = new CImPduClientGroupMsgListResponse(pdu_buf, pdu_len);
		break;
	case CID_GROUP_MSG_READ_ACK:
		pPdu = new CImPduClientGroupMsgReadAck(pdu_buf, pdu_len);
		break;
	case CID_GROUP_CREATE_TMP_GROUP_REQUEST:
		pPdu = new CImPduClientGroupCreateTmpGroupRequest(pdu_buf, pdu_len);
		break;
	case CID_GROUP_CREATE_TMP_GROUP_RESPONSE:
		pPdu = new CImPduClientGroupCreateTmpGroupResponse(pdu_buf, pdu_len);
		break;
	case CID_GROUP_CHANGE_MEMBER_REQUEST:
		pPdu = new CImPduClientGroupChangeMemberRequest(pdu_buf, pdu_len);
		break;
	case CID_GROUP_CHANGE_MEMBER_RESPONSE:
		pPdu = new CImPduClientGroupChangeMemberResponse(pdu_buf, pdu_len);
		break;
    case CID_GROUP_CHANGE_MEMEBER_NOTIFY:
        pPdu = new CImPduClientGroupChangeMemberNotify(pdu_buf, pdu_len);
        break;
    case CID_GROUP_CREATE_NORMAL_GROUP_NOTIFY:
        pPdu = new CImPduClientGroupCreateNormalGroupNotify(pdu_buf, pdu_len);
        break;
	default:
		throw CPduException(SID_GROUP, command_id, ERROR_CODE_WRONG_COMMAND_ID, "wrong command id");
	}

	return pPdu;
}

CImPdu* CImPdu::ReadPduMsg(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len)
{
	CImPdu* pPdu = NULL;
	switch (command_id) {
	case CID_MSG_DATA:
		pPdu = new CImPduClientMsgData(pdu_buf, pdu_len);
		break;
	case CID_MSG_DATA_ACK:
		pPdu = new CImPduClientMsgDataAck(pdu_buf, pdu_len);
		break;
	case CID_MSG_READ_ACK:
		pPdu = new CImPduClientMsgReadAck(pdu_buf, pdu_len);
		break;
	case CID_MSG_TIME_REQUEST:
		pPdu = new CImPduClientTimeRequest(pdu_buf, pdu_len);
		break;
	case CID_MSG_TIME_RESPONSE:
		pPdu = new CImPduClientTimeResponse(pdu_buf, pdu_len);
		break;
	case CID_MSG_UNREAD_CNT_REQUEST:
		pPdu = new CImPduClientUnreadMsgCntRequest(pdu_buf, pdu_len);
		break;
	case CID_MSG_UNREAD_CNT_RESPONSE:
		pPdu = new CImPduClientUnreadMsgCntResponse(pdu_buf, pdu_len);
		break;
	case CID_MSG_UNREAD_MSG_REUQEST:
		pPdu = new CImPduClientUnreadMsgRequest(pdu_buf, pdu_len);
		break;
	case CID_MSG_UNREAD_MSG_RESPONSE:
		pPdu = new CImPduClientMsgListResponse(pdu_buf, pdu_len);
		break;
	default:
		throw CPduException(SID_MSG, command_id, ERROR_CODE_WRONG_COMMAND_ID, "wrong command id");
	}

	return pPdu;
}

CImPdu* CImPdu::ReadPduSwitchService(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len)
{
	CImPdu* pPdu = NULL;
	switch (command_id)
	{
	case CID_SWITCH_P2P_CMD:
		pPdu = new CImPduClientP2PCmdMsg(pdu_buf, pdu_len);
		break;
	default:
		throw CPduException(SID_MSG, command_id, ERROR_CODE_WRONG_COMMAND_ID, "wrong command id");
	}
	return pPdu;
}

CImPdu* CImPdu::ReadPduFile(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len)
{
	CImPdu* pPdu = NULL;

	switch (command_id) {
	case CID_FILE_LOGIN_REQ:
		pPdu = new CImPduClientFileLoginReq(pdu_buf, pdu_len);
		break;
	case CID_FILE_LOGIN_RES:
		pPdu = new CImPduClientFileLoginRes(pdu_buf, pdu_len);
		break;
	case CID_FILE_REQUEST:
		pPdu = new CImPduClientFileRequest(pdu_buf, pdu_len);
		break;
	case CID_FILE_RESPONSE:
		pPdu = new CImPduClientFileResponse(pdu_buf, pdu_len);
		break;
    case CID_FILE_NOTIFY:
        pPdu = new CImPduClientFileNotify(pdu_buf, pdu_len);
        break;
    case CID_FILE_HAS_OFFLINE_REQ:
        pPdu = new CImPduClientFileHasOfflineReq(pdu_buf, pdu_len);
        break;
    case CID_FILE_HAS_OFFLINE_RES:
        pPdu = new CImPduClientFileHasOfflineRes(pdu_buf, pdu_len);
        break;
    case CID_FILE_ADD_OFFLINE_REQ:
        pPdu = new CImPduClientFileAddOfflineReq(pdu_buf, pdu_len);
        break;
    case CID_FILE_DEL_OFFLINE_REQ:
        pPdu = new CImPduClientFileDelOfflineReq(pdu_buf, pdu_len);
        break;
    
    case CID_FILE_STATE:
        pPdu = new CImPduClientFileState(pdu_buf, pdu_len);
        break;
    case CID_FILE_PULL_DATA_REQ:
        pPdu = new CImPduClientFilePullDataReq(pdu_buf, pdu_len);
        break ;
    case CID_FILE_PULL_DATA_RSP:
        pPdu = new CImPduClientFilePullDataRsp(pdu_buf, pdu_len);
        break;
            
    case IM_PDU_TYPE_MSG_FILE_TRANSFER_REQ:
        pPdu = new CImPduMsgFileTransferReq(pdu_buf, pdu_len);
        break;
    case IM_PDU_TYPE_MSG_FILE_TRANSFER_RSP:
        pPdu = new CImPduMsgFileTransferRsp(pdu_buf, pdu_len);
        break ;
    
	default:
		throw CPduException(SID_FILE, command_id, ERROR_CODE_WRONG_COMMAND_ID, "wrong command id");
		break;
	}

	return pPdu;
}

CImPdu* CImPdu::ReadPduOther(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len)
{
	CImPdu* pPdu = NULL;
	switch (command_id)
	{
	// Client Interactive Packet
	case IM_PDU_TYPE_HEARTBEAT:
		pPdu = new CImPduHeartbeat(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_CLIENT_DB_QUERY_REQUEST:
		pPdu = new CImPduClientDBQueryRequest(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_CLIENT_DB_QUERY_RESPONSE:
		pPdu = new CImPduClientDBQueryResponse(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_CLIENT_SERVICE_SETTING:
		pPdu = new CImPduClientServiceSetting(pdu_buf, pdu_len);
		break;
            
#ifndef WIN32
	// business server
    case IM_PDU_TYPE_SERVER_KICK_USER:
        pPdu = new CImPduServerKickUser(pdu_buf, pdu_len);
        break;
	case IM_PDU_TYPE_STOP_RECEIVE_PACKET:
		pPdu = new CImPduStopReceivePacket(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_VALIDATE_RESPONSE:
		pPdu = new CImPduValidateResponse(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_FRIEND_LIST_RESPONSE:
		pPdu = new CImPduFriendListResponse(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_REMOVE_SESSION_RESPONSE:
		pPdu = new CImPduRemoveSessionResponse(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_USERS_INFO_RESPONSE:
	case IM_PDU_TYPE_ALL_USER_RESPONSE:
		pPdu = new CImPduUsersInfoResponse(pdu_buf, pdu_len);
		break;

	case IM_PDU_TYPE_DB_QUERY_RESPONSE:
		pPdu = new CImPduDBQueryResponse(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_MSG_DATA:
		pPdu = new CImPduMsgData(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_UNREAD_MSG_COUNT_RESPONSE:
		pPdu = new CImPduUnreadMsgCountResponse(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_MSG_LIST_RESPONSE:
		pPdu = new CImPduMsgListResponse(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_DB_WRITE_RESPONSE:
		pPdu = new CImPduDBWriteResponse(pdu_buf, pdu_len);
		break;
    case IM_PDU_TYPE_DEPARTMENT_RESPONSE:
        pPdu = new CImPduDepartmentResponse(pdu_buf, pdu_len);
        break;
    case IM_PDU_TYPE_USER_CLIENT_TYPE_REQUEST:
        pPdu = new CImPduUserClientTypeRequest(pdu_buf, pdu_len);
        break;
    case IM_PDU_TYPE_USER_CLIENT_TYPE_RESPONSE:
        pPdu = new CImPduUserClientTypeResponse(pdu_buf, pdu_len);
        break;
            
	// group related
	case IM_PDU_TYPE_GROUP_LIST_RESPONSE:
	case IM_PDU_TYPE_GROUP_DIALOG_LIST_RESPONSE:
		pPdu = new CImPduGroupListResponse(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_GROUP_USER_LIST_RESPONSE:
		pPdu = new CImPduGroupUserListResponse(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_GROUP_UNREAD_MSG_CNT_RESPONSE:
		pPdu = new CImPduGroupUnreadMsgCntResponse(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_GROUP_MSG_LIST_RESPONSE:
		pPdu = new CImPduGroupMsgListResponse(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_GROUP_CREATE_TMP_GROUP_RESPONSE:
		pPdu = new CImPduGroupCreateTmpGroupResponse(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_GROUP_CHANGE_MEMBER_RESPONSE:
		pPdu = new CImPduGroupChangeMemberResponse(pdu_buf, pdu_len);
		break;
    case IM_PDU_TYPE_GROUP_CHANGE_MEMBER_NOTIFY:
        pPdu = new CImPduGroupChangeMemberNotify(pdu_buf, pdu_len);
        break;
    case IM_PDU_TYPE_GROUP_CREATE_NORMAL_GROUP_NOTIFY:
        pPdu = new CImPduGroupCreateNormalGroupNotify(pdu_buf, pdu_len);
        break;

	// Message Server Internal Packet
    case IM_PDU_TYPE_USER_CONN_INFO:
        pPdu = new CImPduUserConnInfo(pdu_buf, pdu_len);
        break;
	case IM_PDU_TYPE_ROLE_SET:
		pPdu = new CImPduRoleSet(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_MSG_SERV_INFO:
		pPdu = new CImPduMsgServInfo(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_ONLINE_USER_INFO:
		pPdu = new CImPduOnlineUserInfo(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_USER_STATUS_UPDATE:
		pPdu = new CImPduUserStatusUpdate(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_USER_CNT_UPDATE:
		pPdu = new CImPduUserCntUpdate(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_FRIEND_STATUS_QUERY:
		pPdu = new CImPduFriendStatusQuery(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_FRIEND_STATUS_LIST:
		pPdu = new CImPduFriendStatusList(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_FRIEND_STATUS_NOTIFY:
		pPdu = new CImPduFriendStatusNotify(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_P2P_MSG:
		pPdu = new CImPduP2PMsg(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_USER_STATUS_REQUEST:
		pPdu = new CImPduUserStatusRequest(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_USERS_STATUS_REQUEST:
		pPdu = new CImPduUsersStatusRequest(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_USER_STATUS_RESPONSE:
		pPdu = new CImPduUserStatusResponse(pdu_buf, pdu_len);
		break;
	case IM_PDU_TYPE_USERS_STATUS_RESPONSE:
		pPdu = new CImPduUsersStatusResponse(pdu_buf, pdu_len);
		break;

    case IM_PDU_TYPE_GROUP_P2P_MSG_RESPONSE:
        pPdu = new CImPduGroupP2PMessageResponse(pdu_buf, pdu_len);
        break;

	// file transfer related
	case IM_PDU_TYPE_FILE_NOTIFY:
		pPdu = new CImPduFileNotify(pdu_buf, pdu_len);
		break;
    case IM_PDU_TYPE_FILE_HAS_OFFLINE_RES:
		pPdu = new CImPduFileHasOfflineRes(pdu_buf, pdu_len);
		break;
    case IM_PDU_TYPE_MSG_FILE_TRANSFER_REQ:
       pPdu = new CImPduMsgFileTransferReq(pdu_buf, pdu_len);
       break;
    case IM_PDU_TYPE_MSG_FILE_TRANSFER_RSP:
       pPdu = new CImPduMsgFileTransferRsp(pdu_buf, pdu_len);
       break;
    case IM_PDU_TYPE_FILE_SERVER_IP_REQUEST:
        pPdu = new CImPduFileServerIPReq(pdu_buf, pdu_len);
        break;
    case IM_PDU_TYPE_FILE_SERVER_IP_RESPONSE:
        pPdu = new CImPduFileServerIPRsp(pdu_buf, pdu_len);
        break;
#endif

	default:
		throw CPduException(SID_OTHER, command_id, ERROR_CODE_WRONG_COMMAND_ID, "wrong packet type");
		return NULL;
	}

	return pPdu;
}

bool CImPdu::_IsPduAvailable(uchar_t* buf, uint32_t len, uint32_t& pdu_len)
{
	if (len < IM_PDU_HEADER_LEN)
		return false;

	pdu_len = CByteStream::ReadUint32(buf);
	if (pdu_len > len)
	{
		//log("pdu_len=%d, len=%d\n", pdu_len, len);
		return false;
	}

	return true;
}

