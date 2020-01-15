//////////////////////////////////////////////////////////////////////
//
// ProtoCollect.h: interface for the CProtoCollect class.
// 专门用于处理返回协议分片汇集及合成的类
// 作    者:肖可伟
// 时    间:2008-3-8
// 最后修改:2008-3-8
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROTOCOLLECT_H__2379CD39_0DF4_41B1_B1C1_EF946D720DD1__INCLUDED_)
#define AFX_PROTOCOLLECT_H__2379CD39_0DF4_41B1_B1C1_EF946D720DD1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _MSC_VER
#pragma warning (disable:4786)
#endif

#include <map>
using namespace std;

#include "VorxProto.h"

namespace vfc
{
class CTaskBase;
class CProtoCollect  
{
public:
	map<WORD,CVorxProto*> m_aFragments;			//协议分片
	CVorxMutex m_mutexFragments;				//分片互斥量
	CPackHead m_head;							//包头信息
	DWORD m_time;								//用于超时检测

public:
	//对外功能函数
	void AddFragment(CVorxProto* p,BOOL bAddClone);
	BOOL CombineFragments(CVorxProto& pack);
	BOOL IsTimeout(DWORD curTime,unsigned int nLimite);
	//内部工具函数
	BOOL HaveAllFrags();						//用于接收->合包
	inline BOOL IsNeddedPacket(CVorxProto* p);	//用于接收->合包
	BOOL SendAllFragments(CTaskBase* pTask);	//用于分包->发送
	int SendAllFragmentsAndWaitRet(CTaskBase* pTask,CVorxProto* pRet,int nTimeout);	//用于分包->发送

public:
	//构造析构函数
	CProtoCollect(CPackHead* pHead);
	virtual ~CProtoCollect();

};
typedef vector<CProtoCollect*> CColList;
typedef CColList::iterator CColPos;
}
#endif // !defined(AFX_PROTOCOLLECT_H__2379CD39_0DF4_41B1_B1C1_EF946D720DD1__INCLUDED_)
