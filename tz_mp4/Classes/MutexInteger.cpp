// MutexObject.cpp: implementation of the CMutexInteger class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MutexInteger.h"
#ifndef WIN32
#include <sys/time.h>
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace vfc
{
	//////////////////////////////////////////////////////////////////////
	// CVorxEvent
	//////////////////////////////////////////////////////////////////////
	CVorxEvent::CVorxEvent()
	{
#ifndef USE_CHECK_EVENT
#ifdef WIN32 
		m_hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
#else // defined WIN32
		pthread_cond_init(&m_hEvent,NULL);
#ifdef VORX_SRV // 高性能linux
		pthread_mutexattr_init(&attr); 
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&m_mutex, &attr);
#else // 低性能linux
		pthread_mutex_init(&m_mutex, NULL);
#endif // defined VORX_SRV
#endif // defined WIN32
#else // defined USE_CHECK_EVENT
		m_mutex.Lock();
		m_bSignal = FALSE;
		m_mutex.Unlock();
#endif // defined USE_CHECK_EVENT
	}
	
	CVorxEvent::CVorxEvent(const CVorxEvent &ref)
	{
#ifdef WIN32 
		m_hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
#else // defined WIN32
		pthread_cond_init(&m_hEvent,NULL);
#ifdef VORX_SRV // 高性能linux
		pthread_mutexattr_init(&attr); 
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&m_mutex, &attr);
#else // 低性能linux
		pthread_mutex_init(&m_mutex, NULL);
