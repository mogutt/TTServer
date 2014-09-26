/*
 * UtilPdu.h
 *
 *  Created on: 2013-8-27
 *      Author: ziteng@mogujie.com
 */

#ifndef UTILPDU_H_
#define UTILPDU_H_

#include "ostype.h"
#include <set>
#include <map>
#include <list>
#include <string>
using namespace std;

#ifdef WIN32
	#ifdef BUILD_PDU
		#define DLL_MODIFIER __declspec(dllexport)
	#else
		#define DLL_MODIFIER __declspec(dllimport)
	#endif
#else
	#define DLL_MODIFIER
#endif

class DLL_MODIFIER CSimpleBuffer
{
public:
	CSimpleBuffer();
	~CSimpleBuffer();
	uchar_t*  GetBuffer() { return m_buffer; }
	uint32_t GetAllocSize() { return m_alloc_size; }
	uint32_t GetWriteOffset() { return m_write_offset; }
	void IncWriteOffset(uint32_t len) { m_write_offset += len; }

	void Extend(uint32_t len);
	uint32_t Write(void* buf, uint32_t len);
	uint32_t Read(void* buf, uint32_t len);
private:
	uchar_t*	m_buffer;
	uint32_t	m_alloc_size;
	uint32_t	m_write_offset;
};

class CByteStream
{
public:
	CByteStream(uchar_t* buf, uint32_t len);
	CByteStream(CSimpleBuffer* pSimpBuf, uint32_t pos);
	~CByteStream() {}

	unsigned char* GetBuf() { return m_pSimpBuf ? m_pSimpBuf->GetBuffer() : m_pBuf; }
	uint32_t GetPos() { return m_pos; }
	uint32_t GetLen() { return m_len; }
	void Skip(uint32_t len) { m_pos += len; }

	static int16_t ReadInt16(uchar_t* buf);
	static uint16_t ReadUint16(uchar_t* buf);
	static int32_t ReadInt32(uchar_t* buf);
	static uint32_t ReadUint32(uchar_t* buf);
	static void WriteInt16(uchar_t* buf, int16_t data);
	static void WriteUint16(uchar_t* buf, uint16_t data);
	static void WriteInt32(uchar_t* buf, int32_t data);
	static void WriteUint32(uchar_t* buf, uint32_t data);

	void operator << (int8_t data);
	void operator << (uint8_t data);
	void operator << (int16_t data);
	void operator << (uint16_t data);
	void operator << (int32_t data);
	void operator << (uint32_t data);

	void operator >> (int8_t& data);
	void operator >> (uint8_t& data);
	void operator >> (int16_t& data);
	void operator >> (uint16_t& data);
	void operator >> (int32_t& data);
	void operator >> (uint32_t& data);

	void WriteString(const char* str);
	void WriteString(const char* str, uint32_t len);
	char* ReadString(uint32_t& len);

	void WriteData(uchar_t* data, uint32_t len);
	uchar_t* ReadData(uint32_t& len);
private:
	void _WriteByte(void* buf, uint32_t len);
	void _ReadByte(void* buf, uint32_t len);
private:
	CSimpleBuffer*	m_pSimpBuf;
	uchar_t*		m_pBuf;
	uint32_t		m_len;
	uint32_t		m_pos;
};

#define ERROR_CODE_PARSE_FAILED 		1
#define ERROR_CODE_WRONG_SERVICE_ID		2
#define ERROR_CODE_WRONG_COMMAND_ID		3
#define ERROR_CODE_ALLOC_FAILED			4

class CPduException {
public:
	CPduException(uint32_t module_id, uint32_t command_id, uint32_t error_code, const char* error_msg)
	{
		m_module_id = module_id;
		m_command_id = command_id;
		m_error_code = error_code;
		m_error_msg = error_msg;
	}

	virtual ~CPduException() {}

	uint32_t GetModuleId() { return m_module_id; }
	uint32_t GetCommandId() { return m_command_id; }
	uint32_t GetErrorCode() { return m_error_code; }
	char* GetErrorMsg() { return (char*)m_error_msg.c_str(); }
private:
	uint32_t	m_module_id;
	uint32_t	m_command_id;
	uint32_t	m_error_code;
	string		m_error_msg;
};

char* idtourl(uint32_t id);
uint32_t urltoid(const char* url);


#endif /* UTILPDU_H_ */
