/********************************************************************
	
	purpose:	
*********************************************************************/
#ifdef WIN32
    #include "stdafx.h"
#else
    #include <stdlib.h>
    #include <string.h>
    #include <stdint.h>
#endif

#include "crypt.h"
#include <netinet/in.h>
#include "zlib.h"
//#include "Portable.h"
//using namespace ac;

namespace ac
{
	void xtea(int32_t* v, // 64bit of data [in/out]
			int32_t* o, // 64bits of data [out]
			int32_t* k, // 128bit key [in]
			int32_t N) // Routine rounds [in]
	{
		uint32_t y = v[0];
		uint32_t z = v[1];
		uint32_t DELTA = 0x9e3779b9; // 0x00000000 - 0x61C88647 == 0x9e3779b9

		if(N>0)
		{
			// Encoding
			uint32_t limit = DELTA*N;
			uint32_t sum = 0;
			while(sum != limit)
			{
				y += (z<<4 ^ z>>5) + z ^ sum + k[sum&3];
				sum += DELTA;
				z += (y<<4 ^ y>>5) + y ^ sum + k[sum>>11&3];
			}
		}
		else
		{
			// Decoding
			uint32_t sum = DELTA*(-N);
			while(sum)
			{
				z -= (y<<4 ^ y>>5) + y ^ sum + k[sum>>11&3];
				sum -= DELTA;
				y -= (z<<4 ^ z>>5) + z ^ sum + k[sum&3];
			}
		}

		o[0] = y;
		o[1] = z;
	}

	/*****************************************************************************
	//	加密后的Buffer结构
	//  ┌──────┬───────────┬────┬────┐
	//  │ PadLength  │  Padded Random BYTEs │  Data  │ Zero s │
	//  ├──────┼───────────┼────┼────┤
	//  │    1Byte   │    PadLength Bytes   │ N Bytes│ 7Bytes │
	//  └──────┴───────────┴────┴────┘
	// Pad部分用于将整个Buffer补齐到8字节对齐
	******************************************************************************/

	#define ZERO_LENGTH 7
	#define ENCRYPT_ROUNDS (32)
	#define DECRYPT_ROUNDS (-32)
	#define ENCRYPT_BLOCK_LENGTH_IN_BYTE (8)

