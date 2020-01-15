//////////////////////////////////////////////////////////////////////
//
// VorxProto.h: interface for the CVorxProto class.
// 用于处理协议解析
// 作    者:肖可伟
// 时    间:2008-3-8
// 最后修改:2008-3-8
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VORXPROTO_H__08A98483_1B7C_4A1E_838E_94EE6AB987DC__INCLUDED_)
#define AFX_VORXPROTO_H__08A98483_1B7C_4A1E_838E_94EE6AB987DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef _MSC_VER
#pragma warning (disable:4786)
#endif

#include <list>
using namespace std;
#include "MutexInteger.h"

namespace vfc
{
class CProtoCollect;

enum VORX_PROTO_TYPE
{
	PROTOTYPE_UNKNOWN		= 'U',	//未知包
	PROTOTYPE_QUERY			= 'Q',	//请求包
	PROTOTYPE_RESPOND		= 'R',	//回应包
	PROTOTYPE_REPORT		= 'P',	//上报包
	PROTOTYPE_CONFIRM		= 'C',	//确认包
};

class CVorxProto  
{
public:
/*
	struct PACK_HEAD
	{
		WORD synch;			//同步字节
		BYTE version;		//协议版本
		BYTE type;			//包类型
		DWORD sn;			//包序列号
		DWORD fun;			//主功能号
		DWORD sub;			//子功能号
		DWORD datalen;		//数据域总长
		DWORD retcode;		//返回码
		WORD total;			//包的分片总数
		WORD index;			//包的分片序号,从0开始
		BYTE patch[3];		//补位字节,使结构长度适应4字节对齐方式
		BYTE checksum;		//异或校验和
	} m_head;
*/
	struct PACK_HEAD // 新结构，与类型长度无关
	{
		ULONG synch   :16;		//同步字节
		ULONG version : 8;		//协议版本
		ULONG type    : 8;		//包类型
		ULONG sn      :32;		//包序列号
		ULONG fun     :32;		//主功能号
		ULONG sub     :32;		//子功能号
		ULONG datalen :32;		//数据域总长
		ULONG retcode :32;		//返回码
		ULONG total   :16;		//包的分片总数
		ULONG index   :16;		//包的分片序号,从0开始
		ULONG patch   :24;		//补位字节,使结构长度适应4字节对齐方式
		ULONG checksum: 8;		//异或校验和
	} m_head;

	struct PACK_BODY
	{
		BYTE* data;
		unsigned len;
	} m_body;

	char m_sRemoteIP[50];	//远程IP地址(来源或目标)
	int  m_nRemotePort;		//远程端口  (来源或目标)

	DWORD m_time;			//用于检测发送包超时
	CVorxEvent m_event;		//发送时用于等待回应包
	CVorxProto* m_pReturn;	//发送时用于接收回应包
public:
	//解析协议数据
	BOOL ParseData(BYTE* pData,int nLen);
	void CloneProto(CVorxProto* p);
	//构建协议包
	void BuildRespondHead(CVorxProto* pQuery,int nRetCode = 0);
	void BuildQueryHead(DWORD fun,DWORD sub);
	void BuildReportHead(DWORD fun,DWORD sub);
	void SetBodyData(BYTE* pData,int nLen);
	void SyncHeadField();
	long GetPackLength();
	void GetPackData(BYTE* pData,BYTE* pKey,int nKeyLen);
	//工具函数
	void SetRemoteParam(LPCTSTR sIP,int nPort);
	void ReleaseBody();
	inline BYTE CheckSum();
	inline BOOL IsDataValid();
	BOOL IsNotSplitted();
	BOOL IsTimeOut(DWORD curTime,int nLimite);
	int SplitToPackets(CProtoCollect& lst,int nSize);

	static void AdjustByteOrder(char* p,int nLen);
	static void SwapFormat(PACK_HEAD* p);

public:
	//构造析构函数
	CVorxProto();
	virtual ~CVorxProto();

};

typedef vector<CVorxProto*> CProtoList;
typedef CProtoList::iterator CProtoPos;

typedef CVorxProto::PACK_HEAD CPackHead;
typedef CVorxProto::PACK_BODY CPackBody;
}
#endif // !defined(AFX_VORXPROTO_H__08A98483_1B7C_4A1E_838E_94EE6AB987DC__INCLUDED_)
