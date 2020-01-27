#include "PlayBackMan.h"

CPlayBackMan* CPlayBackMan::g_pPlayBackMan = new (std::nothrow) CPlayBackMan;

CPlayBackMan::CPlayBackMan(void)
{
    memset(m_bUsed,false,9);
}

CPlayBackMan::~CPlayBackMan(void)
{
}


int CPlayBackMan::getFreePlayId()
{
    for (int i=0;i<MAX_PLAY_CHN;i++)
    {
        if(false == m_bUsed[i])
        {
            CAutoMutex lock(&m_mutex);
            m_bUsed[i] = true;
            return i;
        }
    }
    return -1;
}

void CPlayBackMan::freePlayId(int nPlayId)
{
    if(nPlayId < MAX_PLAY_CHN)
    {
        CAutoMutex lock(&m_mutex);
        if(m_bUsed[nPlayId])
        {
            m_bUsed[nPlayId] = false;
        }
    }
}

CPlayBackMan* CPlayBackMan::GetInstance()
{
    return g_pPlayBackMan;
}

bool CPlayBackMan::open_mp4(unsigned int &lPlayID,const char* sFilePath, unsigned int w,unsigned int h,unsigned int frameRate)
{

    int nPlayID = lPlayID = getFreePlayId();
    if(nPlayID == -1)
        return false;
    CVideoRecordMan* videoRecordMan = new CVideoRecordMan(sFilePath, nPlayID, w, h, frameRate);
    if(!videoRecordMan) {
        return false;
    }
    if (videoRecordMan->init_record() != true) {
        //return false;
    }

    {
        CAutoMutex lock(&m_mutex);
        m_mapVideoRecordMan.insert(make_pair(nPlayID,videoRecordMan));
    }

    return true;
}

CVideoRecordMan* CPlayBackMan::getVideoRecordManHandle(unsigned int lPlayID)
{
    CAutoMutex lock(&m_mutex);
    map<int,CVideoRecordMan*>::const_iterator it;
    if((it = m_mapVideoRecordMan.find(lPlayID)) != m_mapVideoRecordMan.end())
    {
        return it->second;
    }
    return NULL;
}

bool CPlayBackMan::write_frame(unsigned int lPlayID,const char* sData,unsigned int nDateLen)
{
    CVideoRecordMan* videoRecordMan = getVideoRecordManHandle(lPlayID);
    if (videoRecordMan) {
        return videoRecordMan->write_frame(sData, nDateLen);
    }
    return false;
}

bool CPlayBackMan::play_ts(unsigned int lPlayID,unsigned int &ts,unsigned int &cur_ts)
{
    CVideoRecordMan* videoRecordMan = getVideoRecordManHandle(lPlayID);
    if(!videoRecordMan)
        return false;
    return true;
}

bool CPlayBackMan::play_start(unsigned int lPlayID,unsigned int hWnd)
{
    CVideoRecordMan* videoRecordMan = getVideoRecordManHandle(lPlayID);
    if(!videoRecordMan)
        return false;

	return videoRecordMan->play_start(hWnd);
}

bool CPlayBackMan::play_pause(unsigned int lPlayID)
{
    CVideoRecordMan* videoRecordMan = getVideoRecordManHandle(lPlayID);
    if(!videoRecordMan)
        return false;
	return videoRecordMan->play_pause();
}

bool CPlayBackMan::play_resume(unsigned int lPlayID)
{
    CVideoRecordMan* videoRecordMan = getVideoRecordManHandle(lPlayID);
    if(!videoRecordMan)
        return false;
	return videoRecordMan->play_resume();
}

bool CPlayBackMan::play_step(unsigned int lPlayID)
{
    CVideoRecordMan* videoRecordMan = getVideoRecordManHandle(lPlayID);
    if(!videoRecordMan)
        return false;
	return videoRecordMan->play_step();
}

bool CPlayBackMan::play_seek(unsigned int lPlayID,unsigned int ntime)
{
	CVideoRecordMan* videoRecordMan = getVideoRecordManHandle(lPlayID);
	if(!videoRecordMan)
		return false;
	return videoRecordMan->play_seek(ntime);
}
bool CPlayBackMan::play_step_prev(unsigned int lPlayID)
{
    CVideoRecordMan* videoRecordMan = getVideoRecordManHandle(lPlayID);
    if(!videoRecordMan)
        return false;
	return videoRecordMan->play_step_prev();
}

bool CPlayBackMan::play_start_time(unsigned int lPlayID,unsigned int start_time)
{
    CVideoRecordMan* videoRecordMan = getVideoRecordManHandle(lPlayID);
    if(!videoRecordMan)
        return false;
    return true;
}

bool CPlayBackMan::play_save_start(unsigned int lPlayID,const char* sSavePath)
{
    CVideoRecordMan* videoRecordMan = getVideoRecordManHandle(lPlayID);
    if(!videoRecordMan)
        return false;
    return true;
}

bool CPlayBackMan::play_save_stop(unsigned int lPlayID)
{
    CVideoRecordMan* videoRecordMan = getVideoRecordManHandle(lPlayID);
    if(!videoRecordMan)
        return false;
    return true;
}

bool CPlayBackMan::play_speed(unsigned int lPlayID,int speed)
{
    CVideoRecordMan* videoRecordMan = getVideoRecordManHandle(lPlayID);
    if(!videoRecordMan)
        return false;
	return videoRecordMan->play_speed(speed);
}

bool CPlayBackMan::play_snap(unsigned int lPlayID,const char* sFilePath)
{
    CVideoRecordMan* videoRecordMan = getVideoRecordManHandle(lPlayID);
    if(!videoRecordMan)
        return false;
    return videoRecordMan->play_snap(sFilePath);
}
bool CPlayBackMan::play_stop(unsigned int lPlayID)
{
    CVideoRecordMan* videoRecordMan = getVideoRecordManHandle(lPlayID);
    if(!videoRecordMan)
        return false;
    return true;
}

bool CPlayBackMan::close_mp4(unsigned int lPlayID)
{
    CAutoMutex lock(&m_mutex);
    map<int,CVideoRecordMan*>::const_iterator it;
    for(it = m_mapVideoRecordMan.begin(); it != m_mapVideoRecordMan.end(); it++) {
        if (it->first == lPlayID) {
            CVideoRecordMan* videoRecordMan =  it->second;
            safe_freep(videoRecordMan);
            m_mapVideoRecordMan.erase(it);
            break;
        }
    }
    return true;
}
