/*
 * ImPduBase.h
 *
 *  Created on: 2013-8-27
 *      Author: ziteng@mogujie.com
 */

#ifndef IMPDUBASE_H_
#define IMPDUBASE_H_

#include "UtilPdu.h"
#include <string.h>
#include <stdlib.h>
#define IM_PDU_HEADER_LEN		12
#define IM_PDU_VERSION			1
#define ROLE_XIAOXIAN_BIT		0x80000000	//
#define ROLE_XIAOXIA_BIT		0x40000000	//
#define ROLE_MASK				0xFFFFFF	//

// module id
enum {
	SID_LOGIN = 0x0001,			// for login
	SID_BUDDY_LIST = 0x0002,		// for friend list
	SID_MSG = 0x0003,			//
	SID_SWITCH_SERVICE = 0x0004, //
	SID_GROUP = 0x0005,			// for group message
	SID_FILE = 0x0006, 			//
	SID_OTHER = 0x0007,		//
};

// command id for login
enum {
    CID_LOGIN_REQ_MSGSERVER         = 1, 	//
    CID_LOGIN_RES_MSGSERVER         = 2,	//
    CID_LOGIN_REQ_USERLOGIN         = 3,	//
    CID_LOGIN_RES_USERLOGIN         = 4,	//
    CID_LOGIN_KICK_USER				= 7, 	//
};

// command id for msg
enum {
	CID_MSG_DATA			= 1,	//
	CID_MSG_DATA_ACK		= 2, 	//
	CID_MSG_READ_ACK		= 3, 	//
	CID_MSG_TIME_REQUEST 	= 5,	//
	CID_MSG_TIME_RESPONSE	= 6, 	//
	CID_MSG_UNREAD_CNT_REQUEST		= 7, 	//
	CID_MSG_UNREAD_CNT_RESPONSE		= 8,	//
	CID_MSG_UNREAD_MSG_REUQEST		= 9,	//
	//CID_MSG_LIST_RESPONSE			= 11,	//
	CID_MSG_UNREAD_MSG_RESPONSE 	= 14,
};

// command id for buddy list
enum {
	CID_BUDDY_LIST_REQUEST				= 1, //
	CID_BUDDY_LIST_FRIEND_LIST	 		= 3, //
	CID_BUDDY_LIST_ONLINE_FRIEND_LIST	= 4, //
	CID_BUDDY_LIST_STATUS_NOTIFY		= 5, //
	CID_BUDDY_LIST_USER_STATUS_REQUEST	= 8, //
	CID_BUDDY_LIST_USER_STATUS_RESPONSE	= 9, //
	CID_BUDDY_LIST_USER_INFO_RESPONSE	= 10, //
	CID_BUDDY_LIST_USER_INFO_REQUEST	= 11,
	CID_BUDDY_LIST_REMOVE_SESSION_REQ	= 12,
	CID_BUDDY_LIST_REMOVE_SESSION_RES	= 13,
	CID_BUDDY_LIST_ALL_USER_REQUEST		= 14,
	CID_BUDDY_LIST_ALL_USER_RESPONSE	= 15,
	CID_BUDDY_LIST_USERS_STATUS_REQUEST	= 16, //query list user's status
	CID_BUDDY_LIST_USERS_STATUS_RESPONSE= 17, //response list user's status
    CID_BUDDY_LIST_DEPARTMENT_REQUEST   = 18,   // query department
    CID_BUDDY_LIST_DEPARTMENT_RESPONSE  = 19,
};



// command id for group message
enum {
	CID_GROUP_LIST_REQUEST			= 1, 	// get group id list for an user
	CID_GROUP_LIST_RESPONSE			= 2,	// return group id list for an user
	CID_GROUP_USER_LIST_REQUEST		= 3,	// get user id list in a group
	CID_GROUP_USER_LIST_RESPONSE	= 4,	// return user list in a group
	CID_GROUP_UNREAD_CNT_REQUEST	= 5,
	CID_GROUP_UNREAD_CNT_RESPONSE	= 6,
	CID_GROUP_UNREAD_MSG_REQUEST	= 7,
	CID_GROUP_UNREAD_MSG_RESPONSE	= 8,
	CID_GROUP_MSG_READ_ACK			= 11,
	CID_GROUP_CREATE_TMP_GROUP_REQUEST	= 12,
	CID_GROUP_CREATE_TMP_GROUP_RESPONSE	= 13,
	CID_GROUP_CHANGE_MEMBER_REQUEST	= 14,
	CID_GROUP_CHANGE_MEMBER_RESPONSE	= 15,
	CID_GROUP_DIALOG_LIST_REQUEST	= 16,
	CID_GROUP_DIALOG_LIST_RESPONSE	= 17,
    CID_GROUP_CREATE_NORMAL_GROUP_NOTIFY = 18,
    CID_GROUP_CHANGE_MEMEBER_NOTIFY = 19,
};

