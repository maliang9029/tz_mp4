#include "Decoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
//#include <chrono>
#include <atlstr.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

void sleep_select_us(SOCKET s, int64_t usec)
{
	struct timeval tv;
	fd_set dummy;
	FD_ZERO(&dummy);
	FD_SET(s, &dummy);
	tv.tv_sec = usec / 1000000L;
	tv.tv_usec = usec % 1000000L;
	select(0, 0, 0, &dummy, &tv);
	DWORD err = GetLastError();
	if (err != 0)
		printf("Error : %d\n", err);
}

void my_sleep_ms(SOCKET s, int64_t ms, int type)
{
    uint64_t total_used = 0;
    uint64_t used = 0;
    FILETIME begin, end;
    int64_t interval_microseconds = 500;
    LARGE_INTEGER nFreq, time_begin, time_end;
    double duration;
    int ret = 0;
    if (type) {
        ret = QueryPerformanceFrequency(&nFreq);
    }

    while (total_used < ms * 1000) {
		double duration;
        if (ret) {
            QueryPerformanceCounter(&time_begin);
        } else {
            GetSystemTimeAsFileTime(&begin);
        }
		sleep_select_us(s, interval_microseconds);
		//Sleep(1);
        if (ret) {
            QueryPerformanceCounter(&time_end);
            duration =(time_end.QuadPart - time_begin.QuadPart)/(double)nFreq.QuadPart;
            duration *= 1000000;
        } else {
            GetSystemTimeAsFileTime(&end);
            duration = (end.dwLowDateTime - begin.dwLowDateTime)/10;
        }
		used = duration;
#if PRINT_
		//printf("total need sleep %lld us, sleep %lld us, time used %lld us\n", ms*1000, interval_microseconds, used);
#endif
		total_used += used;
	}
}

CRecordInfo::CRecordInfo()
{
    file_name = "";
    pos = 0;
    file_status = FS_DEFAULT;
}


CRecordInfo::~CRecordInfo()
{
}

CRecordInfo::CRecordInfo(const CRecordInfo & c)
{
    file_name = c.file_name;
    if (!c.keyframe_positions.empty()) {
        keyframe_positions = c.keyframe_positions;
    }
    file_status = FS_NORMAL;
    pos = 0;
}

int64_t CRecordInfo::get_last_keyframe_dts(int64_t pos, bool &reach_header)
{
    int64_t keyframe_dts = FIND_KEYFRAME_ERROR_NO_FOUND;
    reach_header = false;
    if (!keyframe_positions.empty()) {
        map<int64_t, int64_t>::iterator it = keyframe_positions.begin();
        if (it->first >= pos) {
            keyframe_dts = FIND_KEYFRAME_ERROR_LAST_FILE;
            return keyframe_dts;
        }
        it = keyframe_positions.end();
        it--;
        if (it->first < pos) {
            return it->second;
        }
        int64_t tmp = -1;
        for (it = keyframe_positions.begin(); it != keyframe_positions.end(); it++) {
            tmp = it->first;
            if (tmp >= pos) {
                it--;
                keyframe_dts = it->second;
                break;
            }
        }
        it = keyframe_positions.begin();
        if (keyframe_dts == it->second) {
            reach_header = true;
        }
    }
    return keyframe_dts;
}

int64_t CRecordInfo::get_first_keyframe_pos()
{
    int64_t keyframe_pos = FIND_KEYFRAME_ERROR_NO_FOUND;
    if (!keyframe_positions.empty()) {
        map<int64_t, int64_t>::iterator it = keyframe_positions.begin();
        keyframe_pos = it->first;
    }
    return keyframe_pos;
}

int64_t CRecordInfo::get_end_keyframe_pos()
{
    int64_t keyframe_pos = FIND_KEYFRAME_ERROR_NO_FOUND;
    if (!keyframe_positions.empty()) {
        map<int64_t, int64_t>::iterator it = keyframe_positions.end();
        it--;
        keyframe_pos = it->first;
    }
    return keyframe_pos;
}

void CRecordInfo::add_keyframe(int64_t pos, int64_t dts)
{
    keyframe_positions[pos] = dts;
}

void CRecordInfo::set_file_name(string filename)
{
    file_name = filename;
}
string CRecordInfo::get_file_name()
{
    return file_name;
}

void CRecordInfo::clear()
{
    file_name = "";
    keyframe_positions.clear();
    pos = 0;
    file_status = FS_DEFAULT;
}

void CRecordInfo::set_pos(int64_t pos)
{
    this->pos = pos;
}
int64_t CRecordInfo::get_pos()
{
    return pos;
}

void CRecordInfo::set_status(FILE_STATUS status)
{
    file_status = status;
}

FILE_STATUS CRecordInfo::get_status()
{
    return file_status;
}

bool CRecordInfo::is_empty()
{
    return keyframe_positions.empty();
}

