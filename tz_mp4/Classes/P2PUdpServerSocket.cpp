#include "StdAfx.h"
#include "P2PUdpServerSocket.h"
#include "MacroDefine.h"
#include "GlobalFunction.h"

#ifdef WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <process.h> 
#include <Iprtrmib.h>
#else
#include <stdio.h>
#endif

namespace vfc
{
	CP2PUdpServerSocket::CP2PUdpServerSocket(int nBuffSize)
	{
		pOnReceive = NULL;
		m_nLocalPort = 0;
		sock_fd = INVALID_SOCKET;
		m_bThreadRuning = FALSE;
		m_pCallBackParam = NULL;
		m_hThread = THREAD_HANDLE_NULL;
		m_bRTPCheck = FALSE;
		m_nRecvBufSize = nBuffSize;
		m_pRecvBuff = NULL;
		m_nMaxRecvSize = 0;
		m_nMaxSendSize = 0;
		
		m_P2PClientList.clear();
	}
	
	CP2PUdpServerSocket::~CP2PUdpServerSocket()
	{
		if(m_pRecvBuff) delete [] m_pRecvBuff;
	}

	THREAD_RETURN_TYPE CP2PUdpServerSocket::udp_recv_thread(void* lParam)
	{
		CP2PUdpServerSocket* pThis = (CP2PUdpServerSocket*) lParam;
		
		int buf_size = pThis->m_nRecvBufSize;	//接收缓冲区大小
		if(pThis->m_pRecvBuff == NULL)
			pThis->m_pRecvBuff = new char[buf_size];
		char *buff = pThis->m_pRecvBuff;		//接收缓冲
		int nRecvLen;							//接收到数据和实际长度
		
		sockaddr_in addr;						//用于接收源地址
		socklen_t addr_len = sizeof(addr);		//地址长度
		
		const int nRTPHeadLen = 12;
		BOOL bRTP = FALSE;						//是否RTP包检测结果
		
		char sFromIP[50];						//来源IP
		int nFromPort;							//来源端口
		
		fd_set	fdset;
		struct timeval tv;
		
		while(pThis->m_bThreadRuning)
		{
			tv.tv_sec = 0;
			tv.tv_usec = 100000;
			FD_ZERO(&fdset);
			FD_SET(pThis->sock_fd, &fdset);
			
			switch(select(pThis->sock_fd + 1,&fdset,NULL,NULL,&tv)) 
			{
			case -1://出错
				break;
			case 0:	//超时
				break;
			default:
				{
					nRecvLen = recvfrom(pThis->sock_fd,buff,buf_size,0,(sockaddr*)&addr,&addr_len);
					if(!pThis->m_bThreadRuning) break;
					if(nRecvLen > 0 && pThis->pOnReceive)
					{
						// 检查是否是P2P相关请求,如果是则回应请求
						SERVER_MESSAGE *pMsg = (SERVER_MESSAGE*)buff;
						switch(pMsg->m_iMsgFlag)
						{
							case MSG_CONNECT: 
							{
								// 更新客户端P2P地址
								Server_Addr_Info addInfo = pMsg->m_addrInfo;
								VGAPI_Address2String(addr.sin_addr,addInfo.m_sP2PIP);
								addInfo.m_iP2PPort = htons(addr.sin_port);
								addInfo.m_blOnline = TRUE;
								pThis->UpdateClientInfo(addInfo);
								// 构建回应消息包 
								pMsg->m_iMsgFlag = MSG_CONNECTACK;
								strcpy(pMsg->m_addrInfo.m_sIP,pThis->m_sLocalIP);
								pMsg->m_addrInfo.m_iPort = pThis->m_nLocalPort;
								int iRet = pThis->SendTo((const char *)pMsg,sizeof(SERVER_MESSAGE),addr);
								break;
							}
							case MSG_OK: 
							{
								// 更新客户端P2P地址,设置客户端在线
								Server_Addr_Info addInfo = pMsg->m_addrInfo;
								VGAPI_Address2String(addr.sin_addr,addInfo.m_sP2PIP);
								addInfo.m_iP2PPort = htons(addr.sin_port);
								addInfo.m_blOnline = TRUE;
								pThis->UpdateClientInfo(addInfo);
								break;
							}
							default:
							{
								bRTP = FALSE;
								//校验是否RTP包
								if(pThis->m_bRTPCheck && nRecvLen >= nRTPHeadLen)
								{
									if(buff[0] == 0x80 && buff[1] == 0x21)
									{
										bRTP = TRUE;
									}
								}
								pThis->m_nMaxRecvSize = max((int)pThis->m_nMaxRecvSize,nRecvLen);
								//						strcpy(sFromIP,inet_ntoa(addr.sin_addr));
								VGAPI_Address2String(addr.sin_addr,sFromIP);
								nFromPort = htons(addr.sin_port);
								if(bRTP)
									pThis->pOnReceive(buff+nRTPHeadLen,nRecvLen-nRTPHeadLen,sFromIP,nFromPort,pThis->m_pCallBackParam);
								else
								pThis->pOnReceive(buff,nRecvLen,sFromIP,nFromPort,pThis->m_pCallBackParam);
								break;
							}
						}
					}//if(nRecvLen > 0 && pThis->pOnReceive)
					break;
				}//default:
			}//switch(select(pThis->sock_fd + 1,&fdset,NULL,NULL,&tv)) 
		}//while(pThis->m_bThreadRuning)
		return TRHEAD_RETURN_VALUE;
	}
	
