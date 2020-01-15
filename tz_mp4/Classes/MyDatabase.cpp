// MyDatabase.cpp: implementation of the CMyDatabase class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MyDatabase.h"
#include <stdlib.h>

namespace vfc
{
	//////////////////////////////////////////////////////////////////////
	// CMyRecordset functions
	//////////////////////////////////////////////////////////////////////
	
	CMyRecordset::CMyRecordset()
	{
		m_pRes = NULL;
		m_row = NULL;
	}
	
	CMyRecordset::~CMyRecordset()
	{
		SetRes(NULL);
	}
	
	void CMyRecordset::SetRes(MYSQL_RES* pRes)
	{
		if(m_pRes != NULL)
		{
			mysql_free_result(m_pRes);
			m_pRes = NULL;
		}
		m_pRes = pRes;
	}
	
	BOOL CMyRecordset::GetFieldInfo(LPCTSTR sField,int* nIndex,enum_field_types* nType)
	{
		MYSQL_FIELD* fd = NULL;
		mysql_field_seek(m_pRes,0);
		for(int i=0;fd = mysql_fetch_field(m_pRes);i++)
		{
			if(_stricmp(fd->name,sField) == 0)
			{
				if(nType)*nType = fd->type;
				if(nIndex)*nIndex = i;
				return TRUE;
			}
		}
		return FALSE;
	}
	
	BOOL CMyRecordset::GetData(LPCTSTR sField,void* pData,int* nLen,enum_field_types* nType)
	{
		int nIndex = -1;
		if(GetFieldInfo(sField,&nIndex,nType) && m_row)
		{
			unsigned long* pLen = mysql_fetch_lengths(m_pRes);
			if(nLen)*nLen = pLen[nIndex];
			if(pData)memcpy(pData,m_row[nIndex],pLen[nIndex]);
			return TRUE;
		}
		return FALSE;
	}
	
	long CMyRecordset::GetRecordCount()
	{
		return m_pRes?(int)mysql_num_rows(m_pRes):0;
	}
	
	BOOL CMyRecordset::IsEOF()
	{
		return (m_row == NULL);
	}
	
	BOOL CMyRecordset::MoveFirst()
	{
		if(m_pRes)
		{
			mysql_data_seek(m_pRes,0);
			return MoveNext();
		}
		return FALSE;
	}
	
	BOOL CMyRecordset::MoveNext()
	{
		if(m_pRes)
		{
			m_row = mysql_fetch_row(m_pRes);
			return TRUE;
		}
		return FALSE;
	}
	
	BOOL CMyRecordset::GetStringField(LPCTSTR sField,char* sRet)
	{
		enum_field_types nType;
		if(!GetFieldInfo(sField,NULL,&nType))
			return FALSE;
		if(nType==FIELD_TYPE_NULL) // ¿ÕÖµ
		{
			sRet[0] = 0;
			return TRUE;
		}
		if(nType!=FIELD_TYPE_STRING && nType!=FIELD_TYPE_VAR_STRING)
			return FALSE;
		
		int nLen = 0;
		if(GetData(sField,sRet,&nLen,NULL))
		{
			sRet[nLen] = 0;
			return TRUE;
		}
		return FALSE;
	}
	
	BOOL CMyRecordset::GetBlobField(LPCTSTR sField,void* pRet,int* nLen)
	{
		enum_field_types nType;
		if(!GetFieldInfo(sField,NULL,&nType))
			return FALSE;
		if (nType == FIELD_TYPE_NULL) // ¿ÕÖµ
		{
			*nLen = 0;
			return TRUE;
		}
		if(nType != FIELD_TYPE_BLOB && nType != FIELD_TYPE_TINY_BLOB && 
			nType != FIELD_TYPE_MEDIUM_BLOB && nType != FIELD_TYPE_LONG_BLOB)
			return FALSE;
		return GetData(sField,pRet,nLen,&nType);
	}
	
	BOOL CMyRecordset::GetLongField(LPCTSTR sField,long* nRet)
	{
		enum_field_types nType;
		if(!GetFieldInfo(sField,NULL,&nType))
			return FALSE;
		if(nType!=FIELD_TYPE_TINY && nType!=FIELD_TYPE_LONG && 
			nType!=FIELD_TYPE_SHORT && nType!=FIELD_TYPE_INT24)
			return FALSE;
		
		char s[50];
		memset(s,0,50);
		if(GetData(sField,s,NULL,NULL))
		{
			if(nRet)*nRet = atol(s);
			return TRUE;
		}
		return FALSE;
	}
	
	BOOL CMyRecordset::GetDoubleField(LPCTSTR sField,double* dRet)
	{
		enum_field_types nType;
		if(!GetFieldInfo(sField,NULL,&nType))
			return FALSE;
		if(nType!=FIELD_TYPE_FLOAT && nType!=FIELD_TYPE_DOUBLE)
			return FALSE;
		
		char s[50];
		memset(s,0,50);
		if(GetData(sField,s,NULL,NULL))
		{
			if(dRet)*dRet = atof(s);
			return TRUE;
		}
		return FALSE;
	}
	
	BOOL CMyRecordset::GetTimeField(LPCTSTR sField,CVorxTime* tRet)
	{
		enum_field_types nType;
		if(!GetFieldInfo(sField,NULL,&nType))
			return FALSE;
		if(nType == FIELD_TYPE_NULL) // ¿ÕÖµ
		{
			tRet->SetValue(0, 0, 0, 0, 0, 0);
			return TRUE;
		}
		if(nType!=FIELD_TYPE_DATETIME)
			return FALSE;
		
		char s[50];
		memset(s,0,50);
		if(GetData(sField,s,NULL,NULL))
		{
			if(tRet)tRet->SetValue(s);
			return TRUE;
		}
		return FALSE;
	}
	
	//////////////////////////////////////////////////////////////////////
	// CMyConnection functions
	//////////////////////////////////////////////////////////////////////
	
	CMyConnection::CMyConnection()
	{
		m_pConn = NULL;
	}
	
	CMyConnection::~CMyConnection()
	{
		Close();
	}

	void CMyConnection::ConvertBlob(char* pRet,void* pData,int nLen)
	{
		mysql_escape_string(pRet,(const char*)pData,nLen);
	}
	
	BOOL CMyConnection::Connect(LPCTSTR sIP,int nPort,LPCTSTR sUserName,LPCTSTR sPassword,LPCTSTR sDatabase)
	{
		if(m_pConn)Close();
		m_pConn = mysql_init(NULL);
		if(m_pConn == NULL)return FALSE;
		mysql_options(m_pConn, MYSQL_SET_CHARSET_NAME, "gb2312");
		return (mysql_real_connect(m_pConn,sIP,sUserName,sPassword,sDatabase,nPort,NULL,0) != NULL);
	}
	
	BOOL CMyConnection::Close()
	{
		if(m_pConn)
		{
			mysql_close(m_pConn);
			m_pConn = NULL;
			return TRUE;
		}
		return FALSE;
	}
	
	BOOL CMyConnection::ExecuteSQL(CMyRecordset* pRec,LPCTSTR sql)
	{
		if(mysql_query(m_pConn,sql) != NULL)return FALSE;
		if(pRec != NULL)
		{
			pRec->SetRes(mysql_store_result(m_pConn));
			pRec->MoveFirst();
		}
		return TRUE;
	}
	
	long CMyConnection::GetIdentity()
	{
		return m_pConn?(long)mysql_insert_id(m_pConn):0;
	}

	const char *CMyConnection::GetErrInfo()
	{
		return mysql_error(m_pConn);
	}
}


