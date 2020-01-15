//////////////////////////////////////////////////////////////////////
//
// UDP SOCKET封装类
// 头 文 件:UdpSocket.h
// 实现文件:UdpSocket.cpp
// 作    者:肖可伟
// 日    期:2004年8月18日
// 最后更新:2007年8月23日
// Windows下使用前请先初始化网络,代码如下:
//		WSADATA wsd;
//		WSAStartup(MAKEWORD(2,2),&wsd);
// Windows下释放网络资源,代码如下:
//		WSACleanup();
//
//////////////////////////////////////////////////////////////////////

#ifndef _UDPSOCKET_H_
#define _UDPSOCKET_H_

#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")	
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "MacroDefine.h"
#include "MutexInteger.h"
#include "VorxThread.h"

namespace vfc
{
	typedef void (*RECV_CALLBACK)(char*,int,char*,int,LPVOID);
	
	class CUdpSocket
	{
	public:
		//创建关闭
		BOOL IsCreated(){return (m_nLocalPort > 0);}
		BOOL Create(int nPort,BOOL bRecv,int nBuffSize,const char*szBindIP);//nPort-网络端口 bRecv-是否接收数据 nBuffSize-接收缓冲大小 szBindIP绑定的IP地址
		BOOL Close();
		BOOL SetRecvThreadPriority(int nPriority= 2);//设置线程优先级

		//加入及退出组播组
		BOOL AddMembership(LPCTSTR sip);
		BOOL DropMembership(LPCTSTR sip);
		//缓冲大小及TTL参数设置
		BOOL SetRecvBuffer(int nSize);
		BOOL SetSendBuffer(int nSize);
		BOOL SetMulticastTTL(int nTTL);
		//接收数据回调设置
		void SetCallBack(RECV_CALLBACK p,LPVOID pParam);
		//向网络发送UDP数据
		int  SendTo(const unsigned char * p, const int nLen, const char * sip, const int port);
		int	 SendTo(const unsigned char * p, const int nLen, const sockaddr_in &addr);
		//取得本SOCKET所绑定的端口
		int GetBindedPort();
		//设置接收RTP校验,如果为RTP包,则自动去掉RTP头
		void SetRTPCheck(BOOL bCheck){m_bRTPCheck = bCheck;}
		//统计参数函数
		int GetMaxRecvSize(){return m_nMaxRecvSize;}
		int GetMaxSendSize(){return m_nMaxSendSize;}
		
	private:
		BOOL			m_bRTPCheck;
		int				m_nRecvBufSize;
		char*			m_pRecvBuff;
		CMutexInteger	m_nMaxRecvSize;
		CMutexInteger	m_nMaxSendSize;
		static THREAD_RETURN_TYPE udp_recv_thread(void* lParam);
		
	private:
		SOCKET	sock_fd;
		int		m_nLocalPort;
		BOOL	m_bThreadRuning;
		HTHREAD	m_hThread;			//接收线程句柄
		//回调指针及参数
		RECV_CALLBACK pOnReceive;	//接收网络数据回调函数
		LPVOID m_pCallBackParam;	//接收回调函数的附带参数
		
	public:
		//构造析构
		CUdpSocket(int nBuffSize = 1024 * 100);
		virtual ~CUdpSocket();
		
	};
}
#endif
