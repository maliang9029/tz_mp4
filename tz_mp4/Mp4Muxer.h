#pragma once
#ifndef _MP4_MUXER_H_
#define _MP4_MUXER_H_
#include "common.h"

extern "C"
{
    #include "ffmpeg\include\libavcodec\avcodec.h"
    #include "ffmpeg\include\libavformat\avformat.h"
    #include "ffmpeg\include\libavutil\channel_layout.h"
    #include "ffmpeg\include\libavutil\common.h"
    #include "ffmpeg\include\libavutil\imgutils.h"
    #include "ffmpeg\include\libswscale\swscale.h"
    #include "ffmpeg\include\libavutil\imgutils.h"
    #include "ffmpeg\include\libavutil\opt.h"
    #include "ffmpeg\include\libavutil\mathematics.h"
    #include "ffmpeg\include\libavutil\samplefmt.h"
    #include "ffmpeg\include\libavutil\avutil.h"
};
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "postproc.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")

enum HEVCNALUnitType {
    HEVC_NAL_TRAIL_N    = 0,
    HEVC_NAL_TRAIL_R    = 1,
    HEVC_NAL_TSA_N      = 2,
    HEVC_NAL_TSA_R      = 3,
    HEVC_NAL_STSA_N     = 4,
    HEVC_NAL_STSA_R     = 5,
    HEVC_NAL_RADL_N     = 6,
    HEVC_NAL_RADL_R     = 7,
    HEVC_NAL_RASL_N     = 8,
    HEVC_NAL_RASL_R     = 9,
    HEVC_NAL_VCL_N10    = 10,
    HEVC_NAL_VCL_R11    = 11,
    HEVC_NAL_VCL_N12    = 12,
    HEVC_NAL_VCL_R13    = 13,
    HEVC_NAL_VCL_N14    = 14,
    HEVC_NAL_VCL_R15    = 15,
    HEVC_NAL_BLA_W_LP   = 16,
    HEVC_NAL_BLA_W_RADL = 17,
    HEVC_NAL_BLA_N_LP   = 18,
    HEVC_NAL_IDR_W_RADL = 19,
    HEVC_NAL_IDR_N_LP   = 20,
    HEVC_NAL_CRA_NUT    = 21,
    HEVC_NAL_IRAP_VCL22 = 22,
    HEVC_NAL_IRAP_VCL23 = 23,
    HEVC_NAL_RSV_VCL24  = 24,
    HEVC_NAL_RSV_VCL25  = 25,
    HEVC_NAL_RSV_VCL26  = 26,
    HEVC_NAL_RSV_VCL27  = 27,
    HEVC_NAL_RSV_VCL28  = 28,
    HEVC_NAL_RSV_VCL29  = 29,
    HEVC_NAL_RSV_VCL30  = 30,
    HEVC_NAL_RSV_VCL31  = 31,
    HEVC_NAL_VPS        = 32,
    HEVC_NAL_SPS        = 33,
    HEVC_NAL_PPS        = 34,
    HEVC_NAL_AUD        = 35,
    HEVC_NAL_EOS_NUT    = 36,
    HEVC_NAL_EOB_NUT    = 37,
    HEVC_NAL_FD_NUT     = 38,
    HEVC_NAL_SEI_PREFIX = 39,
    HEVC_NAL_SEI_SUFFIX = 40,
};

class CMessage
{
public:
    CMessage();
    ~CMessage();
public:
    int create(AVPacket *inpkt);
public:
    AVPacket *pkt;
};

class CMessageArray
{
public:
    CMessageArray(int max_msgs);
    virtual ~CMessageArray();
public:
    CMessage** msgs;
    int max;
public:
    void free(int count);
private:
    void zero(int count);
};

class FastVector
{
private:
    CMessage** msgs;
    int nb_msgs;
    int count;
public:
    FastVector();
    ~FastVector();
public:
    int size();
    int begin();
    int end();
    CMessage** data();
    CMessage* at(int index);
    void clear();
    void erase(int _begin, int _end);
    void push_back(CMessage* msg);
    void free();
};

class CMp4Segment
{
public:
    CMp4Segment(string path, string file_name, int w, int h, int r);
    ~CMp4Segment();
public:
    string full_path;
    string file_name;
    int64_t segment_start_time;
    int64_t segment_end_time;
    int64_t duration;//s
private:
    AVFormatContext *o_fmt_ctx;
    AVStream *o_video_stream;
    AVCodecContext *codec_ctx;
    string path;
    int width;
    int height;
    int framerate;
public:
    int init_segment();
    int64_t get_duration();
    void update_duration(int64_t dts);
    int ffmpeg_muxing(CMessage* msg);
};

class CMp4Muxer
{
public:
    CMp4Muxer(string path, int playid, int w, int h, int r);
    ~CMp4Muxer();
public:
    string path;
    int playid;
    int width;
    int height;
    int framerate;
    int64_t max_fragment;
public:
    bool init_muxing(int fragment = DEFAULT_MAX_FRAGMENT, int record_period = DEFAULT_RECORD_PERIOD, int file_period = DEFAULT_FILE_PERIOD);
    bool write_packet(AVPacket* pkt);
    static int thread_muxing(LPVOID lParam);
    int do_muxing();
    void strat_muxing();
    bool parse_packet(const char* data, unsigned int len);
    void set_record_timing(int64_t timing);
private:
    int dump_packets(int max_count, CMessage** pmsg, int& count);
    bool create_multi_directory(void);
    bool is_directory_exsits(const char* path);
    int segment_open();
    int segment_close();
    int reap_segment();
    int segment_shrink();
    int refresh_file_list();
    int mp4_muxing(CMessage* msg);
    void init_segments();
    void init_segments_ori();
private:
    FastVector msgs;
    CVorxMutex mutex;
    int64_t record_timing;
    CMp4Segment* current;
    //vector<CMp4Segment*> segments;
    //vector<CMp4Segment*> segments_ori;
    CVorxMutex mutex_seg;
    vector<string> segments;
    CVorxMutex mutex_seg_ori;
    vector<string> segments_ori;
    CVorxThread thread;
    AVCodecParserContext *parser;
    AVCodecContext *c;
    int record_window;
    int remove_window;
    string index_filename;
    string index_all_filename;
    bool is_muxing_start;
    int64_t next_dts;
    int64_t dts;
    int64_t next_pts;
    int64_t pts;

};
#endif//_MP4_MUXER_H_

