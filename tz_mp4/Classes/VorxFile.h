// VorxFile.h: interface for the CVorxFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VORXFILE_H__C34442A6_98FF_49ED_8DBC_16311B0B3AAE__INCLUDED_)
#define AFX_VORXFILE_H__C34442A6_98FF_49ED_8DBC_16311B0B3AAE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MacroDefine.h"
#ifndef WIN32
#include <stdarg.h>
#include <dirent.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include <string>
using namespace std;

namespace vfc
{
	class CVorxFile  
	{
	private:
		int m_hFile;
		string m_sFileName;

	public:
		bool FileSync();
		bool Create(const char* sFileName);
		bool Open(const char* sFileName);
		bool Close();
		bool IsOpened();
		UINT64 Seek(int nFrom,UINT64 pos,bool bBack = false);
		UINT64 Tell();
		int Write(void* pData,int nLen);
		int Read(void* pBuff,int nLen);
		bool TryWrite(void* pData,int nLen);
		bool TryRead(void* pBuff,int nLen);
		UINT64 Size();

		CVorxFile();
		virtual ~CVorxFile();
		
	};

	class CFileFinder
	{
	private:
		BOOL m_bSub; // 是否查找子目录
		char* m_sPath;
		char m_szSearchFile[128];
		CFileFinder *m_pFinder;
#ifdef WIN32
		WIN32_FIND_DATA m_findData;
		HANDLE m_hFind;
#else
		DIR* m_pDirectory;
#endif

	public:
		void FindFile(const char *szFileName);
		BOOL FindNext(char* sName,ULONG* nLen,BOOL* bDir);
		CFileFinder(LPCTSTR sPath, BOOL bSub = FALSE);
		~CFileFinder();
	};
}

#endif // !defined(AFX_VORXFILE_H__C34442A6_98FF_49ED_8DBC_16311B0B3AAE__INCLUDED_)
