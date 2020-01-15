// VorxTimer.h: interface for the CVorxTimer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VORXTIMER_H__F919A1C6_8F65_4BCE_8B14_30EF0B93491E__INCLUDED_)
#define AFX_VORXTIMER_H__F919A1C6_8F65_4BCE_8B14_30EF0B93491E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32
#include <Mmsystem.h>
#pragma comment(lib,"Winmm.lib") 
#else
#include <signal.h>
#include <time.h>
#endif
#include "MacroDefine.h"

//////////////////////////////////////////////////////////////
typedef int (*LFTIMER_PROC)(LPVOID lParam);
// 函数说明 : 线程内周期性循环执行的回调函数格式定义
// 返 回 值 : 0:阻塞 1:不阻塞 -1:退出线程
// 注意事项 : 回调函数内严禁调用UI函数
typedef void (*LFTIMER_EXIT)(LPVOID lParam);
// 函数说明 : 线程退出时执行的回调函数
// 注意事项 : 回调函数内严禁调用UI函数
//////////////////////////////////////////////////////////////

namespace vfc
{
	class CVorxTimer  
	{
	private:
		LFTIMER_PROC m_fpProc;
		LFTIMER_EXIT m_fpExit;
		void* m_lParam;
		long m_msCycle;

	public:
#ifdef WIN32
		UINT m_hTimer;
		static void CALLBACK TimerProc(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
#else
		timer_t m_hTimer;
		static void TimerProc(sigval_t v);
#endif
		
	public:
		//设置参数
		void SetParam(LFTIMER_PROC fpFunc,LPVOID lParam,int msCycle,LFTIMER_EXIT fpExit = NULL);
		//启动定时器
		BOOL StartTimer();
		BOOL Start(){return StartTimer();}
		//停止定时器
		void StopTimer();
		void Stop(){StopTimer();}
		//是否运行
		BOOL IsRuning(){return m_hTimer != NULL;}
		//构造析构函数
		CVorxTimer();
		virtual ~CVorxTimer();
		
	};
}
#endif // !defined(AFX_VORXTIMER_H__F919A1C6_8F65_4BCE_8B14_30EF0B93491E__INCLUDED_)
