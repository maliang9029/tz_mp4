#include "StdAfx.h"
#include "GlobalFunction.h"
#include <string.h>
#include <IPHlpApi.h>
#pragma comment(lib,"Iphlpapi.lib")

namespace vfc
{
	long g_nVfcErrorCodeBase = 0;
	void VGAPI_SetErrcodeBase(long val)
	{
		g_nVfcErrorCodeBase = val;
	}

	UINT64 VGAPI_GetMillisecond()
	{
#ifdef WIN32
		LARGE_INTEGER litmp;
		QueryPerformanceFrequency(&litmp);
		double dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率
		QueryPerformanceCounter(&litmp);
		LONGLONG QPart = litmp.QuadPart;// 获得初始值
		return (1000*QPart)/dfFreq;
#else
		struct timeval tv;
		gettimeofday(&tv,NULL);
		return (UINT64)tv.tv_sec*(UINT64)1000+(UINT64)tv.tv_usec/(UINT64)1000;
#endif
	}
	
	void VGAPI_Address2String(in_addr in,char* ret)
	{
		BYTE* p = (BYTE*)&in.s_addr;
		sprintf(ret,"%d.%d.%d.%d",p[0],p[1],p[2],p[3]);
	}
	
	long VGAPI_GetMaxMaskLength(ULONG nip1,ULONG nip2)
	{
		ULONG hip1 = ntohl(nip1);
		ULONG hip2 = ntohl(nip2);
		for(int i=0;i<=32;i++)
		{
			long mask = ((0xffffffff >> i) << i);
			if((hip1&mask) == (hip2&mask))
				return 32-i;
		}
		return 0;
	}
	
	int VGAPI_GetLocalAddress(vector<in_addr>& addrArray)
	{
#ifdef WIN32
		int iCount = 0;
		char buff[100] = {0};
		gethostname(buff,sizeof(buff));
		HOSTENT * pHost = gethostbyname(buff);
		in_addr addr;
		while(pHost&&pHost->h_addr_list[iCount])
		{
			addr.s_addr = *((ULONG*)pHost->h_addr_list[iCount++]);
			addrArray.push_back(addr);
		}
#else
/*
		struct sockaddr_in sin; 
		struct ifreq ifr;
		
		int ifCount = 0;		//网卡序号
		int addrCount = 0;		//地址序号
		
		int sock = socket(AF_INET, SOCK_DGRAM, 0); 
		if(sock >= 0)
		{
			for(ifCount=0;;ifCount++)
			{
				for(addrCount=0;;addrCount++)
				{
					if(addrCount == 0) 
						sprintf(ifr.ifr_name,"eth%d",ifCount);
					else 
						sprintf(ifr.ifr_name,"eth%d:%d",ifCount,addrCount-1);
					
					if(ioctl(sock,SIOCGIFADDR,&ifr) < 0) break;
					memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
					printf("Find Address in Function : %s,ifr_name = %s\n",inet_ntoa(sin.sin_addr),ifr.ifr_name);
					addrArray.push_back(sin.sin_addr);
					iCount ++;
				}
				if(addrCount <= 1) break;
			}
			close(sock);
		}
*/
#ifndef VORX_SRV
		int nIfCount = 0, i;
		struct sockaddr_in sin; 
		struct ifreq buff[32];
		struct ifconf ifc; 
		int sock = socket(AF_INET, SOCK_DGRAM, 0); 
		if(sock >= 0)
		{
			ifc.ifc_len = sizeof(buff);
			ifc.ifc_buf = (caddr_t)buff;
			if(!ioctl(sock, SIOCGIFCONF, (char*)&ifc))
			{
				nIfCount = ifc.ifc_len / sizeof(struct ifreq);
				for (i = 0; i < nIfCount; ++i)
				{
					if (ioctl(sock, SIOCGIFADDR, &buff[i]))
						continue;
					memcpy(&sin, &buff[i].ifr_addr, sizeof(sin));
					printf("Find Address in Function : %s,ifr_name = %s\n", inet_ntoa(sin.sin_addr), buff[i].ifr_name);
					addrArray.push_back(sin.sin_addr);
				}
			}
			close(sock);
		}
#else // !defined VORX_SRV
		struct ifaddrs *ifap0, *ifap;
		struct sockaddr_in *addr4;
// 		struct sockaddr_in6 *addr6;
		if(getifaddrs(&ifap0))
		{
			return 0;
		}
		for (ifap = ifap0; ifap != NULL; ifap=ifap->ifa_next)
		{
			if (ifap->ifa_addr==NULL)
				continue;
			if ((ifap->ifa_flags & IFF_UP) == 0)
				continue;
			if (AF_INET == ifap->ifa_addr->sa_family)
			{
				addr4 = (struct sockaddr_in *)ifap->ifa_addr;
				printf("Find Address in Function : %s,\n", inet_ntoa(addr4->sin_addr));
				addrArray.push_back(addr4->sin_addr);
			}
// 			else if (AF_INET6 == ifap->ifa_addr->sa_family)
// 			{
// 				addr6 = (struct sockaddr_in6 *)ifap->ifa_addr;
// 			}
		}
#endif // !defined VORX_SRV

#endif
		return addrArray.size();
	}
	in_addr VGAPI_NearestAddressEx(vector<in_addr> addrArray,LPCTSTR sCmpIP)
	{
		in_addr ret;
		ret.s_addr = 0;

		if(sCmpIP && strlen(sCmpIP) > 0)
		{
			int nMaxLen = 0;
			unsigned long uCmpIP = inet_addr(sCmpIP);
			string sGeteip = sCmpIP;
			string sSubGateip;
			int nPos = sGeteip.find_last_of(".");
			if(nPos!=string::npos)
			{
				sSubGateip = sGeteip.substr(0,nPos);
			}
			vector<in_addr>::iterator pt;
			for(pt=addrArray.begin();pt!=addrArray.end();pt++)
			{
				in_addr addr = *pt;
				char sLocal[128]={0};
				vfc::VGAPI_Address2String(addr,sLocal);
				string sLocalIp = sLocal;
				int nPos = sLocalIp.find_last_of(".");
				if (nPos!=string::npos)
				{
					string sSubLocal = sLocalIp.substr(0,nPos);
					if (strstr(sSubGateip.c_str(),sSubLocal.c_str())!=NULL)
					{
						ret = addr;
					}
				}
			}
		}
		if(0 == ret.s_addr && !addrArray.empty())
		{
			ret = addrArray[0];
			return ret;
		}
	}
	in_addr VGAPI_NearestAddress(vector<in_addr> addrArray,LPCTSTR sCmpIP)
	{
		in_addr ret;
		ret.s_addr = 0;
		
		if(sCmpIP && strlen(sCmpIP) > 0)
		{
			int nMaxLen = 0;
			unsigned long uCmpIP = inet_addr(sCmpIP);
			
			vector<in_addr>::iterator pt;
			for(pt=addrArray.begin();pt!=addrArray.end();pt++)
			{
				in_addr addr = *pt;
				int nLen = VGAPI_GetMaxMaskLength(addr.s_addr,uCmpIP);
				if(nMaxLen < nLen)
				{
					ret = addr;
					nMaxLen = nLen;
				}
			}
		}
		if(0 == ret.s_addr && !addrArray.empty())
		{
			ret = addrArray[0];
			return ret;
		}
		return ret;
	}
	