CDecoder::CDecoder()
:m_nFrameRate(25)
,m_curPts(0)
,m_bPause(false)
,m_bCapTure(false)
,m_nPlaySpeed(1.0)
,m_pDirect3D9(NULL)
,m_pDirect3DDevice(NULL)
,m_pDirect3DSurfaceRender(NULL)
,m_lRet(0)
,m_screen_h(0)
,m_screen_w(0)
,next_file_callback(NULL)
,pre_file_callback(NULL)
,file_end_param(NULL)
,m_pimgConvert(NULL)
,m_lastDts(0)
,m_lastPts(0)
,m_seekTime(0)
,ifmt_ctx(NULL)
,m_pCodecCtx(NULL)
,m_videoindex(0)
,m_bSaveVideo(false)
,m_pSaveFile(NULL)
,b_strart_save(false)
,m_nPlayedTime(0)
{
	av_register_all();
	avformat_network_init();
    time_base = mux_timebase;
    WORD wVersionRequested = MAKEWORD(1, 0);
	WSADATA wsaData;
	WSAStartup(wVersionRequested, &wsaData);
    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
}

CDecoder::~CDecoder(void)
{
    closesocket(s);
}


void CDecoder::set_play_end_callback(FILE_END_CALL_BACK next_file, FILE_END_CALL_BACK pre_file,void* lUserData)
{
    next_file_callback =  next_file;
    pre_file_callback = pre_file;
    file_end_param = lUserData;
}

void CDecoder::DisPlayFrame(AVFrame* frame)
{
	if(frame->data[0] == NULL)
		return;
    VPicture tempPic;
    for (int i=0; i<V_DATA_POINTERS; i++)
    {
        tempPic.data[i] = frame->data[i];
        tempPic.linesize[i] = frame->linesize[i];
    }
    memcpy(&m_snapPic,&tempPic,sizeof(VPicture));//截图帧
    //pThis->m_pOnVideoDataCallBack(pThis->m_screen_w, pThis->m_screen_h, &tempPic, pThis->m_pOnVideoDataParam);
   // m_curPts = av_q2d(m_pCodecCtx->time_base)*pkt->dts*100;

    int y_size=m_pCodecCtx->width*m_pCodecCtx->height;
#if OUTPUT_YUV420P
    y_size=m_pCodecCtx->width*m_pCodecCtx->height;
    fwrite(m_pFrameYUV->data[0],1,y_size,fp_yuv);    //Y
    fwrite(m_pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
    fwrite(m_pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V
#endif

    uint8_t *py = frame->data[0];
    uint8_t *pu = frame->data[1];
    uint8_t *pv = frame->data[2];
    int sizey = frame->linesize[0];
    int sizeu = frame->linesize[1];
    int sizev = frame->linesize[2];


    if(m_pDirect3DSurfaceRender == NULL)
	    return ;
    D3DLOCKED_RECT d3d_rect;
    m_lRet = m_pDirect3DSurfaceRender->LockRect(&d3d_rect,NULL,D3DLOCK_DONOTWAIT);
    if(FAILED(m_lRet))
	    return ;
    byte* pData = new byte[y_size*12/8];
    AutoFreeA(byte, pData);
    memset(pData,0,y_size);
    byte* pDest = (BYTE *)d3d_rect.pBits;
    int stride = d3d_rect.Pitch;
    unsigned long i = 0;

    for (int i=0; i<m_screen_h; i++)//Y数据拷贝
    {//每次拷贝screen_w个数据
	    memcpy(pData+i*m_screen_w, py+i*sizey, m_screen_w);
    }
    for (int i=0; i<m_screen_h/2; i++)//U数据拷贝
    {
	    memcpy(pData+m_screen_w*m_screen_h+i*m_screen_w/2, pu+i*sizeu, m_screen_w/2);
    }
    for (int i=0; i<m_screen_h/2; i++)//V数据拷贝
    {
	    memcpy(pData+5*m_screen_w*m_screen_h/4+i*m_screen_w/2, pv+i*sizev, m_screen_w/2);
    }


    for(int i = 0;i < m_screen_h;i ++){
	    memcpy(pDest + i * stride,pData + i * m_screen_w, m_screen_w);
    }
    for(int i = 0;i < m_screen_h/2;i ++){
	    memcpy(pDest + stride * m_screen_h + stride * m_screen_h / 4 + i * stride / 2,
		    pData + m_screen_w * m_screen_h + i * m_screen_w / 2, m_screen_w / 2);
    }
    for(int i = 0;i < m_screen_h/2;i ++){
	    memcpy(pDest + stride * m_screen_h + i * stride / 2,
		    pData + m_screen_w * m_screen_h + m_screen_w * m_screen_h / 4 + i * m_screen_w / 2, m_screen_w / 2);
    }


    m_lRet=m_pDirect3DSurfaceRender->UnlockRect();
    if(FAILED(m_lRet))
	    return ;

    if (m_pDirect3DDevice == NULL)
	    return ;

    m_pDirect3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );
    m_pDirect3DDevice->BeginScene();
    IDirect3DSurface9 * pBackBuffer = NULL;
    m_pDirect3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
    m_pDirect3DDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer);
    m_pDirect3DDevice->StretchRect(m_pDirect3DSurfaceRender,NULL,pBackBuffer,&m_rtViewport,D3DTEXF_LINEAR);
    m_pDirect3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
    m_pDirect3DDevice->EndScene();
    m_pDirect3DDevice->Present( NULL, NULL, NULL, NULL );
    pBackBuffer->Release();
}

int CDecoder::thread_play(LPVOID lParam)
{
	CDecoder* pThis = (CDecoder*)lParam;
	if(pThis)
	{
        return pThis->play();
	}
	return 0;
}

