//////////////////////////////////////////////////////////////////////
// ServerSocket.h: interface for the CTcpServer class.
//
// 用SELECT模型实现的串口操作服务器类
// 作    者: 肖可伟
// 日    期: 2005年6月
// 最后修改: 2009年5月4日
// 版权所有: 北京蛙视通信技术有限公司
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMPORT_H__29647570_835A_493D_AC32_23AB4E18C98E__INCLUDED_)
#define AFX_COMPORT_H__29647570_835A_493D_AC32_23AB4E18C98E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MacroDefine.h"
#include "MutexInteger.h"

typedef void (*DATA_RECV)(void* pData,int nLen,void* lParam);

#ifdef WIN32
#define HCOM HANDLE
#else
#define HCOM int
#define INVALID_HANDLE_VALUE -1
#endif

namespace vfc
{
	class CComPort
	{
	private:
		HCOM   m_hCom;					//串口读写句柄
		DATA_RECV m_fnOnDataRecv;		//读数回调指针
		void* m_lParam;					//读数回调参数
		HTHREAD	m_hThread;				//数据线程句柄
		CMutexInteger m_bThreadRun;		//控制线程状态
		static THREAD_RETURN_TYPE read_thread(void* lParam);	//读数据线程

	public:
		void SetDataCallback(DATA_RECV fpDataRecv,void* lParam);
		BOOL Open(int nCom,int nBaudRate,int nDataBits,int nStopBits,int nParity,LPCTSTR sPrefix = NULL);
		void Close();
		BOOL SetParam(int nBaudRate,int nDataBits,int nStopBits,int nParity);
		BOOL WriteData(void* pData,int nLen);
		CComPort();
		virtual ~CComPort();
		
	};
}

#endif // !defined(AFX_COMPORT_H__29647570_835A_493D_AC32_23AB4E18C98E__INCLUDED_)
