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

string CVideoRecordMan::next_file_call_back(string strCurFile, void* lParam)
{
	CVideoRecordMan* pThis = (CVideoRecordMan*)lParam;
	if(pThis)
	{
		pThis->play_next_file(strCurFile);
	}
    return "";
}

string CVideoRecordMan::pre_file_call_back(string strCurFile, void* lParam)
{
	CVideoRecordMan* pThis = (CVideoRecordMan*)lParam;
	if(pThis)
	{
		return pThis->get_pre_file(strCurFile);
	}
    return "";
}

string CVideoRecordMan::get_next_file(string strCurFile)
{
    string next_file = "";
	m_vecFileList.clear();
	m_mapRecordHistory.clear();
	get_record_list(m_vecFileList);
	get_record_history(m_mapRecordHistory);
	std::map<string, FileInfo>::iterator it = m_mapRecordHistory.find(strCurFile);
    if (it != m_mapRecordHistory.end()) {
        it++;
        if (it != m_mapRecordHistory.end()) {
            return it->first;
        }

    }
    printf("is end!!!!!!\n");
    return next_file;
}

string CVideoRecordMan::get_pre_file(string strCurFile)
{
    string pre_file = "";
    m_vecFileList.clear();
	m_mapRecordHistory.clear();
	get_record_list(m_vecFileList);
	get_record_history(m_mapRecordHistory);
	std::map<string,FileInfo>::iterator it = m_mapRecordHistory.find(strCurFile);
    if (it != m_mapRecordHistory.end()) {
        if (it != m_mapRecordHistory.begin()) {
            it--;
            return it->first;
        }
    }
    printf("is begin!!!!!!\n");
    return pre_file;
}

void CVideoRecordMan::play_next_file(string strCurFile)
{
	//if(nPos < m_vecFileList.size())
	{

		string strNextFile = get_next_file(strCurFile);
		if (m_pDecoder && strNextFile != "")
		{
			//m_pDecoder->stopdecoder();
			CAutoMutex lock(&m_mutex);
			if(m_pDecoder) {
				m_pDecoder->init(strNextFile.c_str(), m_hWnd,playid);
            }
		}
	}

}

void CVideoRecordMan::play_pre_file(string strCurFile)
{

}

bool CVideoRecordMan::play_start(unsigned int hWnd)
{
	if(!m_pDecoder) {
        m_pDecoder = new CDecoder;
        m_pDecoder->set_play_end_callback(next_file_call_back, pre_file_call_back, (void*)this);
    }
	if(!m_pDecoder)
		return false;
	m_hWnd = (HWND)hWnd;
	string strFile = m_mapRecordHistory.begin()->first;
    return m_pDecoder->init(strFile.c_str(),(HWND)hWnd,playid);
	//return m_pDecoder->init("D:\\workplace\\tz_mp4\\output.mp4",(HWND)hWnd, playid);
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
	int64_t curTime = ntime*1000;
	int64_t seekTime= 0;
	string strFile;
	std::map<string,FileInfo>::iterator it = m_mapRecordHistory.begin();
	for (;it!=m_mapRecordHistory.end();++it)
	{
		if(it->second.all_time>curTime){
			strFile = it->first;
			seekTime = it->second.all_time - curTime;
			break;
		}
	}
	if(strFile != m_pDecoder->get_cur_play_file())	{
		m_pDecoder->init(strFile.c_str(),m_hWnd,playid);
		Sleep(200);
	}

	return m_pDecoder->play_seek(seekTime);
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
	CAutoMutex lock(&m_mutex);
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

void CVideoRecordMan::get_record_history(map<string, FileInfo> &history)
{
    if (mp4_muxer) {
        return mp4_muxer->get_record_history(history);
    }
    return;
}

string CVideoRecordMan::get_cur_file()
{
	if(m_pDecoder)
		return m_pDecoder->get_cur_play_file();
	return "";
}

int64_t CVideoRecordMan::get_cur_play_time()
{
	if(m_pDecoder)
		return m_pDecoder->get_cur_play_time();
	return 0;
}

bool CVideoRecordMan::play_ts(unsigned int &ts,unsigned int &cur_ts)
{
	std::vector<string> veclist;
	std::map<string,FileInfo> mapHistory;
	get_record_list(veclist);
	get_record_history(mapHistory);
	int64_t _ts = 0,_cur_ts=0;
	std::map<string,FileInfo>::iterator it = mapHistory.begin();
	int64_t nAllTime = 0;
	string curfile = get_cur_file();
	for (;it!=mapHistory.end();++it)
	{
		_ts += it->second.duration;
		if(mapHistory.begin()->first != curfile)
		{
			if(it->first == curfile)
				_cur_ts += nAllTime;
		}

		it->second.all_time =nAllTime+it->second.duration;
		nAllTime = it->second.all_time;
	}
	_cur_ts += get_cur_play_time()*1000;
	m_mapRecordHistory = mapHistory;
	_ts += get_current_dts();
	ts = _ts;
	cur_ts = _cur_ts;
	m_vecFileList.assign(veclist.begin(),veclist.end());

	return true;
}
