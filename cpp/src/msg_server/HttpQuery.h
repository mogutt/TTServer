/*
 * HttpQuery.h
 *
 *  Created on: 2013-10-22
 *      Author: ziteng@mogujie.com
 */

#ifndef HTTPQUERY_H_
#define HTTPQUERY_H_

#include "HttpConn.h"
#include "jsonxx.h"

class CHttpQuery
{
public:
	virtual ~CHttpQuery() {}

	static CHttpQuery* GetInstance();

	static void DispatchQuery(std::string& url, std::string& post_data, CHttpConn* pHttpConn);
private:
	CHttpQuery() {}

	static void _QueryCreateNormalGroup(jsonxx::Object& post_json_obj, CHttpConn* pHttpConn);
    static void _QueryChangeMember(jsonxx::Object& post_json_obj, CHttpConn* pHttpConn);
private:
	static CHttpQuery*	m_query_instance;
};

string PackSendOk();
string URLDecode(const string &sIn);
#endif /* HTTPQUERY_H_ */
