// ImgConvert.cpp: implementation of the CImgConvert class.
//
//////////////////////////////////////////////////////////////////////

#include "ImgConvert.h"

//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
//#endif

//�µ�YUVתRGB����(lgd)
////////////////////////////////////////////////////////////////////////////////
//src0		[I/] DataY
//src1		[I/] DataU
//src2		[I/] DataV
//dst_ori	[/O] output RGB24
//width		[I/] picture width
//height	[I/] picture height
////////////////////////////////////////////////////////////////////////////////
void ConvertYUV2RGB24(unsigned char *src0,unsigned char *src1,unsigned char *src2,unsigned char *dst_ori,int width,int height)
{
	long int crv_tab[256];
	long int cbu_tab[256];
	long int cgu_tab[256];
	long int cgv_tab[256];
	long int tab_76309[256];
	unsigned char clp[1024];   //for clip in CCIR601
	long int crv,cbu,cgu,cgv;
	int ii,ind;   

	crv = 104597; cbu = 132201;  /* fra matrise i global.h */ 
	cgu = 25675;  cgv = 53279;

	for (ii = 0; ii < 256; ii++) {
		crv_tab[ii] = (ii-128) * crv;
		cbu_tab[ii] = (ii-128) * cbu;
		cgu_tab[ii] = (ii-128) * cgu;
		cgv_tab[ii] = (ii-128) * cgv;
		tab_76309[ii] = 76309*(ii-16);
	}

	for (ii=0; ii<384; ii++)
		clp[ii] =0;
	ind=384;
	for (ii=0;ii<256; ii++)
		clp[ind++]=ii;
	ind=640;
	for (ii=0;ii<384;ii++)
		clp[ind++]=255;


	int y1,y2,u,v;
	unsigned char *py1,*py2;
	int i,j, c1, c2, c3, c4;
	unsigned char *d1, *d2;

	py1=src0;
	py2=py1+width;
	d1=dst_ori;
	d2=d1+3*width;
	for (j = 0; j < height; j += 2) { 
		for (i = 0; i < width; i += 2) {

			u = *src1++;
			v = *src2++;

			c1 = crv_tab[v];
			c2 = cgu_tab[u];
			c3 = cgv_tab[v];
			c4 = cbu_tab[u];

			//up-left
			y1 = tab_76309[*py1++]; 
			*d1++ = clp[384+((y1 + c1)>>16)];  
			*d1++ = clp[384+((y1 - c2 - c3)>>16)];
			*d1++ = clp[384+((y1 + c4)>>16)];

			//down-left
			y2 = tab_76309[*py2++];
			*d2++ = clp[384+((y2 + c1)>>16)];  
			*d2++ = clp[384+((y2 - c2 - c3)>>16)];
			*d2++ = clp[384+((y2 + c4)>>16)];

			//up-right
			y1 = tab_76309[*py1++];
			*d1++ = clp[384+((y1 + c1)>>16)];  
			*d1++ = clp[384+((y1 - c2 - c3)>>16)];
			*d1++ = clp[384+((y1 + c4)>>16)];

			//down-right
			y2 = tab_76309[*py2++];
			*d2++ = clp[384+((y2 + c1)>>16)];  
			*d2++ = clp[384+((y2 - c2 - c3)>>16)];
			*d2++ = clp[384+((y2 + c4)>>16)];
		}
		d1 += 3*width;
		d2 += 3*width;
		py1+=   width;
		py2+=   width;
	}  
}

CImgConvert::CImgConvert()
{
	m_pFrameConvert = NULL;
	m_nConvertPixFmt = -1;
}

CImgConvert::~CImgConvert()
{
	DestroyConvert();
}

bool CImgConvert::InitConvert(int pix_fmt, int w, int h)
{

	if(AV_PIX_FMT_RGB24 != pix_fmt)
	{
		return false;
	}
	if(m_nConvertPixFmt == pix_fmt)
	{
		return true;
	}
	m_nConvertPixFmt = pix_fmt;


	m_pFrameConvert = new AVPicture;
	//ASSERT(m_pFrameConvert);
	avpicture_alloc(m_pFrameConvert,(AVPixelFormat)pix_fmt, w, h);	

	return true;
}