int CDecoder::thread_parse_file(LPVOID lParam)
{
    CDecoder* pThis = (CDecoder*)lParam;
	if(pThis)
	{
        return pThis->parse();
	}
	return 0;
}

int CDecoder::initFilter(const char* filters_descr)
{
    char args[512];
    int ret = 0;
    const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    AVRational time_base = ifmt_ctx->streams[m_videoindex]->time_base;
    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE };
    m_pFilter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !m_pFilter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    sprintf(args,"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
            m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
            time_base.num, time_base.den,
            m_pCodecCtx->sample_aspect_ratio.num, m_pCodecCtx->sample_aspect_ratio.den);
    ret = avfilter_graph_create_filter(&m_pBuffersrc_ctx, buffersrc, "in",
                                       args, NULL, m_pFilter_graph);
    if (ret < 0) {
        //av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        goto end;
    }
    /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&m_pBuffersink_ctx, buffersink, "out",
                                       NULL, NULL, m_pFilter_graph);
    if (ret < 0) {
       // av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
        goto end;
    }
    ret = av_opt_set_int_list(m_pBuffersink_ctx, "pix_fmts", pix_fmts,
                              AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        //av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        goto end;
    }
    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */
    /*
     * The buffer source output must be connected to the input pad of
     * the first filter described by filters_descr; since the first
     * filter input label is not specified, it is set to "in" by
     * default.
     */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = m_pBuffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;
    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    inputs->name       = av_strdup("out");
    inputs->filter_ctx = m_pBuffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;
    if ((ret = avfilter_graph_parse_ptr(m_pFilter_graph, filters_descr,
                                    &inputs, &outputs, NULL)) < 0)
        goto end;
    if ((ret = avfilter_graph_config(m_pFilter_graph, NULL)) < 0)
        goto end;
end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    return ret;

}

void CDecoder::Cleanup()
{
	EnterCriticalSection(&m_critial);
	if(m_pDirect3DSurfaceRender)
		m_pDirect3DSurfaceRender->Release();
	if(m_pDirect3DDevice)
		m_pDirect3DDevice->Release();
	if(m_pDirect3D9)
		m_pDirect3D9->Release();
	LeaveCriticalSection(&m_critial);
}


int CDecoder::InitD3D( HWND hwnd, unsigned long lWidth, unsigned long lHeight )
{
	HRESULT lRet;
	InitializeCriticalSection(&m_critial);
	Cleanup();

	m_pDirect3D9 = Direct3DCreate9( D3D_SDK_VERSION );
	if( m_pDirect3D9 == NULL )
		return -1;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	GetClientRect(hwnd,&m_rtViewport);

	lRet=m_pDirect3D9->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,hwnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &m_pDirect3DDevice );
	if(FAILED(lRet))
		return -1;

#if LOAD_BGRA
	lRet=m_pDirect3DDevice->CreateOffscreenPlainSurface(
		lWidth,lHeight,
		D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,
		&m_pDirect3DSurfaceRender,
		NULL);
#elif LOAD_YUV420P
	lRet=m_pDirect3DDevice->CreateOffscreenPlainSurface(
		lWidth,lHeight,
		//m_rtViewport.right-m_rtViewport.left,m_rtViewport.bottom-m_rtViewport.top,
		(D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'),
		D3DPOOL_DEFAULT,
		&m_pDirect3DSurfaceRender,
		NULL);
#endif
	if(FAILED(lRet))
		return -1;
	return 0;
}

bool CDecoder::play_pause()
{
    if (!m_hThread.IsSuspend()) {
        m_hThread.Suspend();
        Sleep(100);
    }
    return true;
}

bool CDecoder::play_resume()
{
    m_hThread.Resume();
    if (!m_hThread.IsRuning()) {
        m_hThread.Start();
    }
    return true;
}

bool CDecoder::init(const char* sFilePath, HWND hWnd, int playid)
{
	m_strCurFileName = sFilePath;
	m_nPlayedTime = 0;
	this->playid = playid;
    current_file.clear();
    current_file.set_file_name(sFilePath);
	m_hWnd = hWnd;
	FILE *fp_yuv=NULL;
	printf("play file: %s\n",sFilePath);

	if(ifmt_ctx) {
		avformat_close_input(&ifmt_ctx);
	}
	if(avformat_open_input(&ifmt_ctx,sFilePath,NULL,NULL)!=0){
		printf("Couldn't open input file:%s\n", sFilePath);
		return false;
	}
	if(avformat_find_stream_info(ifmt_ctx,NULL)<0){
		printf("Couldn't find file:%s information\n", sFilePath);
		return false;
	}

	m_videoindex = -1;
	m_nFrameRate = 25;
	for(int i=0; i<(int)ifmt_ctx->nb_streams; i++)
	{
		if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_videoindex=i;
			int avg_frame_rate = ifmt_ctx->streams[i]->avg_frame_rate.num / ifmt_ctx->streams[i]->avg_frame_rate.den;
            int r_frame_rate = ifmt_ctx->streams[i]->r_frame_rate.num / ifmt_ctx->streams[i]->r_frame_rate.den;
            m_nFrameRate = r_frame_rate;
            if (m_nFrameRate == 0 || m_nFrameRate >= 1000) {
                m_nFrameRate = 25;
            }
            time_base = ifmt_ctx->streams[i]->time_base;
		}
	}

	if(m_videoindex==-1)
	{
		printf("Didn't find a video stream.\n");
		return false;
	}
	m_pCodecCtx=ifmt_ctx->streams[m_videoindex]->codec;

	AVCodec *m_pCodec=avcodec_find_decoder(m_pCodecCtx->codec_id);
	if(avcodec_open2(m_pCodecCtx, m_pCodec,NULL)<0){
		printf("Could not open video codec.\n");
		return false;
	}
    int bit_rate = m_pCodecCtx->bit_rate;

	m_pFrame = av_frame_alloc();

	m_pFrame->data[0] = 0;
	m_pFrame->linesize[0] = 0;

	m_screen_w = m_pCodecCtx->width;
	m_screen_h = m_pCodecCtx->height;

    CreateImgConvert(AV_PIX_FMT_RGB24,m_screen_w,m_screen_h);
	InitD3D(m_hWnd,m_screen_w,m_screen_h);
    const char *filter_descr = "scale=78:24,transpose=cclock";
    initFilter(filter_descr);