#endif // defined VORX_SRV
#endif // defined WIN32
	}

	CVorxEvent::~CVorxEvent()
	{
#ifndef USE_CHECK_EVENT

#ifdef WIN32 
		CloseHandle(m_hEvent);
#else
		pthread_cond_destroy(&m_hEvent);
		pthread_mutex_destroy(&m_mutex);
#endif

#else
		m_mutex.Lock();
		m_bSignal = TRUE;
		m_mutex.Unlock();
#endif
	}
	
	BOOL CVorxEvent::WaitForSignal(int nTimeOut)
	{
#ifndef USE_CHECK_EVENT

#ifdef WIN32
		BOOL bRet = (WaitForSingleObject(m_hEvent,nTimeOut) == WAIT_OBJECT_0);
		ResetEvent(m_hEvent);
		return bRet;
#else
		int ret = 0;
		if (INFINITE == nTimeOut)
		{
			pthread_mutex_lock(&m_mutex);
			ret = pthread_cond_wait(&m_hEvent, &m_mutex);
			pthread_mutex_unlock(&m_mutex);
		}
		else
		{
			struct timeval newtime;
			gettimeofday(&newtime,NULL);
			
			struct timespec timeout;
			timeout.tv_sec = newtime.tv_sec+(nTimeOut/1000);
			timeout.tv_nsec = (newtime.tv_usec+(nTimeOut%1000)*1000)*1000;
			if(timeout.tv_nsec > 999999999)
			{
				timeout.tv_sec += 1;
				timeout.tv_nsec -= 1000000000;
			}
			
			pthread_mutex_lock(&m_mutex);
			ret = pthread_cond_timedwait(&m_hEvent, &m_mutex, &timeout);
			pthread_mutex_unlock(&m_mutex);
		}
		
		return (ret == 0);
#endif

#else
		BOOL ret = FALSE;
		DWORD nTime = GetTickCount()+nTimeOut;
		while(!ret && (nTimeOut==0 || GetTickCount()<nTime))
		{
			m_mutex.Lock();
			if(m_bSignal)
			{
				ret = TRUE;
				m_bSignal = FALSE;
			}
			m_mutex.Unlock();
			if(!ret)M_SLEEP(1);
		}
		return ret;
#endif
	}
	
	void CVorxEvent::Signal()
	{
#ifndef USE_CHECK_EVENT
#ifdef WIN32
		SetEvent(m_hEvent);
#else
		pthread_mutex_lock(&m_mutex);
		pthread_cond_signal(&m_hEvent);
		pthread_mutex_unlock(&m_mutex);
#endif
#else
		m_mutex.Lock();
		m_bSignal = TRUE;
		m_mutex.Unlock();
#endif
	}

	void CVorxEvent::Unsignal()
	{
#ifndef USE_CHECK_EVENT
#ifdef WIN32
		ResetEvent(m_hEvent);
#else // defined WIN32
		pthread_mutex_lock(&m_mutex);
		pthread_cond_destroy(&m_hEvent);
		pthread_cond_init(&m_hEvent, NULL);
		pthread_mutex_unlock(&m_mutex);
#endif // defined WIN32
#else // !defined USE_CHECK_EVENT
		m_mutex.Lock();
		m_bSignal = FALSE;
		m_mutex.Unlock();
#endif // !defined USE_CHECK_EVENT
	}
	
	//////////////////////////////////////////////////////////////////////
	// CMutexInteger
	//////////////////////////////////////////////////////////////////////
	
	CMutexInteger::CMutexInteger()
	{
		m_nValue = 0;
	}
	
	CMutexInteger::CMutexInteger(const CMutexInteger &ref)
	{
		m_nValue = ref.m_nValue;
	}
	
	CMutexInteger::CMutexInteger(const int nVal)
	{
		m_nValue = nVal;
	}
	
	CMutexInteger::~CMutexInteger()
	{
	}

	int CMutexInteger::GetAndSetValue(int nVal)
	{
		CAutoMutex l(&m_mutex);
		int ret = m_nValue;
		m_nValue = nVal;
		return ret;
	}
	
	void CMutexInteger::operator&=(int nParam)
	{
		CAutoMutex l(&m_mutex);
		m_nValue &= nParam;
	}
	
	void CMutexInteger::operator|=(int nParam)
	{
		CAutoMutex l(&m_mutex);
		m_nValue |= nParam;
	}
	
	int  CMutexInteger::operator&(int nParam)
	{
		CAutoMutex l(&m_mutex);
		return (m_nValue & nParam);
	}
	
	int  CMutexInteger::operator|(int nParam)
	{
		CAutoMutex l(&m_mutex);
		return (m_nValue | nParam);
	}
	
	const CMutexInteger& CMutexInteger::operator=(int nParam)
	{
		CAutoMutex l(&m_mutex);
		m_nValue = nParam;
		return *this;
	}
	
	bool CMutexInteger::operator==(int nParam)
	{
		CAutoMutex l(&m_mutex);
		return (m_nValue == nParam);
	}
	
	void CMutexInteger::operator+=(int nParam)
	{
		CAutoMutex l(&m_mutex);
		m_nValue += nParam;
	}
	
	int  CMutexInteger::operator+(int nParam)
	{
		CAutoMutex l(&m_mutex);
		return (m_nValue + nParam);
	}
	
	void CMutexInteger::operator-=(int nParam)
	{
		CAutoMutex l(&m_mutex);
		m_nValue -= nParam;
	}
	
	int  CMutexInteger::operator-(int nParam)
	{
		CAutoMutex l(&m_mutex);
		return (m_nValue - nParam);
	}
	
	bool CMutexInteger::operator!()
	{
		CAutoMutex l(&m_mutex);
		return !m_nValue;
	}
	
	bool CMutexInteger::operator>(int nParam)
	{
		CAutoMutex l(&m_mutex);
		return (m_nValue > nParam);
	}
	
	bool CMutexInteger::operator>=(int nParam)
	{
		CAutoMutex l(&m_mutex);
		return (m_nValue >= nParam);
	}
	
	bool CMutexInteger::operator<(int nParam)
	{
		CAutoMutex l(&m_mutex);
		return (m_nValue < nParam);
	}
	
	bool CMutexInteger::operator<=(int nParam)
	{
		CAutoMutex l(&m_mutex);
		return (m_nValue <= nParam);
	}
	
	bool CMutexInteger::operator!=(int nParam)
	{
		CAutoMutex l(&m_mutex);
		return (m_nValue != nParam);
	}
	
	void CMutexInteger::operator++()
	{
		CAutoMutex l(&m_mutex);
		m_nValue++;
	}
	
	void CMutexInteger::operator--()
	{
		CAutoMutex l(&m_mutex);
		m_nValue--;
	}
	
	CMutexInteger::operator int()
	{
		CAutoMutex l(&m_mutex);
		return m_nValue;
	}
	
	const CMutexInteger& CMutexInteger::operator=(const CMutexInteger& nParam)
	{
		CAutoMutex l(&m_mutex);
		m_nValue = nParam.m_nValue;
		return *this;
	}
}
