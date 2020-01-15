// TcpClient.cpp: implementation of the CTcpClient class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TcpClient2.h"
#include <fcntl.h>
#include "GlobalFunction.h"


#ifdef WIN32
#include <windows.h>
#include <process.h>
#else
#include <sys/types.h>   
#include <sys/time.h>   
#include <unistd.h>
#endif

#define CONST_VIDEO_TCP_VERIFY_HEAD 4
int nTcpHead = 'HEAD';//转换成数字为1212498244
static const char * g_sTcpVerifyHead = (char *)&nTcpHead;



namespace vfc
{
	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////
	
	CTcpClient2::CTcpClient2()
	{
		sock_fd = INVALID_SOCKET;
		
		m_pOnReceive = NULL;
		m_pOnClose = NULL;
		m_lFuncParam = NULL;
		m_nLocalPort = 0;
		
		memset(&m_raddr,0,sizeof(m_raddr));
		m_raddr.sin_family = AF_INET;
		m_bIsTcpSocket = FALSE;
		m_bIsConnected = FALSE;
		m_pCyclePipe = NULL;
		m_hThread.SetParam(tcp_recv_thread,this,1);
		m_hDealRecvBufThread.SetParam(tcp_DealRecvBuf_thread,this,1);
	}
	
	CTcpClient2::~CTcpClient2()
	{
		Close();
		if(m_pCyclePipe){
			delete m_pCyclePipe;
			m_pCyclePipe = NULL;
		}
		//m_recvTcpDataPipe.ResetPipeData();
	}

	void CTcpClient2::SetCallback(TCPCLIENT_RECVE pOnReceive,TCPCLIENT_CLOSE pOnClose,LPVOID lParam)
	{
		m_pOnReceive = pOnReceive;
		m_pOnClose = pOnClose;
		m_lFuncParam = lParam;
	}
	
	void CTcpClient2::GetServerInfo(char *sIP,int* nPort)
	{
		if(sIP)VGAPI_Address2String(m_raddr.sin_addr,sIP);
		if(nPort) *nPort = ntohs(m_raddr.sin_port);
	}
	
