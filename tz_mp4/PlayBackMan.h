#pragma once
#pragma warning (disable:4819)

#ifndef _PLAY_BACK_MAN_H_
#define	_PLAY_BACK_MAN_H_

#include "common.h"
#include "VideoRecordMan.h"

using namespace vfc;

class CPlayBackMan
{
private:
	CPlayBackMan(void);
private:
    //禁止拷贝
    CPlayBackMan(const CPlayBackMan&){};
    //禁止赋值
    CPlayBackMan& operator=(const CPlayBackMan&){};
public:
    ~CPlayBackMan(void);
public:
    // 获取单实例
    static CPlayBackMan* GetInstance();

    bool open_mp4(unsigned int &lPlayID,const char* sFilePath, unsigned int w,unsigned int h,unsigned int frameRate);

    bool write_frame(unsigned int lPlayID,const char* sData,unsigned int nDateLen);

    bool play_start(unsigned int lPlayID,unsigned int hWnd);

    bool play_pause(unsigned int lPlayID);

    bool play_resume(unsigned int lPlayID);

    bool play_step(unsigned int lPlayID);

    bool play_step_prev(unsigned int lPlayID);

    bool play_ts(unsigned int lPlayID,unsigned int &ts,unsigned int &cur_ts);

    bool play_start_time(unsigned int lPlayID,unsigned int start_time);

    bool play_save_start(unsigned int lPlayID,const char* sSavePath);

    bool play_save_stop(unsigned int lPlayID);

    bool play_speed(unsigned int lPlayID,int speed);

    bool play_stop(unsigned int lPlayID);

    bool close_mp4(unsigned int lPlayID);
private:
    int getFreePlayId();

    void freePlayId(int nPlayId);

    CVideoRecordMan* getVideoRecordManHandle(unsigned int lPlayID);
private:
    bool m_bUsed[MAX_PLAY_CHN];

    CTzMutex m_mutex;

    CVideoRecordMan* m_videoRecordMan;
    std::map<int,CVideoRecordMan*>  m_mapVideoRecordMan;
    // 唯一单实例对象指针
    static CPlayBackMan *g_pPlayBackMan;
};

#endif
