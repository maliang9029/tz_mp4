#ifndef _SAVE_AS_FILE_H_
#define _SAVE_AS_FILE_H_
#include "common.h"
#include "Mp4Muxer.h"
class CSaveAsFile
{
public:
	CSaveAsFile(void);
	~CSaveAsFile(void);
public:
	bool init(string sFile,int w,int h,int nFrameRate);
	void write_packet(AVPacket* pkt);
	int stop_write();
private:
	int dump_packets(int max_count, CMessage** pmsg, int& count);
	int do_muxing();
	int mp4_muxing(CMessage* msg);
	int segment_open();
	int segment_close();
	int reap_segment();
	int segment_shrink();
	void start_muxing();
	static int thread_savevideo(LPVOID lParam);
private:
	FastVector m_msgs;
	CMp4Segment* m_current;
	int64_t m_delay_time;
	int64_t m_next_dts;
	int64_t m_dts;
	int64_t m_next_pts;
	int64_t m_pts;
	int64_t m_current_dts;
	int64_t m_max_file_length;
	CVorxThread m_threadSaveVideo;
	CVorxMutex mutex;
	string m_strSaveFile;
	int m_screen_w;
	int m_screen_h;
	int m_nFrameRate;
	AVCodecParserContext *parser;
	AVCodecContext *c;

};
#endif
