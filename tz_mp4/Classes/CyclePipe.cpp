// CycleBuffer.cpp: implementation of the CCycleBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "CyclePipe.h"
#include <assert.h>
#include <string.h>
#include <time.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCyclePipe::CCyclePipe(int nSize,BOOL bOverWrite)
{
	m_pBuffer = NULL;
	m_nSize = 0;
	m_bOverWrite = false;
	m_pHead = m_pBuffer;
	m_pTail = m_pBuffer;
	m_dwLastCntTime = 0;
	if(nSize > 0)
		Initialize(nSize,bOverWrite);
}

void CCyclePipe::Initialize(int nSize,BOOL bOverWrite)
{
	if(m_pBuffer) delete[] m_pBuffer;
	m_nSize = nSize;
	m_bOverWrite = bOverWrite;
	m_pBuffer = new BYTE[nSize+1];
	m_pHead = m_pBuffer;
	m_pTail = m_pBuffer;
}

CCyclePipe::~CCyclePipe()
{
	delete [] m_pBuffer;
}

BYTE* CCyclePipe::GetNextPoint(BYTE* p,int n)
{
	//vfc::CAutoMutex l(&m_Mutex);
	BYTE* pEnd = m_pBuffer + m_nSize;
	if(p + n > pEnd)
		return m_pBuffer + (n - (pEnd - p)) - 1;
	else
		return p + n;
}

BYTE* CCyclePipe::GetPrevPoint(BYTE* p,int n)
{
	//vfc::CAutoMutex l(&m_Mutex);
	BYTE* pEnd = m_pBuffer + m_nSize;
	if(p - n < m_pBuffer)
		return pEnd - (n - (p - m_pBuffer)) + 1;
	else
		return p - n;
}

int CCyclePipe::GetDataSize(BYTE* pStart,BYTE* pEnd)
{
	//
	if(pEnd >= pStart)
		return pEnd - pStart;
	else
		return m_nSize - (pStart - pEnd) + 1;
}

int CCyclePipe::GetPayloadSize()
{
	vfc::CAutoMutex l(&m_Mutex);
	return GetDataSize(m_pHead,m_pTail);
}

void CCyclePipe::TraceDataSize()
{
	time_t t = time(NULL);
	if(t - m_dwLastCntTime > 2 || t-m_dwLastCntTime <0){
		char buff[100] = {0};
		sprintf(buff, "pipe free space:%d%%\n", (GetFreeSize()*100)/m_nSize);
		OutputDebugString(buff);
		m_dwLastCntTime = t;
	}
}
int CCyclePipe::GetFreeSize()
{
	return m_nSize - GetPayloadSize();
}

int CCyclePipe::PushPipe(BYTE* pData,int nDataLen)
{
	vfc::CAutoMutex l(&m_Mutex);	
	int nFreeSize = GetFreeSize();	
	int nLen = nDataLen;
	if(nLen>nFreeSize) return 0;
	if(m_bOverWrite){//覆盖模式
		if(m_pHead<=m_pTail){
			int nRightFreeSize = m_pBuffer+m_nSize-m_pTail;
			int nLeftFreeSize = m_pHead-m_pBuffer;
			if(nLen <= nRightFreeSize){
				memcpy(m_pTail, pData, nLen);
				m_pTail+=nLen;
			}else{
				memcpy(m_pTail, pData, nRightFreeSize);
				if(nLen-nRightFreeSize < nLeftFreeSize){
					memcpy(m_pBuffer, pData+nRightFreeSize, nLen-nRightFreeSize);
					m_pTail = m_pBuffer+nLen-nRightFreeSize;
				}else{
					memcpy(m_pBuffer, pData+nRightFreeSize, nLen-nRightFreeSize);
					m_pTail = m_pBuffer+nLen-nRightFreeSize;
					m_pHead = m_pTail;
				}
			}		
		}else{
			int nRightFreeSize = m_pBuffer+m_nSize-m_pTail;
			int nLeftFreeSize = m_pTail - m_pBuffer;
			if(nLen<=nRightFreeSize){
				memcpy(m_pTail, pData, nLen);
				m_pTail +=nLen;
				if(m_pTail>m_pHead){
					m_pHead = m_pTail;
				}
			}else{
				memcpy(m_pTail, pData, nRightFreeSize);
				memcpy(m_pBuffer, pData+nRightFreeSize, nLen-nRightFreeSize);
				m_pHead = m_pTail;
				m_pTail = m_pBuffer+(nLen-nRightFreeSize);				
			}
		}
	}else{//非覆盖模式
		nLen = min(nFreeSize, nDataLen);
		if(m_pHead <= m_pTail){
			int nRightFreeSize = m_pBuffer+m_nSize-m_pTail;
			int nLeftFreeSize = m_pHead-m_pBuffer;
			if(nLen<=nRightFreeSize){
				memcpy(m_pTail, pData, nLen);			
				m_pTail += nLen;
			}else{
				if(nLen-nRightFreeSize < nLeftFreeSize){
					memcpy(m_pTail, pData, nRightFreeSize);		
					memcpy(m_pBuffer, pData+nRightFreeSize, nLen-nRightFreeSize);			
					m_pTail = m_pBuffer + (nLen-nRightFreeSize);
				}else{
					return 0;
				}
			}
		}else{	
			if(m_pTail+nLen < m_pHead){
				memcpy(m_pTail, pData, nLen);		
				m_pTail+=nLen;
			}else{
				return 0;
			}
		}		
	}	
	return nLen;
}

