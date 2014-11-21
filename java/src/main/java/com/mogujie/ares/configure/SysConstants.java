package com.mogujie.ares.configure;


public class SysConstants {

	// 服务器的默认端口，可以通过启动时的args来修改
	public static final int SERVER_DEFAULT_PORT = 11000;
	
	// 默认消息头的长度
	public static final int PROTOCOL_HEADER_LENGTH = 12;
	
	//packet的当前版本号,最新版本
	public static final int PROTOCOL_CURRENT_VERSION = 2;
	
	public static final int PROTOCOL_PREVIOUS_VERSION=1;		//上一版本的版本号
	
	// 定时任务启动状态
	public static final String TASK_ENABLE_RUN = "1";
	
	// 定时任务停止状态
	public static final String TASK_DISENABLE_RUN = "0";
	
	//停止应用的检查时间
	public static final long CHECK_STOP_GAP_TIME = 2000;
	
	//停止实例配置文件
	public static final String ELEGANT_STOP_FILE = "/tmp/.mogutalk_stop";
	
	public static final String ELEGANT_SHUTDOWN_SHELL_SCRIPT = "mogutalkshutdown";
	
	//停止实例值
	public static final String ELEGANT_STOP_CONTENT = "stop";
	
	// --------------- redis pool ---------------
	
	// 从数据库连接池中取得连接时，对其的有效性进行检查
	public static final boolean TEST_ON_BORROW = true;
	
	// 最大连接数
	public static final int MAX_ACTIVE = 36;
	
	// 最大闲置的连接数
	public static final int MAX_IDLE = 20;
	
	// 最小.....
	public static final int MIN_IDLE = 5;
	
	// 请求最长等待时间/毫秒
	public static final int MAX_WAIT = 1000; 
	
	// 闲置时测试
	public static final boolean TEST_WHILE_IDLE = true;
	
	// redis db的名字 
	public static final String REDIS_INSTANCE_NAME_COUNTER = "counter"; // 总消息计数器
	
	public static final String REDIS_INSTANCE_NAME_SESSION = "session"; // mogujie session
	
	public static final String REDIS_INSTANCE_NAME_UNREAD = "unread"; // 未读消息计数
	
	public static final String REDIS_CINFO_PREFIX_KEY="user_c_";		//redis cinfo的前缀key.
	
	public static final String REDIS_CINFO_SUB_KEY="chatNew";			//redis cinfo 子key
	
	// --------------- db pool -----------------
	
	public static final int DB_MIN_CONNECTIONS_PER_PARTITION = 10;
	
	public static final int DB_MAX_CONNECTIONS_PER_PARTITION = 18;
	
	public static final int DB_IDLE_CONNECTION_TEST_PERIOD = 60;
	
	public static final int DB_PARTITION_COUNT = 1;
}
