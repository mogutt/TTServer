//
//  HttpPdu.cpp
//  http_msg_server
//
//  Created by jianqing.du on 13-9-29.
//  Copyright (c) 2013年 ziteng. All rights reserved.
//

#include "HttpParserWrapper.h"
#include "http_parser.h"

CHttpParserWrapper* CHttpParserWrapper::m_instance = NULL;

CHttpParserWrapper::CHttpParserWrapper()
{

}

CHttpParserWrapper* CHttpParserWrapper::GetInstance()
{
    if (!m_instance) {
        m_instance = new CHttpParserWrapper();
    }
    
    return m_instance;
}

void CHttpParserWrapper::ParseHttpContent(const char* buf, uint32_t len)
{
	http_parser_init(&m_http_parser, HTTP_REQUEST);
	memset(&m_settings, 0, sizeof(m_settings));
	m_settings.on_url = OnUrl;
	m_settings.on_headers_complete = OnHeadersComplete;
	m_settings.on_body = OnBody;
	m_settings.on_message_complete = OnMessageComplete;

    m_read_all = false;
    m_total_length = 0;
    m_url.clear();
    m_body_content.clear();
    
    http_parser_execute(&m_http_parser, &m_settings, buf, len);
}

int CHttpParserWrapper::OnUrl(http_parser* parser, const char *at, size_t length)
{
    m_instance->SetUrl(at, length);
    return 0;
}

int CHttpParserWrapper::OnHeadersComplete (http_parser* parser)
{
    m_instance->SetTotalLength(parser->nread + (uint32_t)parser->content_length);
    return 0;
}

int CHttpParserWrapper::OnBody (http_parser* parser, const char *at, size_t length)
{
    m_instance->SetBodyContent(at, length);
    return 0;
}

int CHttpParserWrapper::OnMessageComplete (http_parser* parser)
{
    m_instance->SetReadAll();
    return 0;
}
