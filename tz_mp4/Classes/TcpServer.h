//////////////////////////////////////////////////////////////////////
// ServerSocket.h: interface for the CTcpServer class.
//
// 用SELECT模型实现的TCP SOCKET服务器类
// 支持TCP SOCKET服务器
// 作    者: 肖可伟
// 日    期: 2005年6月
// 最后修改: 2009年5月4日
// 版权所有: 北京蛙视通信技术有限公司
// 
// 网络初始化
// WSADATA wsd;
// WSAStartup(MAKEWORD(2,2),&wsd);
//
// 释放网络资源
// WSACleanup();
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERSOCKET_H__0B180593_ABBF_4E81_9656_562FB97F1323__INCLUDED_)
#define AFX_SERVERSOCKET_H__0B180593_ABBF_4E81_9656_562FB97F1323__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib ")	
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <vector>
using namespace std;
#include "MutexInteger.h"

namespace vfc
{
	const int BUFF_LENGTH = 100*1024;
	
	typedef void (*TCP_ACCEPT)(SOCKET s,LPCTSTR sIP,int nPort,LPVOID lParam);		
	typedef void (*TCP_RECEIVE)(SOCKET s,LPCTSTR sData,int nLen,LPVOID lParam);	
	typedef void (*TCP_CLOSE)(SOCKET s,LPVOID lParam);								
	
	class CTcpServer  
	{
	private:
		SOCKET			sock_fd;			//listen SOCKET
		vector<SOCKET>  m_aSockList;		//Client SOCKET List
		CMutexInteger	m_bThreadRuning;	//Receive thread status
		HTHREAD			m_hThread;			//Receive thread handle
		static THREAD_RETURN_TYPE tcp_deal_thread(void* lParam);

		//回调指针
		TCP_ACCEPT m_pOnAccept;
		TCP_RECEIVE m_pOnReceive;
		TCP_CLOSE m_pOnClose;
		LPVOID m_lParam;

	public:
		BOOL Create(int nPort);
		void Close();
		BOOL CloseClient(SOCKET s);
		BOOL SendData(SOCKET s,void* pData,int nLen);
		BOOL SetSendBuffer(int nSize);
		BOOL SetSendTimeOut(int nTimeOut,SOCKET s = INVALID_SOCKET);	//ms
		BOOL SetReceiveTimeOut(int nTimeOut,SOCKET s = INVALID_SOCKET);	//ms
		BOOL SetKeepAlive(int nInterval,int nKeepCount,SOCKET s = INVALID_SOCKET);	//ms
		void SetCallback(TCP_ACCEPT pOnAccept,TCP_RECEIVE pOnReceive,TCP_CLOSE pOnClose,LPVOID lParam);
		//构造析构函数
		CTcpServer();
		virtual ~CTcpServer();
		
	};
}
#endif // !defined(AFX_SERVERSOCKET_H__0B180593_ABBF_4E81_9656_562FB97F1323__INCLUDED_)
