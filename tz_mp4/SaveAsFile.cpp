#include "SaveAsFile.h"

CSaveAsFile::CSaveAsFile(void)
:m_nFrameRate(0)
,m_screen_w(0)
,m_screen_h(0)
,m_delay_time(0)
,m_pts(0)
,m_dts(0)
,m_current(NULL)
,m_next_pts(0)
,m_next_dts(0)
,m_current_dts(0)
,m_max_file_length(0)
,c(NULL)
,parser(NULL)
{
	m_threadSaveVideo.SetParam(thread_savevideo,this,500,NULL);
}

CSaveAsFile::~CSaveAsFile(void)
{
}

bool CSaveAsFile::init(string sFile,int w,int h,int nFrameRate)
{
	m_strSaveFile = sFile;
	m_screen_w = w;
	m_screen_h = h;
	m_nFrameRate = nFrameRate;
	const AVCodec *codec;
	codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
	c = avcodec_alloc_context3(codec);
	if (!c) {
		return false;
	}
	parser = av_parser_init(AV_CODEC_ID_HEVC);
	if (!parser) {
		return false;
	}
	if (avcodec_open2(c, codec, NULL) < 0) {
		return false;
	}

	/*max_file_length = (fragment > 0 ? fragment : DEFAULT_MAX_FILE_LENGTH);
	int _record_period = (record_period > 0 ? record_period : DEFAULT_RECORD_PERIOD_TIME);
	int _file_period = (file_period > 0 ? file_period : DEFAULT_RECORD_HISTORY_TIME);
	double tmp = _record_period / (double)max_file_length;
	record_period_window = (int)ceil(tmp);
	tmp = _file_period / (double)max_file_length;
	record_history_window = (int)ceil(tmp);

	if (create_multi_directory() != true) {
		return false;
	}*/

	

	if (segment_open() != MP4_SUCCESS) {
		return false;
	}
	start_muxing();
	return true;
}

void CSaveAsFile::start_muxing()
{
	m_threadSaveVideo.Start();
}

int CSaveAsFile::segment_open()
{
	int ret = MP4_SUCCESS;
	if (m_current) {
		//warn
		safe_freep(m_current);
	}
	string file_name = "";
	string path = "";
	/*CVorxTime tm;
	char stime[20];
	memset(stime, 0, sizeof(stime));
	tm.GetCurrentTime();
	tm.ToString(stime,sizeof(stime),false);
	char tmp[128];
	memset(tmp, 0, sizeof(tmp));
	snprintf(tmp, sizeof(tmp), "%0.3d_%s", playid, stime);
	file_name = tmp;
	file_name += ".mp4";*/
	int nPos = m_strSaveFile.find_last_of("\\");
	if(nPos!= string::npos)
	{
		path = m_strSaveFile.substr(0,nPos+1);
		file_name = m_strSaveFile.substr(nPos+1,m_strSaveFile.size());
	}
	m_current = new CMp4Segment(path, file_name, m_screen_w, m_screen_h, m_nFrameRate);
	return m_current->init_segment();
}

int CSaveAsFile::segment_shrink()
{
	return 0;
}

int CSaveAsFile::segment_close()
{
	int ret = MP4_SUCCESS;
	if (!m_current) {
		return MP4_ERROR;
	}
	//segment_shrink();
	safe_freep(m_current);
	m_current = NULL;
	return ret;
}

int CSaveAsFile::reap_segment()
{
	int ret = MP4_SUCCESS;
	segment_close();
	segment_open();
	return ret;
}

int CSaveAsFile::mp4_muxing(CMessage* msg)
{
	int ret = MP4_SUCCESS;
	if (!m_current) {
		return MP4_ERROR;
	}
	int64_t duration = m_current->get_duration();
	if (duration >= m_max_file_length && msg->pkt->flags == AV_PKT_FLAG_KEY) {
		//ret = reap_segment();
	}
	if (ret != MP4_SUCCESS) {
		return ret;
	}
	m_current_dts = msg->pkt->dts;
	m_current->ffmpeg_muxing(msg);
	duration = m_current->get_duration();
	/*if (duration >= m_delay_time) {
		segment_shrink();
	}*/
	return ret; 0;
}

int CSaveAsFile::dump_packets(int max_count, CMessage** pmsg, int& count)
{
	int ret = MP4_SUCCESS;
	CAutoMutex lock(&mutex);

	int nb_msgs = (int)m_msgs.size();
	if (nb_msgs <= 0 || max_count <= 0) {
		return ret;
	}

	count = min(max_count, nb_msgs);
	CMessage** omsgs = m_msgs.data();
	for (int i = 0; i < count; i++) {
		pmsg[i] = omsgs[i];
	}

	if (count >= nb_msgs) {
		m_msgs.clear();
	} else {
		m_msgs.erase(m_msgs.begin(), m_msgs.begin() + count);
	}

	return ret;
}

int CSaveAsFile::do_muxing()
{
	CMessageArray msgs(MAX_READ_PACKETS);
	int count = 0;
	dump_packets(msgs.max, msgs.msgs, count);
	for (int i = 0; i < count; i++) {
		CMessage* msg = msgs.msgs[i];
		mp4_muxing(msg);
		safe_freep(msg);
	}
	return 0;
}

int CSaveAsFile::thread_savevideo(LPVOID lParam)
{
	CSaveAsFile* pThis = (CSaveAsFile*)lParam;
	if(pThis)
	{
		return pThis->do_muxing();
	}
	return 0;
}

void CSaveAsFile::write_packet(AVPacket* pkt)
{
	CAutoMutex lock(&mutex);
	CMessage* msg = new CMessage;
	msg->create(pkt);
	m_msgs.push_back(msg);
}

int CSaveAsFile::stop_write()
{
	m_threadSaveVideo.Stop();
	safe_freep(m_current);
	m_msgs.free();
	m_msgs.clear();
	if (parser) {
		av_parser_close(parser);
	}
	if (c) {
		avcodec_free_context(&c);
	}
	return 0;
}