//获取当前有效的数据，但不改变指针位置
int CCyclePipe::GetPipeData(BYTE* pRet,int nDataLen)
{
	vfc::CAutoMutex l(&m_Mutex);
	if(m_pTail >= m_pHead){
		if(m_pHead+nDataLen > m_pTail){ 
			return 0;
		}else{
			memcpy(pRet, m_pHead, nDataLen);
		}
	}else{
		int nLen = GetDataSize(m_pHead, m_pTail);
		if(nDataLen > nLen){
			return 0;
		}else{
			int n = m_pBuffer+m_nSize-m_pHead;
			if(nDataLen < n){
				memcpy(pRet, m_pHead, nDataLen);	
			}else{
				memcpy(pRet, m_pHead, n);
				memcpy(pRet+n, m_pBuffer, nDataLen-n);
			}
		}
	}
	return nDataLen;
	/*
	int nLen = min(nDataLen,GetDataSize(m_pHead,m_pTail));
	if(m_pTail >= m_pHead || m_pHead + nLen <= m_pBuffer + m_nSize + 1)
	{
		if(pRet) memcpy(pRet,m_pHead,nLen);
	}
	else
	{
		int nTailSize = m_pBuffer + m_nSize - m_pHead + 1;
		int nHeadSize = nLen - nTailSize;
		if(pRet)
		{
			memcpy(pRet,m_pHead,nTailSize);
			memcpy(pRet+nTailSize,m_pBuffer,nHeadSize);
		}
	}
	memset(pRet + nLen,0,nDataLen - nLen);	
	return nLen;*/
}
int CCyclePipe::PopPipe(BYTE* pRet,int nDataLen)
{
	vfc::CAutoMutex l(&m_Mutex);
	if(m_pTail >= m_pHead){
		if(m_pHead+nDataLen > m_pTail){ 
			return 0;
		}else{
			memcpy(pRet, m_pHead, nDataLen);
			m_pHead +=nDataLen;
		}
	}else{
		int nLen = GetDataSize(m_pHead, m_pTail);
		if(nDataLen > nLen){
			return 0;
		}else{
			int n = m_pBuffer+m_nSize-m_pHead;
			if(nDataLen <= n){
				memcpy(pRet, m_pHead, nDataLen);
				m_pHead += nDataLen;
			}else{
				memcpy(pRet, m_pHead, n);
				memcpy(pRet+n, m_pBuffer, nDataLen-n);
				m_pHead = m_pBuffer+nDataLen-n;
			}
		}
	}	
	return nDataLen;
	/*
	int nLen = min(nDataLen, GetDataSize(m_pHead,m_pTail));
	if(m_pTail >= m_pHead || m_pHead + nLen <= m_pBuffer + m_nSize + 1)
	{
		if(pRet) memcpy(pRet,m_pHead,nLen);
		m_pHead += nLen;
	}
	else
	{
		int nTailSize = m_pBuffer + m_nSize - m_pHead + 1;
		int nHeadSize = nLen - nTailSize;
		if(pRet)
		{
			memcpy(pRet,m_pHead,nTailSize);
			memcpy(pRet+nTailSize,m_pBuffer,nHeadSize);
		}
		m_pHead = m_pBuffer + nHeadSize;
	}
	if(pRet)
		memset(pRet + nLen,0,nDataLen - nLen);
	return nLen;
	*/
}

BYTE* CCyclePipe::SeekData(BYTE* pBegin,BYTE* pSeek,int nSeekLen)
{	
	BYTE* pRet = NULL;
	int nPayloadSize = GetDataSize(m_pHead,m_pTail);
	if(nPayloadSize<nSeekLen) return NULL;
	for(int i=0;i<nPayloadSize-nSeekLen;i++)
	{
		int j = 0;
		for(j=0;j<nSeekLen;j++)
		{
			if(*GetNextPoint(pBegin,i+j) != *(pSeek+j))
				break;
		}
		if(j == nSeekLen)
		{
			pRet = GetNextPoint(pBegin,i);
			break;
		}
	}
	return pRet;
}

//检索数据
BYTE* CCyclePipe::SeekData(BYTE* pSeek,int nLen)
{
	vfc::CAutoMutex l(&m_Mutex);
	return SeekData(m_pHead, pSeek, nLen);
}

//丢弃到...之前的数据
int CCyclePipe::ChuckData(BYTE * pData)
{
	vfc::CAutoMutex l(&m_Mutex);
	assert(pData);
	assert(IsInDataArea(pData));
	int nSize = m_pHead>=pData?(pData-m_pHead):(long)(m_pBuffer+m_nSize-pData+m_pHead);
	m_pHead = pData;
	return nSize;
}

//清除所有的数据
void CCyclePipe::ResetPipeData()
{
	vfc::CAutoMutex l(&m_Mutex);
	m_pHead = m_pTail = m_pBuffer;
	memset(m_pBuffer, 0, m_nSize);
}

BOOL  CCyclePipe::IsInDataArea(BYTE* p)
{
	if(m_pHead <= m_pTail)
		return (p >= m_pHead && p < m_pTail);	
	else
		return (p >= m_pBuffer && p < m_pTail) || (p >= m_pHead && p <= m_pBuffer + m_nSize);
}

BOOL  CCyclePipe::IsInFreeArea(BYTE* p)
{
	if(m_pHead > m_pTail)
		return (p < m_pHead && p >= m_pTail);
	else
		return (p >= m_pBuffer && p < m_pHead) || (p >= m_pTail && p <= m_pBuffer + m_nSize);
}
