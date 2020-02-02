#pragma once

#include "common.h"

#define OUTPUT_YUV420P 0
#define LOAD_YUV420P 1



#define V_DATA_POINTERS 4
#define DelayTime 5
#define HCHN int

#define FIND_KEYFRAME_ERROR_LAST_FILE -1
#define FIND_KEYFRAME_ERROR_NO_FOUND -2



#include "ImgConvert.h"
#include "SaveAsFile.h"
typedef struct VPicture
{
	unsigned char *data[V_DATA_POINTERS];
	int linesize[V_DATA_POINTERS];     ///< number of bytes per line
}VPicture;



/*描述:yuv回调函数
 *参数:nWidth	[/O]	图像的宽
 *     nHeight	[/O]	图像的高度
 *     picture	[/O]	图像数据
 *     lParam	[/O]	回调参数
 *返回值:void
 */
typedef void (*ON_VEDIO_DATA)(int nWidth, int nHeight, VPicture* picture, void* lParam);

class CRecordInfo
{
public:
    CRecordInfo();
    ~CRecordInfo();
    CRecordInfo(const CRecordInfo & c);
private:
    string file_name;
    int64_t pos;//byte position in stream
    map<int64_t, int64_t> keyframe_positions;
public:
    int64_t get_last_keyframe_dts(int64_t pos);
    void add_keyframe(int64_t pos, int64_t dts);
    void set_file_name(string filename);
    string get_file_name();
    void clear();
    void set_pos(int64_t pos);
    int64_t get_pos();
};

class CDecoder
{
public:
	CDecoder(void);
	~CDecoder(void);
public:
	bool init(const char* sFilePath,HWND hWnd);
	void stopdecoder();
	void d3dinit();
	bool play_pause();
    bool play_resume();
	int play();
	bool snapshot(const char* sFilePath);
	bool NextSingleFrame();
    bool PreSingleFrame();
    bool play_speed(int speed);
	bool play_seek(unsigned int ntime);
	bool play_stop();
	bool play_save_start(const char* sSavePath);
	bool play_save_stop();
    static int thread_fun(LPVOID lParam);

    void SetVideoCallBack(ON_VEDIO_DATA pCallBack,void* lUserData);
	IDirect3D9 *m_pDirect3D9;
	IDirect3DDevice9 *m_pDirect3DDevice;
	IDirect3DSurface9 *m_pDirect3DSurfaceRender;
	CRITICAL_SECTION  m_critial;
	RECT m_rtViewport;
	int m_screen_w;
	int m_screen_h;
	int m_nFrameRate;

    int64_t  m_curPts;

	HRESULT m_lRet;
    //ID3DVRInterface*	m_pD3DRender;
    DWORD				m_dwImageIndex;
    ON_VEDIO_DATA		m_pOnVideoDataCallBack;
    void*				m_pOnVideoDataParam;
private:
	int InitD3D( HWND hwnd, unsigned long lWidth, unsigned long lHeight );
	void Cleanup();
	BOOL SaveToFile(const char * pFilename, unsigned char * pSurFrame, int w, int h);
	int initFilter(const char* filter);
    bool CreateImgConvert(int pix_fmt, int width, int height);
    bool DestroyImgConvert();
    void seek( int64_t seekTime,int streamIndex);
    void DisPlayFrame(AVFrame* frame);
private:
	AVFormatContext	*m_pFormatCtx;
	AVCodecContext	*m_pCodecCtx;

	AVFrame	*m_pFrame;
    int64_t  m_lastDts;
    int64_t  m_lastPts;
    int64_t  m_seekTime;
	bool m_bKey;


	AVFilterContext *m_pBuffersink_ctx;
	AVFilterContext *m_pBuffersrc_ctx;
	AVFilterGraph *m_pFilter_graph;


	CVorxMutex mutex;
	CVorxThread m_hThread;
	int m_videoindex;
	HWND m_hWnd;
	bool m_bPause;
	bool m_bCapTure;
	float m_nPlaySpeed;
	CImgConvert*	m_pimgConvert;
    VPicture m_snapPic;
	string m_strSaveFile;
	bool m_bSaveVideo;
    CRecordInfo current_file;
    CRecordInfo pre_file;
	CSaveAsFile* m_pSaveFile;
};
