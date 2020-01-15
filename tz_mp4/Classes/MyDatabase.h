//////////////////////////////////////////////////////////////////////
//
// MyDatabase.h: interface for the CMyDatabase class.
// 类名 : CMyConnection/CMyRecordset
// 说明 : MySQL数据访问类
// 作者 : 肖可伟
// 时间 : 2009年9月
// 版权 : 北京蛙视通信技术有限责任公司
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYDATABASE_H__0D9C1464_78B7_408B_86E1_3E517D35D999__INCLUDED_)
#define AFX_MYDATABASE_H__0D9C1464_78B7_408B_86E1_3E517D35D999__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VorxTime.h"

#ifdef WIN32
#include <winsock2.h>
#include "mysql/win32/include/mysql.h"
#pragma comment(lib, "mysql/win32/lib/libMysql.lib")
#else
#ifndef BIT64
#include "mysql/linux_x86_32bit/include/mysql.h"
#else // !defined BIT64
#include "mysql/linux_x86_64bit/include/mysql.h"
#endif // !defined BIT64
#endif

namespace vfc
{
	class CMyConnection;
	class CMyRecordset
	{
		friend class CMyConnection;
	private:
		MYSQL_RES* m_pRes;
		MYSQL_ROW  m_row;
		void SetRes(MYSQL_RES* pRes);
		BOOL GetFieldInfo(LPCTSTR sField,int* nIndex,enum_field_types* nType);
		BOOL GetData(LPCTSTR sField,void* pData,int* nLen,enum_field_types* nType);
		
	public:
		long GetRecordCount();
		BOOL IsEOF();
		BOOL MoveFirst();
		BOOL MoveNext();
		BOOL GetStringField(LPCTSTR sField,char* sRet);
		BOOL GetLongField(LPCTSTR sField,long* nRet);
		BOOL GetBlobField(LPCTSTR sField,void* pRet,int* nLen);
		BOOL GetDoubleField(LPCTSTR sField,double* dRet);
		BOOL GetTimeField(LPCTSTR sField,CVorxTime* tRet);
		CMyRecordset();
		virtual ~CMyRecordset();
	};
	
	class CMyConnection  
	{
	private:
		MYSQL* m_pConn;
		
	public:
		static void ConvertBlob(char* pRet,void* pData,int nLen);

		BOOL Connect(LPCTSTR sIP,int nPort,LPCTSTR sUserName,LPCTSTR sPassword,LPCTSTR sDatabase);
		BOOL Close();
		BOOL ExecuteSQL(CMyRecordset* pRec,LPCTSTR sql);
		long GetIdentity();
		const char *GetErrInfo();
		CMyConnection();
		virtual ~CMyConnection();
		
	};
}

#endif // !defined(AFX_MYDATABASE_H__0D9C1464_78B7_408B_86E1_3E517D35D999__INCLUDED_)
