//
//  HttpPdu.h
//  http_msg_server
//
//  Created by jianqing.du on 13-9-29.
//  Copyright (c) 2013年 ziteng. All rights reserved.
//

#ifndef http_msg_server_HttpParserWrapper_h
#define http_msg_server_HttpParserWrapper_h

#include "util.h"
#include "http_parser.h"

// extract url and content body from an ajax request
class CHttpParserWrapper {
public:
    virtual ~CHttpParserWrapper() {}
    
    static CHttpParserWrapper* GetInstance();
    
    void ParseHttpContent(const char* buf, uint32_t len);
    
    bool IsReadAll() { return m_read_all; }
    uint32_t GetTotalLength() { return m_total_length; }
    string& GetUrl() { return m_url; }
    string& GetBodyContent() { return m_body_content; }
    
    void SetUrl(const char* url, size_t length) { m_url.append(url, length); }
    void SetBodyContent(const char* content, size_t length) { m_body_content.append(content, length); }
    void SetTotalLength(uint32_t total_len) { m_total_length = total_len; }
    void SetReadAll() { m_read_all = true; }
    
    static int OnUrl(http_parser* parser, const char *at, size_t length);
    static int OnHeadersComplete (http_parser* parser);
    static int OnBody (http_parser* parser, const char *at, size_t length);
    static int OnMessageComplete (http_parser* parser);
private:
    CHttpParserWrapper();

private:
    static CHttpParserWrapper*        m_instance;
    
    http_parser             m_http_parser;
    http_parser_settings    m_settings;
    
    bool        m_read_all;
    uint32_t    m_total_length;
    string      m_url;
    string      m_body_content;
};

#endif
