#include "VideoRecordMan.h"

CVideoRecordMan::CVideoRecordMan(const char* sFilePath, int nPlayID, unsigned int w,unsigned int h, unsigned int frameRate)
{
	m_pDecoder = NULL;
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

void CVideoRecordMan::MyON_VEDIO_DATA(int width, int height, VPicture* pic, void* lParam)
{

}

bool CVideoRecordMan::play_start(unsigned int hWnd)
{
	if(!m_pDecoder) {
		m_pDecoder = new CDecoder();
    }
	if(!m_pDecoder) {
		return false;
    }

	if(!m_vecFileList.size()) {
		//return false;
    }
	std::vector<string>::iterator it;
	//string strFile = m_vecFileList.at(m_vecFileList.size()-1);
	//m_pDecoder->SetVideoCallBack(MyON_VEDIO_DATA,(void*)this);
    //return m_pDecoder->init(strFile.c_str(),(HWND)hWnd);
    //return m_pDecoder->init("D:\\RAW_DATA.h265",(HWND)hWnd);
	return m_pDecoder->init("D:\\workplace\\tz_mp4\\output.mp4",(HWND)hWnd, playid);
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
    return mp4_muxer->init_muxing(true);
}

bool CVideoRecordMan::play_pause()
{
    if(!m_pDecoder) {
        return false;
    }
    return m_pDecoder->play_pause();
}

bool CVideoRecordMan::play_resume()
{
    if(!m_pDecoder) {
        return false;
    }
    return m_pDecoder->play_resume();
}

bool CVideoRecordMan::play_step()
{
    if(!m_pDecoder) {
        return false;
    }
    return m_pDecoder->NextSingleFrame();
}

bool CVideoRecordMan::play_seek(unsigned int ntime)
{
	if(!m_pDecoder) {
		return false;
    }
	return m_pDecoder->play_seek(ntime);
}

bool CVideoRecordMan::play_step_prev()
{
    if(!m_pDecoder) {
        return false;
    }
    return m_pDecoder->PreSingleFrame();
}

bool CVideoRecordMan::play_speed(int speed)
{
    if(!m_pDecoder) {
        return false;
    }
    return m_pDecoder->play_speed(speed);
}

bool CVideoRecordMan::play_snap(const char* sFilePath)
{
    if(!m_pDecoder) {
        return false;
    }
    return m_pDecoder->snapshot(sFilePath);
}

bool CVideoRecordMan::play_save_start(const char* sSavePath)
{
	if(!m_pDecoder) {
		return false;
    }
	return m_pDecoder->play_save_start(sSavePath);
}
bool CVideoRecordMan::play_save_stop()
{
	if(!m_pDecoder) {
		return false;
    }
	return m_pDecoder->play_save_stop();
}
bool CVideoRecordMan::play_stop()
{
	if(!m_pDecoder) {
		return false;
    }
	m_pDecoder->play_stop();
	if(m_pDecoder) {
		delete m_pDecoder;
		m_pDecoder = NULL;
	}
	return true;
}
int64_t CVideoRecordMan::get_current_dts()
{
    if (mp4_muxer) {
        return mp4_muxer->get_current_dts();
    }
    return 0;
}

void CVideoRecordMan::get_record_list(vector<string> &vec)
{
    if (mp4_muxer) {
        return mp4_muxer->get_record_list(vec);
    }
    return;
}

void CVideoRecordMan::get_record_history(map<string, int> &history)
{
    if (mp4_muxer) {
        return mp4_muxer->get_record_history(history);
    }
    return;
}

bool CVideoRecordMan::play_ts(unsigned int &ts,unsigned int &cur_ts)
{
	std::vector<string> veclist;
	get_record_list(veclist);
	if(veclist.size() == 1) {
		int64_t tm = get_current_dts();
		cur_ts = tm;
		ts = tm;
	} else {
	}
	m_vecFileList.assign(veclist.begin(),veclist.end());
	return true;
}