	BOOL VGAPI_IsValidMask(LPCTSTR sMask)
	{
		if(sMask)
		{
			int a=-1,b=-1,c=-1,d=-1;
			sscanf(sMask,"%d.%d.%d.%d",&a,&b,&c,&d);
			unsigned long val = a<<24 | b<<16 | c<<8 | d;
			
			unsigned long mask = 0xFFFFFFFF;
			for(int i=1;i<32;i++)
			{
				if(val == ((mask>>i)<<i))
					return TRUE;
			}
		}
		return FALSE;
	}
	
	BOOL VGAPI_IsValidMulticast(LPCTSTR sIP)
	{
		if(sIP)
		{
			int a=-1,b=-1,c=-1,d=-1;
			sscanf(sIP,"%d.%d.%d.%d",&a,&b,&c,&d);
			return (a>223 && a<240 && b>=0 && b<255 && c>=0 && c<255 && d>0 && d<255);
		}
		return FALSE;
	}
	
	BOOL VGAPI_IsValidIPAddr(LPCTSTR sIP)
	{
		if(sIP)
		{
			int a=-1,b=-1,c=-1,d=-1;
			sscanf(sIP,"%d.%d.%d.%d",&a,&b,&c,&d);
			return (a>0 && a<255 && b>=0 && b<255 && c>=0 && c<255 && d>0 && d<255);
		}
		return FALSE;
	}
	
	BOOL VGAPI_IsValidHostIP(LPCTSTR sIP)
	{
		return (VGAPI_IsValidIPAddr(sIP) && !VGAPI_IsValidMulticast(sIP));
	}
	
	BOOL VGAPI_IsValidMacAddr(LPCTSTR sMac)
	{
		if(sMac)
		{
			int a=-1,b=-1,c=-1,d=-1,e=-1,f=-1;
			sscanf(sMac,"%x:%x:%x:%x:%x:%x",&a,&b,&c,&d,&e,&f);
			return (a>=0 && b>=0 && c>=0 && d>=0 && e>=0 && f>=0);
		}
		return FALSE;
	}
	
	char VGAPI_GetChar()
	{
#ifdef WIN32
		return getch();
#else
		struct termios oldt,newt;
		int ch;
		tcgetattr( STDIN_FILENO, &oldt );
		newt = oldt;
		newt.c_lflag &= ~( ICANON | ECHO );
		tcsetattr( STDIN_FILENO, TCSANOW, &newt );
		ch = getchar();
		tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
		return ch;
#endif
	}
	
	long VGAPI_GetFreeNetPort()
	{
		SOCKET s = socket(AF_INET,SOCK_DGRAM,0);
		if(INVALID_SOCKET ==s) return 0;
		struct sockaddr_in addr;
		socklen_t len = sizeof(addr);
		memset(&addr,0,len);
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(0);
		if(bind(s,(sockaddr*)&addr,len)<0)
		{
			closesocket(s);
			return 0;
		}
		memset(&addr,0,len);
		if(SOCKET_ERROR == getsockname(s,(sockaddr*)&addr,&len)) 
		{
			closesocket(s);
			return 0;
		}
		closesocket(s);
		return ntohs(addr.sin_port);
	}

	string VGAPI_GetGatIPAdress()
	{

		string strAdapterInfo,strSubItemValue; 
		PIP_ADAPTER_INFO pAdapterInfo; 
		PIP_ADAPTER_INFO pAdapter = NULL; 
		DWORD dwRetVal = 0;   
		ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO); 
		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof (IP_ADAPTER_INFO)); 
		if (pAdapterInfo == NULL) 
		{     
			//TRACE("Error allocating memory needed to call GetAdaptersinfo\n");    
			return NULL; 
		} 
		if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
		{    
			free(pAdapterInfo);     
			pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);    
			if (pAdapterInfo == NULL)     
			{        
				//TRACE("Error allocating memory needed to call GetAdaptersinfo\n");        
				return NULL;    
			} 
		} 

		if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) 
		{
			pAdapter = pAdapterInfo;        
			if (pAdapter)     
			{
				return strSubItemValue = pAdapter->GatewayList.IpAddress.String;
			}
		}
	}
}

