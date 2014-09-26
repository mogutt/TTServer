/*
 * ServInfo.cpp
 *
 *  Created on: 2013-8-8
 *      Author: ziteng@mogujie.com
 */

#include "ServInfo.h"

serv_info_t* read_server_config(CConfigFileReader* config_file, const char* server_ip_format,
		const char* server_port_format, uint32_t& server_count)
{
	char server_ip_key[64];
	char server_port_key[64];

	server_count = 0;

	// get server_count first;
	while (true) {
		sprintf(server_ip_key, "%s%d", server_ip_format, server_count + 1);
		sprintf(server_port_key, "%s%d", server_port_format, server_count + 1);
		char* server_ip_value = config_file->GetConfigName(server_ip_key);
		char* server_port_value = config_file->GetConfigName(server_port_key);
		if (!server_ip_value || !server_port_value) {
			break;
		}

		server_count++;
	}

	if (server_count == 0) {
		return NULL;
	}

	serv_info_t* server_list = new serv_info_t [server_count];

	for (uint32_t i = 0; i < server_count; i++) {
		sprintf(server_ip_key, "%s%d", server_ip_format, i + 1);
		sprintf(server_port_key, "%s%d", server_port_format, i + 1);
		char* server_ip_value = config_file->GetConfigName(server_ip_key);
		char* server_port_value = config_file->GetConfigName(server_port_key);
		server_list[i].server_ip = server_ip_value;
		server_list[i].server_port = atoi(server_port_value);
	}

	return server_list;
}
