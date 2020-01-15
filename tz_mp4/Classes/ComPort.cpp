// ComPort.cpp: implementation of the CComPort class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ComPort.h"
#include <stdio.h>
#include <fcntl.h>

#ifdef WIN32
#include <io.h>
#else
#include <stdio.h>   
#include <string.h>   
#include <unistd.h>   
#include <errno.h>   
#include <termios.h>   
#include <sys/time.h> 
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace vfc
{
	CComPort::CComPort()
	{
		m_hCom = INVALID_HANDLE_VALUE;
		m_fnOnDataRecv = NULL;
		m_lParam = NULL;
		m_hThread = THREAD_HANDLE_NULL;
		m_bThreadRun = FALSE;
	}
	
	CComPort::~CComPort()
	{
		Close();
	}

	void CComPort::SetDataCallback(DATA_RECV fpDataRecv,void* lParam)
	{
		m_fnOnDataRecv = fpDataRecv;
		m_lParam = lParam;
	}
	
	BOOL CComPort::Open(int nCom,int nBaudRate,int nDataBits,int nStopBits,int nParity,LPCTSTR sPrefix)
	{
		if(m_hCom != INVALID_HANDLE_VALUE) Close();
		char sDev[100] = {0};
#ifdef WIN32
		sprintf(sDev,"COM%d",nCom);
		m_hCom = CreateFile(sDev,GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,0);
#else
		if(sPrefix && strlen(sPrefix)>0)sprintf(sDev,"%s%d",sPrefix,nCom);
		else sprintf(sDev,"/dev/ttyS%d",nCom);
		m_hCom = open(sDev,O_RDWR|O_NOCTTY);
#endif
		if(m_hCom == INVALID_HANDLE_VALUE)
		{
#ifdef VBON_DEBUG
			printf("open com port : %s failed!\n",sDev);
#endif
			return FALSE;
		}
		if(!SetParam(nBaudRate,nDataBits,nStopBits,nParity))
		{
			printf("Set com parameter failed!\n");
			return FALSE;
		}
		m_bThreadRun = TRUE;
		CREATE_THREAD(m_hThread,read_thread);
		return TRUE;
	}

	void CComPort::Close()
	{
		if(m_hCom != INVALID_HANDLE_VALUE)
		{
			m_bThreadRun = FALSE;
			if(m_hThread != THREAD_HANDLE_NULL)
			{
				WAIT_FOR_THREAD(m_hThread);
				m_hThread = THREAD_HANDLE_NULL;
			}
#ifdef WIN32
			CloseHandle(m_hCom);
#else
			close(m_hCom);
#endif
			m_hCom = INVALID_HANDLE_VALUE;
		}
	}

	BOOL CComPort::WriteData(void* pData,int nLen)
	{
		int nWrited = 0,nTryTimes = 0;
		while(nWrited < nLen && nTryTimes++ < 5)
		{
#ifdef WIN32
			OVERLAPPED ol;
			ZeroMemory(&ol,sizeof(ol));
			ol.hEvent = CreateEvent(0,TRUE,FALSE,0);
			ULONG nBytes = 0;
			BOOL ret = FALSE;
			if(WriteFile(m_hCom,(char*)pData+nWrited,nLen-nWrited,&nBytes,&ol))
			{
				nWrited += nBytes;
			}
			else if(GetLastError() == ERROR_IO_PENDING)
			{
				ret = GetOverlappedResult(m_hCom,&ol,&nBytes,TRUE);
				if(ret && nBytes > 0) nWrited += nBytes;
			}
#else
			nWrited += write(m_hCom,(char*)pData+nWrited,nLen-nWrited);
#endif
		}
		return (nLen == nWrited);
	}
	
	BOOL CComPort::SetParam(int nBaudRate,int nDataBits,int nStopBits,int nParity)
	{
#ifdef WIN32
		//1.设定串口超时
		COMMTIMEOUTS to;
		memset(&to,0,sizeof(COMMTIMEOUTS));
		to.ReadIntervalTimeout = 20;
		to.ReadTotalTimeoutMultiplier = 20;
		to.ReadTotalTimeoutConstant = 500;
		to.WriteTotalTimeoutMultiplier = 20;
		to.WriteTotalTimeoutConstant = 100;
		SetCommTimeouts(m_hCom, &to);
		//2.设定串口参数
		DCB dcb;
		if(!GetCommState(m_hCom,&dcb))return FALSE;
		if(nBaudRate>0)dcb.BaudRate = nBaudRate;
		if(nDataBits>0)dcb.ByteSize = nDataBits;

		if(nStopBits == 2)dcb.StopBits = TWOSTOPBITS;
		else dcb.StopBits = ONESTOPBIT;

		if(nParity>=0)dcb.Parity = nParity;

		if(!SetCommState(m_hCom,&dcb))return FALSE;
#else
		struct termios options; 
		memset(&options, 0, sizeof(options));
		cfmakeraw(&options);

		int speed_arr[] = {B115200,B57600,B38400,B115200,B19200,B9600,B4800,B2400,B1800,B1200,B600,B300,B200,B150,B134,B110,B75,B50,B0};
		int name_arr[] = {  115200, 57600, 38400, 115200, 19200, 9600, 4800, 2400, 1800, 1200, 600, 300, 200, 150, 134, 110, 75, 50, 0};
		for(int i=0;i<sizeof(speed_arr)/sizeof(int);i++)
		{
			if(nBaudRate == name_arr[i])
			{
				cfsetispeed(&options, speed_arr[i]);
				cfsetospeed(&options, speed_arr[i]);
			}
		}
		options.c_cflag |= CLOCAL | CREAD;      /* | CRTSCTS */
		options.c_oflag  &= ~OPOST;   /*Output*/
		options.c_cflag &= ~CSIZE;
		switch (nDataBits) /*setdatabits*/
		{
		case 5:
			options.c_cflag |= CS5;
			break;
		case 6:
			options.c_cflag |= CS6;
			break;
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			printf("Unsupported data size : %d\n",nDataBits);
			return FALSE;
		}
		
		switch (nParity) /*setParity*/
		{
		case 0:
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */
			break;
		case 1:
			options.c_cflag |= PARENB;
			options.c_cflag |= PARODD;   
			//m_Options.c_iflag |= INPCK;             /* Disnable parity checking */
			break;
		case 2:
			options.c_cflag |= PARENB;     /* Enable parity */
			options.c_cflag &= ~PARODD;     
			//m_Options.c_iflag |= INPCK;       /* Disnable parity checking */
			break;
		default:
			perror("Unsupported parity\n");
			return FALSE;
		}

		switch (nStopBits) /*setstopbits*/
		{
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			perror("Unsupported stop bits\n");
			return FALSE;
		}
		/* Set input parity option */
		if (nParity != 0) options.c_iflag |= INPCK;
		
		options.c_cc[VTIME] = 1; // 0.1 seconds
		options.c_cc[VMIN] = 1;
		
		tcflush(m_hCom,TCIOFLUSH); /* Update the m_Options and do it NOW */
		if (tcsetattr(m_hCom,TCSANOW,&options) != 0)
		{
			perror("setSerialPort() tcsetattr -- error\n");
			return FALSE;
		}
#endif
		return TRUE;
	}

	THREAD_RETURN_TYPE CComPort::read_thread(void* lParam)
	{
		CComPort* pThis = (CComPort*)lParam;
		char sBuff[1024];
#ifdef WIN32
		OVERLAPPED ol;
		ZeroMemory(&ol,sizeof(ol));
		ol.hEvent = CreateEvent(0,TRUE,FALSE,0);
		ULONG nBytes = 0;
		BOOL ret = FALSE;
		while(pThis->m_bThreadRun)
		{
			if(ReadFile(pThis->m_hCom,sBuff,1024,&nBytes,&ol))
			{
				if(nBytes > 0 && pThis->m_fnOnDataRecv)
				{
					pThis->m_fnOnDataRecv(sBuff,nBytes,pThis->m_lParam);
				}
			}
			else if(GetLastError() == ERROR_IO_PENDING)
			{
				if(WaitForSingleObject(ol.hEvent,500) == WAIT_OBJECT_0)
				{
					ret = GetOverlappedResult(pThis->m_hCom,&ol,&nBytes,FALSE);
					if(ret && nBytes>0 && pThis->m_fnOnDataRecv)
					{
						pThis->m_fnOnDataRecv(sBuff,nBytes,pThis->m_lParam);
					}
				}
			}
		}
#else
		struct timeval tv;
		fd_set	fdset;
		while(pThis->m_bThreadRun)
		{
			tv.tv_sec = 0;		
			tv.tv_usec = 100;	//超时设为100毫秒
			FD_ZERO(&fdset);
			FD_SET(pThis->m_hCom,&fdset);
			if(select(pThis->m_hCom+1,&fdset,NULL,NULL,&tv) > 0)
			{
				int nBytes = read(pThis->m_hCom,sBuff,1024);
				if(nBytes > 0 && pThis->m_fnOnDataRecv)
				{
					pThis->m_fnOnDataRecv(sBuff,nBytes,pThis->m_lParam);
				}
			}
		}
#endif			
		return TRHEAD_RETURN_VALUE;
	}
}
