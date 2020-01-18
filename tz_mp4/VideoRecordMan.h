#pragma once
#pragma warning (disable:4819)
/*
	录像处理管理类
*/
#ifndef _VIDEO_RECORD_MAN_H_
#define	_VIDEO_RECORD_MAN_H_
#include "common.h"

class CVideoRecordMan
{
public:
    CVideoRecordMan(const char* sFilePath, int nPlayID, unsigned int w,unsigned int h, unsigned int frameRate);
    ~CVideoRecordMan(void);
public:
    bool play_start(unsigned int hWnd);
    bool write_frame(const char* data, unsigned int len);
private:
    string path;
    int playid;
    int width;
    int height;
    int framerate;

};
#endif