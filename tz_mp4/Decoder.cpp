#include "Decoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <atlstr.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

CRecordInfo::CRecordInfo()
{
    file_name = "";
    pos = 0;
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
}

int64_t CRecordInfo::get_last_keyframe_dts(int64_t pos)
{
    int64_t keyframe_dts = FIND_KEYFRAME_ERROR_NO_FOUND;
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
    }
    return keyframe_dts;
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
}

void CRecordInfo::set_pos(int64_t pos)
{
    this->pos = pos;
}
int64_t CRecordInfo::get_pos()
{
    return pos;
}

CDecoder::CDecoder(void)
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
,m_pOnVideoDataCallBack(NULL)
,m_pOnVideoDataParam(NULL)
,m_pimgConvert(NULL)
,m_lastDts(0)
,m_lastPts(0)
,m_seekTime(0)
,m_pFormatCtx(NULL)
,m_pCodecCtx(NULL)
,m_videoindex(0)
,m_bSaveVideo(false)
,m_pSaveFile(NULL)
,m_bKey(false)
{

}

CDecoder::~CDecoder(void)
{
}


void CDecoder::SetVideoCallBack(ON_VEDIO_DATA pCallBack,void* lUserData)
{
    m_pOnVideoDataCallBack =  pCallBack;
    m_pOnVideoDataParam = lUserData;
}

