// SharedMemory.cpp: implementation of the CSharedMemory class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SharedMemory.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#ifndef WIN32
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#endif

namespace vfc
{
	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////
	
	CSharedMemory::CSharedMemory()
	{
		m_pMemoryAddr = NULL;
#ifdef WIN32
		m_hHandle = NULL;
		m_hSemaphore = NULL;
#else
		m_nMemoryID = -1;
		m_nSemID = -1;
#endif
	}
	
	CSharedMemory::~CSharedMemory()
	{
		Release();
	}
	
	BOOL CSharedMemory::Create(int nKey,int nLen)
	{
#ifdef WIN32
		char sName[30];
		sprintf(sName,"VORXSMKEY%08d",nKey);
		m_hHandle = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,nLen,sName);
		if(!m_hHandle)return FALSE;
		m_pMemoryAddr = MapViewOfFile(m_hHandle,FILE_MAP_ALL_ACCESS,0,0,0);
		if(!m_pMemoryAddr)return FALSE;
		
		strcat(sName,"lk");
		m_hSemaphore = CreateSemaphore(NULL,1,1,sName);
		if(!m_hSemaphore)return FALSE;
#else//Linux
		m_nMemoryID = shmget(nKey,nLen,IPC_CREAT);
		if(m_nMemoryID == -1)return FALSE;
		m_pMemoryAddr = shmat(m_nMemoryID,NULL,0);
		
		m_nSemID = semget((key_t)nKey,1,IPC_CREAT);
		if(m_nSemID == -1)return FALSE;

		//…Ë÷√≥ı º◊¥Ã¨
		union semun semopts;
		semopts.val=0;
		semctl(m_nSemID,0,SETVAL,semopts);
#endif
		return TRUE;
	}
	
	BOOL CSharedMemory::Release()
	{
#ifdef WIN32
		if(m_hHandle)
		{
			UnmapViewOfFile(m_hHandle);
			CloseHandle(m_hHandle);
			m_hHandle = NULL;
		}
		if(m_hSemaphore)
		{
			CloseHandle(m_hSemaphore);
			m_hSemaphore = NULL;
		}
#else
		if(m_nMemoryID != -1)
		{
			shmdt(m_pMemoryAddr);
			m_nMemoryID = -1;
		}
		if(m_nSemID != -1)
		{
			semctl(m_nSemID,0,IPC_RMID,NULL);
			m_nSemID = -1;
		}
#endif
		return TRUE;
	}
	
	void CSharedMemory::Lock()
	{
#ifdef WIN32
		WaitForSingleObject(m_hSemaphore,INFINITE);
#else
		struct sembuf op[2];
		op[0].sem_num = 0;
		op[1].sem_num = 0;
		op[0].sem_op = 0;
		op[1].sem_op = 1;
		op[0].sem_flg = 0;
		op[1].sem_flg = 0;
		semop(m_nSemID,op,2);
#endif
	}
	
	void CSharedMemory::Unlock()
	{
#ifdef WIN32
		ReleaseSemaphore(m_hSemaphore,1,NULL);
#else
		struct sembuf op;
		op.sem_num = 0;
		op.sem_op = -1;
		op.sem_flg = 0;
		semop(m_nSemID,&op,1);
#endif
	}
}