	int CXTEA::Encrypt( char* pbyInBuffer, int nInBufferLength, char* pbyOutBuffer, int nOutBufferLength, char arrbyKey[16] )
	{
		if(pbyInBuffer == NULL || nInBufferLength <= 0)
		{
			return 0;
		}

		// 计算需要的输出Buffer大小
		int nPadDataZero = 1/*Pad Length*/ + nInBufferLength + ZERO_LENGTH;
		int nPadLength = nPadDataZero % ENCRYPT_BLOCK_LENGTH_IN_BYTE;
		if(nPadLength != 0)
		{
			nPadLength = ENCRYPT_BLOCK_LENGTH_IN_BYTE - nPadLength;
		}

		int nTotalLength = nPadDataZero + nPadLength;

		if(nTotalLength > nOutBufferLength || pbyOutBuffer == NULL)
		{
			return 0;
		}

		char* pbyInCursor = pbyInBuffer;
		char* pbyOutCurosr = pbyOutBuffer;

		memset(pbyOutBuffer, 0, nOutBufferLength);

		char arrbyFirst8Bytes[ENCRYPT_BLOCK_LENGTH_IN_BYTE] = {0};
		// Pad length, 只使用最低三位，高5位用随机数填充
		arrbyFirst8Bytes[0] = (((char)rand()) & 0xf8) | ((char)nPadLength);
			//arrbyFirst8Bytes[0] = ((char)nPadLength);

		// 用随机数填充Pad区
		for(int i =1; i<=nPadLength; ++i)
		{
			arrbyFirst8Bytes[i] = (char)rand();
					//arrbyFirst8Bytes[i] = (char)0;
		}

		// 用待加密数据补满第一块明文
		for(int i = 1+nPadLength; i<ENCRYPT_BLOCK_LENGTH_IN_BYTE; ++i)
		{
			arrbyFirst8Bytes[i] = *pbyInCursor++;
		}

		// 上一个加密块的明文与密文，用于后面的异或操作
		char* pbyLast8BytesCryptedData = pbyOutBuffer;
		char* pbyLast8BytesPlainData = arrbyFirst8Bytes;

		// 第一段Buffer，不需要异或操作
		xtea((int32_t*)arrbyFirst8Bytes, (int32_t*)pbyOutCurosr, (int32_t*)arrbyKey, ENCRYPT_ROUNDS);
	//	xtea((Int32*)pbyOutCurosr, (Int32*)arrbyFirst8Bytes, (Int32*)arrbyKey, DECRYPT_ROUNDS);
		pbyOutCurosr += ENCRYPT_BLOCK_LENGTH_IN_BYTE;

		// 下面这段是是用于不更改InBuffer的加密过程
		char arrbySrcBuffer[ENCRYPT_BLOCK_LENGTH_IN_BYTE] = {0};
		while((pbyInCursor - pbyInBuffer) < (nInBufferLength - 1))
		{
			memcpy(arrbySrcBuffer, pbyInCursor, ENCRYPT_BLOCK_LENGTH_IN_BYTE);
			// 和上一块密文异或
			for(int i=0; i<ENCRYPT_BLOCK_LENGTH_IN_BYTE; ++i)
			{
				arrbySrcBuffer[i] ^= pbyLast8BytesCryptedData[i];
			}
			xtea((int32_t*)arrbySrcBuffer, (int32_t*)pbyOutCurosr, (int32_t*)arrbyKey, ENCRYPT_ROUNDS);
			// 和上一块明文异或
			for(int i=0; i<ENCRYPT_BLOCK_LENGTH_IN_BYTE; ++i)
			{
				pbyOutCurosr[i] ^= pbyLast8BytesPlainData[i];
			}

			pbyLast8BytesCryptedData = pbyOutCurosr;
			pbyLast8BytesPlainData = pbyInCursor;

			pbyOutCurosr += ENCRYPT_BLOCK_LENGTH_IN_BYTE;
			pbyInCursor += ENCRYPT_BLOCK_LENGTH_IN_BYTE;
		}

		// 这一段和上一段被注释掉的功能相同，只不过会修改InBuffer的内容
	/*	while((pbyInCursor - pbyInBuffer) < (nInBufferLength - 1))
		{
			// 和上一块密文异或
			for(int i=0; i<ENCRYPT_BLOCK_LENGTH_IN_BYTE; ++i)
			{
				pbyInCursor[i] ^= pbyLast8BytesCryptedData[i];
			}
			xtea((Int32*)pbyInCursor, (Int32*)pbyOutCurosr, (Int32*)arrbyKey, ENCRYPT_ROUNDS);
			// 和上一块明文异或
			for(int i=0; i<ENCRYPT_BLOCK_LENGTH_IN_BYTE; ++i)
			{
				pbyOutCurosr[i] ^= pbyLast8BytesPlainData[i];
			}

			pbyLast8BytesCryptedData = pbyOutCurosr;
			pbyLast8BytesPlainData = pbyInCursor;

			pbyOutCurosr += ENCRYPT_BLOCK_LENGTH_IN_BYTE;
			pbyInCursor += ENCRYPT_BLOCK_LENGTH_IN_BYTE;
		}
	*/
		// 结尾的 1Byte数据 + 7Byte 校验
		char arrbyLast8Bytes[ENCRYPT_BLOCK_LENGTH_IN_BYTE] = {0};
		arrbyLast8Bytes[0] = *pbyInCursor;

		// 和上一块密文异或
		for(int i=0; i<ENCRYPT_BLOCK_LENGTH_IN_BYTE; ++i)
		{
			arrbyLast8Bytes[i] ^= pbyLast8BytesCryptedData[i];
		}
		xtea((int32_t*)arrbyLast8Bytes, (int32_t*)pbyOutCurosr, (int32_t*)arrbyKey, ENCRYPT_ROUNDS);
		// 和上一块明文异或
		for(int i=0; i<ENCRYPT_BLOCK_LENGTH_IN_BYTE; ++i)
		{
			pbyOutCurosr[i] ^= pbyLast8BytesPlainData[i];
		}

		return nTotalLength;
	}
		// TODO: OutBufferLength判断
	int CXTEA::Decrypt( char* pbyInBuffer, int nInBufferLength, char* pbyOutBuffer, int nOutBufferLength, char arrbyKey[16] )
	{
		if(pbyInBuffer == NULL || nInBufferLength <= 0)
		{
			return false;
		}

		// Buffer长度应该是能被 ENCRYPT_BLOCK_LENGTH_IN_BYTE 整除的
		if(nInBufferLength%ENCRYPT_BLOCK_LENGTH_IN_BYTE || nInBufferLength <= 8)
		{
			return 0;
		}

		char* pbyInCursor = pbyInBuffer;
		char* pbyOutCursor = pbyOutBuffer;

		// 先解出最前面包含Pad的ENCRYPT_BLOCK_LENGTH_IN_BYTE个Byte
		char arrbyFirst8Bytes[ENCRYPT_BLOCK_LENGTH_IN_BYTE] = {0};
		xtea((int32_t*)pbyInCursor, (int32_t*)arrbyFirst8Bytes, (int32_t*)arrbyKey, DECRYPT_ROUNDS);
		pbyInCursor += ENCRYPT_BLOCK_LENGTH_IN_BYTE;
		// Pad长度只是用了第一个字节的最低3bit，高5bit是随机数
		int nPadLength = arrbyFirst8Bytes[0] & 0x07;

		// 计算原始数据的长度
		int nPlainDataLength = nInBufferLength - 1/*PadLength Length*/ - nPadLength - ZERO_LENGTH;
		if(nPlainDataLength <= 0 || pbyOutBuffer == NULL)
		{
			return 0;
		}

		// OutBuffer不够
		if(nPlainDataLength > nOutBufferLength)
		{
			return 0;
		}

		// 前一块的明文和密文，用于后面的异或操作
		char* pbyLast8BytesCryptedData = pbyInBuffer;
		char* pbyLast8BytesPlainData = arrbyFirst8Bytes;

		// 将第一块里Pad信息之后的数据移到输出Buffer
		for(int i=0; i < 7/*ENCRYPT_BLOCK_LENGTH_IN_BYTE - 1*/ - nPadLength; ++i)
		{
			*pbyOutCursor++ = arrbyFirst8Bytes[1+nPadLength + i];
		}

		// 解密除了最后一块以外的所有块
		// 同加密过程，下面注释掉的，是不改动InBuffer的

		char arrbySrcBuffer[8] = {0};
		//for(int i=8; i<nInBufferLength-8; i+=8)
		while(pbyInCursor - pbyInBuffer < nInBufferLength - 8)
		{
			memcpy(arrbySrcBuffer, pbyInCursor, 8);
			// 和上一个8char明文异或
			for(int i=0; i<8; ++i)
			{
				arrbySrcBuffer[i] ^= pbyLast8BytesPlainData[i];
			}
			xtea((int32_t*)arrbySrcBuffer, (int32_t*)pbyOutCursor, (int32_t*)arrbyKey, DECRYPT_ROUNDS);
			// 和上一个8char密文异或
			for(int i=0; i<8; ++i)
			{
				pbyOutCursor[i] ^= pbyLast8BytesCryptedData[i];
			}

			pbyLast8BytesCryptedData = pbyInCursor;
			pbyLast8BytesPlainData = pbyOutCursor;

			pbyInCursor += 8;
			pbyOutCursor += 8;
		}

		// 直接改InBuffer的版本
	/*	while(pbyInCursor - pbyInBuffer < nInBufferLength - ENCRYPT_BLOCK_LENGTH_IN_BYTE)
		{
			// 和上一个8Byte明文异或
			for(int i=0; i<ENCRYPT_BLOCK_LENGTH_IN_BYTE; ++i)
			{
				pbyInCursor[i] ^= pbyLast8BytesPlainData[i];
			}
			xtea((Int32*)pbyInCursor, (Int32*)pbyOutCursor, (Int32*)arrbyKey, DECRYPT_ROUNDS);
			// 和上一个8Byte密文异或
			for(int i=0; i<ENCRYPT_BLOCK_LENGTH_IN_BYTE; ++i)
			{
				pbyOutCursor[i] ^= pbyLast8BytesCryptedData[i];
			}

			pbyLast8BytesCryptedData = pbyInCursor;
			pbyLast8BytesPlainData = pbyOutCursor;

			pbyInCursor += ENCRYPT_BLOCK_LENGTH_IN_BYTE;
			pbyOutCursor += ENCRYPT_BLOCK_LENGTH_IN_BYTE;
		}
	*/
		// 最后8Byte， 最后有7Byte的校验
		char arrbyLast8Bytes[ENCRYPT_BLOCK_LENGTH_IN_BYTE] = {0};
		// 和上一个8Byte明文异或
		memcpy(arrbySrcBuffer, pbyInCursor, 8);
		for(int i=0; i<ENCRYPT_BLOCK_LENGTH_IN_BYTE; ++i)
		{
			arrbySrcBuffer[i] ^= pbyLast8BytesPlainData[i];
		}
		xtea((int32_t*)arrbySrcBuffer, (int32_t*)arrbyLast8Bytes, (int32_t*)arrbyKey, DECRYPT_ROUNDS);
		// 和上一个8Byte密文异或
		for(int i=0; i<ENCRYPT_BLOCK_LENGTH_IN_BYTE; ++i)
		{
			arrbyLast8Bytes[i] ^= pbyLast8BytesCryptedData[i];
		}

		// 校验最后的0
		for(int i=1; i<ENCRYPT_BLOCK_LENGTH_IN_BYTE; ++i)
		{
			if(arrbyLast8Bytes[i] != 0)
			{
				return 0;
			}
		}
		*pbyOutCursor = arrbyLast8Bytes[0];

		return nPlainDataLength;
	}




#define DELTA 0x9e3779b9 /* sqr(5)-1 * 2^31 */
    /// TEA加/解密单元
    /*
    @param	rounds IN	轮数，大于0时，用于加密，小于0时，用于解密	
    @param	in IN		要加/解密的64bit数据
    @param	out OUT		存放加/解密结果的64bit
    @param	key	IN		加/解密的key,128bit
    @remark	该函数是TEA算法的核心，一次只能加/解密64bit的数据，结果也是64bit的数据
    */
    static void TEACore(UINT32 in[2], UINT32 out[2], UINT32 key[4], int rounds)
    {
        UINT32 y = in[0], z = in[1];
        UINT32 limit = 0, sum = 0;

        if(rounds > 0)	// encrypt
        {
            limit = DELTA * rounds;
            while(sum != limit)
            {
                y += ((z<<4)^(z>>5)) + (z^sum) + key[sum&3];
                sum += DELTA;
                z += ((y<<4)^(y>>5)) + (y^sum) + key[(sum>>11)&3];
            }
        }
        else	// decrypt
        {
            sum = DELTA * (-rounds);
            while(sum)
            {
                z -= ((y<<4)^(y>>5)) + (y^sum) + key[(sum>>11)&3];
                sum -= DELTA;
                y -= ((z<<4)^(z>>5)) + (z^sum) + key[sum&3];
            }
        }

        out[0] = y; out[1] = z;
    }



