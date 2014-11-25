#ifndef __UTIL_H__
#define __UTIL_H__

#define _CRT_SECURE_NO_DEPRECATE	// remove warning C4996, 

#include "ostype.h"
#include "UtilPdu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define	snprintf	sprintf_s
#else
#include <stdarg.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#endif

#ifdef _WIN32
#include <hash_map>
using namespace stdext;
#else
#include <ext/hash_map>
using namespace __gnu_cxx;
#endif

#define MAX_LOG_FILE_SIZE	0x4000000	// 64MB
#define NOTUSED_ARG(v) ((void)v)		// used this to remove warning C4100, unreferenced parameter

class CThread
{
public:
	CThread();
	virtual ~CThread();

#ifdef _WIN32
	static DWORD WINAPI StartRoutine(LPVOID arg);
#else
	static void* StartRoutine(void* arg);
#endif

	virtual void StartThread(void);
	virtual void OnThreadRun(void) = 0;
protected:
#ifdef _WIN32
	DWORD		m_thread_id;
#else
	pthread_t	m_thread_id;
#endif
};

class CEventThread : public CThread
{
public:
	CEventThread();
	virtual ~CEventThread();

 	virtual void OnThreadTick(void) = 0;
	virtual void OnThreadRun(void);
	virtual void StartThread();
	virtual void StopThread();
	bool IsRunning() { return m_bRunning; }
private:
	bool 		m_bRunning;
};

class CThreadLock
{
public:
	CThreadLock();
	~CThreadLock();
	void Lock(void);
	void Unlock(void);
private:
#ifdef _WIN32
	CRITICAL_SECTION	m_critical_section;
#else
	pthread_mutex_t 	m_mutex;
	pthread_mutexattr_t	m_mutexattr;
#endif
};

class CFuncLock
{
public:
	CFuncLock(CThreadLock* lock) 
	{ 
		m_lock = lock; 
		if (m_lock)
			m_lock->Lock(); 
	}

	~CFuncLock() 
	{ 
		if (m_lock)
			m_lock->Unlock(); 
	}
private:
	CThreadLock*	m_lock;
};

class CRefObject
{
public:
	CRefObject();
	virtual ~CRefObject();

	void SetLock(CThreadLock* lock) { m_lock = lock; }
	void AddRef();
	void ReleaseRef();
private:
	int				m_refCount;
	CThreadLock*	m_lock;
};

/// yunfan modify 2014.8.12
#if defined(_WIN32) || defined(_WIN64)
#define log(fmt, ...)  logger("<%s>\t<%d>\t<%s>,"fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define log(fmt, args...)  logger("<%s>|<%d>|<%s>," fmt, __FILE__, __LINE__, __FUNCTION__, ##args)
#endif
void logger(const char* fmt, ...);
/// yunfan modify end

uint64_t get_tick_count();
void util_sleep(uint32_t millisecond);

class CStrExplode
{
public:
	CStrExplode(char* str, char seperator);
	virtual ~CStrExplode();

	uint32_t GetItemCnt() { return m_item_cnt; }
	char* GetItem(uint32_t idx) { return m_item_list[idx]; }
private:
	uint32_t	m_item_cnt;
	char** 		m_item_list;
};
#endif
