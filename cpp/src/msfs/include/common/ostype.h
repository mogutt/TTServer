// OS dependant type definition
#ifndef __OS_TYPE_H__
#define __OS_TYPE_H__

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#else
#ifdef __APPLE__
#include <sys/event.h>
#include <sys/time.h>
#include <sys/syscall.h>	// syscall(SYS_gettid)
#else
#include <sys/epoll.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>		// define int8_t ...
#include <signal.h>
#define closesocket close
#define ioctlsocket ioctl
#endif

#ifdef _WIN32
typedef char			int8_t;
typedef short			int16_t;
typedef int				int32_t;
typedef	long long		int64_t;
typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned int	uint32_t;
typedef	unsigned long long	uint64_t;
typedef int				socklen_t;
#else
#ifdef linux
const int TRUE = 1;
const int FALSE = 0;
#endif
typedef int	SOCKET;
typedef int BOOL;
const int SOCKET_ERROR	= -1;
const int INVALID_SOCKET = -1;
#endif

typedef unsigned char	uchar_t;
typedef int				net_handle_t;
typedef int				conn_handle_t;

enum {
	NETLIB_OK		= 0,
	NETLIB_ERROR	= -1
};

#define NETLIB_INVALID_HANDLE	-1

enum
{
	NETLIB_MSG_CONNECT = 1,
	NETLIB_MSG_CONFIRM,
	NETLIB_MSG_READ,
	NETLIB_MSG_WRITE,
	NETLIB_MSG_CLOSE,
	NETLIB_MSG_TIMER,
	NETLIB_MSG_LOOP,
};

typedef void (*callback_t)(void* callback_data, uint8_t msg, uint32_t handle, void* pParam);

//windows
#ifdef _WIN32
#define MSFS_WIN            1
#endif

//linux
#ifdef linux
#define MSFS_LINUX      1
#endif

//freebsd
#ifdef __FreeBSD__
#define MSFS_BSD            1
#endif

#ifdef __APPLE__
#define MSFS_BSD            1
#endif
//unix
#ifdef _UNIX
#define MSFS_UNIX           1
#endif

#endif