// command id for switch service
enum {
	CID_SWITCH_P2P_CMD	= 1,	//
};

enum {
	// To FileServer
	CID_FILE_LOGIN_REQ				= 1, // sender/receiver need to login to
	CID_FILE_LOGIN_RES				= 2, // login success or failure
    CID_FILE_STATE                  = 3,
    CID_FILE_PULL_DATA_REQ          = 4,
    CID_FILE_PULL_DATA_RSP          = 5,

	// To MsgServer
	CID_FILE_REQUEST 				= 10, // sender -> receiver
	CID_FILE_RESPONSE				= 11, // receiver -> sender
	CID_FILE_NOTIFY              = 12,
	CID_FILE_HAS_OFFLINE_REQ		= 16,
	CID_FILE_HAS_OFFLINE_RES		= 17,
	CID_FILE_ADD_OFFLINE_REQ		= 18,
	CID_FILE_DEL_OFFLINE_REQ		= 19,
    
};

// message type: 1-between two user(peer to peer), 2-group message  3-audio message

#define MSG_TYPE_P2P_FLAG       0x00
#define MSG_TYPE_GROUP_FLAG     0x10

#define CHECK_MSG_TYPE_GROUP(msg_type) \
    ({\
        bool bRet = false;\
        if ((msg_type & MSG_TYPE_GROUP_FLAG) == MSG_TYPE_GROUP_FLAG)\
        {\
            bRet = true;\
        }\
        bRet;\
    })

enum {
	MSG_TYPE_P2P_TEXT       = MSG_TYPE_P2P_FLAG + 0x01,
    MSG_TYPE_P2P_AUDIO      = MSG_TYPE_P2P_FLAG + 0x02,
    MSG_TYPE_GROUP_TEXT     = MSG_TYPE_GROUP_FLAG + 0x01,
    MSG_TYPE_GROUP_AUDIO    = MSG_TYPE_GROUP_FLAG + 0x02,
};

// client type:
#define CLIENT_TYPE_FLAG_NONE    0x00
#define CLIENT_TYPE_FLAG_PC      0x01
#define CLIENT_TYPE_FLAG_MOBILE  0x02
#define CLIENT_TYPE_FLAG_BOTH    0x03

enum {
    CLIENT_TYPE_WINDOWS     = 0x01,
    CLIENT_TYPE_MAC         = 0x02,
    CLIENT_TYPE_IOS         = 0x11,
    CLIENT_TYPE_ANDROID     = 0x12,
};

#define CHECK_CLIENT_TYPE_PC(type) \
    ({\
        bool bRet = false;\
        if ((type & 0x10) == 0x00)\
        {\
            bRet = true;\
        }\
        bRet;\
    })

#define CHECK_CLIENT_TYPE_MOBILE(type) \
    ({\
        bool bRet = false;\
        if ((type & 0x10) == 0x10)\
        {\
            bRet = true;\
        }\
        bRet;\
    })



enum {
    GROUP_MEMBER_ADD_TYPE   = 0x00,
    GROUP_MEMBER_DEL_TYPE   = 0x01,
};

#define PARSE_PACKET_ASSERT if (is.GetPos() != (len - IM_PDU_HEADER_LEN)) { \
		throw CPduException(m_pdu_header.module_id, m_pdu_header.command_id, ERROR_CODE_PARSE_FAILED, "parse packet failed"); \
	}

#define ALLOC_FAIL_ASSERT(p) if (p == NULL) { \
		throw CPduException(m_pdu_header.module_id, m_pdu_header.command_id, ERROR_CODE_ALLOC_FAILED, "allocate failed"); \
	}

#ifdef WIN32
	#ifdef BUILD_PDU
	#define DLL_MODIFIER __declspec(dllexport)
	#else
	#define DLL_MODIFIER __declspec(dllimport)
	#endif
#else
	#define DLL_MODIFIER
#endif

typedef struct {
	uint32_t 	user_id;
	uint32_t	name_len;
	char*		name;
	uint32_t	nick_name_len;
	char*		nick_name;
	uint32_t	avatar_len;
	char*		avatar_url;		// user image url
    uint32_t    title_len;
    char*       title;          // 职务
    uint32_t    position_len;
    char*       position;       // 地址
    uint32_t    role_status;    // 用户在职状态 0:在职 1:离职
    uint32_t    sex;            // 0:女 1:男
    uint32_t    depart_id;      // 部门id
    uint32_t    job_num;        // 工号
    uint32_t    telphone_len;
    char*       telphone;       // 电话
    uint32_t    email_len;
    char*       email;          // 邮箱
	uint32_t	user_updated;   // 更新时间
} user_info_t;

