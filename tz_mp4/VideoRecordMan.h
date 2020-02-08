#pragma once
#pragma warning (disable:4819)
/*
	录像处理管理类
*/
#ifndef _VIDEO_RECORD_MAN_H_
#define	_VIDEO_RECORD_MAN_H_
#include "common.h"
#include "Mp4Muxer.h"
#include "Decoder.h"

class CVideoRecordMan
{
public:
    CVideoRecordMan(const char* sFilePath, int nPlayID, unsigned int w,unsigned int h, unsigned int frameRate);
    ~CVideoRecordMan(void);
public:
    static string next_file_call_back(string strCurFile, void* lParam);
    static string pre_file_call_back(string strCurFile, void* lParam);
	bool play_ts(unsigned int &ts,unsigned int &cur_ts);
    bool play_start(unsigned int hWnd);
    bool write_frame(const char* data, unsigned int len);
    bool init_record();
	bool play_pause();
    bool play_resume();
    bool play_step();
	bool play_seek(unsigned int ntime);
    bool play_step_prev();
    bool play_speed(int speed);
    bool play_snap(const char* sFilePath);
	bool play_stop();
	bool play_save_start(const char* sSavePath);
	bool play_save_stop();
	int64_t get_current_dts();
	void get_record_list(vector<string> &vec);
	void play_next_file(string strCurFile);
    void play_pre_file(string strCurFile);
	void get_record_history(map<string, FileInfo> &history);
private:
	string get_next_file(string strCurFile);
    string get_pre_file(string strCurFile);
	string get_cur_file();
	int64_t get_cur_play_time();
public:
	std::vector<string> m_vecFileList;
private:
    string path;
    int playid;
    int width;
    int height;
    int framerate;
    CMp4Muxer* mp4_muxer;
	CDecoder* m_pDecoder;
	HWND m_hWnd;
	std::map<string,FileInfo> m_mapRecordHistory;
	CVorxMutex m_mutex;

};
#endif