	BOOL CTcpClient2::Create(int nLocalPort,LPCTSTR sRemoteIP,int nRemotePort,BOOL bRecv)
	{
		//1.创建SOCKET
		sock_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if(sock_fd == INVALID_SOCKET) return FALSE;
		//2.绑定本地端口
		struct sockaddr_in addr;
		memset(&addr,0,sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if(nLocalPort < 0)
			return FALSE;
		addr.sin_port = htons(nLocalPort);
		if(SOCKET_ERROR == bind(sock_fd,(struct sockaddr*)&addr,sizeof(addr)))
		{
			Close();
			return FALSE;
		}
		m_bIsTcpSocket = TRUE;

		sockaddr_in addrMy;
		memset(&addrMy,0,sizeof(addrMy));
		socklen_t len = sizeof(addrMy);
		if(SOCKET_ERROR == getsockname(sock_fd,(sockaddr*)&addrMy,&len)) 
		{
			Close();
			return 0;
		}
		m_nLocalPort = ntohs(addrMy.sin_port);
		//3.连接服务器
//		m_raddr.sin_addr.s_addr = inet_addr(sRemoteIP);
//		m_raddr.sin_port = htons(nRemotePort);
//		if(SOCKET_ERROR == connect(sock_fd,(sockaddr*)&m_raddr,sizeof(m_raddr)))
//		{
//			Close();
//			return FALSE;
//		}
//		//4.设为非阻塞
//#ifndef WIN32
//		int flags = fcntl(sock_fd, F_GETFL, 0); 
//		fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK); 
//#endif
//		//5.启动接收线程
//		m_bThreadRuning = TRUE;
//		CREATE_THREAD(m_hThread,tcp_recv_thread);
		return TRUE;
	}
	
	void CTcpClient2::Close(BOOL bWaitForThread)
	{
		m_bIsConnected = FALSE;
		if(INVALID_SOCKET != sock_fd) 
		{
			closesocket(sock_fd);
			sock_fd = INVALID_SOCKET;
		}
		m_hThread.Stop();
		m_hDealRecvBufThread.Stop();
	}
	
	BOOL CTcpClient2::Send(void* pData,int nLen)
	{
		if(INVALID_SOCKET == sock_fd) return FALSE;
		
		int nWrited = 0,nTryTimes = 0;
		while(nWrited < nLen && nTryTimes++ < 5)
		{
			nWrited += send(sock_fd,(const char*)pData,nLen,0);
		}
		return (nLen == nWrited);
	}

	int CTcpClient2::tcp_DealRecvBuf_thread(void* lParam)
	{
		CTcpClient2* pThis = (CTcpClient2*)lParam;

		BYTE * pFind = pThis->m_pCyclePipe->SeekData((BYTE*)g_sTcpVerifyHead,CONST_VIDEO_TCP_VERIFY_HEAD);
		if(pFind)      // 如果找到了10字节直接固定特征码
		{
			DWORD tms = GetTickCount();
			pThis->m_pCyclePipe->ChuckData(pFind);//丢掉头标记之前的数据
			int nLen = pThis->m_pCyclePipe->GetPayloadSize();
			if(nLen < sizeof(TCP_DATA_HEAD)){
				//OutputDebugStringA("payload < head_len\n");	
				return 0;
			
			}
			BYTE head_data[sizeof(TCP_DATA_HEAD)] = {0};
			int n = pThis->m_pCyclePipe->GetPipeData(head_data, sizeof(TCP_DATA_HEAD));
			if(n != sizeof(TCP_DATA_HEAD)){
				//OutputDebugStringA("GetPipeData error\n");	
				return 0;
			}
			TCP_DATA_HEAD* pHead = (TCP_DATA_HEAD*)head_data;
			if(pHead == NULL) return 0;
			if(nLen < pHead->lSize){
				//OutputDebugStringA("data len not enough\n");	
				return 0;
			}
			BYTE* pBuff = new BYTE[pHead->lSize];
			//memset(pBuff,0,nLen);
			if(pBuff == NULL ) return 0;
			n = pThis->m_pCyclePipe->PopPipe(pBuff,pHead->lSize);
			if(n != pHead->lSize) 
			{
				//OutputDebugStringA("GetPipeData error\n");	
 				delete pBuff;
 				pBuff = NULL;
				return 0;
			}
		
			if(pThis->m_pOnReceive)
			{
				pThis->m_pOnReceive(pBuff+sizeof(TCP_DATA_HEAD), pHead->lSize-sizeof(TCP_DATA_HEAD), pThis->m_lFuncParam);	

			}
			if(pBuff)
			{
 				delete pBuff;
 				pBuff = NULL;
			}				
		}
		else  // 没有找到10字节直接固定特征码，可能缓冲不够，直接返回
		{
			return 0;
		}
		return 0;
	}

	int CTcpClient2::tcp_recv_thread(void* lParam)
	{
		CTcpClient2* pThis = (CTcpClient2*)lParam;
		char* buf = new char[SOCKET_REC_BUF];
		int nRet = 0;
		struct timeval tv;
		fd_set	fdset;
		tv.tv_sec = 0;		
		tv.tv_usec = 100;	//超时设为100毫秒
		FD_ZERO(&fdset);
		FD_SET(pThis->sock_fd,&fdset);
		if(select(pThis->sock_fd+1,&fdset,NULL,NULL,&tv) > 0)
		{
			memset(buf,0,SOCKET_REC_BUF);
			nRet = recv(pThis->sock_fd,buf,SOCKET_REC_BUF,0);								
			if(nRet == SOCKET_ERROR || nRet == 0)
			{
				delete[] buf;
				return 0;
			}
			BYTE * pVideoData = (BYTE*)buf;
			// 将数据放入循环缓冲
			int iPushLen = pThis->m_pCyclePipe->PushPipe(pVideoData,nRet);
			if(iPushLen != nRet){
				OutputDebugStringA("push pipe error!\n");
			}
			pThis->m_pCyclePipe->TraceDataSize();
		}
		delete[] buf;
		return 0;
	}

	BOOL CTcpClient2::ConnectTcpServer( const char* sSerIp,int nPort )
	{
		m_raddr.sin_addr.s_addr = inet_addr(sSerIp);
		m_raddr.sin_port = htons(nPort);
		if(SOCKET_ERROR == connect(sock_fd,(sockaddr*)&m_raddr,sizeof(m_raddr)))
		{
			int err = GetLastError();
			Close();
			return FALSE;
		}
		else
		{
			m_bIsConnected = TRUE;
		}
		//4.设为非阻塞
#ifndef WIN32
		int flags = fcntl(sock_fd, F_GETFL, 0); 
		fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK); 
#endif

		if(NULL == m_pCyclePipe)
		{
			m_pCyclePipe = new CCyclePipe();
		}
		if(m_pCyclePipe)
		{
			m_pCyclePipe->Initialize(1024*1024,FALSE);
		}
		//5.启动处理接收队列线程
		m_hDealRecvBufThread.Start();
		//6.启动接收线程
		m_hThread.Start();
		return TRUE;
	}
	BOOL CTcpClient2::IsConnected()
	{
		return m_bIsConnected;
	}
	BOOL CTcpClient2::SetRecBuffer(int nSize)
	{
		return SOCKET_ERROR != setsockopt(sock_fd,SOL_SOCKET,SO_RCVBUF,(char *)&nSize,4);
	}

	BOOL CTcpClient2::SetRecvThreadPriority( int nPriority/*= 2*/ )
	{
		if (/*m_hThread*/1)
		{
#ifdef WIN32
			int nVal = 0;
			switch(nPriority)
			{
			case -3:
				nVal = THREAD_PRIORITY_IDLE;break;
			case -2:
				nVal = THREAD_PRIORITY_LOWEST;break;
			case -1:
				nVal = THREAD_PRIORITY_BELOW_NORMAL;break;
			case 0:
				nVal = THREAD_PRIORITY_NORMAL;break;
			case 1:
				nVal = THREAD_PRIORITY_ABOVE_NORMAL;break;
			case 2:
				nVal = THREAD_PRIORITY_HIGHEST;break;
			case 3:
				nVal = THREAD_PRIORITY_TIME_CRITICAL;break;
			}
		//	return SetThreadPriority(m_hThread,nVal);
			return TRUE;
#else
			struct sched_param  hThreadPrior;
			int policy;
			pthread_getschedparam(m_hThread, &policy, &hThreadPrior);
			policy = SCHED_RR;
			hThreadPrior.sched_priority = nPriority;
			return (pthread_setschedparam(m_hThread,policy,&hThreadPrior) == 0);
#endif
		}
		else return FALSE;
	}

}