    bool TEAEncrypt(BYTE* pInBuffer, int nInBufferLen, BYTE* pOutBuffer, int* pnOutBufferLen, BYTE pKey[16]/* = DEF_KEY*/, UINT uRounds/* = 16*/)
    {
        if (nInBufferLen < 1 || !pnOutBufferLen)
        {
            return FALSE;
        }

        /// padding
        const int nPadLen = ((nInBufferLen % 8) == 0 ? 0 : (8 - (nInBufferLen % 8)));

        int nBufferLenToEncrypt = nInBufferLen + nPadLen;

        if (*pnOutBufferLen < (nBufferLenToEncrypt + 1))	// check: *pnOutBufferLen is length enough
        {
            return FALSE;
        }
        //  BYTE* pBufferToEncrypt = pInBuffer;
        //	BYTE* pBufferToEncrypt = new BYTE[nBufferLenToEncrypt];
        //	memcpy(pBufferToEncrypt, pInBuffer, nInBufferLen);
        for (int i = 0; i < nPadLen; i++)	// pad with 0
        {
            pInBuffer[nInBufferLen + i] = 0;
        }

        /// core
        for (int i = 0; i < nBufferLenToEncrypt; i += 8)
        {
            TEACore((UINT32*)(pInBuffer + i), (UINT32*)(pOutBuffer + i + 1), (UINT32*)pKey, uRounds);
        }

        pOutBuffer[0] = nPadLen;
        if (pnOutBufferLen)
        {
            *pnOutBufferLen = nBufferLenToEncrypt + 1;
        }

        //	delete[] pBufferToEncrypt;
        //	pBufferToEncrypt = NULL;

        return TRUE;
    }

