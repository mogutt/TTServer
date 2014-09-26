/*
 * ThreadPool.h
 *
 *  Created on: 2014年7月21日
 *      Author: ziteng
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <pthread.h>
#include <list>
#include "ostype.h"
using namespace std;

class CThreadNotify
{
public:
	CThreadNotify();
	~CThreadNotify();
	void Lock() { pthread_mutex_lock(&m_mutex); }
	void Unlock() { pthread_mutex_unlock(&m_mutex); }
	void Wait() { pthread_cond_wait(&m_cond, &m_mutex); }
	void Signal() { pthread_cond_signal(&m_cond); }
private:
	pthread_mutex_t 	m_mutex;
	pthread_mutexattr_t	m_mutexattr;

	pthread_cond_t 		m_cond;
};

class CTask {
public:
	CTask();
	virtual ~CTask();

	virtual void run() = 0;
private:
};

class CWorkerThread {
public:
	CWorkerThread();
	~CWorkerThread();

	static void* StartRoutine(void* arg);

	void Start();
	void Execute();
	void PushTask(CTask* pTask);

	void SetThreadIdx(uint32_t idx) { m_thread_idx = idx; }
private:

	uint32_t		m_thread_idx;
	uint32_t		m_task_cnt;
	pthread_t		m_thread_id;
	CThreadNotify	m_thread_notify;
	list<CTask*>	m_task_list;
};

class CThreadPool {
public:
	CThreadPool();
	virtual ~CThreadPool();

	int Init(uint32_t worker_size);
	void AddTask(CTask* pTask);
private:
	uint32_t 		m_worker_size;
	CWorkerThread* 	m_worker_list;
};



#endif /* THREADPOOL_H_ */
