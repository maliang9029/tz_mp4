#include "Tz_Mp4.h"
#include "PlayBackMan.h"

CPlayBackMan* g_PlayBackMan = NULL;

TZ_MP4 bool  __stdcall open_mp4(unsigned int &lPlayID,const char* sFilePath, unsigned int w,unsigned int h,unsigned int frameRate)
{
	g_PlayBackMan = new CPlayBackMan;
	if (!g_PlayBackMan)
		return false;

	return g_PlayBackMan->open_mp4(lPlayID,sFilePath,w,h,frameRate);
}

TZ_MP4 bool __stdcall write_frame(unsigned int lPlayID,const char* sData,unsigned int nDateLen)
{
	if (!g_PlayBackMan)
		return false;
	return g_PlayBackMan->write_frame(lPlayID,sData,nDateLen);
}

TZ_MP4 bool __stdcall play_start(unsigned int lPlayID,unsigned int hWnd)
{
	if (!g_PlayBackMan)
		return false;
	return g_PlayBackMan->play_start(lPlayID,hWnd);
}

TZ_MP4 bool __stdcall play_pause(unsigned int lPlayID)
{
	if (!g_PlayBackMan)
		return false;
	return g_PlayBackMan->play_pause(lPlayID);
}

TZ_MP4 bool __stdcall play_resume(unsigned int lPlayID)
{
	if (!g_PlayBackMan)
		return false;
	return g_PlayBackMan->play_resume(lPlayID);
}

TZ_MP4 bool __stdcall play_step(unsigned int lPlayID)
{
	if (!g_PlayBackMan)
		return false;
	return g_PlayBackMan->play_step(lPlayID);
}

TZ_MP4 bool __stdcall play_step_prev(unsigned int lPlayID)
{
	if (!g_PlayBackMan)
		return false;
	return g_PlayBackMan->play_step_prev(lPlayID);
}

TZ_MP4 bool __stdcall play_ts(unsigned int lPlayID,unsigned int &ts,unsigned int &cur_ts)
{
	if (!g_PlayBackMan)
		return false;
	return g_PlayBackMan->play_ts(lPlayID,ts,cur_ts);
}

TZ_MP4 bool __stdcall play_start_time(unsigned int lPlayID,unsigned int start_time)
{
	if (!g_PlayBackMan)
		return false;
	return g_PlayBackMan->play_start_time(lPlayID,start_time);
}

TZ_MP4 bool __stdcall play_save_start(unsigned int lPlayID,const char* sSavePath)
{
	if (!g_PlayBackMan)
		return false;
	return g_PlayBackMan->play_save_start(lPlayID,sSavePath);
}

TZ_MP4 bool __stdcall play_save_stop(unsigned int lPlayID)
{
	if (!g_PlayBackMan)
		return false;
	return g_PlayBackMan->play_save_stop(lPlayID);
}

TZ_MP4 bool __stdcall play_speed(unsigned int lPlayID,int speed)
{
	if (!g_PlayBackMan)
		return false;
	return g_PlayBackMan->play_speed(lPlayID,speed);
}

TZ_MP4 bool __stdcall play_stop(unsigned int lPlayID)
{
	if (!g_PlayBackMan)
		return false;
	return g_PlayBackMan->play_stop(lPlayID);
}

TZ_MP4 bool __stdcall close_mp4(unsigned int lPlayID)
{
	if (!g_PlayBackMan)
		return false;
	return g_PlayBackMan->close_mp4(lPlayID);
}


