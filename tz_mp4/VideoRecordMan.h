#pragma once

/*
	录像处理管理类
*/
#ifndef _VIDEO_RECORD_MAN_H_
#define	_VIDEO_RECORD_MAN_H_

class CVideoRecordMan
{
public:
	CVideoRecordMan(const char* sFilePath, unsigned int w,unsigned int h,unsigned int frameRate);
	~CVideoRecordMan(void);
public:
	bool play_start(unsigned int hWnd);
};
#endif