typedef struct {
	uint32_t	id_len;
	char* 		id_url;
	uint32_t	name_len;
	char*		name;
	uint32_t	nick_name_len;
	char*		nick_name;
	uint32_t	avatar_len;
	char*		avatar_url;		// user image url
    uint32_t    title_len;
    char*       title;          // 职务
    uint32_t    position_len;
    char*       position;       // 地址
    uint32_t    role_status;     // 用户在职状态 0:在职 1:离职
    uint32_t    sex;            // 0:女 1:男
    uint32_t    depart_id_len;
    char*       depart_id_url;  // 部门id
    uint32_t    job_num;        // 工号
    uint32_t    telphone_len;
    char*       telphone;       // 电话
    uint32_t    email_len;
    char*       email;          // 邮箱
	uint32_t	user_updated;   // 更新时间
} client_user_info_t;


typedef struct {
    uint32_t    depart_id;          //部门id
    uint32_t    title_len;
    char*       title;             //部门名称
    uint32_t    description_len;
    char*       description;        //部门描述
    uint32_t    parent_depart_id;   //上级部门id
    uint32_t    leader;             //部门leader id
    uint32_t    status;         //部门状态  0:正常 1:删除
}department_info_t;

typedef struct {
    uint32_t    depart_id_url_len;
    char*       depart_id_url;      //部门id的url
    uint32_t    title_len;
    char*       title;             //部门名称
    uint32_t    description_len;
    char*       description;        //部门描述
    uint32_t    parent_depart_id_url_len;
    char*       parent_depart_id_url;   //上级部门id
    uint32_t    leader_len;
    char*       leader;             //部门leader id
    uint32_t    status;         //部门状态  0:正常 1:删除
} client_department_info_t;



typedef struct {
	uint32_t user_id;
	uint32_t status;
} user_stat_t;

typedef struct {
	uint32_t	id_len;
	char*		id_url;
	uint32_t	status;
} client_user_stat_t;

// 0x01: CLIENT_TYPE_PC
// 0x02: CLIENT_TYPE_MOBILE
// 0x03: CLIENT_TYPE_BOTH
typedef struct {
    uint32_t user_id;
    uint32_t status;
    uint32_t client_type_flag;
} user_conn_stat_t;

typedef struct {
	uint32_t 	from_user_id;
	uint32_t	from_name_len;
	char*	 	from_name;
	uint32_t	from_nick_name_len;
	char*		from_nick_name;
	uint32_t	from_avatar_len;
	char*		from_avatar_url;

	uint32_t 	create_time;
	uint8_t	 	msg_type;
	uint32_t 	msg_len;
	uchar_t* 	msg_data;
} server_msg_t;

typedef struct {
	uint32_t from_user_id;
	uint32_t unread_msg_cnt;
} UserUnreadMsgCnt_t;

typedef struct ip_addr_t{
    uint32_t ip_len;
    char*    ip;
    uint16_t port;
} ip_addr;

typedef struct svr_ip_addr_t{
	std::string ip;
	uint16_t port;
	
	svr_ip_addr_t() {
		port = 0;
	}

	svr_ip_addr_t (const char* addr, uint16_t p) {
		ip = addr;
		port = p;
	}
}svr_ip_addr;

//////////////////////////////
typedef struct {
	uint32_t 	length;		// the whole pdu length
	uint16_t	module_id;	//
	uint16_t	command_id;	//
	uint16_t 	version;	// pdu version number
	uint16_t	reserved;	//
} PduHeader_t;

class DLL_MODIFIER CImPdu
{
public:
	CImPdu();
	virtual ~CImPdu() {}

	uchar_t* GetBuffer();
	uint32_t GetLength();

	uint16_t GetVersion() { return m_pdu_header.version; }
	uint16_t GetModuleId() { return m_pdu_header.module_id; }
	uint16_t GetCommandId() { return m_pdu_header.command_id; }
	uint16_t GetReserved() { return m_pdu_header.reserved; }

	void SetVersion(uint16_t version);
	void SetReserved(uint16_t reserved);

	void WriteHeader();
	virtual uint16_t GetPduType() { return 0; }

	static int ReadPduHeader(uchar_t* buf, uint32_t len, PduHeader_t* header);
	static CImPdu* ReadPdu(uchar_t* buf, uint32_t len);
private:
	static CImPdu* ReadPduLogin(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len);
	static CImPdu* ReadPduBuddyList(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len);
	static CImPdu* ReadPduGroup(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len);
	static CImPdu* ReadPduMsg(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len);
	static CImPdu* ReadPduSwitchService(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len);
	static CImPdu* ReadPduFile(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len);
	static CImPdu* ReadPduOther(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len);

	static bool _IsPduAvailable(uchar_t* buf, uint32_t len, uint32_t& pdu_len);
	void _SetIncomingLen(uint32_t len) { m_incoming_len = len; }
	void _SetIncomingBuf(uchar_t* buf) { m_incoming_buf = buf; }

protected:
	CSimpleBuffer	m_buf;
	uchar_t*		m_incoming_buf;
	uint32_t		m_incoming_len;
	PduHeader_t		m_pdu_header;
};


#endif /* IMPDUBASE_H_ */
