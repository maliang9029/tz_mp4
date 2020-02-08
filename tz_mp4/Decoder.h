#pragma once

#include "common.h"

#define OUTPUT_YUV420P 0
#define LOAD_YUV420P 1



#define V_DATA_POINTERS 4
#define DelayTime 0
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
typedef string (*FILE_END_CALL_BACK)(string strCurFile, void* lParam);

enum FILE_STATUS {
    FS_DEFAULT = 0,
    FS_NOFOUND,
    FS_NORMAL
};


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
    FILE_STATUS file_status;
public:
    int64_t get_last_keyframe_dts(int64_t pos, bool &reach_header);
    int64_t get_first_keyframe_pos();
    int64_t get_end_keyframe_pos();
    void add_keyframe(int64_t pos, int64_t dts);
    void set_file_name(string filename);
    string get_file_name();
    void clear();
    void set_pos(int64_t pos);
    int64_t get_pos();
    void set_status(FILE_STATUS status);
    FILE_STATUS get_status();
    bool is_empty();
};

class CDecoder
{
public:
	CDecoder();
	~CDecoder(void);
public:
	bool init(const char* sFilePath, HWND hWnd, int playid);
	void stop_decode();
	void d3dinit();
	bool play_pause();
    bool play_resume();
	bool snapshot(const char* sFilePath);
	bool NextSingleFrame();
    bool PreSingleFrame();
    bool play_speed(int speed);
	bool play_seek(unsigned int ntime);
	bool play_stop();
	bool play_save_start(const char* sSavePath);
	bool play_save_stop();
	int64_t get_cur_play_time();
	string get_cur_play_file();
    void set_play_end_callback(FILE_END_CALL_BACK next_file, FILE_END_CALL_BACK pre_file,void* lUserData);
    static int thread_play(LPVOID lParam);
    static int thread_parse_file(LPVOID lParam);
private:
    bool internal_init(string sFilePath);
    void internal_stop();
	int InitD3D( HWND hwnd, unsigned long lWidth, unsigned long lHeight );
	void Cleanup();
	BOOL SaveToFile(const char * pFilename, unsigned char * pSurFrame, int w, int h);
	int initFilter(const char* filter);
    bool CreateImgConvert(int pix_fmt, int width, int height);
    bool DestroyImgConvert();
    void seek( int64_t seekTime,int streamIndex);
    void DisPlayFrame(AVFrame* frame);
    int play();
    int parse();
private:
    IDirect3D9 *m_pDirect3D9;
	IDirect3DDevice9 *m_pDirect3DDevice;
	IDirect3DSurface9 *m_pDirect3DSurfaceRender;
	CRITICAL_SECTION  m_critial;
	RECT m_rtViewport;
	int m_screen_w;
	int m_screen_h;
	int m_nFrameRate;
    int64_t  m_curPts;
	string m_strCurFileName;
	HRESULT m_lRet;
    //ID3DVRInterface*	m_pD3DRender;
    DWORD				m_dwImageIndex;
    FILE_END_CALL_BACK	next_file_callback;
    FILE_END_CALL_BACK	pre_file_callback;
    void*				file_end_param;
	AVFormatContext	*ifmt_ctx;
	AVCodecContext	*m_pCodecCtx;
	AVFrame	*m_pFrame;
    int64_t  m_lastDts;
    int64_t  m_lastPts;
    int64_t  m_seekTime;
	bool b_strart_save;
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
    CVorxThread thread_parse;
    CVorxMutex mutex_save_file;
	CSaveAsFile* m_pSaveFile;
    int playid;
	int64_t m_nPlayedTime;
    AVRational time_base;
    SOCKET s;
};
