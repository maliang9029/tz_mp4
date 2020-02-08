#pragma once
#ifndef _MP4_MUXER_H_
#define _MP4_MUXER_H_
#include "common.h"

typedef struct FileInfo {
    int64_t start_time;
    int64_t duration;
	int64_t all_time;
}FileInfo;

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
    string path;
    int width;
    int height;
    int framerate;
public:
    int init_segment(AVFormatContext *ifmt_ctx = NULL);
    int64_t get_start_time();
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
    int64_t max_file_length;
public:
    bool init_muxing(bool change_file = true, int file_len = DEFAULT_MAX_FILE_LENGTH, int record_period_len = DEFAULT_RECORD_PERIOD_TIME, int record_history_len = DEFAULT_RECORD_HISTORY_TIME);
    bool write_packet(AVPacket* pkt);
    static int thread_muxing(LPVOID lParam);
    int do_muxing();
    void strat_muxing();
    bool parse_packet(const char* data, unsigned int len);
    void set_delay_time(int64_t delay);
    int64_t get_current_dts();
    void get_record_list(vector<string> &vec);
    void get_record_history(map<string, FileInfo> &history);
private:
    int dump_packets(int max_count, CMessage** pmsg, int& count);
    bool create_multi_directory(void);
    bool is_directory_exsits(const char* path);
    int segment_open();
    int segment_close();
    int reap_segment();
    int segment_shrink(bool is_close = true);
    int refresh_period_list(string filename);
    int refresh_history_list(string filename);
    int mp4_muxing(CMessage* msg);
    void init_segments_period();
    void init_segments_history();
private:
    FastVector msgs;
    CVorxMutex mutex;
    int64_t delay_time;
    CMp4Segment* current;
    //vector<CMp4Segment*> segments_period;
    //vector<CMp4Segment*> segments_history;
    CVorxMutex mutex_period;
    vector<string> segments_period;
    CVorxMutex mutex_history;
    map<string, FileInfo> segments_history;
    CVorxThread thread;
    AVCodecParserContext *parser;
    AVCodecContext *c;
    int record_period_window;
    int record_history_window;
    string index_filename;
    string index_all_filename;
    bool is_muxing_start;
    int64_t next_dts;
    int64_t dts;
    int64_t next_pts;
    int64_t pts;
    int64_t current_dts;
    bool need_change_file;

};
#endif//_MP4_MUXER_H_