#if OUTPUT_YUV420P
	fp_yuv=fopen("output.yuv","wb+");
#endif

 	m_hThread.SetParam(thread_play,this,10);
 	m_hThread.Start();
    return true;
}

bool CDecoder::internal_init(string sFilePath)
{
    m_strCurFileName = sFilePath;
	m_nPlayedTime = 0;

	printf("play file: %s\n",sFilePath.c_str());

	if(ifmt_ctx) {
		avformat_close_input(&ifmt_ctx);
	}
	if(avformat_open_input(&ifmt_ctx, sFilePath.c_str(), NULL, NULL) != 0) {
		printf("Couldn't open input file:%s\n", sFilePath.c_str());
		return false;
	}
	if(avformat_find_stream_info(ifmt_ctx, NULL)<0) {
		printf("Couldn't find file:%s information\n", sFilePath);
		return false;
	}

	m_videoindex = -1;
	m_nFrameRate = 25;
	for(int i=0; i<(int)ifmt_ctx->nb_streams; i++)
	{
		if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_videoindex=i;
			int avg_frame_rate = ifmt_ctx->streams[i]->avg_frame_rate.num / ifmt_ctx->streams[i]->avg_frame_rate.den;
            int r_frame_rate = ifmt_ctx->streams[i]->r_frame_rate.num / ifmt_ctx->streams[i]->r_frame_rate.den;
            m_nFrameRate = r_frame_rate;
            if (m_nFrameRate == 0 || m_nFrameRate >= 1000) {
                m_nFrameRate = 25;
            }
            time_base = ifmt_ctx->streams[i]->time_base;
		}
	}

	if(m_videoindex==-1)
	{
		printf("Didn't find a video stream in file:%s\n", sFilePath.c_str());
		return false;
	}
	m_pCodecCtx = ifmt_ctx->streams[m_videoindex]->codec;
	AVCodec *m_pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
	if(avcodec_open2(m_pCodecCtx, m_pCodec, NULL) < 0) {
		printf("Could not open video codec in file:%s\n", sFilePath.c_str());
		return false;
	}
    int bit_rate = m_pCodecCtx->bit_rate;

    m_pFrame = av_frame_alloc();
    m_pFrame->data[0] = 0;
    m_pFrame->linesize[0] = 0;

	m_screen_w = m_pCodecCtx->width;
	m_screen_h = m_pCodecCtx->height;

    CreateImgConvert(AV_PIX_FMT_RGB24,m_screen_w,m_screen_h);
	InitD3D(m_hWnd,m_screen_w,m_screen_h);
    const char *filter_descr = "scale=78:24,transpose=cclock";
#if OUTPUT_YUV420P
	fp_yuv=fopen("output.yuv","wb+");
#endif
 	m_hThread.SetParam(thread_play,this,10);
    return true;
}

bool CDecoder::CreateImgConvert(int pix_fmt, int width, int height)
{
    if(m_pimgConvert != NULL)
    {
        m_pimgConvert->DestroyConvert();
        delete m_pimgConvert;
        m_pimgConvert = NULL;
    }
    m_pimgConvert = new CImgConvert;


    if(!m_pimgConvert->InitConvert(pix_fmt, width, height))
    {
        DestroyImgConvert();
        return false;
    }
    return true;
}

bool CDecoder::DestroyImgConvert()
{
    if(m_pimgConvert != NULL)
    {
        m_pimgConvert->DestroyConvert();
        delete m_pimgConvert;
        m_pimgConvert = NULL;
    }
    return true;
}

void CDecoder::stop_decode()
{
	//Cleanup();
	m_hThread.Stop();
    Sleep(100);
	if(m_pFrame) {
		av_frame_free(&m_pFrame);
	}
    safe_freep(m_pSaveFile);
	if (ifmt_ctx != NULL) {
		avformat_close_input(&ifmt_ctx);
	}
	Cleanup();
	DestroyImgConvert();
}

void CDecoder::internal_stop()
{
    m_hThread.Stop();
    Sleep(100);
    if(m_pFrame) {
		av_frame_free(&m_pFrame);
	}
	if (ifmt_ctx != NULL) {
		avformat_close_input(&ifmt_ctx);
	}
}

