#include "VorxThread.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace vfc
{
CVorxThread::CVorxThread()
{
	//线程控制参数
	m_hThread = 0;
	m_bThread = FALSE;
	m_bSuspend = FALSE;
	//线程对外参数
	m_lParam = 0;
	m_fun = 0;
	m_nCycle = 0;
	m_exit = 0;
	//
	m_nStackSize = 0;
}

CVorxThread::~CVorxThread()
{
	StopThread();
}

BOOL CVorxThread::SetParam(LPFUN_THREAD fun,LPVOID lParam,int nCycle,LPTHREAD_EXIT fpExit)
{
	if(m_bThread) return FALSE;
	m_fun = fun;
	m_exit = fpExit;
	m_lParam = lParam;
	m_nCycle = nCycle;
	return TRUE;
}

BOOL CVorxThread::StartThread()
{
	if(m_bThread || !m_fun) return FALSE;
	m_bThread = TRUE;
#ifdef WIN32
	m_hThread = (HANDLE)_beginthread(thread_func,m_nStackSize,this);
#else
	pthread_create(&m_hThread,NULL,thread_func,this);
#endif
	return (m_hThread != 0);
}

void CVorxThread::StopThread()
{
	if(m_bThread)
	{
		m_bThread = FALSE;
		//激发事件,通知线程结束
		m_oEvent.Signal();
		//等待线程返回
		if(m_hThread)
		{
#ifdef WIN32
			if(WaitForSingleObject(m_hThread,3000) == WAIT_TIMEOUT)
			{
				DWORD dwExitCode;
				GetExitCodeThread(m_hThread,&dwExitCode);
				TerminateThread(m_hThread,dwExitCode);
			}
#else
			pthread_join(m_hThread,NULL);
#endif
		}
	}
}

void CVorxThread::Suspend()
{
	m_bSuspend = TRUE;
}

void CVorxThread::Resume()
{
	m_bSuspend = FALSE;
}

BOOL CVorxThread::SetPriority(int val)
{
	if(val < -3 || val > 3) return FALSE;
#ifdef WIN32
	int nVal = 0;
	switch(val)
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
	hThreadPrior.sched_priority = val;
	return (pthread_setschedparam(m_hThread,policy,&hThreadPrior) == 0);
#endif
}

void CVorxThread::SetStackSize(unsigned int nSize)
{
	m_nStackSize = nSize;
}

THREAD_RETURN CVorxThread::thread_func(void* lParam)
{
	CVorxThread* pThis = (CVorxThread*)lParam;

// 	UINT64 tTime;			// 不会越界
// 	int nSleepTime;			//所需要休眠的毫秒数
// 	int nAmendment = 0;		//修正上次不够毫秒数
	int nExecRes = 0;
	while(pThis->m_bThread)
	{
		if (pThis->m_bSuspend) // 暂停
		{
			Sleep(pThis->m_nCycle);
			continue;
		}

		//执行函数
// 		tTime = VGAPI_GetMillisecond();
		if(pThis->m_fun)
		{
			nExecRes = pThis->m_fun(pThis->m_lParam);
		}
		if(!pThis->m_bThread)break;

		switch(nExecRes)
		{
		case 1://下一循环
			break;

		case 0://实现休眠,修正校时可能会产品的误差
			Sleep(pThis->m_nCycle);
/*
			if (3600000 >= VGAPI_GetMillisecond()-tTime) // 一小时以内的值有效
			{
				nSleepTime = pThis->m_nCycle - (int)(VGAPI_GetMillisecond()-tTime) + nAmendment;
				if(nSleepTime > 0)
				{
					pThis->m_oEvent.WaitForSignal(nSleepTime);
				}
				nAmendment = pThis->m_nCycle - (int)(VGAPI_GetMillisecond()-tTime);
				if (-3600000 >= nAmendment) // 负修正时间一小时以内有效
				{
					nAmendment = 0;
				}
			}
			else
			{
				nAmendment = 0;
			}
*/
			break;

		case -1: //退出线程
			pThis->m_bThread = false;
			pThis->m_hThread = 0;
		}
	}
	if(pThis->m_exit)
	{
		pThis->m_exit(pThis->m_lParam);
	}
	return THREAD_VALUE;
}

BOOL CVorxThread::WaitTime(DWORD dwTime)
{
	if(m_oEvent.WaitForSignal(dwTime))
	{
		return FALSE; // 接收到退出事件
	}
	return TRUE; // 等待了dwtime时间
}
}

