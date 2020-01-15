// DataBuffer.h: interface for the CDataBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATABUFFER_H__739488FD_3063_4869_9B2B_D0AE3530829E__INCLUDED_)
#define AFX_DATABUFFER_H__739488FD_3063_4869_9B2B_D0AE3530829E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MutexInteger.h"

namespace vfc
{
	class CDataBuffer  
	{
	private:
		char *m_pData;			//数据指针
		int  m_nBuffLen;		//内存长度
		int  m_nDataLen;		//数据长度
		CVorxMutex m_mutex;		//互斥量
		BOOL m_bAutoAdd;
		void ScanHexString(LPCTSTR sText,BYTE* pData,int* nLen);
		
	public:
		//追加数据
		void PushData(void* pData,int nLen);
		//移除数据
		void RemoveData(int nLen);
		//清空数据
		void ClearData();
		//申请内存
		void AssertBuffLen(int nLen);
		//把数据打印成十六进制文本
		void PrintToHexString(char* sText);
		//把十六进制文本转换成数据
		void PushFromHexString(const char* sText);

		//取得数据指针
		void* DataPtr(){return m_pData;}
		//取得数据长度
		int DataLen(){return m_nDataLen;}
		//锁定数据
		void Lock(){m_mutex.Lock();}
		//解锁数据
		void Unlock(){m_mutex.Unlock();}

		//清除所有值
		void SetAllBits(int val);
		//设置比特值,nByte和nBit从0开始
		bool SetBitVal(int nByte,int nBit,int val);
		//获取比特值,nByte和nBit从0开始
		int GetBitVal(int nByte,int nBit);

		//查询数据
		void* SearchData(void* pData,int nLen,void* pFrom = NULL,BOOL bLock = TRUE);
		BOOL IsInData(void* p);

		//构造析构函数
		CDataBuffer(int nBuffLen = 1024,BOOL bAutoMaloc = TRUE);
		virtual ~CDataBuffer();
		
	};

	class CDataParser
	{
	private:
		char* m_pData;
		vector<int> m_aSubList;

	public:
		char* sub(int nIndex);
		int GetSubNum(){return m_aSubList.size();}
		CDataParser(LPCTSTR sData,char flag);
		~CDataParser();
	};
}

#endif // !defined(AFX_DATABUFFER_H__739488FD_3063_4869_9B2B_D0AE3530829E__INCLUDED_)