bool CDecoder::play_speed(int speed)
{
    switch(speed)
    {
    case -2:
        m_nPlaySpeed = 4.0;
        break;
    case -1:
        m_nPlaySpeed = 2.0;
        break;
    case 0:
        m_nPlaySpeed = 1.0;
        break;
    case 1:
        m_nPlaySpeed = 0.5;
        break;
    case 2:
        m_nPlaySpeed = 0.25;
        break;
    default:
        m_nPlaySpeed = 1.0;
    }
    return true;
}

int CDecoder::parse()
{
    string _file_name = "";
    int ret = 0;
    int i;
    AVFormatContext	*fmt_ctx;
    AVPacket* pkt = av_packet_alloc();
    if (!pkt) {
        goto FAIL;;
    }
    if ((_file_name = pre_file.get_file_name()) == "") {
        goto FAIL;;
    }
    printf("parse file: %s\n", _file_name.c_str());
    fmt_ctx = avformat_alloc_context();
    if (!fmt_ctx) {
        goto FAIL;
    }
    if(avformat_open_input(&fmt_ctx, _file_name.c_str(), NULL, NULL) !=0 ) {
		printf("Couldn't open input file %s\n", _file_name.c_str());
		goto FAIL;
	}
	if(avformat_find_stream_info(fmt_ctx, NULL) < 0) {
		printf("Couldn't find file:%s information\n", _file_name.c_str());
		goto FAIL;
	}

    while(ret == 0) {
        ret = av_read_frame(fmt_ctx, pkt);
        if (ret == 0) {
            if (pkt->flags == AV_PKT_FLAG_KEY) {
                pre_file.add_keyframe(pkt->pos, pkt->dts);
                printf("parse key frame pos: %lld, pts: %lld, dts: %lld\n", pkt->pos, pkt->pts, pkt->dts);
            }
        }
    }

FAIL:
    if (pre_file.is_empty()) {
        pre_file.clear();
    }
    if (pkt) {
        av_packet_free(&pkt);
    }
    if (ifmt_ctx != NULL) {
		avformat_close_input(&fmt_ctx);
	}
    return -1;
}

int CDecoder::play()
{
	if(ifmt_ctx == NULL)
		return 0;
    int got_picture = 0;
    int ret = 0;
    static int i = 0;
    uint64_t total_used = 0;
    AVPacket* pkt = av_packet_alloc();
    int nSleep= 1000 / m_nFrameRate;
    int64_t duration = av_rescale_q(pkt->duration, time_base, TIME_BASE) / 1000;
    int64_t diff = pkt->dts - m_lastDts;
    if (duration == 0) {
        duration = av_rescale_q(diff, time_base, TIME_BASE) / 1000;
    }
    if (duration > DelayTime && duration < 1000) {
        nSleep = (duration - DelayTime)*m_nPlaySpeed;
	}
#if PRINT_
	printf("sleep time: %d\n",nSleep);
#endif
    ret = av_read_frame(ifmt_ctx, pkt);
    if (ret == 0) {
        if (pkt->stream_index == m_videoindex) {
            m_nPlayedTime = av_rescale_q(pkt->dts, time_base, TIME_BASE) / 1000;//ms
#if 0
			printf("packet time:%d,dts:%lld,pts:%lld,pos:%lld\n", m_nPlayedTime,pkt->dts, pkt->pts, pkt->pos);
#endif
            ret = avcodec_decode_video2(m_pCodecCtx, m_pFrame, &got_picture, pkt);
			if(m_bSaveVideo) {
               CAutoMutex lock(&mutex_save_file);
               if (m_pSaveFile) {
                    if (pkt->flags == AV_PKT_FLAG_KEY) {
                        b_strart_save = true;
                    }
                    if (b_strart_save) {
			            m_pSaveFile->write_packet(pkt);
                    }
               }
			}
            if (ret < 0) {
                printf("file: %s decode error:%d\n", current_file.get_file_name().c_str(), ret);
                goto FAIL;
            }
            if (pkt->flags == AV_PKT_FLAG_KEY) {
                current_file.add_keyframe(pkt->pos, pkt->dts);
                printf("key frame pos: %lld, pts: %lld, dts: %lld, index: %d\n", pkt->pos, pkt->pts, pkt->dts, i);
                i = 0;
            }
            i++;
            if (got_picture) {
                current_file.set_pos(m_pFrame->pkt_pos);
                m_pFrame->pts = m_pFrame->best_effort_timestamp;
                if (m_pFrame->pict_type == AV_PICTURE_TYPE_I) {
                    //current_file.add_keyframe(m_pFrame->pkt_pos, m_pFrame->pkt_pts);
                    printf("frame pts:%lld,pkt_pts:%lld,pkt_dts:%lld, pkt_pos:%lld\n",
                        m_pFrame->pts, m_pFrame->pkt_pts, m_pFrame->pkt_dts, m_pFrame->pkt_pos);
                }
#if 0
                printf("frame pts:%lld,pkt_pts:%lld,pkt_dts:%lld, pkt_pos:%lld\n",
                    m_pFrame->pts, m_pFrame->pkt_pts, m_pFrame->pkt_dts, m_pFrame->pkt_pos);
#endif
                DisPlayFrame(m_pFrame);
                my_sleep_ms(s, nSleep, 0);
                m_lastDts = pkt->dts;
                if (m_seekTime > 0) {
                    seek(m_seekTime, 0);
                    m_seekTime = -1;
                }
                got_picture = 0;
            }
        }
    } else if (ret == AVERROR_EOF) {
        //flush decoder
        AVFrame *frame;
        frame = av_frame_alloc();
        if (!frame) {
            goto FAIL;
        }
        while (true) {
            AVPacket avpkt;
            av_init_packet(&avpkt);
            avpkt.data = NULL;
            avpkt.size = 0;
            ret = avcodec_decode_video2(m_pCodecCtx, frame, &got_picture, &avpkt);
            if (ret < 0 || !got_picture) {
                break;
            }
            frame->pts = frame->best_effort_timestamp;
            if (frame->pts <= m_pFrame->pts) {
                continue;
            }
            av_frame_unref(m_pFrame);
            av_frame_ref(m_pFrame, frame);
            current_file.set_pos(m_pFrame->pkt_pos);
            DisPlayFrame(m_pFrame);
            my_sleep_ms(s, nSleep, 0);
            printf("play flush decoder\n");
        }
        av_frame_free(&frame);
        //read next file
		if(next_file_callback) {
            pre_file.clear();
            pre_file = current_file;
            current_file.clear();
			next_file_callback(m_strCurFileName,file_end_param);
		}
		printf("next file: %d\n",i);
    } else {
        //error
    }
FAIL:
    av_packet_free(&pkt);
    return 1;
}