	BOOL CP2PUdpServerSocket::Create(int nPort,BOOL bRecv,int nBuffSize,const char*szBindIP)
	{
		if(IsCreated()) Close();
		if(nBuffSize > 0) m_nRecvBufSize = nBuffSize;
		
		sock_fd = socket(AF_INET,SOCK_DGRAM,0);
		if(sock_fd == INVALID_SOCKET) return FALSE;
		
		struct sockaddr_in addr;
		memset(&addr,0,sizeof(addr));
		addr.sin_family = AF_INET;
		if (NULL != szBindIP && 0 < strlen(szBindIP))
		{
			addr.sin_addr.s_addr = inet_addr(szBindIP);
		}
		else
		{
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
		}
		if(nPort < 0)
			return FALSE;
		addr.sin_port = htons(nPort);
		if(bind(sock_fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
		{
			closesocket(sock_fd);
			return FALSE;
		}
		memset(&addr,0,sizeof(addr));
		socklen_t len = sizeof(addr);
		if(SOCKET_ERROR == getsockname(sock_fd,(sockaddr*)&addr,&len)) 
		{
			closesocket(sock_fd);
			return 0;
		}
		m_nLocalPort = ntohs(addr.sin_port);
		strcpy(m_sLocalIP,inet_ntoa(addr.sin_addr));    // CKK ADD 
		if(bRecv)
		{
			m_bThreadRuning = TRUE;
			CREATE_THREAD(m_hThread,udp_recv_thread);
		}
		return TRUE;
	}
	
	BOOL CP2PUdpServerSocket::Close()
	{
		m_bThreadRuning = FALSE;
		if(sock_fd != INVALID_SOCKET)
		{
			SendTo("EXIT",5,"127.0.0.1",GetBindedPort());
			closesocket(sock_fd);
			sock_fd = INVALID_SOCKET;
		}
		if(m_hThread != THREAD_HANDLE_NULL)
		{
			WAIT_FOR_THREAD(m_hThread);
			//		CloseHandle(m_hThread);
			m_hThread = THREAD_HANDLE_NULL;
		}
		m_nLocalPort = 0;
		return TRUE;
	}
	
	BOOL CP2PUdpServerSocket::SetRecvThreadPriority(int nPriority)
	{
		if (m_hThread)
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
			return SetThreadPriority(m_hThread,nVal);
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

	BOOL CP2PUdpServerSocket::AddMembership(LPCTSTR sip)
	{
		int val = 1;
		if(setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,(char*)&val,sizeof(val)) < 0)
		{
			return FALSE;
		}
		if(setsockopt(sock_fd,IPPROTO_IP,IP_MULTICAST_LOOP,(char*)&val,sizeof(val))<0)
		{
			perror("setsockopt:IP_MULTICAST_LOOP");
			return FALSE;
		}

		struct ip_mreq command;
		memset(&command,0,sizeof(command));
		command.imr_multiaddr.s_addr = inet_addr(sip);
		command.imr_interface.s_addr = htonl(INADDR_ANY);
		if(command.imr_multiaddr.s_addr == 0xffffffff)
		{
			return FALSE;
		}
		if(setsockopt(sock_fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,
			(char*)&command,sizeof(command))<0)
		{
			return FALSE;
		}

		val = 3;
		if(setsockopt(sock_fd,IPPROTO_IP,IP_MULTICAST_TTL,(char*)&val,sizeof(val))<0)
		{
			return FALSE;
		}
		return TRUE;
	}
	
	BOOL CP2PUdpServerSocket::DropMembership(LPCTSTR sip)
	{
		if(sock_fd == INVALID_SOCKET) return FALSE;
		struct ip_mreq cmd;
		cmd.imr_multiaddr.s_addr = inet_addr(sip);
		cmd.imr_interface.s_addr = htonl(INADDR_ANY);
		
		return INVALID_SOCKET != setsockopt(sock_fd,IPPROTO_IP,IP_DROP_MEMBERSHIP,(char*)&cmd,sizeof(cmd));
	}
	
	int CP2PUdpServerSocket::SendTo(LPCTSTR pData,int nLen,LPCTSTR sip,int port)
	{
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		Server_Addr_Info addrInfo;
		strcpy(addrInfo.m_sIP,sip);
		addrInfo.m_iPort = port;
		GetClientAddrInfo(addrInfo);
		if((strlen(addrInfo.m_sP2PIP) != 0) && (addrInfo.m_iP2PPort != 0))
		{
			addr.sin_addr.s_addr = inet_addr(addrInfo.m_sP2PIP);
			addr.sin_port = htons(addrInfo.m_iP2PPort);
		}
		else
		{	
			addr.sin_addr.s_addr = inet_addr(sip);
			addr.sin_port = htons(port);
		}
		return SendTo(pData,nLen,addr);
	}
	
	int	CP2PUdpServerSocket::SendTo(LPCTSTR pData,int nLen,sockaddr_in addr)
	{
		if(nLen>m_nMaxSendSize)m_nMaxSendSize = nLen;

		int ret = 0,nTryTimes = 0;
		while(ret<nLen && nTryTimes++<3)
			ret += sendto(sock_fd,pData+ret,nLen-ret,0,(sockaddr*)&addr,sizeof(addr));
		return ret;
	}
	
	void CP2PUdpServerSocket::SetCallBack(RECV_CALLBACK pFun,LPVOID lParam)
	{
		pOnReceive = pFun;
		m_pCallBackParam = lParam;
	}
	
	BOOL CP2PUdpServerSocket::SetRecvBuffer(int nSize)
	{
		return SOCKET_ERROR != setsockopt(sock_fd,SOL_SOCKET,SO_RCVBUF,(char *)&nSize,sizeof(nSize));
	}
	
	BOOL CP2PUdpServerSocket::SetSendBuffer(int nSize)
	{
		return SOCKET_ERROR != setsockopt(sock_fd,SOL_SOCKET,SO_SNDBUF,(char *)&nSize,sizeof(nSize));
	}
	
	BOOL CP2PUdpServerSocket::SetMulticastTTL(int nTTL)
	{
		return SOCKET_ERROR != setsockopt(sock_fd,IPPROTO_IP,IP_MULTICAST_TTL,(char*)&nTTL,sizeof(nTTL));
	}
	
	int CP2PUdpServerSocket::GetBindedPort()
	{
		return m_nLocalPort;
		/*
		struct sockaddr_in addr;
		socklen_t len = sizeof(sockaddr_in);
		if(SOCKET_ERROR != getsockname(sock_fd,(sockaddr*)&addr,&len))
		return (ntohs(addr.sin_port));
		int dd = ntohs(addr.sin_port);
		return 0;
		*/
	}
	const char * CP2PUdpServerSocket::GetBindedIP()
	{
		return m_sLocalIP;
	}
	void CP2PUdpServerSocket::UpdateClientInfo(Server_Addr_Info _addrInfo)
	{
		CAutoMutex am(&m_P2PClientListMutex);
		list<Server_Addr_Info>::iterator it = m_P2PClientList.begin();
		for(;it != m_P2PClientList.end();++it)
		{
			if((*it) == _addrInfo)
			{
				strcpy((*it).m_sP2PIP,_addrInfo.m_sP2PIP);
				(*it).m_iP2PPort = _addrInfo.m_iP2PPort;
				(*it).m_blOnline = _addrInfo.m_blOnline;
				break;
			}
		}
		if(it == m_P2PClientList.end())
		{
			m_P2PClientList.push_back(_addrInfo);
		}
	}
	void CP2PUdpServerSocket::GetClientAddrInfo(Server_Addr_Info & _addrInfo) 
	{
		CAutoMutex am(&m_P2PClientListMutex);
		list<Server_Addr_Info>::iterator it = m_P2PClientList.begin();
		for(;it != m_P2PClientList.end();++it)
		{
			if((*it) == _addrInfo)
			{
				strcpy(_addrInfo.m_sP2PIP,(*it).m_sP2PIP);
				_addrInfo.m_iP2PPort = (*it).m_iP2PPort;
				_addrInfo.m_lActiveTime = (*it).m_lActiveTime;
				_addrInfo.m_blOnline = (*it).m_blOnline;
				break;
			}
		}

	}
	BOOL CP2PUdpServerSocket::TestP2PClientOnline(const char * _sClientIp,int _iClientPort,int _iTimeOut)
	{
		Server_Addr_Info addrInfo;
		strcpy(addrInfo.m_sIP,_sClientIp);
		addrInfo.m_iPort = _iClientPort;
		GetClientAddrInfo(addrInfo);
		if(addrInfo.m_blOnline)
		{
			return TRUE;
		}
		else
		{
			// 构建消息体
			SERVER_MESSAGE msg;
			msg.m_iMsgFlag = MSG_RING;
			strcpy(msg.m_addrInfo.m_sIP,m_sLocalIP);
			msg.m_addrInfo.m_iPort = m_nLocalPort;
			int msgLen = sizeof(SERVER_MESSAGE); 
			int iRet = SendTo((const char*)&msg,msgLen,_sClientIp,_iClientPort);
			
			for(int j = 0; j < 10; ++j)
			{
				GetClientAddrInfo(addrInfo);
				if(addrInfo.m_blOnline)
				{
					return TRUE;
				}
				else
				{
					Sleep(_iTimeOut/10);
				}
			}
			return FALSE;
		}
	}
}