bool CImgConvert::ConvertImg(int src_pix_fmt, void * pSource, void * pTarget, int w, int h)
{
	bool bRet = false;
	AVPicture * pSur = (AVPicture*)pSource;
	AVPicture * pTrg = (AVPicture*)pTarget;

	if(src_pix_fmt != AV_PIX_FMT_YUV420P)
		return false;

	ConvertYUV2RGB24(pSur->data[0], pSur->data[1], pSur->data[2], pTrg->data[0], w, h);

	bRet = true;

	return bRet;
}

void * CImgConvert::ConvertImg(int src_pix_fmt, void * pSource, int w, int h)
{
	if(m_pFrameConvert)
	{
		if(ConvertImg(src_pix_fmt, pSource, m_pFrameConvert, w, h))
		{
			return m_pFrameConvert;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

bool CImgConvert::DestroyConvert()
{
	if(m_pFrameConvert)
	{
		avpicture_free(m_pFrameConvert);
		delete m_pFrameConvert;
		m_pFrameConvert = NULL;
	}
	m_nConvertPixFmt = -1;
	return true;
}

bool CImgConvert::SaveToFile( const char * pFilename, void * pSurFrame, int w, int h)
{
    unsigned char *lpbuffer;
	unsigned char *word;
	AVPicture * pFrame = (AVPicture*)pSurFrame;
	unsigned char * pDIBImage = pFrame->data[0];

	BITMAPFILEHEADER  bf; //bmp�ļ�ͷ
	BITMAPINFOHEADER  bi; //bmp��Ϣͷ
	bi.biSize = 40;  //λͼ��Ϣͷ��С
	bi.biWidth = w;  //ͼ����
	bi.biHeight = h;  //ͼ��߶�
	bi.biPlanes = 1;   //λƽ����=1
	bi.biBitCount = 24;  //��λ���ص�λ��
	bi.biCompression = 0;  //ͼƬ��ѹ�����ԣ�bmp��ѹ��������0
	//bi.biSizeImage = WIDTHBYTES(bi.biWidth * bi.biBitCount) * bi.biHeight;
	bi.biSizeImage = w * h * bi.biBitCount;
	//��ʾbmpͼƬ�������Ĵ�С������һ������biCompression����0ʱ�������ֵ����ʡ�Բ���

	bi.biXPelsPerMeter = 0; //ˮƽ�ֱ���
	bi.biYPelsPerMeter = 0; //��ֱ�ֱ���
	bi.biClrUsed = 0;   //��ʾʹ���˶��ٸ���ɫ������һ��biBitCount����С��16�Ż��õ�������0ʱ��ʾ��2^biBitCount����ɫ������
	bi.biClrImportant = 0;  //��ʾ�ж��ٸ���Ҫ����ɫ������0ʱ��ʾ������ɫ������Ҫ

	//Set BITMAPFILEHEADER  ����bmpͼƬ���ļ�ͷ��ʽ
	bf.bfType = 0x4d42;  //2���ֽڣ������0x4d42��ascii�ַ���BM��
	bf.bfSize = 54 + bi.biSizeImage; //�ļ���С����4���ֽ�Ϊ��λ
	bf.bfReserved1 = 0;  //����
	bf.bfReserved2 = 0;  //����
	bf.bfOffBits = 54;   //���������ļ��е�λ��ƫ����

	FILE *  fp =  fopen(pFilename, "wb+");
	if (fp == NULL)
	{
		return false;
	}
	fwrite(&bf, 14, 1, fp); //���ļ���д��ͼƬ�ļ�ͷ
	fwrite(&bi, 40, 1, fp); //���ļ���д��ͼƬ��Ϣͷ
	fseek(fp, 0x36, SEEK_SET);
	lpbuffer = pDIBImage+w*3*(h - 1);
	for(int i=0; i<h; i++)    //bmp file scan line is arraned by BGR|BGR|BGR|........
	{
		word = lpbuffer;
		for(int j=0; j<w; j++)
		{
			/*************add in 2011 8.2 23:04*****************/
			fputc( *(word+2), fp); // B
			fputc( *(word+1), fp); // G
			fputc( *(word+0), fp); // R			
			/***********************************************/
			/*fputc( *(word+2), fp); // B
			fputc( *(word+1), fp); // G
			fputc( *(word+0), fp); // R*/
			word+=3;
		}
		lpbuffer -= w*3; // ָ��ת����һ�еĿ�ʼ
	}
	fclose(fp);
	return true;
}