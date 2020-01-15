//////////////////////////////////////////////////////////////////////
//
// ProtoParse.h: interface for the CProtoParse class.
// 说明:VORX XML协议解析类
// 作者:肖可伟
// 版权:北京蛙视通信技术有限责任公司
// 日期:2007-9-5
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROTOPARSE_H__C2F9607E_CA16_4449_8F57_9B341D57042C__INCLUDED_)
#define AFX_PROTOPARSE_H__C2F9607E_CA16_4449_8F57_9B341D57042C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "./TinyXml/tinyxml.h"
#include "UdpSocket.h"


enum PROTO_TYPE
{
	PROTOTYPE_UNKNOWN	= 'U',
	PROTOTYPE_QUERY		= 'Q',
	PROTOTYPE_RESPOND	= 'R',
	PROTOTYPE_REPORT	= 'P',
};

class CProtoParam
{
private:
	TiXmlDocument m_doc;
	TiXmlElement* m_pItem;

public:
	//类型转换,是否有效
	operator bool(){return (m_pItem != NULL);}

	//取值及子参数
	CProtoParam GetFirstChildParam(LPCTSTR sName);
	CProtoParam GetNextSiblingParam(LPCTSTR sName);
	LPCTSTR GetValue(LPCTSTR sName);
	BOOL GetValue(char* sRet,LPCTSTR sName);
	int GetValueInt(LPCTSTR sName);
	void SetValue(LPCTSTR sName,LPCTSTR sVal);
	void SetValue(LPCTSTR sName,int nVal);
	void AddChildParam(CProtoParam child);

	//其它函数
	CProtoParam GetClone();
	TiXmlElement GetXmlElement(){return *m_pItem;}

public:
	//构造析构函数
	CProtoParam(const CProtoParam& ct){m_pItem = ct.m_pItem;}	//拷贝构造函数
	CProtoParam(LPCTSTR sName = NULL);							//用名称构造一个参数
	CProtoParam(TiXmlElement* p){m_pItem = p;}					//传入节点构造参数
	~CProtoParam();

};

class CProtoParse 
{
public:
	TiXmlDocument m_doc;
	TiXmlElement* m_pRoot;

public:
	//类型转换,是否有效
	operator bool(){return (m_pRoot != NULL);}
	CProtoParse& operator =(const CProtoParse& other);

	BOOL ParseString(LPCTSTR sText);
	CProtoParam GetFirstParam(LPCTSTR sName);
	void AddParam(CProtoParam pm);
	void ClearParam();

	//设置值
	void SetMainFun(DWORD fun);
	void SetSubFun(DWORD sub);
	void SetSerialNumber(int sn = 0);
	void SetType(PROTO_TYPE type);
	void SetVersion(LPCTSTR sVer);
	void SetRetCode(int nRetCode);

	//取值
	DWORD GetMainFun();
	DWORD GetSubFun();
	int GetSerialNumber();
	PROTO_TYPE GetType();
	LPCTSTR GetVersion();
	int GetRetCode();

	//发送协议字符串
	BOOL Send(CUdpSocket* pSock,sockaddr_in addr);
	BOOL Send(CUdpSocket* pSock,LPCTSTR sIP,int nPort);
	BOOL Send(SOCKET s);

	//构造析构函数
	CProtoParse(DWORD fun=0,DWORD sub=0,PROTO_TYPE type=PROTOTYPE_QUERY);
	virtual ~CProtoParse();

public:
	//工具函数
	static DWORD str2dw(LPCTSTR str);
	static char* dw2str(DWORD dw);
	static char* pt2str(PROTO_TYPE val);
	static PROTO_TYPE str2pt(LPCTSTR str);
	
private:
	char m_sTemp[5];	//数据转换时临时参数

};

#endif // !defined(AFX_PROTOPARSE_H__C2F9607E_CA16_4449_8F57_9B341D57042C__INCLUDED_)
