#include "VideoRecordMan.h"

CVideoRecordMan::CVideoRecordMan(const char* sFilePath, int nPlayID, unsigned int w,unsigned int h, unsigned int frameRate)
{
    path = sFilePath;
    playid = nPlayID;
    width = w;
    height = h;
    framerate = frameRate;
}

CVideoRecordMan::~CVideoRecordMan(void)
{
}

bool CVideoRecordMan::play_start(unsigned int hWnd)
{
    return true;
}

bool CVideoRecordMan::write_frame(const char * data, unsigned int len)
{
    return false;
}