bool CDecoder::snapshot(const char* sFilePath)
{
	m_bCapTure = true;
    if(!strlen(sFilePath))
        return false;
    if(m_snapPic.linesize[0]&&m_pimgConvert)
    {

        char fullFileName[MAX_PATH];
        char date[50];
        char fileName[50];
        memset(fullFileName, 0, MAX_PATH);
        memset(date, 0, 50);
        memset(fileName, 0, 50);
        SYSTEMTIME time;
        GetLocalTime(&time);

        sprintf(fileName, "\\%04d_%02d_%02d_%02d_%02d_%02d.bmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
        strcat(fullFileName,sFilePath);
        strcat(fullFileName,fileName);
        AVPicture picTemp;
        ZeroMemory(&picTemp, sizeof(AVPicture));
        picTemp.data[0]		= m_snapPic.data[0];
        picTemp.data[1]		= m_snapPic.data[1];
        picTemp.data[2]		= m_snapPic.data[2];
        picTemp.data[3]		= m_snapPic.data[3];
        picTemp.linesize[0] = m_snapPic.linesize[0];
        picTemp.linesize[1] = m_snapPic.linesize[1];
        picTemp.linesize[2] = m_snapPic.linesize[2];
        picTemp.linesize[3] = m_snapPic.linesize[3];
        AVPicture * pFrameRGB = (AVPicture *)&picTemp;
        pFrameRGB = (AVPicture *) m_pimgConvert->ConvertImg(AV_PIX_FMT_YUV420P,&picTemp,m_screen_w,m_screen_h);
        if(m_screen_w*m_screen_h<704*576)
        {
            m_pimgConvert->SaveToFile(sFilePath,pFrameRGB,704,576);
        }
        else
        {
            m_pimgConvert->SaveToFile(sFilePath,pFrameRGB,m_screen_w,m_screen_h);
        }
    }
    return true;
}
void CDecoder::seek( int64_t seekTime,int streamIndex)
{
    int defaultStreamIndex = av_find_default_stream_index(ifmt_ctx);
    AVRational time_base = ifmt_ctx->streams[defaultStreamIndex]->time_base;
    int64_t newDts = ifmt_ctx->streams[defaultStreamIndex]->start_time + av_rescale(seekTime, time_base.den, time_base.num);
    if(newDts > ifmt_ctx->streams[streamIndex]->cur_dts)
    {
        av_seek_frame(ifmt_ctx, defaultStreamIndex, newDts, AVSEEK_FLAG_BACKWARD);
    }
    else
    {
        av_seek_frame(ifmt_ctx, defaultStreamIndex, newDts, AVSEEK_FLAG_BACKWARD);
    }
}
bool CDecoder::play_seek(unsigned int ntime)
{
	m_seekTime = 5;
	return true;
}

bool CDecoder::PreSingleFrame()
{
    int ret = 0;
    int i = 0;
    play_pause();
    int64_t cur_pos;
    bool reach_header;
    bool at_the_end = false;
    int64_t last_keyframe_dts;
    int64_t first_keyframe_pos;
    FILE_STATUS status = FS_DEFAULT;
    int64_t cur_pts = m_pFrame->pts;
BEGIN:
    cur_pos = current_file.get_pos();
    printf("file:%s\n",current_file.get_file_name().c_str());
    //first_keyframe_pos = current_file.get_first_keyframe_pos();
    last_keyframe_dts = current_file.get_last_keyframe_dts(cur_pos, reach_header);
    //last_keyframe_dts = av_rescale_q(last_keyframe_dts, time_base, TIME_BASE) / 1000;
    if (last_keyframe_dts == FIND_KEYFRAME_ERROR_NO_FOUND) {
        return false;
    }
    if (last_keyframe_dts == FIND_KEYFRAME_ERROR_LAST_FILE) {
        //TODO: open pre file?
        string _pre_file_name = pre_file.get_file_name();
        FILE_STATUS status = pre_file.get_status();
        if (_pre_file_name == "" || status != FS_NORMAL) {
            return false;
        }
        internal_stop();
        ret = internal_init(_pre_file_name);
        if (!ret) {
            return false;
        }
        current_file.clear();
        current_file = pre_file;
        pre_file.clear();
        at_the_end = true;
        cur_pos = current_file.get_end_keyframe_pos() + 1;
        current_file.set_pos(cur_pos);
        cur_pts = 0x7fffffffffffffff;
        goto BEGIN;
    }
    if ((status = pre_file.get_status()) == FS_DEFAULT) {
        // prepare to open pre file
        if (pre_file_callback) {
            pre_file.clear();
            string pre_file_name = pre_file_callback(m_strCurFileName,file_end_param);
            if (pre_file_name == "") {
                pre_file.set_status(FS_NOFOUND);
            } else {
                pre_file.set_file_name(pre_file_name);
                thread_parse.SetParam(thread_parse_file,this,10);
 	            thread_parse.Start();
                pre_file.set_status(FS_NORMAL);
            }
        }
    }
    printf("current frame pts:%lld, last_keyframe_pos:%lld, cur_pos:%lld\n",m_pFrame->pts, last_keyframe_dts, cur_pos);
    //av_seek_frame(ifmt_ctx, m_videoindex, 0, AVSEEK_FLAG_BACKWARD);
    ret = av_seek_frame(ifmt_ctx, m_videoindex, last_keyframe_dts, AVSEEK_FLAG_ANY );
    if (ret < 0) {
        printf("seek error %d\n", ret);
    }
    avcodec_flush_buffers(ifmt_ctx->streams[m_videoindex]->codec);  // 清空缓冲

    int got_picture = 0;
    AVPacket* pkt;
    AVFrame *frame;
    AVFrame *last_frame;
    bool has_found = false;
    pkt = av_packet_alloc();
    if (!pkt) {
        goto FAIL;
    }
    frame = av_frame_alloc();
    if (!frame) {
        goto FAIL;
    }
    last_frame = av_frame_alloc();
    if (!last_frame) {
        goto FAIL;
    }
    while (true) {
        ret = av_read_frame(ifmt_ctx, pkt);
        if (ret == 0) {
            if (pkt->stream_index == m_videoindex) {
                printf("pkt pts:%lld,dts:%lld,pos:%lld\n",pkt->pts,pkt->dts, pkt->pos);
                ret = avcodec_decode_video2(m_pCodecCtx, frame, &got_picture, pkt);
                if (ret < 0) {
                    goto FAIL;
                }
                if (ret >= 0 && got_picture) {
                    //frame->pts = frame->best_effort_timestamp;
                    frame->pts = av_frame_get_best_effort_timestamp(frame);
                    printf("frame pts:%lld, pkt_dts:%lld, pos:%lld\n", frame->pts, frame->pkt_dts, frame->pkt_pos);
                    if (frame->pts >= cur_pts) {
                        has_found = true;
                        break;
                    } else {
                        i++;
                        av_frame_unref(last_frame);
                        av_frame_ref(last_frame, frame);
                    }
                }
            }
        } else if (ret == AVERROR_EOF) {
            while (true) {
                AVPacket avpkt;
                av_init_packet(&avpkt);
                avpkt.data = NULL;
                avpkt.size = 0;
                ret = avcodec_decode_video2(m_pCodecCtx, frame, &got_picture, &avpkt);
                if (ret < 0 || !got_picture) {
                    break;
                }
                frame->pts = frame->best_effort_timestamp;
                printf("frame pts:%lld, pkt_dts:%lld, pos:%lld\n", frame->pts, frame->pkt_dts, frame->pkt_pos);
                if (frame->pts <= last_frame->pts) {
                    continue;
                }
                if (frame->pts >= cur_pts) {
                    has_found = true;
                    break;
                }
                else {
                    i++;
                    av_frame_unref(last_frame);
                    av_frame_ref(last_frame, frame);
                }
            }
            break;
        } else {
            goto FAIL;
        }
    }

    if (has_found || at_the_end) {
        printf("found frame pts:%lld, cur_pts:%lld, total skip %d\n",last_frame->pts, cur_pts, i);
        av_frame_unref(m_pFrame);
        av_frame_ref(m_pFrame, last_frame);
        current_file.set_pos(m_pFrame->pkt_pos);
        DisPlayFrame(m_pFrame);
    }
FAIL:
    av_frame_free(&last_frame);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    return true;
}

bool CDecoder::NextSingleFrame()
{
    play_pause();
	int ret = 0;
	int got_picture = -1;
    AVFrame *frame;
    frame = av_frame_alloc();
    if (!frame) {
        goto FAIL;
    }
	AVPacket* pkt = av_packet_alloc();
    while (true) {
        ret = av_read_frame(ifmt_ctx, pkt);
        if (ret == 0) {
            if (pkt->stream_index == m_videoindex) {
                if (pkt->flags == AV_PKT_FLAG_KEY) {
                    current_file.add_keyframe(pkt->pos, pkt->dts);
                }
                ret = avcodec_decode_video2(m_pCodecCtx, m_pFrame, &got_picture, pkt);
                if (ret < 0) {
                    goto FAIL;
                }
                if (got_picture) {
                    m_pFrame->pts = m_pFrame->best_effort_timestamp;
                    current_file.set_pos(m_pFrame->pkt_pos);
                    DisPlayFrame(m_pFrame);
                    break;
                }
            }
        }
        else if (ret == AVERROR_EOF) {
            //fulsh decoder
            AVPacket avpkt;
            av_init_packet(&avpkt);
            avpkt.data = NULL;
            avpkt.size = 0;
            ret = avcodec_decode_video2(m_pCodecCtx, frame, &got_picture, &avpkt);
            if (ret >= 0 && got_picture) {
                frame->pts = frame->best_effort_timestamp;
                if (frame->pts <= m_pFrame->pts) {
                    break;
                }
                av_frame_unref(m_pFrame);
                av_frame_ref(m_pFrame, frame);
                current_file.set_pos(m_pFrame->pkt_pos);
                DisPlayFrame(m_pFrame);
            }
            else {
                //read next file
                if (next_file_callback) {
                    pre_file.clear();
                    pre_file = current_file;
                    current_file.clear();
                    next_file_callback(m_strCurFileName, file_end_param);
                }
                printf("next file\n");
            }
        }
        else {
            //error
            goto FAIL;
        }
    }
FAIL:
    av_frame_free(&frame);
	av_packet_free(&pkt);
    return true;
}


BOOL CDecoder::SaveToFile(const char * pFilename, unsigned char * pSurFrame, int w, int h)
{
	unsigned char *lpbuffer;
	unsigned char *word;
	//AvPicture * pFrame = (AvPicture*)pSurFrame;
	//unsigned char * pDIBImage = pFrame->data[0];
	unsigned char * pDIBImage  = pSurFrame;
	BITMAPFILEHEADER  bf; //bmp文件头
	BITMAPINFOHEADER  bi; //bmp信息头
	bi.biSize = 40;  //位图信息头大小
	bi.biWidth = w;  //图像宽度
	bi.biHeight = h;  //图像高度
	bi.biPlanes = 1;   //位平面树=1
	bi.biBitCount = 24;  //单位像素的位数
	bi.biCompression = 0;  //图片的压缩属性，bmp不压缩，等于0
	//bi.biSizeImage = WIDTHBYTES(bi.biWidth * bi.biBitCount) * bi.biHeight;
	bi.biSizeImage = w * h * bi.biBitCount;
	//表示bmp图片数据区的大小，当上一个属性biCompression等于0时，这里的值可以省略不填

	bi.biXPelsPerMeter = 0; //水平分辨率
	bi.biYPelsPerMeter = 0; //垂直分辨率
	bi.biClrUsed = 0;   //表示使用了多少哥颜色索引表，一般biBitCount属性小于16才会用到，等于0时表示有2^biBitCount个颜色索引表
	bi.biClrImportant = 0;  //表示有多少个重要的颜色，等于0时表示所有颜色都很重要

	//Set BITMAPFILEHEADER  设置bmp图片的文件头格式
	bf.bfType = 0x4d42;  //2个字节，恒等于0x4d42，ascii字符“BM”
	bf.bfSize = 54 + bi.biSizeImage; //文件大小，以4个字节为单位
	bf.bfReserved1 = 0;  //备用
	bf.bfReserved2 = 0;  //备用
	bf.bfOffBits = 54;   //数据区在文件中的位置偏移量

	FILE *  fp =  fopen(pFilename, "wb");
	if(fp == NULL)
	{
		int err = GetLastError();
		return FALSE;
	}
	fwrite(&bf, 14, 1, fp); //向文件中写入图片文件头
	fwrite(&bi, 40, 1, fp); //向文件中写入图片信息头
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
		lpbuffer -= w*3; // 指针转到上一行的开始
	}
	fclose(fp);
	return TRUE;
}
bool CDecoder::play_stop()
{
	stop_decode();
	return true;
}
bool CDecoder::play_save_start(const char* sSavePath)
{
    int ret;
    if(sSavePath == NULL)
		return false;
	m_strSaveFile = string(sSavePath);
    CAutoMutex lock(&mutex_save_file);
    safe_freep(m_pSaveFile);
    m_pSaveFile = new CSaveAsFile;
    //ifmt_ctx可能需要加锁
    ret = m_pSaveFile->init(m_strSaveFile,playid,m_screen_w,m_screen_h,m_nFrameRate,ifmt_ctx);
    if (!ret) {
        safe_freep(m_pSaveFile);
        return false;
    }
    m_bSaveVideo = true;
    b_strart_save = false;
	return true;
}
bool CDecoder::play_save_stop()
{
	m_bSaveVideo = false;
    b_strart_save = false;
    CAutoMutex lock(&mutex_save_file);
	safe_freep(m_pSaveFile);
	return true;
}

int64_t CDecoder::get_cur_play_time()
{
	return m_nPlayedTime;
}

string CDecoder::get_cur_play_file()
{
	return m_strCurFileName;
}
