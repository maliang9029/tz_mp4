#include "VideoRecordMan.h"

CVideoRecordMan::CVideoRecordMan(const char* sFilePath, int nPlayID, unsigned int w,unsigned int h, unsigned int frameRate)
{
    path = sFilePath;
    playid = nPlayID;
    width = w;
    height = h;
    framerate = frameRate;
    mp4_muxer = new CMp4Muxer(path, playid, width, height, framerate);
}

CVideoRecordMan::~CVideoRecordMan(void)
{
    safe_freep(mp4_muxer);
}

bool CVideoRecordMan::play_start(unsigned int hWnd)
{
    return true;
}

bool CVideoRecordMan::write_frame(const char * data, unsigned int len)
{
    if (mp4_muxer) {
        return mp4_muxer->parse_packet(data, len);
    }
    return false;
}

bool CVideoRecordMan::init_record()
{
    if (path == "") {
        return false;
    }
    if (!mp4_muxer) {
        mp4_muxer = new CMp4Muxer(path, playid, width, height, framerate);
    }
    return mp4_muxer->init_muxing();
}

