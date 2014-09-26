/*
 * HttpQuery.cpp
 *
 *  Created on: 2013-10-22
 *      Author: ziteng@mogujie.com
 */

#include "HttpQuery.h"
#include "RouteServConn.h"
#include "DBServConn.h"
#include "LoginServConn.h"
#include "ImUser.h"
#include "GroupChat.h"
static uint32_t g_total_query = 0;
static uint32_t g_last_year = 0;
static uint32_t g_last_month = 0;
static uint32_t g_last_mday = 0;

#define HTTP_QUEYR_HEADER "HTTP/1.1 200 OK\r\n"\
"Cache-Control:no-cache\r\n"\
"Connection:close\r\n"\
"Content-Length:%d\r\n"\
"Content-Type:text/html;charset=utf-8\r\n\r\n%s"

CHttpQuery* CHttpQuery::m_query_instance = NULL;

string PackSendOk()
{
    jsonxx::Object json_obj;
    
	json_obj << "result" << 0;
    
	std::string json_str = json_obj.json();
    char szRes[512] = {0};
    sprintf(szRes, HTTP_QUEYR_HEADER, json_str.size(), json_str.c_str());
    string response = szRes;
	return response;
}

inline char fromHex(const char &x)
{
    return isdigit(x) ? x-'0' : x-'A'+10;
}

string URLDecode(const string &sIn)
{
    string sOut;
    for( size_t ix = 0; ix < sIn.size(); ix++ )
    {
        char ch = 0;
        if(sIn[ix]=='%')
        {
            ch = (fromHex(sIn[ix+1])<<4);
            ch |= fromHex(sIn[ix+2]);
            ix += 2;
        }
        else if(sIn[ix] == '+')
        {
            ch = ' ';
        }
        else
        {
            ch = sIn[ix];
        }
        sOut += (char)ch;
    }
    return sOut;
}


void http_query_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	struct tm* tm;
	time_t currTime;

	time(&currTime);
	tm = localtime(&currTime);

	uint32_t year = tm->tm_year + 1900;
	uint32_t mon = tm->tm_mon + 1;
	uint32_t mday = tm->tm_mday;
	if (year != g_last_year || mon != g_last_month || mday != g_last_mday) {
		// a new day begin, clear the count
		log("a new day begin, g_total_query=%u\n", g_total_query);
		g_total_query = 0;
		g_last_year = year;
		g_last_month = mon;
		g_last_mday = mday;
	}
}

CHttpQuery* CHttpQuery::GetInstance()
{
	if (!m_query_instance) {
		m_query_instance = new CHttpQuery();
		netlib_register_timer(http_query_timer_callback, NULL, 1000);
	}

	return m_query_instance;
}

void CHttpQuery::DispatchQuery(std::string& url, std::string& post_data, CHttpConn* pHttpConn)
{
	++g_total_query;
	log("DispatchQuery, url=%s, content=%s\n", url.c_str(), post_data.c_str());
	jsonxx::Object json_obj;

	if ( !json_obj.parse(post_data) ) {
		log("json parse failed, post_data=%s\n", post_data.c_str());
		pHttpConn->Close();
		return;
	}

	// process post request with post content
	if (strcmp(url.c_str(), "/query/createNormalGroup") == 0) {
		_QueryCreateNormalGroup(json_obj, pHttpConn);
	}
    else if (strcmp(url.c_str(), "/query/changeMembers") == 0) {
        _QueryChangeMember(json_obj, pHttpConn);
    }
    else {
		log("url not support\n");
		pHttpConn->Close();
		return;
	}
}

