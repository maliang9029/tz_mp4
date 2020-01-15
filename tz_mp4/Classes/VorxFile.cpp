// VorxFile.cpp: implementation of the CVorxFile class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "VorxFile.h"

#ifdef WIN32
#include <io.h>
#else
#include <fnmatch.h>
#include <string.h>
#endif

namespace vfc
{
	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////
	
	CVorxFile::CVorxFile()
	{
		m_hFile = -1;
//		m_sFileName[0] = 0;
	}
	
	CVorxFile::~CVorxFile()
	{
		Close();
	}

	// wy修改: 不能使用append选项, append选项只能在文件末尾追加内容, 不能用write修改前面的内容
	bool CVorxFile::Create(const char* sFileName)
	{
#ifdef WIN32
		if((m_hFile = _open(sFileName,_O_CREAT|_O_BINARY|_O_RDWR,_S_IWRITE)) > 0)
#else
		if((m_hFile = open64(sFileName,O_RDWR|O_CREAT|O_LARGEFILE|O_NONBLOCK,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))  > 0)
#endif
		{
			m_sFileName = sFileName;
			return true;
		}
		m_hFile = -1;
		return false;
	}
	
	bool CVorxFile::Open(const char* sFileName)
	{
#ifdef WIN32
		if((m_hFile = _open(sFileName,_O_RDONLY|_O_BINARY)) > 0)
#else
		if((m_hFile = open64(sFileName,O_RDONLY|O_LARGEFILE))  > 0)
#endif
		{
			m_sFileName = sFileName;
			return true;
		}
		m_hFile = -1;
		return false;
	}
	
	bool CVorxFile::Close()
	{
		if(m_hFile != -1)
		{
#ifndef WIN32
			close(m_hFile);
#else
			_close(m_hFile);
#endif
			m_hFile = -1;
			return true;
		}
		return false;
	}
	
	bool CVorxFile::IsOpened()
	{
		return m_hFile > 0;
	}
	
	UINT64 CVorxFile::Seek(int nFrom,UINT64 offset,bool bBack)
	{
#ifndef WIN32
		if(bBack)
			return  lseek64(m_hFile, -offset,nFrom);
		else
			return  lseek64(m_hFile, offset,nFrom);
#else
		if(bBack)
			return _lseeki64(m_hFile,-offset,nFrom);
		else
			return _lseeki64(m_hFile,offset,nFrom);
#endif
	}
	
	UINT64 CVorxFile::Tell()
	{
#ifndef WIN32
		return lseek64(m_hFile,0,SEEK_CUR);
#else
		return _telli64(m_hFile);
#endif
	}

	int CVorxFile::Write(void* pData,int nLen)
	{
#ifndef WIN32
		return  write(m_hFile,pData,nLen);
#else
		return _write(m_hFile,pData,nLen);
#endif
	}
	
	int CVorxFile::Read(void* pBuff,int nLen)
	{
#ifndef WIN32
		return  read(m_hFile,pBuff,nLen);
#else
		return _read(m_hFile,pBuff,nLen);
#endif
	}

	bool CVorxFile::TryWrite(void* pData,int nLen)
	{
		int nBytes = Write(pData,nLen);
		return (nBytes==nLen || nBytes+Write((char*)pData+nBytes,nLen-nBytes)==nLen);
	}

	bool CVorxFile::TryRead(void* pBuff,int nLen)
	{
		int nBytes = Read(pBuff,nLen);
		return (nBytes==nLen || nBytes+Read((char*)pBuff+nBytes,nLen-nBytes)==nLen);
	}
	
	UINT64 CVorxFile::Size()
	{
#ifndef WIN32
		struct stat64 info;
		if(fstat64(m_hFile,&info) == 0)
		{
			return info.st_size;
		}
#else
		struct _stati64 info;
		if(_fstati64(m_hFile,&info) == 0)
		{
			return info.st_size;
		}
#endif
		return 0;
	}
	
	bool CVorxFile::FileSync()
	{
#ifdef WIN32
		return (_commit(m_hFile) == 0);
#else
		return (fsync(m_hFile) == 0);
#endif
	}

