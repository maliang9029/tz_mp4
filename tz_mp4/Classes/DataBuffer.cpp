// DataBuffer.cpp: implementation of the CDataBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DataBuffer.h"
#ifndef WIN32
#include <string.h>
#include <stdlib.h>
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace vfc
{
	CDataBuffer::CDataBuffer(int nBuffLen,BOOL bAutoAdd)
	{
		if(nBuffLen > 0)
			m_pData = new char[nBuffLen];
		else
			m_pData = NULL;
		m_nBuffLen = nBuffLen;
		m_nDataLen = 0;
		m_bAutoAdd = bAutoAdd;
	}
	
	CDataBuffer::~CDataBuffer()
	{
		if(m_pData)delete[] m_pData;
	}
	
	void CDataBuffer::PushData(void *pData, int nLen)
	{
		if(m_bAutoAdd)
			AssertBuffLen(m_nDataLen+nLen);
		else
			RemoveData(m_nDataLen+nLen-m_nBuffLen);
		
		m_mutex.Lock();
		if(pData && nLen>0)
		{
			memcpy(m_pData+m_nDataLen,pData,nLen);
			m_nDataLen += nLen;
		}
		m_mutex.Unlock();
	}
	
	void CDataBuffer::RemoveData(int nLen)
	{
		CAutoMutex am(&m_mutex);
		if(nLen <= 0) return;
		if(nLen >= m_nDataLen) 
		{
			m_nDataLen = 0;
			return;
		}

		int nBytes = nLen;				//单次移动数据
		int nFinish = 0;				//已完成的数据
		int nRest = m_nDataLen-nLen;	//未完成的数据
		while(nRest > 0)
		{
			if(nRest<nLen)nBytes=nRest;
			memcpy(m_pData+nFinish,m_pData+nFinish+nLen,nBytes);
			nRest -= nBytes;
			nFinish += nBytes;
		}
		m_nDataLen -= nLen;
	}
	
	void CDataBuffer::ClearData()
	{
		CAutoMutex am(&m_mutex);
		m_nDataLen = 0;
	}

	void CDataBuffer::AssertBuffLen(int nLen)
	{
		CAutoMutex am(&m_mutex);
		if(m_nBuffLen < nLen)
		{
			char* p = new char[nLen];
			if(m_nDataLen > 0)
				memcpy(p,m_pData,m_nDataLen);
			delete[] m_pData;
			m_pData = p;
			m_nBuffLen = nLen;
		}
	}

	void CDataBuffer::PrintToHexString(char* sText)
	{
		CAutoMutex am(&m_mutex);
		if(sText)
		{
			for(int i=0;i<m_nDataLen;i++)
				sprintf(sText+3*i,"%02x ",(unsigned char)m_pData[i]);
		}
	}
	
	void CDataBuffer::ScanHexString(LPCTSTR sText,BYTE* pData,int* nLen)
	{
		*nLen = 0;
		int nLoc = 0,nStrLen = strlen(sText);
		char s[3]={0,0,0};
		while(nLoc < nStrLen)
		{
			if(nLoc<nStrLen-1 && sText[nLoc]=='0' && (sText[nLoc+1]=='x'||sText[nLoc+1]=='X'))
				nLoc += 2;
			s[0] = sText[nLoc];
			nLoc++;
			s[1] = nLoc<nStrLen?sText[nLoc]:0;
			nLoc++;
			while(nLoc<nStrLen && sText[nLoc]==' ')
				nLoc++;
			pData[*nLen] = strtol(s,NULL,16);
			(*nLen)++;
		}
	}
	
	void CDataBuffer::PushFromHexString(const char* sText)
	{
		CAutoMutex am(&m_mutex);
		ScanHexString(sText,(BYTE*)m_pData,&m_nDataLen);
	}

	//清除所有值
	void CDataBuffer::SetAllBits(int val)
	{
		CAutoMutex am(&m_mutex);
		if(m_nBuffLen > 0)
			memset(m_pData,val,m_nBuffLen);
	}

	//设置比特值
	bool CDataBuffer::SetBitVal(int nByte,int nBit,int val)
	{
		CAutoMutex am(&m_mutex);
		if(nByte<m_nBuffLen && nBit<8)
		{
			if(val)
				*(m_pData+nByte) |= (1<<nBit);
			else
				*(m_pData+nByte) &= ~(1<<nBit);
			return true;
		}
		return false;
	}

	//获取比特值
	int CDataBuffer::GetBitVal(int nByte,int nBit)
	{
		CAutoMutex am(&m_mutex);
		if(nByte<m_nBuffLen && nBit<8)
		{
			return *(m_pData+nByte) & (1<<nBit);
		}
		return 0;
	}

	//数据检索
	void* CDataBuffer::SearchData(void* pData,int nLen,void* pFrom,BOOL bLock)
	{
		if(bLock)m_mutex.Lock();
		void* pRet = NULL;
		if(!pFrom) pFrom = m_pData;
		if(pFrom >= m_pData && pFrom <= m_pData+m_nDataLen-1)
		{
			for(char* p=(char*)pFrom;pRet && p<=m_pData+m_nDataLen-nLen;p++)
			{
				if(memcmp(p,pData,nLen) == 0)
				{
					pRet = p;
				}
			}
		}
		if(bLock)m_mutex.Unlock();
		return pRet;
	}

	BOOL CDataBuffer::IsInData(void* p)
	{
		return (p>=m_pData && p<m_pData+m_nDataLen);
	}

	////////////////////////////////////////////////////////////////////
	//
	// Class CDataParser Member Functions
	//
	////////////////////////////////////////////////////////////////////

	CDataParser::CDataParser(LPCTSTR sData,char flag)
	{
		int nLen = strlen(sData);
		m_pData = new char[nLen+1];
		strcpy(m_pData,sData);

		m_aSubList.push_back(0);
		for(int i=0;i<nLen;i++)
		{
			if(m_pData[i] == flag)
			{
				m_pData[i] = 0;
				m_aSubList.push_back(i+1);
			}
		}
	}

	CDataParser::~CDataParser()
	{
		if(m_pData)delete[] m_pData;
	}

	char* CDataParser::sub(int nIndex)
	{
		if(nIndex >= 0 && nIndex < m_aSubList.size())
			return &m_pData[m_aSubList[nIndex]];
		else return NULL;
	}
}
