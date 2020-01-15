// VorxTimer.cpp: implementation of the CVorxTimer class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "VorxTimer.h"
#ifdef WIN32
#include <mmsystem.h>
#endif
//#pragma comment(lib, "Kernel32.lib")
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace vfc
{
	CVorxTimer::CVorxTimer()
	{
		m_fpProc = NULL;
		m_fpExit = NULL;
		m_lParam = NULL;
		m_msCycle = 0;
		m_hTimer = NULL;
	}
	
	CVorxTimer::~CVorxTimer()
	{
		Stop();
	}
	
	//设置参数
	void CVorxTimer::SetParam(LFTIMER_PROC fpFunc,LPVOID lParam,int msCycle,LFTIMER_EXIT fpExit)
	{
		m_fpProc = fpFunc;
		m_fpExit = fpExit;
		m_lParam = lParam;
		m_msCycle = msCycle;
	}
	
#ifndef TIME_KILL_SYNCHRONOUS
#define TIME_KILL_SYNCHRONOUS (0x0100)
#endif
	//启动定时器
	BOOL CVorxTimer::StartTimer()
	{
		if(IsRuning())StopTimer();
#ifdef WIN32
		m_hTimer = timeSetEvent(m_msCycle,1,TimerProc,(ULONG)this,TIME_PERIODIC|TIME_KILL_SYNCHRONOUS);
#else
		struct sigevent se;   
		memset(&se,0,sizeof(se));
		se.sigev_notify = SIGEV_THREAD;
		se.sigev_notify_function = TimerProc;   
		se.sigev_value.sival_int = (long)this;

		BOOL ret = FALSE;
		if(timer_create(CLOCK_REALTIME,&se,&m_hTimer) == 0)
		{
			struct itimerspec ts,ots;
			ts.it_interval.tv_sec = m_msCycle/1000;
			ts.it_interval.tv_nsec = (m_msCycle%1000)*1000000;
			ts.it_value = ts.it_interval;
			if(timer_settime(m_hTimer,TIMER_ABSTIME,&ts,NULL/*&ots*/) == 0)
			{
				ret = TRUE;
			}
		}
		if(!ret && m_hTimer != NULL)
		{
			timer_delete(m_hTimer);
			m_hTimer = NULL;
		}
#endif
		return (m_hTimer != NULL);
	}
	
	//停止定时器
	void CVorxTimer::StopTimer()
	{
		if(IsRuning())
		{
#ifdef WIN32
			timeKillEvent(m_hTimer);
#else
			timer_delete(m_hTimer);
#endif
			m_hTimer = NULL;
			if(m_fpExit)m_fpExit(m_lParam);
		}	
	}
	
#ifdef WIN32
	void CALLBACK CVorxTimer::TimerProc(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
#else
	void CVorxTimer::TimerProc(sigval_t v)
#endif
	{
#ifdef WIN32
		CVorxTimer* pThis = (CVorxTimer*)dwUser;
#else
		CVorxTimer* pThis = (CVorxTimer*)v.sival_int;
#endif
		if(pThis->m_fpProc)
		{
			switch(pThis->m_fpProc(pThis->m_lParam))
			{
			case -1:
				pThis->StopTimer();
				break;
			default:
				break;
			}
		}
	}
}