	///////////////////////////////////////////////////////////////
	//
	// Class CFileFinder member functions
	//
	///////////////////////////////////////////////////////////////
	CFileFinder::CFileFinder(LPCTSTR sPath, BOOL bSub/* = FALSE*/)
	{
		m_pFinder = NULL;

		if(sPath)
		{
			int nLen = strlen(sPath);
			m_sPath = new char[nLen+1];
			strcpy(m_sPath,sPath);
			if(m_sPath[nLen-1] == '\\' || m_sPath[nLen-1] == '/')
			{
				m_sPath[nLen-1] = 0;
			}
		}
		else m_sPath = NULL;

		m_bSub = bSub;
		strcpy(m_szSearchFile, "*.*");

#ifdef WIN32
		m_hFind = INVALID_HANDLE_VALUE;
#else
		m_pDirectory = NULL;
#endif
	}

	CFileFinder::~CFileFinder()
	{
		if(m_sPath)delete[] m_sPath;
#ifdef WIN32
		if (INVALID_HANDLE_VALUE != m_hFind)
		{
			FindClose(m_hFind);
			m_hFind = INVALID_HANDLE_VALUE;
		}
#endif
		if (NULL != m_pFinder)
			delete m_pFinder;
	}

	void CFileFinder::FindFile(const char *szFileName)
	{
		strcpy(m_szSearchFile, szFileName);
	}

	// windows版本暂时未支持在子目录中查找
	BOOL CFileFinder::FindNext(char* sName,ULONG* nLen,BOOL* bDir)
	{
#ifdef WIN32
		BOOL bRet = FALSE;
		if(m_hFind == INVALID_HANDLE_VALUE)
		{
			char* sTarget = new char[strlen(m_sPath)+512];
			sprintf(sTarget,"%s\\%s", m_sPath, m_szSearchFile);
			m_hFind = FindFirstFile(sTarget,&m_findData);
			bRet = (m_hFind != INVALID_HANDLE_VALUE);
			if (NULL != sTarget) delete [] sTarget;
		}
		else bRet = FindNextFile(m_hFind,&m_findData);

		if(bRet)
		{
			if(sName)
				sprintf(sName, "%s/%s", m_sPath, m_findData.cFileName);
			if(nLen)*nLen = m_findData.nFileSizeLow;
			if(bDir)*bDir = (FILE_ATTRIBUTE_DIRECTORY == m_findData.dwFileAttributes);
		}
		return bRet;
#else
		if(!m_pDirectory)
		{
			m_pDirectory = opendir(m_sPath);
		}

		/*
		d_type值定义
		01	enum
		02	{
		03	    DT_UNKNOWN = 0,
		05	    DT_FIFO = 1,
		07	    DT_CHR = 2,
		09	    DT_DIR = 4,
		11	    DT_BLK = 6,
		13	    DT_REG = 8,
		15	    DT_LNK = 10,
		17	    DT_SOCK = 12,
		19	    DT_WHT = 14
		21	};
		*/
		char szPath[1024];
		if(m_pDirectory)
		{
			// 正在查询子目录
			if (m_bSub && NULL != m_pFinder)
			{
				// 从子目录中查找下一个
				if (m_pFinder->FindNext(sName, nLen, bDir))
					return TRUE;
				// 子目录中没有下一个
				delete m_pFinder;
				m_pFinder = NULL;
			}

			dirent* entry = readdir(m_pDirectory);
			while (entry)
			{
				// 比较
				if (0 != fnmatch(m_szSearchFile, entry->d_name, FNM_FILE_NAME))
				{
					// 查子目录
					if (m_bSub)
					{
						if ((DT_UNKNOWN == entry->d_type || DT_DIR == entry->d_type) && '.' != entry->d_name[0])
						{
							strcpy(szPath, m_sPath);
							if ('/' != m_sPath[strlen(m_sPath)-1])
								strcat(szPath, "/");
							strcat(szPath, entry->d_name);
							m_pFinder =  new CFileFinder(szPath, TRUE);
							m_pFinder->FindFile(m_szSearchFile);
							if (m_pFinder->FindNext(sName, nLen, bDir))
								return TRUE;
						}
					}

					// 下一文件
					entry = readdir(m_pDirectory);
					continue;
				}

				if(sName)
					sprintf(sName,"%s/%s",m_sPath,entry->d_name);
				if(nLen)
				{
					struct stat buf;
					if(stat(sName,&buf) < 0)
						*nLen = 0;
					else
						*nLen = (DWORD)buf.st_size;
					
				}
				if(bDir)*bDir = (entry->d_type == DT_DIR);
				return TRUE;
			}
		}
		return FALSE;
#endif
	}
}

