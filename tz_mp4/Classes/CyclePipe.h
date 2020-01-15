//////////////////////////////////////////////////////////////////////
//
// CycleBuffer.h: interface for the CCycleBuffer class.
// 循环管道工具类,适用于串口数据/音视频等多媒体数据处理
// 作者:肖可伟
// 日期:2007年3月
// 修改:2007年9月
// 版权:北京蛙视通信技术有限责任公司
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CYCLEBUFFER_H__527CF46D_4518_4BC3_B169_C7F9B7AE0330__INCLUDED_)
#define AFX_CYCLEBUFFER_H__527CF46D_4518_4BC3_B169_C7F9B7AE0330__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MacroDefine.h"
#include "MutexInteger.h"
class CCyclePipe  
{
private:
	BYTE*	m_pBuffer;
	int		m_nSize;
	BOOL	m_bOverWrite;
	BYTE*	m_pHead;
	BYTE*	m_pTail;

public:
	//初始化
	void  Initialize(int nSize,BOOL bOverWrite);
	//加入数据
	int   PushPipe(BYTE* pData,int nDataLen);
	//取出数据
	int   PopPipe(BYTE* pRet,int nDataLen);	
public:
	//从有效的数据开始检索数据
	BYTE* SeekData(BYTE* pSeek,int nLen);
	//丢弃到...之前的数据
	int ChuckData(BYTE * pData);
	//获取当前有效的数据，但不改变指针位置
	int GetPipeData(BYTE* pRet,int nDataLen);
	//清除所有的数据
	void ResetPipeData();
	int   GetPayloadSize();	
	int   GetFreeSize();
private:
	//检索数据
	BYTE* SeekData(BYTE* pBegin,BYTE* pSeek,int nLen);

	//其它函数
	int   GetBuffSize(){return m_nSize;};
	int   GetDataSize(BYTE* pStart,BYTE* pEnd);	
	BYTE* GetHeadPoint(){return m_pHead;}
	BYTE* GetNextPoint(BYTE* p,int n = 1);
	BYTE* GetPrevPoint(BYTE* p,int n = 1);
	BOOL  IsInDataArea(BYTE* p);
	BOOL  IsInFreeArea(BYTE* p);
public:
	CCyclePipe(int nSize = 0,BOOL bOverWrite = TRUE);
	virtual ~CCyclePipe();
	void TraceDataSize();

private:
	vfc::CVorxMutex m_Mutex;
	time_t m_dwLastCntTime;
};

#endif // !defined(AFX_CYCLEBUFFER_H__527CF46D_4518_4BC3_B169_C7F9B7AE0330__INCLUDED_)