void CDecoder::DisPlayFrame(AVFrame* frame)
{
    VPicture tempPic;
    for (int i=0; i<V_DATA_POINTERS; i++)
    {
        tempPic.data[i] = frame->data[i];
        tempPic.linesize[i] = frame->linesize[i];
    }
    memcpy(&m_snapPic,&tempPic,sizeof(VPicture));
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

int CDecoder::thread_fun(LPVOID lParam)
{
	CDecoder* pThis = (CDecoder*)lParam;
	if(pThis)
	{
        return pThis->play();
	}
	return 0;
}

int CDecoder::initFilter(const char* filters_descr)
{
//	char sreg[512]={0};
//
//	int ret = 0;
//
//	const AVFilter* bufferSrc = avfilter_get_by_name("buffer");
//	const AVFilter *buffersink = avfilter_get_by_name("ffbuffersink");
//	AVFilterInOut *outputs = avfilter_inout_alloc();
//	AVFilterInOut *inputs  = avfilter_inout_alloc();
//	enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
////	AVBufferSinkParams *buffersink_params=NULL;
//
//	m_pFilter_graph = avfilter_graph_alloc();
//
//	/* buffer video source: the decoded frames from the decoder will be inserted here. */
//
//	sprintf(sreg, /*sizeof(sreg),*/
//		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
//		m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
//		m_pCodecCtx->time_base.num, m_pCodecCtx->time_base.den,
//		m_pCodecCtx->sample_aspect_ratio.num, m_pCodecCtx->sample_aspect_ratio.den);
//
//
//	//buffersink_params = av_buffersink_params_alloc();
    char args[512];
    int ret = 0;
    const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    AVRational time_base = m_pFormatCtx->streams[m_videoindex]->time_base;
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
	m_hThread.Suspend();
    return true;
}

bool CDecoder::play_resume()
{
    m_hThread.Resume();
    return true;
}

bool CDecoder::init(const char* sFilePath,HWND hWnd)
{
    current_file.set_file_name(sFilePath);
	m_hWnd = hWnd;
	FILE *fp_yuv=NULL;
	int y_size=0;
	unsigned char *out_buffer=NULL;
	struct SwsContext *img_convert_ctx=NULL;

	av_register_all();
	avformat_network_init();
	m_pFormatCtx = avformat_alloc_context();
	if(avformat_open_input(&m_pFormatCtx,sFilePath,NULL,NULL)!=0){
		printf("Couldn't open input stream.\n");
		return false;
	}
	if(avformat_find_stream_info(m_pFormatCtx,NULL)<0){
		printf("Couldn't find stream information.\n");
		return false;
	}

	m_videoindex = -1;
	m_nFrameRate = 25;
	for(int i=0; i<(int)m_pFormatCtx->nb_streams; i++)
	{
		if(m_pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			m_videoindex=i;
			m_nFrameRate = m_pFormatCtx->streams[i]->avg_frame_rate.num / m_pFormatCtx->streams[i]->avg_frame_rate.den;
		}

	}

	if(m_videoindex==-1)
	{
		printf("Didn't find a video stream.\n");
		return false;
	}
	m_pCodecCtx=m_pFormatCtx->streams[m_videoindex]->codec;
    AVRational aa;
    aa.num =1;
    aa.den = 1000000;
    double times = av_q2d(aa)*m_pFormatCtx->duration;

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
   /* m_pD3DRender = D3D_VIDEO_RENDER::D3D_Video_Render_Create();
    if ( !m_pD3DRender->CreateBackBuffer ( m_hWnd, m_screen_w, m_screen_h ) )
    {
        return false;
    }
    RECT rect = {0,0,m_screen_w,m_screen_h};
    bool bret = m_pD3DRender->CreateImageBuffer ( &m_dwImageIndex, m_screen_w, m_screen_h, ID3DVRInterface::CS_I420, &rect ) ;
    if ( !bret )
    {
        return false;
    }*/
    CreateImgConvert(AV_PIX_FMT_RGB24,m_screen_w,m_screen_h);
	InitD3D(m_hWnd,m_screen_w,m_screen_h);
    const char *filter_descr = "scale=78:24,transpose=cclock";
    initFilter(filter_descr);
#if OUTPUT_YUV420P
	fp_yuv=fopen("output.yuv","wb+");
#endif

 	m_hThread.SetParam(thread_fun,this,0);
 	m_hThread.Start();
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

void CDecoder::stopdecoder()
{
	//Cleanup();
	m_hThread.Stop();

	if (m_pFrame != NULL)
	{
		av_free(m_pFrame);
		m_pFrame = NULL;
	}
	if (m_pCodecCtx != NULL)
	{
		avcodec_close(m_pCodecCtx);
		av_free(m_pCodecCtx);
		m_pCodecCtx = NULL;
	}
	if(m_pSaveFile)
	{
		delete m_pSaveFile;
		m_pSaveFile = NULL;
	}
	/*if (m_pFormatCtx != NULL)
	{
		avformat_free_context(m_pFormatCtx);
	}*/
	Cleanup();
	DestroyImgConvert();
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

int CDecoder::play()
{
    int got_picture = 0;
    int ret = 0;
    AVPacket* pkt = av_packet_alloc();
    //AVFrame *filt_frame = av_frame_alloc();
    ret = av_read_frame(m_pFormatCtx, pkt);
    if (ret == 0) {
        if (pkt->stream_index == m_videoindex) {
            printf("packet dts:%lld,pts:%lld,pos:%lld\n", pkt->dts, pkt->pts, pkt->pos);
            ret = avcodec_decode_video2(m_pCodecCtx, m_pFrame, &got_picture, pkt);
			if(m_pFrame->key_frame) {
				m_bKey = true;
			}
			if(m_bSaveVideo && m_pSaveFile && m_bKey) {
				m_pSaveFile->write_packet(pkt);
			}
            if (ret < 0) {
                //decode error
                //TODO:exit?
                printf("file: %s decode error:%d\n", current_file.get_file_name().c_str(), ret);
                goto FAIL;
            }
            current_file.set_pos(pkt->pos);
            if (pkt->flags == AV_PKT_FLAG_KEY) {
                current_file.add_keyframe(pkt->pos, pkt->dts);
            }
            if (got_picture) {
                m_pFrame->pts = m_pFrame->best_effort_timestamp;
                printf("frame pts:%lld,pkt_pts:%lld,pkt_dts:%lld, pkt_pos:%lld\n",
                    m_pFrame->pts, m_pFrame->pkt_pts, m_pFrame->pkt_dts, m_pFrame->pkt_pos);

                DisPlayFrame(m_pFrame);
                // av_frame_unref(filt_frame);
                  //Sleep(20/(DWORD)pThis->m_nPlayRate);
                  /*int nRate = (1000/pThis->m_nFrameRate)*pThis->m_nPlaySpeed;
                  Sleep(1000/pThis->m_nFrameRate);*/
                int64_t diff = pkt->dts * 2 - m_lastDts;
                int64_t duration = diff * 1000 / (m_pFormatCtx->streams[m_videoindex]->time_base.den
                    / m_pFormatCtx->streams[m_videoindex]->time_base.num);
                if (duration > DelayTime && duration < 1000) {
                    int nSleep = (duration - DelayTime)*m_nPlaySpeed;
                    Sleep(40);
                }
                m_lastDts = pkt->dts;

                if (m_seekTime > 0) {
                    seek(m_seekTime, 0);
                    m_seekTime = -1;
                }
                got_picture = 0;
            }
        }
    } else if (ret == AVERROR_EOF){
        //read next file
    } else {
        //error
    }
FAIL:
    av_packet_free(&pkt);
    return 0;
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
            m_pimgConvert->SaveToFile(fullFileName,pFrameRGB,704,576);
        }
        else
        {
            m_pimgConvert->SaveToFile(fullFileName,pFrameRGB,m_screen_w,m_screen_h);
        }
    }
    return true;
}
void CDecoder::seek( int64_t seekTime,int streamIndex)
{
    int defaultStreamIndex = av_find_default_stream_index(m_pFormatCtx);
    AVRational time_base = m_pFormatCtx->streams[defaultStreamIndex]->time_base;
    int64_t newDts = m_pFormatCtx->streams[defaultStreamIndex]->start_time + av_rescale(seekTime, time_base.den, time_base.num);
    if(newDts > m_pFormatCtx->streams[streamIndex]->cur_dts)
    {
        av_seek_frame(m_pFormatCtx, defaultStreamIndex, newDts, AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD);
    }
    else
    {
        av_seek_frame(m_pFormatCtx, defaultStreamIndex, newDts, AVSEEK_FLAG_ANY | AVSEEK_FLAG_BACKWARD);
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
    Sleep(100);
    int64_t cur_pos = current_file.get_pos();
    int64_t last_keyframe_pos = current_file.get_last_keyframe_dts(cur_pos);
    if (last_keyframe_pos == FIND_KEYFRAME_ERROR_LAST_FILE || last_keyframe_pos == FIND_KEYFRAME_ERROR_NO_FOUND) {
        //TODO: open pre file?
        return false;
    }
    int64_t cur_pts = m_pFrame->pts;
    printf("current frame pts:%lld, last_keyframe_pos:%lld, cur_pos:%lld\n",m_pFrame->pts, last_keyframe_pos, cur_pos);
    //av_seek_frame(m_pFormatCtx, m_videoindex, 0, AVSEEK_FLAG_BACKWARD);
    ret = av_seek_frame(m_pFormatCtx, m_videoindex, last_keyframe_pos, AVSEEK_FLAG_ANY );
    if (ret < 0) {
        printf("seek error %d\n", ret);
    }
    avcodec_flush_buffers(m_pFormatCtx->streams[m_videoindex]->codec);  // 清空缓冲
    int got_picture = -1;

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
        ret = av_read_frame(m_pFormatCtx, pkt);
        if (ret == 0) {
            if (pkt->stream_index == m_videoindex) {
                //printf("pkt dts:%lld,pos:%lld\n",pkt->dts, pkt->pos);
                ret = avcodec_decode_video2(m_pCodecCtx, frame, &got_picture, pkt);
                if (ret < 0) {
                    goto FAIL;
                }
                if (got_picture) {
                    //frame->pts = frame->best_effort_timestamp;
                    if (frame->pts >= cur_pts) {
                        printf("found frame pts:%lld, cur_pts:%lld, total skip %d\n",last_frame->pts, cur_pts, i);
                        has_found = true;
                        break;
                    } else {
                        i++;
                        current_file.set_pos(pkt->pos);
                        av_frame_unref(last_frame);
                        av_frame_ref(last_frame, frame);
                    }

                }
            }
        } else {
            goto FAIL;
        }
    }

    if (has_found) {
        av_frame_unref(m_pFrame);
        av_frame_ref(m_pFrame, last_frame);
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
    Sleep(100);
	int ret = 0;
	int got_picture = -1;
	AVPacket* pkt = av_packet_alloc();
    while (true) {
        ret = av_read_frame(m_pFormatCtx, pkt);
        if (ret == 0) {
            if (pkt->stream_index == m_videoindex) {
                if (pkt->flags == AV_PKT_FLAG_KEY) {
                    current_file.add_keyframe(pkt->pos, pkt->dts);
                }
                ret = avcodec_decode_video2(m_pCodecCtx, m_pFrame, &got_picture, pkt);
                if (ret < 0)
                {
                    goto FAIL;
                }
                if (got_picture)
                {
                    current_file.set_pos(pkt->pos);
                    DisPlayFrame(m_pFrame);
                    break;
                }
            }
        }
        else if (ret == AVERROR_EOF) {
            //read next file
            goto FAIL;
        }
        else {
            //error
            goto FAIL;
        }
    }
FAIL:
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
	stopdecoder();
	return true;
}
bool CDecoder::play_save_start(const char* sSavePath)
{
	if(sSavePath == NULL )
		return false;
	m_strSaveFile = string(sSavePath);
	m_bSaveVideo = true;
	if(!m_pSaveFile)
		m_pSaveFile = new CSaveAsFile;
	if(m_pSaveFile)
	{
		m_pSaveFile->init(m_strSaveFile,m_screen_w,m_screen_h,m_nFrameRate);
	}
	return true;
}
bool CDecoder::play_save_stop()
{
	m_bSaveVideo = false;
	m_bKey = false;
	if(m_pSaveFile)
	{
		m_pSaveFile->stop_write();
	}
	return true;
}
