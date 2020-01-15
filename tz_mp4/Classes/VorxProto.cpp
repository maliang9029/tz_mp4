// VorxProto.cpp: implementation of the CVorxProto class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MacroDefine.h"
#include "VorxProto.h"
#include "ProtoCollect.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace vfc
{
CVorxProto::CVorxProto()
{
	memset(&m_head,0,sizeof(PACK_HEAD));
	memset(&m_body,0,sizeof(PACK_BODY));

	m_head.synch = 0xFFFA;
	m_head.version = 0x01;
	m_head.total = 1;
	m_head.index = 0;
	m_time = GetTickCount();

	m_sRemoteIP[0] = 0;
	m_nRemotePort = 0;

	m_pReturn = NULL;
}

CVorxProto::~CVorxProto()
{
	ReleaseBody();
}

void CVorxProto::ReleaseBody()
{
	if(m_body.data)
	{
		delete[] m_body.data;
		m_body.data = NULL;
		m_body.len = 0;
	}
}

BYTE CVorxProto::CheckSum()
{
	BYTE sum = 0;
	for(int i=0;i<sizeof(PACK_HEAD)-1;i++)
		sum ^= *((BYTE*)&m_head+i);
	return sum;
}

void CVorxProto::SetRemoteParam(LPCTSTR sIP,int nPort)
{
	strcpy(m_sRemoteIP,sIP);
	m_nRemotePort = nPort;
}

BOOL CVorxProto::IsDataValid()
{
	//校验起始字节及数据长度
	if(m_head.datalen != m_body.len || m_head.synch != 0xFFFA) 
		return FALSE;
	//校验校验和
	if(m_head.checksum != CheckSum()) return FALSE;
	//校验分片
	if(m_head.index >= m_head.total || m_head.index < 0)
		return FALSE;
	return TRUE;
}

BOOL CVorxProto::IsNotSplitted()
{
	return (m_head.total < 2);
}

BOOL CVorxProto::IsTimeOut(DWORD curTime,int nLimite)
{
	return (nLimite <= curTime-m_time);
}

int CVorxProto::SplitToPackets(CProtoCollect& lst,int nSize)
{
	int nPackNum = 1 + (m_body.len-1)/nSize;//向上取整
	for(int i=0 ; m_body.len>nSize*i ; i++)
	{
		CVorxProto* pNew = new CVorxProto;
		memcpy(&pNew->m_head,&m_head,sizeof(PACK_HEAD));
		pNew->SetRemoteParam(m_sRemoteIP,m_nRemotePort);
		pNew->m_head.index = i;
		pNew->m_head.total = nPackNum;
		pNew->SetBodyData(m_body.data+i*nSize,min((unsigned int)nSize,m_body.len-i*nSize));
		pNew->SyncHeadField();
		lst.AddFragment(pNew,FALSE);
	}
	return nPackNum;
}

BOOL CVorxProto::ParseData(BYTE* pData,int nLen)
{
	if(nLen < sizeof(PACK_HEAD))return FALSE;
	memcpy(&m_head,pData,sizeof(PACK_HEAD));
	SetBodyData(pData+sizeof(PACK_HEAD),nLen - sizeof(PACK_HEAD));
	return IsDataValid();
}

void CVorxProto::CloneProto(CVorxProto* p)
{
//	memcpy(&m_head,&p->m_head,sizeof(PACK_HEAD));
	m_head = p->m_head;
	SetRemoteParam(p->m_sRemoteIP, p->m_nRemotePort);
	SetBodyData(p->m_body.data,p->m_body.len);
	SyncHeadField();
}

void CVorxProto::BuildQueryHead(DWORD fun,DWORD sub)
{
	m_head.fun = fun;
	m_head.sub = sub;
	m_head.type = PROTOTYPE_QUERY;

	static DWORD snid = 0;
	m_head.sn = ++snid;
}

void CVorxProto::BuildReportHead(DWORD fun,DWORD sub)
{
	BuildQueryHead(fun,sub);
	m_head.type = PROTOTYPE_REPORT;
}

void CVorxProto::SetBodyData(BYTE* pData,int nLen)
{
	if(nLen != m_body.len)
	{
		ReleaseBody();
		if(nLen > 0)m_body.data = new BYTE[nLen];
	}
	m_body.len = nLen;
	if(nLen > 0)memcpy(m_body.data,pData,m_body.len);
}

void CVorxProto::SyncHeadField()
{
	m_head.datalen = m_body.len;
	m_head.checksum = CheckSum();
}

long  CVorxProto::GetPackLength()
{
	return (sizeof(PACK_HEAD) + m_body.len);
}

void CVorxProto::GetPackData(BYTE* pData,BYTE* pKey,int nKeyLen)
{
	SyncHeadField();
	memcpy(pData,&m_head,sizeof(PACK_HEAD));
	PACK_HEAD* pHead = (PACK_HEAD*)pData;
	SwapFormat(pHead);
	memcpy(pData+sizeof(PACK_HEAD),m_body.data,m_body.len);
	if(nKeyLen > 0)
	{
		for(int i=0;i<sizeof(PACK_HEAD)+m_body.len;i++)
			pData[i] = ~pData[i];
	}
}

void CVorxProto::BuildRespondHead(CVorxProto* pQuery,int nRetCode)
{
	m_head.fun = pQuery->m_head.fun;
	m_head.sub = pQuery->m_head.sub;
	m_head.type = PROTOTYPE_RESPOND;
	m_head.sn = pQuery->m_head.sn;
	m_head.retcode = nRetCode;
	SetRemoteParam(pQuery->m_sRemoteIP,pQuery->m_nRemotePort);
}

void CVorxProto::AdjustByteOrder(char* p,int nLen)
{
#ifdef BIG_EDIAN
	char cTemp = 0;
	for(int i=0;i<nLen/2;i++)
	{
		cTemp = p[i];
		p[i] = p[nLen-i-1];
		p[nLen-i-1] = cTemp;
	}
#endif
}

void CVorxProto::SwapFormat(PACK_HEAD* p)
{
/*
	AdjustByteOrder((char*)&p->synch,2);
	AdjustByteOrder((char*)&p->datalen,4);
	AdjustByteOrder((char*)&p->fun,4);
	AdjustByteOrder((char*)&p->sub,4);
	AdjustByteOrder((char*)&p->sn,4);
	AdjustByteOrder((char*)&p->retcode,4);
	AdjustByteOrder((char*)&p->total,2);
	AdjustByteOrder((char*)&p->index,2);
*/
	// 位域不支持指针访问
	AdjustByteOrder((char*)p,2);
	AdjustByteOrder(((char*)p)+4,4);
	AdjustByteOrder(((char*)p)+8,4);
	AdjustByteOrder(((char*)p)+12,4);
	AdjustByteOrder(((char*)p)+16,4);
	AdjustByteOrder(((char*)p)+20,4);
	AdjustByteOrder(((char*)p)+24,2);
	AdjustByteOrder(((char*)p)+26,2);
}
}
