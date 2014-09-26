/*
 * msg_server.cpp
 *
 *  Created on: 2013-6-21
 *      Author: ziteng@mogujie.com
 */

#include "netlib.h"
#include "ConfigFileReader.h"
#include "MsgConn.h"
#include "LoginServConn.h"
#include "RouteServConn.h"
#include "DBServConn.h"
#include "HttpConn.h"
#include "FileServConn.h"
#include "version.h"

#define DEFAULT_CONCURRENT_DB_CONN_CNT  10

// for client connect in
void msg_serv_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	if (msg == NETLIB_MSG_CONNECT)
	{
		CMsgConn* pConn = new CMsgConn();
		pConn->OnConnect(handle);
	}
	else
	{
		log("!!!error msg: %d\n", msg);
	}
}

// for client connect in
void http_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	if (msg == NETLIB_MSG_CONNECT)
	{
		CHttpConn* pConn = new CHttpConn();
		pConn->OnConnect(handle);
	}
	else
	{
		log("!!!error msg: %d\n", msg);
	}
}


int main(int argc, char* argv[])
{
	if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
		printf("Server Version: MsgServer/%s\n", VERSION);
		printf("Server Build: %s %s\n", __DATE__, __TIME__);
		return 0;
	}

	signal(SIGPIPE, SIG_IGN);
	srand(time(NULL));

	log("MsgServer max files can open: %d\n", getdtablesize());

	CConfigFileReader config_file("msgserver.conf");

	char* listen_ip = config_file.GetConfigName("ListenIP");
	char* str_listen_port = config_file.GetConfigName("ListenPort");
	char* ip_addr1 = config_file.GetConfigName("IpAddr1");	// 电信IP
	char* ip_addr2 = config_file.GetConfigName("IpAddr2");	// 网通IP
	char* str_max_conn_cnt = config_file.GetConfigName("MaxConnCnt");
    char* http_listen_ip = config_file.GetConfigName("HttpListenIP");
    char* str_http_listen_port = config_file.GetConfigName("HttpListenPort");
    
	uint32_t db_server_count = 0;
	serv_info_t* db_server_list = read_server_config(&config_file, "DBServerIP", "DBServerPort", db_server_count);

	uint32_t login_server_count = 0;
	serv_info_t* login_server_list = read_server_config(&config_file, "LoginServerIP", "LoginServerPort", login_server_count);

	uint32_t route_server_count = 0;
	serv_info_t* route_server_list = read_server_config(&config_file, "RouteServerIP", "RouteServerPort", route_server_count);

    uint32_t file_server_count = 0;
    serv_info_t* file_server_list = read_server_config(&config_file, "FileServerIP",
                                                       "FileServerPort", file_server_count);
	// 必须至少配置2个BusinessServer实例, 一个用于用户登录业务，一个用于其他业务
	// 这样当其他业务量非常繁忙时，也不会影响客服端的登录验证
	// 建议配置4个实例，这样更新BusinessServer时，不会影响业务
	if (db_server_count < 2) {
		log("DBServerIP need 2 instance at lest\n");
		return 1;
	}

	// 到BusinessServer的开多个并发的连接
	uint32_t concurrent_db_conn_cnt = DEFAULT_CONCURRENT_DB_CONN_CNT;
	uint32_t db_server_count2 = db_server_count * DEFAULT_CONCURRENT_DB_CONN_CNT;
	char* concurrent_db_conn = config_file.GetConfigName("ConcurrentDBConnCnt");
	if (concurrent_db_conn) {
		concurrent_db_conn_cnt  = atoi(concurrent_db_conn);
		db_server_count2 = db_server_count * concurrent_db_conn_cnt;
	}

	serv_info_t* db_server_list2 = new serv_info_t [ db_server_count2];
	for (uint32_t i = 0; i < db_server_count2; i++) {
		db_server_list2[i].server_ip = db_server_list[i / concurrent_db_conn_cnt].server_ip.c_str();
		db_server_list2[i].server_port = db_server_list[i / concurrent_db_conn_cnt].server_port;
	}

	if (!listen_ip || !str_listen_port || !ip_addr1 || !http_listen_ip || !str_http_listen_port) {
		log("config file miss, exit...\n");
		return -1;
	}

	// 没有IP2，就用第一个IP
	if (!ip_addr2) {
		ip_addr2 = ip_addr1;
	}

	uint16_t listen_port = atoi(str_listen_port);
	uint32_t max_conn_cnt = atoi(str_max_conn_cnt);
    uint16_t http_listen_port = atoi(str_http_listen_port);

	int ret = netlib_init();

	if (ret == NETLIB_ERROR)
		return ret;

	CStrExplode listen_ip_list(listen_ip, ';');
	for (uint32_t i = 0; i < listen_ip_list.GetItemCnt(); i++) {
		ret = netlib_listen(listen_ip_list.GetItem(i), listen_port, msg_serv_callback, NULL);
		if (ret == NETLIB_ERROR)
			return ret;
	}

    if (netlib_listen(http_listen_ip, http_listen_port, http_callback, NULL) == NETLIB_ERROR)
    {
        log("http listen failed, ip: %s, port:%u\n", http_listen_ip, http_listen_port);
    }
    
	printf("server start listen on: %s:%d\n", listen_ip, listen_port);

	init_msg_conn();
    
    init_http_conn();

    init_file_serv_conn(file_server_list, file_server_count);
    
	init_db_serv_conn(db_server_list2, db_server_count2, concurrent_db_conn_cnt);

	init_login_serv_conn(login_server_list, login_server_count, ip_addr1, ip_addr2, listen_port, max_conn_cnt);

	init_route_serv_conn(route_server_list, route_server_count);

	printf("now enter the event loop...\n");

	netlib_eventloop();

	return 0;
}
