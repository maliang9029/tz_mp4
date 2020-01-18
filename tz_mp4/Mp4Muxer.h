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
};
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "postproc.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")

class CMessage
{
public:
    CMessage();
    ~CMessage();
public:
    void create(char* data, int size);
public:
    char* data;
    int size;
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
    CMp4Segment();
    ~CMp4Segment();
public:
    string full_path;
    string file_name;
    INT64 segment_start_time;
    INT64 segment_end_time;
    double duration;//s
    double get_duration();
    void update_duration(double segment_duration);

};

class CMp4Muxer
{
public:
    CMp4Muxer();
    ~CMp4Muxer();
public:
    string path;
    int playid;
    int width;
    int height;
    int framerate;
public:
    bool write_packet(const char* data, unsigned int len);
    static int thread_muxing(LPVOID lParam);
    int do_muxing();
private:
    int dump_packets(int max_count, CMessage** pmsg, int& count);
    bool create_multi_directory(void);
    bool is_directory_exsits(const char* path);
    int segment_open();
    int segment_close();
    int reap_segment();
    int segment_shrink();
    int refresh_file_list();
    int mp4_muxing();

private:
    FastVector msgs;
    CVorxMutex mutex;
    vector<CMp4Segment*> segments;
    vector<CMp4Segment*> segments_ori;
    CVorxThread thread;

};
#endif//_MP4_MUXER_H_

