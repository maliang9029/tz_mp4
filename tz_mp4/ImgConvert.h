// ImgConvert.h: interface for the CImgConvert class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMGCONVERT_H__08A0A620_581B_4E08_A66A_0BA33EC0338D__INCLUDED_)
#define AFX_IMGCONVERT_H__08A0A620_581B_4E08_A66A_0BA33EC0338D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#include "common.h"


class CImgConvert  
{
public:
	CImgConvert();
	virtual ~CImgConvert();

public:
	bool InitConvert(int pix_fmt, int w, int h);
	bool DestroyConvert();
	bool ConvertImg(int src_pix_fmt, void * pSource, void * pTarget, int w, int h);
	void * ConvertImg(int src_pix_fmt, void * pSource, int w, int h);
    bool SaveToFile( const char * pFilename, void * pSurFrame, int w, int h);

protected:
	int			m_nConvertPixFmt;
	AVPicture*	m_pFrameConvert;
};

#endif // !defined(AFX_IMGCONVERT_H__08A0A620_581B_4E08_A66A_0BA33EC0338D__INCLUDED_)