    bool TEADecrypt(BYTE* pInBuffer, int nInBufferLen, BYTE* pOutBuffer, int* pnOutBufferLen, BYTE pKey[16]/* = DEF_KEY*/, UINT uRounds/* = 16*/)
    {
        if (nInBufferLen < 9 || ((nInBufferLen - 1) % 8) != 0 || !pnOutBufferLen || *pnOutBufferLen < (nInBufferLen - 1))
        {
            return FALSE;
        }

        for (int i = 1; i < nInBufferLen; i += 8)
        {
            TEACore((UINT32*)(pInBuffer + i), (UINT32*)(pOutBuffer + i - 1), (UINT32*)pKey, -1 * uRounds);
        }

        const int nPadLen = pInBuffer[0];
        if (nPadLen > 0)
        {
            for (int i = 0; i < nPadLen; i++)
            {
                if (pOutBuffer[nInBufferLen - 1 - nPadLen + i] != 0)	// check: padding BYTEs MUST equal 0
                {
                    return FALSE;
                }
            }
        }

        if (*pnOutBufferLen)
        {
            *pnOutBufferLen = nInBufferLen - 1 - nPadLen;
        }

        return TRUE;
    }
    int  TEAEncryptLen(int nInBufferLen){
        if (nInBufferLen < 1 )
        {
            return -1;
        }
        const int nPadLen = ((nInBufferLen % 8) == 0 ? 0 : (8 - (nInBufferLen % 8)));

        int nBufferLenToEncrypt = nInBufferLen + nPadLen;
        return nBufferLenToEncrypt;
    }

