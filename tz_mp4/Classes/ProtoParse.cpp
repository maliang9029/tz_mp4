// ProtoParse.cpp: implementation of the CProtoParse class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MacroDefine.h"
#include "ProtoParse.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CProtoParam Functions Define
//////////////////////////////////////////////////////////////////////
CProtoParam::CProtoParam(LPCTSTR sName)
{
	if(sName && strlen(sName) > 0)
	{
		TiXmlElement item(sName);
		m_pItem = (TiXmlElement*)m_doc.InsertEndChild(item);
	}
	else m_pItem = NULL;
}

CProtoParam::~CProtoParam()
{
}

CProtoParam CProtoParam::GetNextSiblingParam(LPCTSTR sName)
{
	return m_pItem->NextSiblingElement(sName);
}

CProtoParam CProtoParam::GetFirstChildParam(LPCTSTR sName)
{
	if(m_pItem)
		return m_pItem->FirstChildElement(sName);
	else
		return (TiXmlElement*)NULL;
}

LPCTSTR CProtoParam::GetValue(LPCTSTR sName)
{
	try
	{
		if(m_pItem)
			return m_pItem->Attribute(sName);
		else
			return NULL;
	}
	catch(...)
	{
		return NULL;
	}
}

BOOL CProtoParam::GetValue(char* sRet,LPCTSTR sName)
{
	if(m_pItem)
	{
		LPCTSTR s = m_pItem->Attribute(sName);
		if(s)
		{
			strcpy(sRet,s);
			return TRUE;
		}
	}
	return FALSE;
}

int CProtoParam::GetValueInt(LPCTSTR sName)
{
	LPCTSTR s = GetValue(sName);
	if(s) return atoi(s);
	else return 0;
}

void CProtoParam::SetValue(LPCTSTR sName,LPCTSTR sVal)
{
	if(m_pItem)
		m_pItem->SetAttribute(sName,sVal);
}

void CProtoParam::SetValue(LPCTSTR sName,int nVal)
{
	if(m_pItem)
		m_pItem->SetAttribute(sName,nVal);
}

void CProtoParam::AddChildParam(CProtoParam child)
{
	if(m_pItem)
		m_pItem->InsertEndChild(*child.m_pItem);
}

CProtoParam CProtoParam::GetClone()
{
	return (TiXmlElement*)m_pItem->Clone();
}

//////////////////////////////////////////////////////////////////////
// CProtoParse functions Define
//////////////////////////////////////////////////////////////////////

CProtoParse::CProtoParse(DWORD fun,DWORD sub,PROTO_TYPE type)
{
	static int snCount = 0;

	TiXmlElement item("proto");
	m_pRoot = (TiXmlElement*)m_doc.InsertEndChild(item);
	SetMainFun(fun);
	SetSubFun(sub);
	SetSerialNumber();
	SetType(type);
	SetVersion("2.1.1");
	SetRetCode(0);
}

CProtoParse::~CProtoParse()
{

}

