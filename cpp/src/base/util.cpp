#include "util.h"

CThread::CThread()
{
	m_thread_id = 0;
}

CThread::~CThread()
{
}

#ifdef _WIN32
DWORD WINAPI CThread::StartRoutine(LPVOID arg)	
#else
void* CThread::StartRoutine(void* arg)
#endif
{
	CThread* pThread = (CThread*)arg;
	
	pThread->OnThreadRun();

#ifdef _WIN32
	return 0;
#else
	return NULL;
#endif
}

void CThread::StartThread()
{
#ifdef _WIN32
	(void)CreateThread(NULL, 0, StartRoutine, this, 0, &m_thread_id);
#else
	(void)pthread_create(&m_thread_id, NULL, StartRoutine, this);
#endif
}

CEventThread::CEventThread()
{
	m_bRunning = false;
}

CEventThread::~CEventThread()
{
	StopThread();
}

void CEventThread::StartThread()
{
	m_bRunning = true;
	CThread::StartThread();
}

void CEventThread::StopThread()
{
	m_bRunning = false;
}

void CEventThread::OnThreadRun()
{
	while (m_bRunning)
	{
		OnThreadTick();
	}
}

/////////// CThreadLock ///////////
CThreadLock::CThreadLock()
{
#ifdef _WIN32
	InitializeCriticalSection(&m_critical_section);
#else
	pthread_mutexattr_init(&m_mutexattr);
	pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_mutex, &m_mutexattr);
#endif
}

CThreadLock::~CThreadLock()
{
#ifdef _WIN32
	DeleteCriticalSection(&m_critical_section);
#else
	pthread_mutexattr_destroy(&m_mutexattr);
	pthread_mutex_destroy(&m_mutex);
#endif
}

void CThreadLock::Lock(void)
{
#ifdef _WIN32
	EnterCriticalSection(&m_critical_section);
#else
	pthread_mutex_lock(&m_mutex);
#endif
}

void CThreadLock::Unlock(void)
{
#ifdef _WIN32
	LeaveCriticalSection(&m_critical_section);
#else
	pthread_mutex_unlock(&m_mutex);
#endif
}

CRefObject::CRefObject()
{
	m_lock = NULL;
	m_refCount = 1;
}

CRefObject::~CRefObject()
{

}

void CRefObject::AddRef()
{
	if (m_lock)
	{
		m_lock->Lock();
		m_refCount++;
		m_lock->Unlock();
	}
	else
	{
		m_refCount++;
	}
}

void CRefObject::ReleaseRef()
{
	if (m_lock)
	{
		m_lock->Lock();
		m_refCount--;
		if (m_refCount == 0)
		{
			delete this;
			return;
		}
		m_lock->Unlock();
	}
	else
	{
		m_refCount--;
		if (m_refCount == 0)
			delete this;
	}
}

/*
static void print_thread_id(FILE* fp)
{
	uint16_t thread_id = 0;
#ifdef _WIN32
	thread_id = (uint16_t)GetCurrentThreadId();
#elif __APPLE__
	thread_id = syscall(SYS_gettid);
#else
	thread_id = (uint16_t)pthread_self();
#endif
	fprintf(fp, "(tid=%d)", thread_id);
}
*/

static void print_format_time(FILE* fp)
{
#ifdef _WIN32
	SYSTEMTIME systemTime;

	GetLocalTime(&systemTime);
	fprintf(fp, "%04d-%02d-%02d, %02d:%02d:%02d.%03d, ", systemTime.wYear, systemTime.wMonth, systemTime.wDay,
		systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);
#else
	struct timeval tval;
	struct tm* tm;
	time_t currTime;

	time(&currTime);
	tm = localtime(&currTime);
	gettimeofday(&tval, NULL);
	fprintf(fp, "%04d-%02d-%02d, %02d:%02d:%02d.%03d, ", 1900 + tm->tm_year, 1 + tm->tm_mon,
		tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (int)tval.tv_usec / 1000);
#endif
}

void logger(const char* fmt, ...)
{
	static int file_no = 1;
	static FILE* log_fp = NULL;
	if (log_fp == NULL)
	{
		char log_name[64];
		uint32_t pid = 0;
#ifdef _WIN32
		pid = (uint32_t)GetCurrentProcessId();
#else
		pid = (uint32_t)getpid();
#endif
		snprintf(log_name, 64, "log_%d_%d.txt", pid, file_no);
		log_fp = fopen(log_name, "w");
		if (log_fp == NULL)
			return;
	}

	// single thread, so remove print_thread_id
	//print_thread_id(log_fp);

	print_format_time(log_fp);

	va_list ap;
	va_start(ap, fmt);
	vfprintf(log_fp, fmt, ap);
	va_end(ap);
	fflush(log_fp);

	if (ftell(log_fp) > MAX_LOG_FILE_SIZE)
	{
		fclose(log_fp);
		log_fp = NULL;
		file_no++;
	}
}

uint64_t get_tick_count()
{
#ifdef _WIN32
	LARGE_INTEGER liCounter; 
	LARGE_INTEGER liCurrent;

	if (!QueryPerformanceFrequency(&liCounter))
		return GetTickCount();

	QueryPerformanceCounter(&liCurrent);
	return (uint64_t)(liCurrent.QuadPart * 1000 / liCounter.QuadPart);
#else
	struct timeval tval;
	uint64_t ret_tick;

	gettimeofday(&tval, NULL);

	ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
	return ret_tick;
#endif
}

void util_sleep(uint32_t millisecond)
{
#ifdef _WIN32
	Sleep(millisecond);
#else
	usleep(millisecond * 1000);
#endif
}

CStrExplode::CStrExplode(char* str, char seperator)
{
	m_item_cnt = 1;
	char* pos = str;
	while (*pos) {
		if (*pos == seperator) {
			m_item_cnt++;
		}

		pos++;
	}

	m_item_list = new char* [m_item_cnt];

	int idx = 0;
	char* start = pos = str;
	while (*pos) {
		if (*pos == seperator && pos != start) {
			uint32_t len = pos - start;
			m_item_list[idx] = new char [len + 1];
			strncpy(m_item_list[idx], start, len);
			m_item_list[idx][len]  = '\0';
			idx++;

			start = pos + 1;
		}

		pos++;
	}

	uint32_t len = pos - start;
	if(len != 0){
		m_item_list[idx] = new char [len + 1];
		strncpy(m_item_list[idx], start, len);
		m_item_list[idx][len]  = '\0';
	} 
}

CStrExplode::~CStrExplode()
{
	for (uint32_t i = 0; i < m_item_cnt; i++) {
		delete [] m_item_list[i];
	}

	delete [] m_item_list;
}

