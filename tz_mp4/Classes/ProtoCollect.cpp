// ProtoCollect.cpp: implementation of the CProtoCollect class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ProtoCollect.h"
#include "TaskBase.h"

namespace vfc
{
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProtoCollect::CProtoCollect(CPackHead* pHead)
{
	if(pHead)memcpy(&m_head,pHead,sizeof(CPackHead));
	else memset(&m_head,0,sizeof(CPackHead));
	m_time = GetTickCount();
}

CProtoCollect::~CProtoCollect()
{
	CAutoMutex am(&m_mutexFragments);
	for(map<WORD,CVorxProto*>::iterator p=m_aFragments.begin();p!=m_aFragments.end();p++)
		delete p->second;
	m_aFragments.clear();
}

void CProtoCollect::AddFragment(CVorxProto* pFrag,BOOL bAddClone)
{
	//判断是否需要的数据
	if(!IsNeddedPacket(pFrag) && m_head.total != 0) 
	{
		if(!bAddClone) delete pFrag;
		return;
	}
	if(pFrag->m_head.sn == m_head.sn || m_head.sn == 0)
	{
		CVorxProto* pNew = NULL;
		if(bAddClone)
		{
			pNew = new CVorxProto;
			pNew->CloneProto(pFrag);
		}
		else pNew = pFrag;

		m_mutexFragments.Lock();
		m_aFragments.insert(map<WORD,CVorxProto*>::value_type(pNew->m_head.index,pNew));
		m_mutexFragments.Unlock();

		//如果未设置包头
		if(m_head.total == 0)
		{
			memcpy(&m_head,&pNew->m_head,sizeof(CPackHead));
			//means? remed by xkw in 2008-7-10
			//m_head.total = 1;
			//m_head.index = 1;
		}
		else m_head.datalen += pNew->m_body.len;
	}
}

BOOL CProtoCollect::HaveAllFrags()
{
	CAutoMutex am(&m_mutexFragments);
	return (m_aFragments.size() == m_head.total);
}

BOOL CProtoCollect::IsTimeout(DWORD curTime,unsigned int nLimite)
{
	return (nLimite <= curTime - m_time);
}

BOOL CProtoCollect::CombineFragments(CVorxProto& pack)
{
	if(HaveAllFrags())
	{
		memcpy(&pack.m_head,&m_head,sizeof(CPackHead));
		pack.m_body.len  = m_head.datalen;
		pack.m_body.data = new BYTE[pack.m_body.len];
		//按顺序组包
		int nRealLen = 0;
		map<WORD,CVorxProto*>::iterator pos,end;
		for(int i=0;i<m_head.total;i++)
		{
			m_mutexFragments.Lock();
			pos = m_aFragments.find(i);
			end = m_aFragments.end();
			m_mutexFragments.Unlock();

			if(pos == end)
			{
				pack.ReleaseBody();
				return FALSE;
			}
			memcpy(pack.m_body.data+nRealLen,pos->second->m_body.data,pos->second->m_body.len);
			nRealLen += pos->second->m_body.len;
			if (0 == strlen(pack.m_sRemoteIP))
			{
				pack.SetRemoteParam(pos->second->m_sRemoteIP, pos->second->m_nRemotePort);
			}
		}
		pack.SyncHeadField();
		return TRUE;
	}
	return FALSE;
}

BOOL CProtoCollect::IsNeddedPacket(CVorxProto* p)
{
	if(m_head.fun != 0 && m_head.fun != p->m_head.fun)
		return FALSE;
	if(m_head.sub != 0 && m_head.sub != p->m_head.sub)
		return FALSE;
	if(m_head.sn  != 0 && m_head.sn  != p->m_head.sn)
		return FALSE;
	if(m_head.total != 0 && m_head.total != p->m_head.total)
		return FALSE;
	CAutoMutex am(&m_mutexFragments);
	if(m_aFragments.find(p->m_head.index) != m_aFragments.end())
		return FALSE;
	return TRUE;
}

BOOL CProtoCollect::SendAllFragments(CTaskBase* pTask)
{
	BOOL bRet = TRUE;
	m_mutexFragments.Lock();

	int iCounter = 0;//发送计数器
	for(map<WORD,CVorxProto*>::iterator p=m_aFragments.begin();p!=m_aFragments.end();p++)
	{
		bRet  = bRet && (ERR_SUCCESS == pTask->SendQueryProto(p->second,NULL,0));
		{
			if(iCounter++ == 10)
			{
				Sleep(1);
				iCounter = 0;
			}
		}
	}
	m_mutexFragments.Unlock();
	return bRet;
}

int CProtoCollect::SendAllFragmentsAndWaitRet(CTaskBase* pTask,CVorxProto* pRet,int nTimeout)
{
	int nRet = 0;
	m_mutexFragments.Lock();

	int iCounter = 0;//发送计数器
	int i(0), nSize = m_aFragments.size();
	for(map<WORD,CVorxProto*>::iterator p=m_aFragments.begin();p!=m_aFragments.end();++p, ++i)
	{
		if (i+1 == nSize) // 最后一包，等应答
			nRet  = pTask->SendQueryProto(p->second,pRet,nTimeout);
		else
			nRet  = pTask->SendQueryProto(p->second,NULL,0);
		if (ERR_SUCCESS != nRet)
			return nRet;

		if(iCounter++ == 10)
		{
			Sleep(1);
			iCounter = 0;
		}
	}
	m_mutexFragments.Unlock();
	return 0;
}
}
