// ServerSocket.cpp: implementation of the CTcpServer class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TcpServer.h"
#include "GlobalFunction.h"

#ifdef WIN32
#include <process.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

struct TCP_KEEPALIVE {
    u_long  onoff;
    u_long  keepalivetime;
    u_long  keepaliveinterval;
} ;

#ifdef WIN32
	#define SHUT_RDWR          SD_BOTH  
#endif
#define SIO_KEEPALIVE_VALS   _WSAIOW(IOC_VENDOR,4)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace vfc
{
	CTcpServer::CTcpServer()
	{
		m_hThread = THREAD_HANDLE_NULL;
		m_bThreadRuning = FALSE;
		sock_fd = INVALID_SOCKET;
		
		m_pOnAccept = NULL;
		m_pOnReceive = NULL;
		m_pOnClose = NULL;
		m_lParam = NULL;
	}
	
	CTcpServer::~CTcpServer()
	{
		Close();
	}

	BOOL CTcpServer::Create(int nPort)
	{
		if(sock_fd != INVALID_SOCKET) Close();

		sock_fd = socket(AF_INET,SOCK_STREAM,0);
		if(sock_fd == INVALID_SOCKET) return FALSE;
		
		struct sockaddr_in addr;
		memset(&addr,0,sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(nPort);

		if(bind(sock_fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
		{
			return FALSE;
		}

        if(listen(sock_fd,SOMAXCONN)<0)   
        {   
			return FALSE;
        } 

		m_bThreadRuning = TRUE;
		CREATE_THREAD(m_hThread,tcp_deal_thread);

		return TRUE;
	}
	
	void CTcpServer::Close()
	{
		m_bThreadRuning = FALSE;
		if(m_hThread != THREAD_HANDLE_NULL)
		{
			WAIT_FOR_THREAD(m_hThread);
			m_hThread = THREAD_HANDLE_NULL;
		}
		if(sock_fd != INVALID_SOCKET)
		{
			shutdown(sock_fd,SHUT_RDWR);
			closesocket(sock_fd);
		}
		sock_fd = INVALID_SOCKET;
		return;
	}

	BOOL CTcpServer::CloseClient(SOCKET s)
	{
		for(vector<SOCKET>::iterator it=m_aSockList.begin();it!=m_aSockList.end();it++)
		{
			if(s == *it)
			{
				shutdown(s,SHUT_RDWR);
				closesocket(s);
				m_aSockList.erase(it);
				return TRUE;
			}
		}
		return FALSE;
	}
	
	BOOL CTcpServer::SetSendBuffer(int nSize)
	{
		return SOCKET_ERROR != setsockopt(sock_fd,SOL_SOCKET,SO_SNDBUF,(char *)&nSize,sizeof(nSize));
	}

	BOOL CTcpServer::SetSendTimeOut(int nTimeOut,SOCKET s)
	{
		if(INVALID_SOCKET == s)
			s = sock_fd;
		return SOCKET_ERROR != setsockopt(s,SOL_SOCKET,SO_SNDTIMEO,(char *)&nTimeOut,sizeof(nTimeOut));
	}	

	BOOL CTcpServer::SetReceiveTimeOut(int nTimeOut,SOCKET s)
	{
		if(INVALID_SOCKET == s)
			s = sock_fd;
		return SOCKET_ERROR != setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,(char *)&nTimeOut,sizeof(nTimeOut));
	}
#ifdef WIN32
	BOOL CTcpServer::SetKeepAlive(int nInterval,int nKeepCount,SOCKET s)
	{
		BOOL bRet = TRUE;
		if(INVALID_SOCKET == s)
			s = sock_fd;
		TCP_KEEPALIVE inKeepAlive = {0}; //输入参数
        unsigned long ulInLen = sizeof(TCP_KEEPALIVE); 

        TCP_KEEPALIVE outKeepAlive = {0}; //输出参数
        unsigned long ulOutLen = sizeof(TCP_KEEPALIVE); 
        
        unsigned long ulBytesReturn = 0; 

        //设置socket的keep alive为5秒，并且发送次数为3次  
        inKeepAlive.onoff = 1; 
        inKeepAlive.keepaliveinterval = 500*nInterval; //两次KeepAlive探测间的时间间隔
        inKeepAlive.keepalivetime = 500*nInterval; //开始首次KeepAlive探测前的TCP空闭时间
        if(WSAIoctl((unsigned int)s, SIO_KEEPALIVE_VALS,
			(LPVOID)&inKeepAlive, ulInLen,
			(LPVOID)&outKeepAlive, ulOutLen,
			&ulBytesReturn, NULL, NULL) == SOCKET_ERROR)
        {
			bRet = FALSE;
        }
		return bRet;
	}
#else
	BOOL CTcpServer::SetKeepAlive(int nInterval,int nKeepCount,SOCKET s)
	{
		BOOL bRet = TRUE;
		if(INVALID_SOCKET == s)
			s = sock_fd;
		int keepAlive = 1;
		int keepIdle = nInterval;
		int keepInterval = nInterval;
		int keepCount = nKeepCount;
        if(setsockopt(s,SOL_SOCKET,SO_KEEPALIVE,(void*)&keepAlive,sizeof(keepAlive)) == -1)
        {
			bRet = FALSE;
        }
        if(setsockopt(s,SOL_TCP,TCP_KEEPIDLE,(void *)&keepIdle,sizeof(keepIdle)) == -1)
        {
			bRet = FALSE;
        }
        if(setsockopt(s,SOL_TCP,TCP_KEEPINTVL,(void *)&keepInterval,sizeof(keepInterval)) == -1)
        {
			bRet = FALSE;
        }
        if(setsockopt(s,SOL_TCP,TCP_KEEPCNT,(void *)&keepCount,sizeof(keepCount)) == -1)
        {
			bRet = FALSE;
        }
		return bRet;
	}
#endif
	BOOL CTcpServer::SendData(SOCKET s,void* pData,int nLen)
	{
		int nWritted = 0,nTryTimes = 5,nRet = 0;
		while(nWritted < nLen && nTryTimes-- > 0)
		{
			if(nTryTimes <4 )
				printf("nTryTimes = %d",nTryTimes);
			nRet = send(s,(const char*)pData+nWritted,nLen-nWritted,0);
			if(nRet > 0)
				nWritted += nRet;
		}
		return nLen == nWritted;
	}

	void CTcpServer::SetCallback(TCP_ACCEPT pOnAccept,TCP_RECEIVE pOnReceive,TCP_CLOSE pOnClose,LPVOID lParam)
	{
		m_pOnAccept  = pOnAccept;
		m_pOnReceive = pOnReceive;
		m_pOnClose   = pOnClose;
		m_lParam     = lParam;
	}

	THREAD_RETURN_TYPE CTcpServer::tcp_deal_thread(void* lParam)
	{
		CTcpServer* pThis = (CTcpServer*)lParam;

		fd_set rd;					//Read fd
		int nfds;					//Max SOCKET value  
		struct timeval tv;			//select timeout value
		struct sockaddr_in addr;	//Client IP

		const int nBuffLen = 10240;	//Receive buffer size
		char sBuff[nBuffLen];
		int nRecvLen = 0;

		while(pThis->m_bThreadRuning)
		{
			tv.tv_sec = 0;		
			tv.tv_usec = 100;	//Set timeout 100ms

			FD_ZERO(&rd);
			FD_SET(pThis->sock_fd,&rd);
			nfds = pThis->sock_fd;
			for(int i=0;i<pThis->m_aSockList.size();i++)
			{
				nfds = max(nfds,pThis->m_aSockList[i]);
				FD_SET(pThis->m_aSockList[i],&rd);
			}

			if(select(nfds+1,&rd,NULL,NULL,&tv) > 0)
			{
				if(!pThis->m_bThreadRuning) break;

				if(FD_ISSET(pThis->sock_fd,&rd))
				{
					memset(&addr,0,sizeof(addr)); 
#ifdef WIN32
					int addrlen = sizeof(addr);
#else
					unsigned int addrlen = sizeof(addr);
#endif
					SOCKET s = accept(pThis->sock_fd,(struct sockaddr*)&addr,&addrlen);
					if(s != INVALID_SOCKET)
					{
						if(pThis->m_pOnAccept)
						{
							char sIP[100] = {0};
							VGAPI_Address2String(addr.sin_addr,sIP);
							pThis->m_pOnAccept(s,(LPCTSTR)sIP,ntohs(addr.sin_port),pThis->m_lParam);
						}
						pThis->m_aSockList.push_back(s);
						nfds = max(nfds,s);
					}
				}
				else
				{
					for(vector<SOCKET>::iterator it=pThis->m_aSockList.begin();it!=pThis->m_aSockList.end();it++)
					{
						if(FD_ISSET(*it,&rd))
						{
							nRecvLen = recv(*it,sBuff,nBuffLen,0);  
							if(nRecvLen > 0)
							{
								if(pThis->m_pOnReceive)
									pThis->m_pOnReceive(*it,(LPCTSTR)sBuff,nRecvLen,pThis->m_lParam);
							}
							else
							{
								//1.callback function
								if(pThis->m_pOnClose)
									pThis->m_pOnClose(*it,pThis->m_lParam);
								//2.close the socket
								shutdown(*it,SHUT_RDWR);
								closesocket(*it);
								//3.remove it from fd
								if(pThis->m_aSockList.begin() == it)
								{
									pThis->m_aSockList.erase(it);
									if(pThis->m_aSockList.empty())
										break;
									it = pThis->m_aSockList.begin();
									continue;
								}
								else
								{
									it--;
									pThis->m_aSockList.erase(it + 1);
									continue;
								}
							}
						}
					}
				}
			}
		}

		for(int i=0;i<pThis->m_aSockList.size();i++)
		{
			shutdown(pThis->m_aSockList[i],SHUT_RDWR);
			closesocket(pThis->m_aSockList[i]);
		}
		pThis->m_aSockList.clear();

		return TRHEAD_RETURN_VALUE;
	}
}

