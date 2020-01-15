// TaskCenter.cpp: implementation of the CTaskBase class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TaskBase.h"
#include "VorxProto.h"
#include "GlobalFunction.h"

namespace vfc
{
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTaskBase::CTaskBase()
{
	m_pLogCallback = NULL;
	m_pLogParam = NULL;
	m_pAsynCallback = NULL;
	m_pAsynParam = NULL;
	m_pEncryptKey = NULL;
	m_nKeyLength = 0;

	m_aQueryQueue.Initialize(1000,TRUE);
	m_aReturnQueue.Initialize(1000,TRUE);

	m_sock.SetCallBack(OnUdpReceive,this);

	m_bWriteQuery = 0;
	m_bWriteReturn = 0;
	m_oTaskThread[0].SetParam(QueryDealThread,this,10);
	m_oTaskThread[1].SetParam(ReturnDealThread,this,10);
}

CTaskBase::~CTaskBase()
{
	m_aQueryQueue.Destroy();
	m_aReturnQueue.Destroy();
	if(m_pEncryptKey)
	{
		delete[] m_pEncryptKey;
		m_nKeyLength = 0;
	}
}

void CTaskBase::SetLogCallback(OUTPUT_LOG fun,LPVOID lParam)
{
	m_pLogCallback = fun;
	m_pLogParam = lParam;
}

void CTaskBase::SetAsynCallback(ASYN_RET fun,LPVOID lParam)
{
	m_pAsynCallback = fun;
	m_pAsynParam = lParam;
}

void CTaskBase::OutputLog(int nCode,LPCTSTR sText)
{
	if(m_pLogCallback)m_pLogCallback(nCode,sText,m_pLogParam);
#ifndef WIN32
	if(nCode != 0)printf("%s\n",sText);
#endif
}

void CTaskBase::AsnyNotify(CVorxProto* p)
{
	if(p->m_head.fun == 'HB')
		OnDealHeartReturn(p);
	else
		OnDealAsnyReturn(p);
// 	else if(m_pAsynCallback)
// 		m_pAsynCallback(p->m_head.fun,p->m_head.sub,p->m_head.retcode,p->m_body.data,p->m_body.len,m_pAsynParam);
}

int  CTaskBase::GetSocketPort()
{
	return m_sock.GetBindedPort();
}

BOOL CTaskBase::GetNearestLocalAddr(LPCTSTR sRemote,char* sLocal)
{
	vector<in_addr> addrVector;
	int iCount = VGAPI_GetLocalAddress(addrVector);
	if(iCount > 0)
	{
		in_addr addr = VGAPI_NearestAddress(addrVector,sRemote);
//		strcpy(sLocal,inet_ntoa(addr));
		VGAPI_Address2String(addr,sLocal);
		return TRUE;
	}
	return FALSE;
}

void CTaskBase::OnDealAsnyReturn(CVorxProto *pTask)
{
	if(m_pAsynCallback)
		m_pAsynCallback(pTask->m_head.fun,pTask->m_head.sub,pTask->m_head.retcode,pTask->m_body.data,pTask->m_body.len,m_pAsynParam);
}

bool CTaskBase::Create(int nPort, const char *szBindIP)
{
	m_oTaskThread[0].StartThread();
	m_oTaskThread[1].StartThread();
	if(!m_sock.Create(nPort,TRUE,1024*100,szBindIP))
	{
		m_oTaskThread[0].StopThread();
		m_oTaskThread[1].StopThread();
		OutputLog(1,"CTaskBase类创建UDP SOCKET失败!可能是端口已占用!");
		return false;	
	}
	m_sock.SetSendBuffer(32*1024);
	m_sock.SetRecvBuffer(32*1024);
	return OnTaskStart();
}

void CTaskBase::Close()
{
	OnTaskStop();
	m_sock.Close();
	m_oTaskThread[0].StopThread();
	m_oTaskThread[1].StopThread();
	m_aQueryQueue.ResetQueue();
	m_aReturnQueue.ResetQueue();
	ReleaseSendList();
	ReleaseWaitList();
}

void CTaskBase::SetEncrypt(void* pKey,int nKeyLen)
{
	if(m_pEncryptKey) delete[] m_pEncryptKey;
	m_pEncryptKey = new BYTE[nKeyLen];
	memcpy(m_pEncryptKey,pKey,nKeyLen);
	m_nKeyLength = nKeyLen;
}

void CTaskBase::OnUdpReceive(char* pData,int nLen,char* sIP,int nPort,LPVOID lParam)
{
	CTaskBase* pThis = (CTaskBase*)lParam;
	CNodeQueue<CVorxProto> *pQueue = NULL;
	if(nLen >= sizeof(CPackHead))
	{
		CPackHead* pHead = (CPackHead*)pData;

		if(pThis->m_nKeyLength > 0)
		{
			for(int i=0;i<nLen;i++)
				pData[i] = ~pData[i];
		}
		CVorxProto::SwapFormat(pHead);

		if(pHead->type == PROTOTYPE_RESPOND || pHead->fun == 'HB')
			pQueue = &pThis->m_aReturnQueue;
		else
			pQueue = &pThis->m_aQueryQueue;
		
		BOOL bOverWrite = FALSE;
		CVorxProto* p = pQueue->GetBuffer(&bOverWrite);
		if(bOverWrite)
		{
			pThis->OutputLog(1,"UDP数据加入任务队列时覆盖已有数据单元!");
// 			FILE *pf = fopen("/tmp/ss.log", "a+");
// 			if (NULL != pf)
// 			{
// 				fprintf(pf, "UDP任务队列满\n");
// 				fclose(pf);
// 			}
		}

		if(p)
		{
			if(p->ParseData((BYTE*)pData,nLen))
			{
				p->SetRemoteParam(sIP,nPort);
				pQueue->Enqueue(p);	
			}else{
				pThis->OutputLog(1,"UDP数据协议解析失败!本包被丢失!");
				pQueue->Recycle(p);
// 				FILE *pf = fopen("/tmp/ss.log", "a+");
// 				if (NULL != pf)
// 				{
// 					fprintf(pf, "UDP数据协议解析失败\n");
// 					fclose(pf);
// 				}
			}
		}
		else pThis->OutputLog(1,"协议队列取空节点失败!本包被丢失!");
	}
	else pThis->OutputLog(1,"UDP数据长度小于包头长度!本包被丢失!");
}

BOOL CTaskBase::QueryDealThread(LPVOID lParam)
{
	CTaskBase* pThis = (CTaskBase*)lParam;
#ifndef WIN32
	if (!pThis->m_bWriteQuery)
	{
// 		FILE *f = fopen("/tmp/SSLog", "a+");
// 		if (NULL != f)
// 		{
// 			// 写日志
// 			fprintf(f, "PID %d is ssprotomodule QueryDealThread\n", getpid());
// 			
// 			fclose(f);
// 		}
		pThis->m_bWriteQuery = 1;
	}
#endif // !defined WIN32

	CVorxProto* p = pThis->m_aQueryQueue.Dequeue();
	if(p)
	{
		if(p->IsNotSplitted())
			pThis->DealIntegratedTask(p);
		else
			pThis->DealPacketFragment(p);
		p->ReleaseBody();
		pThis->m_aQueryQueue.Recycle(p);
		return 1;
	}
	return 0;
}

BOOL CTaskBase::ReturnDealThread(LPVOID lParam)
{
	CTaskBase* pThis = (CTaskBase*)lParam;
#ifndef WIN32
	if (!pThis->m_bWriteReturn)
	{
// 		FILE *f = fopen("/tmp/SSLog", "a+");
// 		if (NULL != f)
// 		{
// 			// 写日志
// 			fprintf(f, "PID %d is ssprotomodule ReturnDealThread\n", getpid());
// 			
// 			fclose(f);
// 		}
		pThis->m_bWriteReturn = 1;
	}
#endif // !defined WIN32

	CVorxProto* p = pThis->m_aReturnQueue.Dequeue();
	if(p)
	{
		if(p->IsNotSplitted())
			pThis->DealIntegratedTask(p);
		else
			pThis->DealPacketFragment(p);
		p->ReleaseBody();
		pThis->m_aReturnQueue.Recycle(p);
		return 1;
	}
	return 0;
}

bool CTaskBase::DeleteWaitItem(CProtoCollect* p)
{
	CAutoMutex am(&m_waitMutex);
	for(CColPos pos=m_aWaitList.begin();pos!=m_aWaitList.end();pos++)
	{
		if((*pos) == p)
		{
			delete (*pos);
			m_aWaitList.erase(pos);
			return true;
		}
	}
	return false;
}

CProtoCollect* CTaskBase::GetWaitPos(unsigned int sn)
{
	DWORD curTime = GetTickCount();
	CAutoMutex am(&m_waitMutex);
	for(CColPos p=m_aWaitList.begin();p!=m_aWaitList.end();)
	{
		if((*p)->m_head.sn == sn) return (*p);
		if((*p)->IsTimeout(curTime,1000))
		{
			delete (*p);
			p = m_aWaitList.erase(p);
			OutputLog(1,"发现超时的不完整包,抛弃!");
		}
		else p++;
	}
	return NULL;
}

void CTaskBase::DealPacketFragment(CVorxProto* pTask)
{
	CProtoCollect* p = GetWaitPos(pTask->m_head.sn);
	if(p == NULL) 
	{
		p = new CProtoCollect(&pTask->m_head);
		p->AddFragment(pTask,TRUE);
#ifdef VORX_DEBUG
		char sLog[100];
		sprintf(sLog,"收到分片包序号:total=%d,index=%d",pTask->m_head.total,pTask->m_head.index);
		OutputLog(1,sLog);
#endif
		m_waitMutex.Lock();
		m_aWaitList.push_back(p);
		m_waitMutex.Unlock();
	}
	else
	{
		p->AddFragment(pTask,TRUE);
		if(p->HaveAllFrags())
		{
			CVorxProto proto;
			if(p->CombineFragments(proto))
				DealIntegratedTask(&proto);
			DeleteWaitItem(p);
		}
	}
}

BOOL CTaskBase::RemoveSendNode(CVorxProto* pSend)
{
	CAutoMutex am(&m_sendMutex);
	for(CProtoPos p=m_aSendList.begin();p!=m_aSendList.end();p++)
	{
		if(pSend == (*p))
		{
			m_aSendList.erase(p);
			return TRUE;
		}
	}
	return FALSE;
}

void CTaskBase::ReleaseSendList()
{
	CAutoMutex am(&m_sendMutex);
// 	for(CProtoPos p = m_aSendList.begin();p!=m_aSendList.end();p++)
// 		delete (*p);
	m_aSendList.clear();
}

void CTaskBase::ReleaseWaitList()
{
	CAutoMutex am(&m_waitMutex);
	for(CColPos pos = m_aWaitList.begin();pos!=m_aWaitList.end();pos++)
		delete (*pos);
	m_aWaitList.clear();
}

CVorxProto* CTaskBase::GetSendPos(unsigned int sn)
{
	CAutoMutex am(&m_sendMutex);
	for(CProtoPos p=m_aSendList.begin();p!=m_aSendList.end();p++)
	{
		if((*p)->m_head.sn == sn) return (*p);
	}
	return NULL;
}

long CTaskBase::SendQueryProto(CVorxProto* pSend,CVorxProto* pRet,int nTimeout)
{
	long retcode = ERR_SUCCESS;

	int len = pSend->GetPackLength();
	BYTE* pData = new BYTE[len];	
	pSend->GetPackData(pData,m_pEncryptKey,m_nKeyLength);

	if(nTimeout > 0)
	{
		pSend->m_time = GetTickCount();
		CAutoMutex am(&m_sendMutex);
		m_aSendList.push_back(pSend);
	}

	if(len == m_sock.SendTo(pData,len,pSend->m_sRemoteIP,pSend->m_nRemotePort))
	{
		if(nTimeout > 0)
		{
			if(pRet) pSend->m_pReturn = pRet;
			else pSend->m_pReturn = new CVorxProto;

			if(!pSend->m_event.WaitForSignal(nTimeout))
				retcode = ERR_NETTIMEOUT;
			else 
				retcode = pSend->m_pReturn->m_head.retcode;

			if(!pRet)
			{
				m_sendMutex.Lock();
				delete pSend->m_pReturn;
				pSend->m_pReturn = NULL;
				m_sendMutex.Unlock();
			}
		}
	}
	else retcode = ERR_NETSEND;

	if(nTimeout > 0) RemoveSendNode(pSend);

	delete[] pData;		//modified by xkw in 2008-7-30
	return retcode;
}

long CTaskBase::SendQueryProtoEx(CVorxProto* pSend,CVorxProto* pRet,int nTimeout,long nMaxBodySize)
{
	long retcode = ERR_SUCCESS;
	if(nMaxBodySize == 0 || pSend->GetPackLength() <= nMaxBodySize)
		retcode =  SendQueryProto(pSend,pRet,nTimeout);
	else
	{
		CProtoCollect lstProto(NULL);
		pSend->SplitToPackets(lstProto,nMaxBodySize);
		retcode = lstProto.SendAllFragmentsAndWaitRet(this,pRet,nTimeout);
	}

	return retcode;
}

BOOL CTaskBase::SendRespondProto(CVorxProto* pSend,long nMaxBodySize)
{
	BOOL bRet = TRUE;
	pSend->SyncHeadField();
	if(nMaxBodySize == 0 || pSend->GetPackLength() <= nMaxBodySize)
		bRet =  (ERR_SUCCESS == SendQueryProto(pSend,NULL,0));
	else
	{
		CProtoCollect lstProto(NULL);
		pSend->SplitToPackets(lstProto,nMaxBodySize);
		bRet = lstProto.SendAllFragments(this);
	}
	return bRet;
}

void CTaskBase::RespondErrorCommand(CVorxProto* pTask,long nErrCode)
{
	CVorxProto snd;
	snd.BuildRespondHead(pTask,nErrCode);
	SendRespondProto(&snd);
}

bool CTaskBase::AddMulticastMemberShip(LPCTSTR sMulticastIP)
{
	return m_sock.AddMembership(sMulticastIP)?true:false;
}

void CTaskBase::DealIntegratedTask(CVorxProto* pTask)
{
	switch(pTask->m_head.type)
	{
	case PROTOTYPE_RESPOND://返回包
		{
			CAutoMutex am(&m_sendMutex);
			CVorxProto* pFind = NULL;
			for(CProtoPos p = m_aSendList.begin();p!=m_aSendList.end();p++)
			{
				if((*p)->m_head.sn == pTask->m_head.sn) 
				{
					pFind = *p;
					break;
				}
			}
			if(pFind != NULL)
			{
				if(pFind->m_pReturn != NULL)
				{
					pFind->m_pReturn->CloneProto(pTask);
					pFind->m_event.Signal();
				}
			}
			else AsnyNotify(pTask);
		}
		break;
	case PROTOTYPE_QUERY://请求包
		if(pTask->m_head.fun == 'HB')
			OnDealHeartQuery(pTask);
		else
			OnDealQueryTask(pTask);
		break;
	case PROTOTYPE_REPORT://上报包
		OnDealReportTask(pTask);
		break;
	case PROTOTYPE_CONFIRM://确认包
		break;
	default://未知包
		OutputLog(1,"未知类型的协议包!抛弃!");
		break;
	}
}
}