    int TEADecryptLen(int nInBufferLen)
    {
        if (nInBufferLen < 9 || ((nInBufferLen - 1) % 8) != 0 )
        {
            return -1;
        }
        return  nInBufferLen - 1;
    }

#define PIANYI 3
	bool StreamDecrypt(const char *inbuf,int inbuflen,char *outbuf,int &outbuflen,char key[16],int type)
	{
		char *outbuf2 = outbuf + PIANYI;
		int outbuflen2 = outbuflen - PIANYI;
		if(type == 1)
		{
			if(!TEADecrypt((unsigned char*)(inbuf + PIANYI),inbuflen - PIANYI,(unsigned char*)outbuf2,&outbuflen2,(unsigned char*)key))
				return false;
		}
		else
		{
			int ret = CXTEA::Decrypt((char*)(inbuf + PIANYI),inbuflen - PIANYI,outbuf2,outbuflen2,key);
			if(ret == 0)
				return false;
			else
				outbuflen2 = ret;
		}
		outbuflen2 += PIANYI;

		short packlen = htons((short)outbuflen2);
		memcpy(outbuf,&packlen,sizeof(short));
		outbuflen = outbuflen2;
		return true;
	}

	bool StreamEncrypt(const char *inbuf,int inbuflen,char *outbuf,int &outbuflen,char key[16],int type)
	{
		char *outbuf2 = outbuf + PIANYI;
		int outbuflen2 = outbuflen - PIANYI;
		if(type == 1)
		{
			if(!TEAEncrypt((unsigned char*)(inbuf + PIANYI),inbuflen - PIANYI,(unsigned char*)outbuf2,&outbuflen2,(unsigned char*)key))
				return false;
		}
		else
		{
			int ret = CXTEA::Encrypt((char*)(inbuf + PIANYI),inbuflen - PIANYI,outbuf2,outbuflen2,key);
			if(ret == 0)
				return false;
			else
				outbuflen2 = ret;
		}
		outbuflen2 += PIANYI;

		short packlen = htons((short)outbuflen2);
		memcpy(outbuf,&packlen,sizeof(short));
		if(type == 1)
			outbuf[2] = TEAFLAG;
		else
			outbuf[2] = XTEAFLAG;

		outbuflen = outbuflen2;
		return true;
	}
	bool StreamCompress(const char *inbuf,int inbuflen,char *outbuf,int &outbuflen)
	{
		unsigned char* outbuf2 = (unsigned char*)outbuf + PIANYI;
		unsigned long outbuflen2 = outbuflen - PIANYI;
		unsigned char *inbuf2 = (unsigned char*)inbuf + PIANYI;
		unsigned long inbuflen2 = inbuflen - PIANYI;
		if(compress(outbuf2,&outbuflen2,inbuf2,inbuflen2) != 0)
			return false;
		outbuflen2 += PIANYI;

		short packlen = htons((short)outbuflen2);
		memcpy(outbuf,&packlen,sizeof(short));
		outbuf[2] = COMPRESSFLAG;
		outbuflen = outbuflen2;

		return true;
	}
	bool StreamUnCompress(const char *inbuf,int inbuflen,char *outbuf,int &outbuflen)
	{
		unsigned char* outbuf2 = (unsigned char*)outbuf + PIANYI;
		unsigned long outbuflen2 = outbuflen - PIANYI;
		unsigned char *inbuf2 = (unsigned char*)inbuf + PIANYI;
		unsigned long inbuflen2 = inbuflen - PIANYI;

		if(uncompress(outbuf2,&outbuflen2,inbuf2,inbuflen2) != 0)
			return false;

		outbuflen2 += PIANYI;

		short packlen = htons((short)outbuflen2);
		memcpy(outbuf,&packlen,sizeof(short));
		outbuflen = outbuflen2;

		return true;
	}
}

