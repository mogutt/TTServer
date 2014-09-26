/*
 * ThreadPool.cpp
 *
 *  Created on: 2014年7月21日
 *      Author: ziteng
 */

#include <stdlib.h>
#include "util.h"
#include "ThreadPool.h"

CThreadNotify::CThreadNotify()
{
	pthread_mutexattr_init(&m_mutexattr);
	pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_mutex, &m_mutexattr);

	pthread_cond_init(&m_cond, NULL);
}

CThreadNotify::~CThreadNotify()
{
	pthread_mutexattr_destroy(&m_mutexattr);
	pthread_mutex_destroy(&m_mutex);

	pthread_cond_destroy(&m_cond);
}

////////////////////
CTask::CTask()
{
}

CTask::~CTask()
{
}

void CTask::run()
{
}

///////////
CWorkerThread::CWorkerThread()
{
	m_task_cnt = 0;
}

CWorkerThread::~CWorkerThread()
{

}

void* CWorkerThread::StartRoutine(void* arg)
{
	CWorkerThread* pThread = (CWorkerThread*)arg;

	pThread->Execute();

	return NULL;
}

void CWorkerThread::Start()
{
	(void)pthread_create(&m_thread_id, NULL, StartRoutine, this);
}

void CWorkerThread::Execute()
{
	while (true) {
		m_thread_notify.Lock();

		// put wait in while cause there can be spurious wake up (due to signal/ENITR)
		while (m_task_list.empty()) {
			m_thread_notify.Wait();
		}

		CTask* pTask = m_task_list.front();
		m_task_list.pop_front();
		m_thread_notify.Unlock();

		pTask->run();

		delete pTask;

		m_task_cnt++;
		log("%d have the execute %d task\n", m_thread_idx, m_task_cnt);
	}
}

void CWorkerThread::PushTask(CTask* pTask)
{
	m_thread_notify.Lock();
	m_task_list.push_back(pTask);
	m_thread_notify.Signal();
	m_thread_notify.Unlock();
}

//////////////
CThreadPool::CThreadPool()
{
	m_worker_size = 0;
	m_worker_list = NULL;
}

CThreadPool::~CThreadPool()
{

}

int CThreadPool::Init(uint32_t worker_size)
{
    m_worker_size = worker_size;
	m_worker_list = new CWorkerThread [m_worker_size];
	if (!m_worker_list) {
		return 1;
	}

	for (uint32_t i = 0; i < m_worker_size; i++) {
		m_worker_list[i].SetThreadIdx(i);
		m_worker_list[i].Start();
	}

	return 0;
}

void CThreadPool::AddTask(CTask* pTask)
{
	/*
	 * select a random thread to push task
	 * we can also select a thread that has less task to do
	 * but that will scan the whole thread list and use thread lock to get each task size
	 */
	uint32_t thread_idx = random() % m_worker_size;
	m_worker_list[thread_idx].PushTask(pTask);
}

