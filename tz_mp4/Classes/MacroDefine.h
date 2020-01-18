#ifndef _VORX_MACRO_DEFINE_HEAD_FILE
#define _VORX_MACRO_DEFINE_HEAD_FILE

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#pragma warning( disable : 4251 )
#endif

#include <vector>
#include <list>
using namespace std;
#include <stdio.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Mmsystem.h>
#pragma comment(lib,"Winmm.lib") 
#include <process.h>
#include <conio.h>
#include <Winsock2.h>
#pragma comment(lib,"Ws2_32.lib")
#else
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// CKK ADD 2010-09-13
#include <sys/types.h>
#include <netdb.h>
#include <ifaddrs.h>

#define TRACE printf
#endif

#define MIN(a,b) (a>b?b:a)
#define MAX(a,b) (a>b?a:b)
#define HRES  long
#define HTASK long
#define UINT16 unsigned short
#define UINT8  unsigned char

#ifdef WIN32//Windows操作系统下宏定义

#ifdef VFC_EXPORTS
#define VFC_CLS __declspec(dllexport)
#else
#define VFC_CLS __declspec(dllimport)
#endif
#define THREAD_RETURN_TYPE void						//线程返回值类型
#define TRHEAD_RETURN_VALUE								//线程函数返回值
#define THREAD_HANDLE_NULL NULL						//线程句柄非法值
#define HTHREAD HANDLE										//线程句柄
#define CREATE_THREAD(handle,addr) handle = (HANDLE)_beginthread(addr,0,this)		//启动线程
#define BEGIN_THREAD(handle,addr,param) handle = (HANDLE)_beginthread(addr,0,param)	//启动线程
#define WAIT_FOR_THREAD(handle) \
	if(WaitForSingleObject(handle,1000) == WAIT_TIMEOUT)\
	{\
		DWORD dwExitCode;\
		GetExitCodeThread(handle,&dwExitCode);\
		TerminateThread(handle,dwExitCode);\
	}

typedef int socklen_t;

#else//Linux操作系统下宏定义

#define VFC_CLS
#define UINT64 unsigned long long
#define INT64  long long
#define UINT32 unsigned int
#define INT32  int
#define USHORT unsigned short
#define __stdcall
#define THREAD_RETURN_TYPE void*
#define TRHEAD_RETURN_VALUE NULL
#define THREAD_HANDLE_NULL 0
#define HTHREAD pthread_t
#define CREATE_THREAD(handle,addr) pthread_create(&handle,NULL,addr,this)
#define BEGIN_THREAD(handle,addr,param) pthread_create(&handle,NULL,addr,param)
#define WAIT_FOR_THREAD(handle) pthread_join(handle,NULL)
#define BYTE unsigned char
#define ULONG unsigned long
#define UINT unsigned int
#define WORD unsigned short
#define DWORD unsigned long
#define LPCTSTR const char*
#define BOOL int
#define TRUE 1
#define FALSE 0
#define LPVOID void*
#define HANDLE void*
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
#define CloseHandle(xx)
#define HOSTENT hostent
#define SOCKET int
inline long GetTickCount(){return (1000*times(NULL))/sysconf(_SC_CLK_TCK);}
inline void Sleep(DWORD dwMilliseconds){usleep(1000 * dwMilliseconds);}
#define _stricmp strcasecmp

#endif//ifdef WIN32

typedef void (__stdcall *OUTPUT_LOG)(int nType,LPCTSTR sMsg,void* lParam);
typedef void (__stdcall *ASYN_RET)(int nFun,int nSub,int nRetCode,unsigned char* pBody,int nLen,void* lParam);


#endif//ifndef _VORX_MACRO_DEFINE_HEAD_FILE