void CHttpQuery::_QueryCreateNormalGroup(jsonxx::Object &post_json_obj, CHttpConn *pHttpConn)
{
    CRouteServConn *pConn = get_route_serv_conn();
    if (!pConn) {
		log("no connection to RouteServConn\n");
		pHttpConn->Close();
		return;
	}
    if ( !post_json_obj.has<jsonxx::Number>("group_id") ) {
		log("no group id\n");
		pHttpConn->Close();
		return;
	}
    
    if (!post_json_obj.has<jsonxx::String>("group_name")) {
        log("no group name\n");
        pHttpConn->Close();
		return;
    }
    
    if (!post_json_obj.has<jsonxx::String>("group_avatar")) {
        log("no group avatar\n");
        pHttpConn->Close();
		return;
    }
    
    if (!post_json_obj.has<jsonxx::Array>("user_list")) {
        log("no user list\n");
        pHttpConn->Close();
		return;
    }
    
    string response = PackSendOk();
    pHttpConn->Send((void*)response.c_str(), response.length());
    pHttpConn->Close();
    
    uint32_t group_id = post_json_obj.get<jsonxx::Number>("group_id");
    string group_name = post_json_obj.get<jsonxx::String>("group_name");
    string group_avatar = post_json_obj.get<jsonxx::String>("group_avatar");
    group_name = URLDecode(group_name);
    jsonxx::Array user_array = post_json_obj.get<jsonxx::Array>("user_list");
    uint32_t user_cnt = user_array.size();
    log("QueryCreateNormalGroup, group_id: %u, group_name: %s, user_cnt: %u.\n", group_id,
        group_name.c_str(), user_cnt);
    uint32_t* user_list = NULL;
    if (user_cnt > 0) {
        user_list = new uint32_t[user_cnt];
        if (NULL == user_list) {
            log("_QueryCreateNormalGroup, alloced failed");
            return;
        }
        for (uint32_t i = 0; i < user_array.size(); i++) {
            user_list[i] = user_array.get<jsonxx::Number>(i);
        }
    }
    
    CGroupChat::GetInstance()->HandleGroupCreateNormalGroupNotify(group_id, group_name.c_str(),
                                                                 group_avatar.c_str(), user_cnt, user_list);
    
    CImPduGroupCreateNormalGroupNotify pdu(group_id, group_name.c_str(), group_avatar.c_str(),
                                           user_cnt, user_list);
    pConn->SendPdu(&pdu);
}

void CHttpQuery::_QueryChangeMember(jsonxx::Object &post_json_obj, CHttpConn *pHttpConn)
{
    CRouteServConn *pConn = get_route_serv_conn();
    if (!pConn) {
		log("no connection to RouteServConn\n");
		pHttpConn->Close();
		return;
	}
    if ( !post_json_obj.has<jsonxx::Number>("group_id") ) {
		log("no group id\n");
		pHttpConn->Close();
		return;
	}
    
    if (!post_json_obj.has<jsonxx::Array>("user_list")) {
        log("no user list\n");
        pHttpConn->Close();
		return;
    }
    
    string response = PackSendOk();
    pHttpConn->Send((void*)response.c_str(), response.length());
    pHttpConn->Close();
    
    uint32_t group_id = post_json_obj.get<jsonxx::Number>("group_id");
    jsonxx::Array user_array = post_json_obj.get<jsonxx::Array>("user_list");
    uint32_t user_cnt = user_array.size();
    log("QueryChangeMember, group_id: %u, user_cnt: %u.\n", group_id,
        user_cnt);

    uint32_t* user_list = NULL;
    if (user_cnt > 0) {
        user_list = new uint32_t[user_cnt];
        if (NULL == user_list) {
            log("_QueryCreateNormalGroup, alloced failed");
            return;
        }
        for (uint32_t i = 0; i < user_array.size(); i++) {
            user_list[i] = user_array.get<jsonxx::Number>(i);
        }
    }
    CGroupChat::GetInstance()->HandleGroupChangeMemberNotify(group_id, user_cnt,
                                                             user_list);

    CImPduGroupChangeMemberNotify pdu( group_id, user_cnt, user_list);
    pConn->SendPdu(&pdu);
    
}

