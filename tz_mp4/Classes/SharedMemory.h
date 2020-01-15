//////////////////////////////////////////////////////////////////////
//
// 文件 : SharedMemory.h: interface for the CSharedMemory class.
// 功能 : 进程之间共享内存及其同步操作类
// 作者 : 肖可伟
// 日期 : 2009的年9月18日
// 版权 : 北京蛙视通信技术有限责任公司
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHAREDMEMORY_H__31E8C1DC_BC12_4936_8012_9CDF704C4AF4__INCLUDED_)
#define AFX_SHAREDMEMORY_H__31E8C1DC_BC12_4936_8012_9CDF704C4AF4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MacroDefine.h"

namespace vfc
{
#ifndef WIN32
	union semun 
	{ 
		int val; 
		struct semid_ds *buf; 
		unsigned short int *array; 
		struct seminfo *__buf; 
	}; 
#endif

	class CSharedMemory  
	{
	private:
		void *m_pMemoryAddr;	//Shared Memory Address
#ifdef WIN32
		HANDLE m_hHandle;
		HANDLE m_hSemaphore;
#else
		int m_nMemoryID;
		int m_nSemID;
#endif
		
	public:
		BOOL Create(int nKey,int nLen);
		BOOL Release();
		void* GetPoint(){return m_pMemoryAddr;}
		void Lock();
		void Unlock();
		CSharedMemory();
		virtual ~CSharedMemory();
		
	};
}

#endif // !defined(AFX_SHAREDMEMORY_H__31E8C1DC_BC12_4936_8012_9CDF704C4AF4__INCLUDED_)
