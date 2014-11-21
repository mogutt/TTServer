package com.mogujie.ares.configure;

/**
 * 
 * @Description: 业务相关的常量定义
 * @author ziye - ziye[at]mogujie.com
 * @date 2013-9-7 下午1:11:34
 * 
 */
public class BizConstants {

    // 默认的serviceId
    public static final int DEFAULT_SERVICEID = 7; // 与C++端通信的ID

    // ---------------- 消息类型的常量定义 ------------------

    public static final int COMMANDID_HEARTBEAT = 1; // 心跳包的commandId

    public static final int COMMANDID_STOP_RECEIVE = 100; // 停止的commandId

    public static final int COMMANDID_SEND_MESSAGE = 111; // 发送消息的commandId

    // ---------------- 用户角色定义 ----------------

    public static final int ROLE_NORMAL_USER = 0; // normal user

    public static final int ROLE_SERVICE_USER = 1; // admin user

    // ---------------- 用户信息默认值 ---------------

    public static final String USER_INFO_DEFAULT_AVATAR = "/avatar/avatar_default.jpg"; // 默认用户头像

    public static final String GROUP_INFO_DEFAULT_AVATAR = "/avatar/group_avatar_default.jpg"; // 默认群组头像
    
    // ---------------- 消息 -------------------
    // 消息里图片的占位符
    public static final String MESSAGE_IMAGE_PREFIX = "&$#@~^@[{:";
    public static final String MESSAGE_IMAGE_SUFFIX = ":}]&$~@#@";
    public static final String Message_IMAGE_URL_REGEX = "(http://)?s\\d{1,2}";
    public static final String MESSAGE_IMAGE_FULL_PLACEHOLDER_REGEX = "(&\\$#@~\\^@\\[\\{:)"
            + "((http[s]?://)?s\\d{1,2}\\.mogujie\\.(com|cn))(/pic/\\d{6}/[0-9a-z_]{30,40}\\d{1,10}x\\d{1,10}.*"
            + ":\\}\\]&\\$~@#@)"; // 图片占位符的正则表达式
    public static final String MESSAGE_IMAGE_PART_PLACEHOLDER_REGEX = "(&\\$#@~\\^@\\[\\{:)"
            + "(/pic/\\d{6}/[0-9a-z_]{30,40}\\d{1,10}x\\d{1,10}.*"
            + ":\\}\\]&\\$~@#@)"; // 图片占位符的正则表达式

    // ------------ IM消息的type ------------    
	public static final int MESSAGE_TYPE_IM = 0x01; // 普通用户+系统消息
	public static final int MESSAGE_TYPE_IM_GROUP = 0x11;
	public static final int MESSAGE_TYPE_IM_AUDIO = 0x02;   			
	public static final int MESSAGE_TYPE_IM_GROUP_AUDIO = 0x12;

    // 群组
    public static final String GROUP_TOTAL_MSG_COUNTER_REDIS_KEY_SUFFIX = "_group_msg"; // GROUP_TOTAL_MSG_COUNTER_REDIS_KEY_SUFFIX

    // {userId} + "_" + {groupId} + GROUP_USER_MSG_COUNTER_REDIS_KEY_SUFFIX
    public static final String GROUP_USER_MSG_COUNTER_REDIS_KEY_SUFFIX = "_user_group";

    // hashtable里的两个subkey
    public static final String GROUP_COUNTER_SUBKEY_COUNTER = "count";
    public static final String GROUP_COUNTER_SUBKEY_LASTID = "lastId";

    public static final int GROUP_UNREAD_MAX_COUNTER = 150; // 用户一次读取未读群消息不能超过200

    public static final int GROUP_TYPE_FIXED = 1;

    public static final int GROUP_TYPE_TEMP = 2;

    public static final int UNREAD_MAX_COUNTER = 100; // 用户一次读取未读消息不能超过100

    // ------------- 服务号小T ---------------
    public static final int SYS_SERVER_USER_ID = 10000;
    public static final String SYS_SERVER_USER_NAME = "小T";

}
