#include "Tz_Mp4.h"
#include "PlayBackMan.h"

TZ_MP4 bool  __stdcall open_mp4(unsigned int &lPlayID,const char* sFilePath, unsigned int w,unsigned int h,unsigned int frameRate)
{
    CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
	if (!pPlayBackMan)
		return false;

	return pPlayBackMan->open_mp4(lPlayID,sFilePath,w,h,frameRate);
}

TZ_MP4 bool __stdcall write_frame(unsigned int lPlayID,const char* sData,unsigned int nDateLen)
{
    CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
	if (!pPlayBackMan)
		return false;
	return pPlayBackMan->write_frame(lPlayID,sData,nDateLen);
}

TZ_MP4 bool __stdcall play_start(unsigned int lPlayID,unsigned int hWnd)
{
    CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
	if (!pPlayBackMan)
		return false;
	return pPlayBackMan->play_start(lPlayID,hWnd);
}

TZ_MP4 bool __stdcall play_pause(unsigned int lPlayID)
{
    CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
	if (!pPlayBackMan)
		return false;
	return pPlayBackMan->play_pause(lPlayID);
}

TZ_MP4 bool __stdcall play_resume(unsigned int lPlayID)
{
    CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
	if (!pPlayBackMan)
		return false;
	return pPlayBackMan->play_resume(lPlayID);
}

TZ_MP4 bool __stdcall play_step(unsigned int lPlayID)
{
    CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
	if (!pPlayBackMan)
		return false;
	return pPlayBackMan->play_step(lPlayID);
}

TZ_MP4 bool __stdcall play_seek(unsigned int lPlayID,unsigned int ntime)
{
	CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
	if (!pPlayBackMan)
		return false;
	return pPlayBackMan->play_seek(lPlayID,ntime);
}

TZ_MP4 bool __stdcall play_step_prev(unsigned int lPlayID)
{
    CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
	if (!pPlayBackMan)
		return false;
	return pPlayBackMan->play_step_prev(lPlayID);
}

TZ_MP4 bool __stdcall play_ts(unsigned int lPlayID,unsigned int &ts,unsigned int &cur_ts)
{
    CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
	if (!pPlayBackMan)
		return false;
	return pPlayBackMan->play_ts(lPlayID,ts,cur_ts);
}

TZ_MP4 bool __stdcall play_start_time(unsigned int lPlayID,unsigned int start_time)
{
    CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
	if (!pPlayBackMan)
		return false;
	return pPlayBackMan->play_start_time(lPlayID,start_time);
}

TZ_MP4 bool __stdcall play_save_start(unsigned int lPlayID,const char* sSavePath)
{
    CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
	if (!pPlayBackMan)
		return false;
	return pPlayBackMan->play_save_start(lPlayID,sSavePath);
}

TZ_MP4 bool __stdcall play_save_stop(unsigned int lPlayID)
{
    CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
	if (!pPlayBackMan)
		return false;
	return pPlayBackMan->play_save_stop(lPlayID);
}

TZ_MP4 bool __stdcall play_speed(unsigned int lPlayID,int speed)
{
    CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
	if (!pPlayBackMan)
		return false;
	return pPlayBackMan->play_speed(lPlayID,speed);
}

TZ_MP4 bool __stdcall play_snap(unsigned int lPlayID,const char* sFilePath)
{
	CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
    if (!pPlayBackMan)
        return false;
    return pPlayBackMan->play_snap(lPlayID,sFilePath);
}

TZ_MP4 bool __stdcall play_stop(unsigned int lPlayID)
{
    CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
	if (!pPlayBackMan)
		return false;
	return pPlayBackMan->play_stop(lPlayID);
}

TZ_MP4 bool __stdcall close_mp4(unsigned int lPlayID)
{
    CPlayBackMan* pPlayBackMan = CPlayBackMan::GetInstance();
	if (!pPlayBackMan)
		return false;
	return pPlayBackMan->close_mp4(lPlayID);
}


