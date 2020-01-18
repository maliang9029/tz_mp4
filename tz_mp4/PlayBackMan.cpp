#include "PlayBackMan.h"

CPlayBackMan* CPlayBackMan::g_pPlayBackMan = new (std::nothrow) CPlayBackMan;

CPlayBackMan::CPlayBackMan(void)
{
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
    if(!videoRecordMan)
        return false;
    m_mapVideoRecordMan.insert(make_pair(nPlayID,videoRecordMan));


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
    return true;
}

bool CPlayBackMan::play_ts(unsigned int lPlayID,unsigned int &ts,unsigned int &cur_ts)
{
    return true;
}

bool CPlayBackMan::play_start(unsigned int lPlayID,unsigned int hWnd)
{
    CVideoRecordMan* videoRecordMan = getVideoRecordManHandle(lPlayID);
    if(!videoRecordMan)
        return false;

    return true;
}

bool CPlayBackMan::play_pause(unsigned int lPlayID)
{
    return true;
}

bool CPlayBackMan::play_resume(unsigned int lPlayID)
{
    return true;
}

bool CPlayBackMan::play_step(unsigned int lPlayID)
{
    return true;
}

bool CPlayBackMan::play_step_prev(unsigned int lPlayID)
{
    return true;
}

bool CPlayBackMan::play_start_time(unsigned int lPlayID,unsigned int start_time)
{
    return true;
}

bool CPlayBackMan::play_save_start(unsigned int lPlayID,const char* sSavePath)
{
    return true;
}

bool CPlayBackMan::play_save_stop(unsigned int lPlayID)
{
    return true;
}

bool CPlayBackMan::play_speed(unsigned int lPlayID,int speed)
{
    return true;
}

bool CPlayBackMan::play_stop(unsigned int lPlayID)
{
    return true;
}

bool CPlayBackMan::close_mp4(unsigned int lPlayID)
{
    return true;
}
