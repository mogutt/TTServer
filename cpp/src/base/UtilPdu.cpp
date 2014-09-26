/*
 * UtilPdu.cpp
 *
 *  Created on: 2013-8-27
 *      Author: ziteng@mogujie.com
 */

#include "UtilPdu.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

///////////// CSimpleBuffer ////////////////
CSimpleBuffer::CSimpleBuffer()
{
	m_buffer = NULL;

	m_alloc_size = 0;
	m_write_offset = 0;
}

CSimpleBuffer::~CSimpleBuffer()
{
	m_alloc_size = 0;
	m_write_offset = 0;
	if (m_buffer)
	{
		free(m_buffer);
		m_buffer = NULL;
	}
}

void CSimpleBuffer::Extend(uint32_t len)
{
	m_alloc_size = m_write_offset + len;
	m_alloc_size += m_alloc_size >> 2;	// increase by 1/4 allocate size
	uchar_t* new_buf = (uchar_t*)realloc(m_buffer, m_alloc_size);
    if(new_buf != NULL)
    {
    	m_buffer = new_buf;
    }
}

uint32_t CSimpleBuffer::Write(void* buf, uint32_t len)
{
	if (m_write_offset + len > m_alloc_size)
	{
		Extend(len);
	}

	if (buf)
	{
		memcpy(m_buffer + m_write_offset, buf, len);
	}

	m_write_offset += len;

	return len;
}

uint32_t CSimpleBuffer::Read(void* buf, uint32_t len)
{
	if (len > m_write_offset)
		len = m_write_offset;

	if (buf)
		memcpy(buf, m_buffer, len);

	m_write_offset -= len;
	memmove(m_buffer, m_buffer + len, m_write_offset);
	return len;
}

////// CByteStream //////
CByteStream::CByteStream(uchar_t* buf, uint32_t len)
{
	m_pBuf = buf;
	m_len = len;
	m_pSimpBuf = NULL;
	m_pos = 0;
}

CByteStream::CByteStream(CSimpleBuffer* pSimpBuf, uint32_t pos)
{
	m_pSimpBuf = pSimpBuf;
	m_pos = pos;
	m_pBuf = NULL;
	m_len = 0;
}

int16_t CByteStream::ReadInt16(uchar_t *buf)
{
	int16_t data = (buf[0] << 8) | buf[1];
	return data;
}

uint16_t CByteStream::ReadUint16(uchar_t* buf)
{
	uint16_t data = (buf[0] << 8) | buf[1];
	return data;
}

int32_t CByteStream::ReadInt32(uchar_t *buf)
{
	int32_t data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
	return data;
}

uint32_t CByteStream::ReadUint32(uchar_t *buf)
{
	uint32_t data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
	return data;
}

void CByteStream::WriteInt16(uchar_t *buf, int16_t data)
{
	buf[0] = static_cast<uchar_t>(data >> 8);
	buf[1] = static_cast<uchar_t>(data & 0xFF);
}

void CByteStream::WriteUint16(uchar_t *buf, uint16_t data)
{
	buf[0] = static_cast<uchar_t>(data >> 8);
	buf[1] = static_cast<uchar_t>(data & 0xFF);
}

void CByteStream::WriteInt32(uchar_t *buf, int32_t data)
{
	buf[0] = static_cast<uchar_t>(data >> 24);
	buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
	buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
	buf[3] = static_cast<uchar_t>(data & 0xFF);
}

void CByteStream::WriteUint32(uchar_t *buf, uint32_t data)
{
	buf[0] = static_cast<uchar_t>(data >> 24);
	buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
	buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
	buf[3] = static_cast<uchar_t>(data & 0xFF);
}

void CByteStream::operator << (int8_t data)
{
	_WriteByte(&data, 1);
}

void CByteStream::operator << (uint8_t data)
{
	_WriteByte(&data, 1);
}

void CByteStream::operator << (int16_t data)
{
	unsigned char buf[2];
	buf[0] = static_cast<uchar_t>(data >> 8);
	buf[1] = static_cast<uchar_t>(data & 0xFF);
	_WriteByte(buf, 2);
}

void CByteStream::operator << (uint16_t data)
{
	unsigned char buf[2];
	buf[0] = static_cast<uchar_t>(data >> 8);
	buf[1] = static_cast<uchar_t>(data & 0xFF);
	_WriteByte(buf, 2);
}

void CByteStream::operator << (int32_t data)
{
	unsigned char buf[4];
	buf[0] = static_cast<uchar_t>(data >> 24);
	buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
	buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
	buf[3] = static_cast<uchar_t>(data & 0xFF);
	_WriteByte(buf, 4);
}

void CByteStream::operator << (uint32_t data)
{
	unsigned char buf[4];
	buf[0] = static_cast<uchar_t>(data >> 24);
	buf[1] = static_cast<uchar_t>((data >> 16) & 0xFF);
	buf[2] = static_cast<uchar_t>((data >> 8) & 0xFF);
	buf[3] = static_cast<uchar_t>(data & 0xFF);
	_WriteByte(buf, 4);
}

void CByteStream::operator >> (int8_t& data)
{
	_ReadByte(&data, 1);
}

void CByteStream::operator >> (uint8_t& data)
{
	_ReadByte(&data, 1);
}

void CByteStream::operator >> (int16_t& data)
{
	unsigned char buf[2];

	_ReadByte(buf, 2);

	data = (buf[0] << 8) | buf[1];
}

void CByteStream::operator >> (uint16_t& data)
{
	unsigned char buf[2];

	_ReadByte(buf, 2);

	data = (buf[0] << 8) | buf[1];
}

void CByteStream::operator >> (int32_t& data)
{
	unsigned char buf[4];

	_ReadByte(buf, 4);

	data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

void CByteStream::operator >> (uint32_t& data)
{
	unsigned char buf[4];

	_ReadByte(buf, 4);

	data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

void CByteStream::WriteString(const char *str)
{
	uint32_t size = str ? (uint32_t)strlen(str) : 0;

	*this << size;
	_WriteByte((void*)str, size);
}

void CByteStream::WriteString(const char *str, uint32_t len)
{
	*this << len;
	_WriteByte((void*)str, len);
}

char* CByteStream::ReadString(uint32_t& len)
{
	*this >> len;
	char* pStr = (char*)GetBuf() + GetPos();
	Skip(len);
	return pStr;
}

void CByteStream::WriteData(uchar_t *data, uint32_t len)
{
	*this << len;
	_WriteByte(data, len);
}

uchar_t* CByteStream::ReadData(uint32_t &len)
{
	*this >> len;
	uchar_t* pData = (uchar_t*)GetBuf() + GetPos();
	Skip(len);
	return pData;
}

void CByteStream::_ReadByte(void* buf, uint32_t len)
{
	if (m_pos + len > m_len)
		return;

	if (m_pSimpBuf)
		m_pSimpBuf->Read((char*)buf, len);
	else
		memcpy(buf, m_pBuf + m_pos, len);

	m_pos += len;
}

void CByteStream::_WriteByte(void* buf, uint32_t len)
{
	if (m_pBuf && (m_pos + len > m_len))
		return;

	if (m_pSimpBuf)
		m_pSimpBuf->Write((char*)buf, len);
	else
		memcpy(m_pBuf + m_pos, buf, len);

	m_pos += len;
}

/*
 * Warning!!!
 * This function return a static char pointer, caller must immediately copy the string to other place
 */
char* idtourl(uint32_t id)
{
	static char buf[64];
    sprintf(buf, "%d", id );
	return buf;
}

uint32_t urltoid(const char* url)
{
    uint32_t id = atoi(url);
	return id;
}
