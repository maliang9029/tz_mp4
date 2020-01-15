//////////////////////////////////////////////////////////////////////
//
// BufferQueue.h: interface for the CNodeQueue class.
// 循环队列类CNodeQueue头文件,使用前先用Intialize()函数初始化
// 使用:GetBuffer() -> Enqueue(),Dequeue() -> Recycle()
// 注意:Enqueue()和Recycle()参数必须为GetBuffer()和Dequeue()得来
// 作者:肖可伟
// 日期:2006年4月11日(2007年8月15日修改)
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUFFERQUEUE_H__6BC6E26E_AE9C_4F50_98F7_2DB189057F33__INCLUDED_)
#define AFX_BUFFERQUEUE_H__6BC6E26E_AE9C_4F50_98F7_2DB189057F33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MacroDefine.h"
#include "MutexInteger.h"

#include <queue>
using namespace std;

namespace vfc
{
template <typename T>
class CNodeQueue
{
public:
	//初始化函数
	BOOL Initialize(int nQueueSize,BOOL bOverWrite);
	//销毁函数
	void Destroy();
	//数据节点入列函数
	void Enqueue(T* pBuffer);
	//数据节点出列函数
	T* Dequeue();
	//申请内存节点函数
	T* GetBuffer(BOOL* bOverWrited = NULL);
	//回收内存节点函数
	void Recycle(T* pBuffer);
	//回收所有节点
	void ResetQueue();
	// 空闲节点个数
	int FreeSize();
	// 使用结点个数
	int DataSize();

public:
	//构造析构函数
	CNodeQueue()
	{
		m_bOverWrite = FALSE;
	}
	virtual ~CNodeQueue()
	{
		Destroy();
	}

private:
	CMutexInteger m_bOverWrite;		//是否覆盖
	CVorxMutex m_mutex;

	//队列
	queue<T*> m_aFreeArray;//空闲队列
	queue<T*> m_aDataArray;//数据队列

};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//nQueueSize 队列中节点个数
//bOverWrite 是否覆盖,即当队列存满时,是否覆盖原有数据
template <typename T>
BOOL CNodeQueue<T>::Initialize(int nQueueSize,BOOL bOverWrite)
{
	m_bOverWrite = bOverWrite;

	Destroy();
	{
		CAutoMutex am(&m_mutex);
		for(int i=0;i<nQueueSize;i++)
		{
			m_aFreeArray.push(new T);
		}
	}
	return TRUE;
}

template <typename T>
void CNodeQueue<T>::ResetQueue()
{
	T* p = NULL;
	while((p = Dequeue()))
	{
		Recycle(p);
	}
}

template <typename T>
void CNodeQueue<T>::Destroy()
{
	CAutoMutex am(&m_mutex);
	while(!m_aDataArray.empty())
	{
		T* p = m_aDataArray.front();
		delete p;
		m_aDataArray.pop();
	}

	while(!m_aFreeArray.empty())
	{
		T* p = m_aFreeArray.front();
		delete p;
		m_aFreeArray.pop();
	}
}

template <typename T>
void CNodeQueue<T>::Enqueue(T* pBuffer)
{
	CAutoMutex am(&m_mutex);
	if(pBuffer)m_aDataArray.push(pBuffer);
}

template <typename T>
T* CNodeQueue<T>::Dequeue()
{
	CAutoMutex am(&m_mutex);
	T* pRet = NULL;
	if(!m_aDataArray.empty())
	{
		pRet = m_aDataArray.front();
		m_aDataArray.pop();
	}
	return pRet;
}

template <typename T>
T* CNodeQueue<T>::GetBuffer(BOOL* bOverWrited)
{
	CAutoMutex am(&m_mutex);
	T* pRet = NULL;
	if(!m_aFreeArray.empty())
	{
		if(bOverWrited)*bOverWrited = FALSE;
		pRet = m_aFreeArray.front();
		m_aFreeArray.pop();
	}
	else if(m_bOverWrite && !m_aDataArray.empty())
	{
		if(bOverWrited) *bOverWrited = TRUE;
		pRet = m_aDataArray.front();
		m_aDataArray.pop();
	}
	return pRet;
}

template <typename T>
void CNodeQueue<T>::Recycle(T* pBuffer)
{
	CAutoMutex am(&m_mutex);
	if(pBuffer)m_aFreeArray.push(pBuffer);
}

template <typename T>
int CNodeQueue<T>::FreeSize()
{
	CAutoMutex am(&m_mutex);
	return m_aFreeArray.size();
}

template <typename T>
int CNodeQueue<T>::DataSize()
{
	CAutoMutex am(&m_mutex);
	return m_aDataArray.size();
}

}
#endif // !defined(AFX_BUFFERQUEUE_H__6BC6E26E_AE9C_4F50_98F7_2DB189057F33__INCLUDED_)