BOOL CProtoParse::ParseString(LPCTSTR sText)
{
	try
	{
		m_doc.Clear();
		m_doc.Parse(sText);
		m_pRoot = m_doc.RootElement();
		if(!m_pRoot) return FALSE;
		if(strcmp(m_pRoot->Value(),"proto") != 0) 
		{
			m_doc.Clear();
			m_pRoot = NULL;
			return FALSE;
		}
		const char* p = NULL;
		int nLen = 0;
		//主功能号
		SetMainFun(str2dw(m_pRoot->Attribute("fun")));
		//子功能号
		SetSubFun(str2dw(m_pRoot->Attribute("sub")));
		//包序列号
		p = m_pRoot->Attribute("sn");
		if(p) SetSerialNumber(atoi(p));
		else SetSerialNumber(0);
		//协议类型
		p = m_pRoot->Attribute("type");
		if(strlen(p) > 0) SetType(str2pt(p));
		else SetType(PROTOTYPE_UNKNOWN);
		//版 本 号
		SetVersion(m_pRoot->Attribute("ver"));
		//返 回 码
		p = m_pRoot->Attribute("retcode");
		if(p) SetRetCode(atoi(p));
		else SetRetCode(0);
		return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}

CProtoParse& CProtoParse::operator =(const CProtoParse& other)
{
	if(this == &other) return *this;
	m_doc = other.m_doc;
	m_pRoot = m_doc.RootElement();
	return *this;
}

CProtoParam CProtoParse::GetFirstParam(LPCTSTR sName)
{	
	if(m_pRoot)
		return m_pRoot->FirstChildElement(sName);
	else
		return (TiXmlElement*)NULL;
}

void CProtoParse::AddParam(CProtoParam pm)
{
	if(m_pRoot) m_pRoot->InsertEndChild(pm.GetXmlElement());
}

void CProtoParse::ClearParam()
{
	if(m_pRoot) m_pRoot->Clear();
}

//设置值
void CProtoParse::SetVersion(LPCTSTR sVer)
{
	if(m_pRoot) m_pRoot->SetAttribute("ver",sVer);
}

void CProtoParse::SetMainFun(DWORD fun)
{
	if(m_pRoot) m_pRoot->SetAttribute("fun",dw2str(fun));
}

void CProtoParse::SetSubFun(DWORD sub)
{
	if(m_pRoot) m_pRoot->SetAttribute("sub",dw2str(sub));
}

void CProtoParse::SetSerialNumber(int sn)
{
	if(m_pRoot) 
	{
		if(sn > 0)
		{
			m_pRoot->SetAttribute("sn",sn);
		}
		else
		{
			static int iii = 0;
			m_pRoot->SetAttribute("sn",++iii);
		}
	}
}

void CProtoParse::SetType(PROTO_TYPE type)
{
	if(m_pRoot) m_pRoot->SetAttribute("type",pt2str(type));
}

void CProtoParse::SetRetCode(int nRetCode)
{
	if(m_pRoot) m_pRoot->SetAttribute("retcode",nRetCode);
}

//取值
DWORD CProtoParse::GetMainFun()
{
	if(m_pRoot)
		return str2dw(m_pRoot->Attribute("fun"));
	else
		return 0;
}

DWORD CProtoParse::GetSubFun()
{
	if(m_pRoot)
		return str2dw(m_pRoot->Attribute("sub"));
	else
		return 0;
}

int CProtoParse::GetSerialNumber()
{
	if(m_pRoot)
		return atoi(m_pRoot->Attribute("sn"));
	else
		return 0;
}

PROTO_TYPE CProtoParse::GetType()
{
	if(m_pRoot)
		return str2pt(m_pRoot->Attribute("type"));
	else
		return PROTOTYPE_UNKNOWN;
}

LPCTSTR CProtoParse::GetVersion()
{
	if(m_pRoot)
		return m_pRoot->Attribute("ver");
	else
		return "";
}

int CProtoParse::GetRetCode()
{
	if(m_pRoot)
		return atoi(m_pRoot->Attribute("retcode"));
	else
		return 0;
}

BOOL CProtoParse::Send(CUdpSocket* pSock,sockaddr_in addr)
{
	TiXmlPrinter pnt;
	pnt.SetIndent(NULL);
	m_doc.Accept(&pnt);
	int nSendSize = pnt.Size();

	return (nSendSize == pSock->SendTo(pnt.CStr(),nSendSize+1,addr)-1);
}

BOOL CProtoParse::Send(CUdpSocket* pSock,LPCTSTR sIP,int nPort)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(sIP);
	addr.sin_port = htons(nPort);
	return Send(pSock,addr);
}

BOOL CProtoParse::Send(SOCKET s)
{
	TiXmlPrinter pnt;
	pnt.SetIndent(NULL);
	m_doc.Accept(&pnt);
	int nSendSize = pnt.Size();
	return (nSendSize == send(s,pnt.CStr(),nSendSize+1,0)-1);
}

DWORD CProtoParse::str2dw(LPCTSTR str)
{
	int nLen = MIN(strlen(str),4);
	DWORD dwRet = 0;
	for(int i=0;i<nLen;i++)
	{
		dwRet = dwRet << 8;
		dwRet += str[i];
	}
	return dwRet;
}

char* CProtoParse::dw2str(DWORD dw)
{
	static char sTemp[5];
	memset(sTemp,0,sizeof(sTemp));
	int n = 0;
	for(int i=3;i>=0;i--)
	{
		int val = ((dw & (0xFF << i*8)) >> i*8);
		if(val)	sTemp[n++] = val;
	}
	return sTemp;
}

char* CProtoParse::pt2str(PROTO_TYPE val)
{
	static char ret[2] = {0};
	ret[0] = val;
	return ret;
}

PROTO_TYPE CProtoParse::str2pt(LPCTSTR str)
{
	if(str && strlen(str) > 0)
	{
		return (PROTO_TYPE)str[0];
	}
	else return PROTOTYPE_UNKNOWN;
}
