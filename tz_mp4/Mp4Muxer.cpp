#include "Mp4Muxer.h"
#include <io.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#define SEPARATOR_CHAR "?"

int64_t pkt_time_stamp = 0;
AVRational in_time_base = AVRational{1, 1200000};
AVRational mux_timebase = in_time_base;
AVRational TIME_BASE = AVRational{1, AV_TIME_BASE};

static int hevc_probe(char *buf, int buf_size)
{
    uint32_t code = -1;
    int vps = 0, sps = 0, pps = 0, irap = 0;
    int i;

    for (i = 0; i < buf_size - 1; i++) {
        code = (code << 8) + buf[i];
        if ((code & 0xffffff00) == 0x100) {
            uint8_t nal2 = buf[i + 1];
            int type = (code & 0x7E) >> 1;

            if (code & 0x81) // forbidden and reserved zero bits
                return 0;

            if (nal2 & 0xf8) // reserved zero
                return 0;

            switch (type) {
            case HEVC_NAL_VPS:        vps++;  break;
            case HEVC_NAL_SPS:        sps++;  break;
            case HEVC_NAL_PPS:        pps++;  break;
            case HEVC_NAL_BLA_N_LP:
            case HEVC_NAL_BLA_W_LP:
            case HEVC_NAL_BLA_W_RADL:
            case HEVC_NAL_CRA_NUT:
            case HEVC_NAL_IDR_N_LP:
            case HEVC_NAL_IDR_W_RADL: irap++; break;
            }
        }
    }

    if (vps && sps && pps && irap)
        return AVPROBE_SCORE_EXTENSION + 1; // 1 more than .mpg
    return 0;
}


CMessage::CMessage()
{
    pkt = NULL;
}

CMessage::~CMessage()
{
    av_packet_free(&pkt);
}

int CMessage::create(AVPacket *inpkt)
{
    int ret = MP4_SUCCESS;
    pkt = av_packet_clone(inpkt);
    if (!pkt) {
        return MP4_ERROR;
    }
    return ret;
}

CMessageArray::CMessageArray(int max_msgs)
{
    if (max_msgs <= 0) {
        max_msgs = MAX_READ_PACKETS;
    }
    msgs = new CMessage*[max_msgs];
    max = max_msgs;

    zero(max_msgs);
}

CMessageArray::~CMessageArray()
{
    safe_freepa(msgs);
}

void CMessageArray::free(int count)
{
    for (int i = 0; i < count; i++) {
        CMessage* msg = msgs[i];
        safe_freep(msg);

        msgs[i] = NULL;
    }
}

void CMessageArray::zero(int count)
{
    for (int i = 0; i < count; i++) {
        msgs[i] = NULL;
    }
}

FastVector::FastVector()
{
    count = 0;
    nb_msgs = MAX_READ_PACKETS * 8;
    msgs = new CMessage*[nb_msgs];
}

FastVector::~FastVector()
{
    free();
    safe_freepa(msgs);
}

int FastVector::size()
{
    return count;
}

int FastVector::begin()
{
    return 0;
}

int FastVector::end()
{
    return count;
}

CMessage** FastVector::data()
{
    return msgs;
}

CMessage* FastVector::at(int index)
{
    if (index < count) {
        return msgs[index];
    }
    return NULL;
}

void FastVector::clear()
{
    count = 0;
}

void FastVector::erase(int _begin, int _end)
{
    if (_begin >= _end) {
        return;
    }

    // move all erased to previous.
    for (int i = 0; i < count - _end; i++) {
        msgs[_begin + i] = msgs[_end + i];
    }

    // update the count.
    count -= _end - _begin;
}

void FastVector::push_back(CMessage* msg)
{
    // increase vector.
    if (count >= nb_msgs) {
        int size = nb_msgs * 2;
        CMessage** buf = new CMessage*[size];
        for (int i = 0; i < nb_msgs; i++) {
            buf[i] = msgs[i];
        }

        // use new array.
        safe_freepa(msgs);
        msgs = buf;
        nb_msgs = size;
    }

    msgs[count++] = msg;
}

void FastVector::free()
{
    for (int i = 0; i < count; i++) {
        CMessage* msg = msgs[i];
        safe_freep(msg);
    }
    count = 0;
}

CMp4Segment::CMp4Segment(string path, string file_name, int w, int h, int r)
{
    const char *tmp = path.c_str();
    if (strcmp(tmp+path.length()-1,"\\") != 0) {
        path += "\\";
    }
    full_path = path + file_name;
    this->file_name = file_name;
    width = w;
    height = h;
    framerate = r;
    o_fmt_ctx = NULL;
    o_video_stream = NULL;
    codec_ctx = NULL;
    duration = 0;
    segment_start_time = AV_NOPTS_VALUE;
    segment_end_time = AV_NOPTS_VALUE;
}


CMp4Segment::~CMp4Segment()
{
    if (o_fmt_ctx) {
        av_write_trailer(o_fmt_ctx);
    }
    if (codec_ctx) {
       avcodec_free_context(&codec_ctx);
    }
    if (o_fmt_ctx) {
        avio_closep(&o_fmt_ctx->pb);
        avformat_free_context(o_fmt_ctx);
    }
}

int CMp4Segment::init_segment()
{
    int ret = MP4_SUCCESS;
    char *filename = (char*)full_path.c_str();
    AVCodec *codec = NULL;
    AVDictionary *opts = NULL;
    codec = avcodec_find_encoder(AV_CODEC_ID_HEVC);
    if (!codec) {
        return MP4_ERROR;
    }
    avformat_alloc_output_context2(&o_fmt_ctx, NULL, NULL, filename);
    if (!o_fmt_ctx) {
        return MP4_ERROR;
    }
    o_video_stream = avformat_new_stream(o_fmt_ctx, codec);
    o_video_stream->time_base = in_time_base;
    if (!o_video_stream) {
        return MP4_ERROR;
    }
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        return MP4_ERROR;
    }
    codec_ctx->width = width;
    codec_ctx->height = height;
    codec_ctx->time_base.den = framerate;
    codec_ctx->time_base.num = 1;
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    if (o_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    ret = avcodec_parameters_from_context(o_video_stream->codecpar, codec_ctx);
    if (ret < 0) {
        return MP4_ERROR;
    }

    //创建并打开MP4文件
	ret = avio_open(&o_fmt_ctx->pb, filename, AVIO_FLAG_WRITE);
    if (ret < 0) {
        return MP4_ERROR;
    }
    //设置输出FMP4的参数，并写文件头信�?
    av_dict_set(&opts, "movflags", "frag_keyframe+empty_moov", 0);
	ret = avformat_write_header(o_fmt_ctx, &opts);
    if (ret < 0) {
        return MP4_ERROR;
    }
	av_dict_free(&opts);
    return ret;
}

int64_t CMp4Segment::get_duration()
{
    //ms
    return av_rescale_q(duration, mux_timebase, TIME_BASE) / 1000;
}

void CMp4Segment::update_duration(int64_t dts)
{
    if (segment_start_time != AV_NOPTS_VALUE) {
        if (segment_start_time < dts) {
            duration = dts - segment_start_time;
        }
        segment_end_time = dts;
    } else {
        segment_start_time = segment_end_time = dts;
    }
    return;
}

int CMp4Segment::ffmpeg_muxing(CMessage* msg)
{
     int ret = MP4_SUCCESS;
     if (!o_fmt_ctx) {
         return MP4_ERROR;
     }

     update_duration(msg->pkt->dts);
     int64_t t = get_duration();
     printf("dts:%lld, duration: %lld, size:%d, key_frame:%d\n", msg->pkt->dts, t, msg->pkt->size, msg->pkt->flags);
     ret = av_interleaved_write_frame(o_fmt_ctx, msg->pkt);
     if (ret < 0) {
        return MP4_ERROR;
     }
     return ret;
}

CMp4Muxer::CMp4Muxer(string path, int playid, int w, int h, int r)
{
    parser = NULL;
    c = NULL;
    current = NULL;
    this->path = path;
    this->playid = playid;
    width = w;
    height = h;
    framerate = (r > 0 ? r : 25);
    delay_time = DEFAULT_DELAY_TIME;
    thread.SetParam(thread_muxing, this, 500, NULL);
    string fullpath = path;
    const char *tmp = fullpath.c_str();
    if (strcmp(tmp+fullpath.length()-1,"\\") != 0) {
        fullpath += "\\";
    }
	char sPlayid[25]={0};
    int radix = 10;
    _itoa_s(playid, sPlayid, sizeof(sPlayid) - 1, radix);
    index_filename = fullpath + string(sPlayid) + "_record_period.list";
    index_all_filename = fullpath + string(sPlayid) + "_record_history.list";
    is_muxing_start = false;
    next_dts = AV_NOPTS_VALUE;
    dts = AV_NOPTS_VALUE;
    next_pts = AV_NOPTS_VALUE;
    pts = AV_NOPTS_VALUE;
    current_dts = 0;
    printf("path:%s, index_filename:%s, index_all_filename:%s\n", path.c_str(), index_filename.c_str(), index_all_filename.c_str());
}

bool CMp4Muxer::init_muxing(int file_len, int record_period_len, int record_history_len)
{
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

    max_file_length = (file_len > 0 ? file_len : DEFAULT_MAX_FILE_LENGTH);
    int _record_period = (record_period_len > 0 ? record_period_len : DEFAULT_RECORD_PERIOD_TIME);
    int _file_period = (record_history_len > 0 ? record_history_len : DEFAULT_RECORD_HISTORY_TIME);
    double tmp = _record_period / (double)max_file_length;
    record_period_window = (int)ceil(tmp);
    tmp = _file_period / (double)max_file_length;
    record_history_window = (int)ceil(tmp);

    if (create_multi_directory() != true) {
        return false;
    }

    init_segments_period();
    init_segments_history();

    if (segment_open() != MP4_SUCCESS) {
        return false;
    }
    strat_muxing();
    return true;
}

CMp4Muxer::~CMp4Muxer()
{
    thread.Stop();
    safe_freep(current);
    msgs.free();
    msgs.clear();
    if (parser) {
        av_parser_close(parser);
    }
    if (c) {
        avcodec_free_context(&c);
    }
#if 0
    vector<CMp4Segment*>::iterator it;
    for (it = segments_history.begin(); it != segments_history.end(); ++it) {
        CMp4Segment* segment = *it;
        safe_freep(segment);
    }
    segments_period.clear();
    segments_history.clear();
 #endif
}

bool CMp4Muxer::write_packet(AVPacket* pkt)
{
    CAutoMutex lock(&mutex);
    CMessage* msg = new CMessage;
    msg->create(pkt);
    msgs.push_back(msg);
    return true;
}

bool CMp4Muxer::parse_packet(const char* data, unsigned int len)
{
    int ret;
    uint8_t *inbuf = (uint8_t *)data;
    int data_size = len;
    AVPacket out_pkt = { 0 };
    while (data_size > 0) {
        av_init_packet(&out_pkt);
        ret = av_parser_parse2(parser, c, &out_pkt.data, &out_pkt.size,
                               inbuf, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret < 0) {
            //exit(1);
            return false;
        }
        inbuf += ret;
        data_size -= ret;
        if (out_pkt.size) {
            ret = av_packet_make_refcounted(&out_pkt);
            if (ret < 0) {
                //exit(1);
                return false;
            }
            uint8_t *extradata;
            int extradata_size;
            bool key_frame = (parser->key_frame == 1 ||
                (parser->key_frame == -1 && parser->pict_type == AV_PICTURE_TYPE_I));

            if (key_frame && is_muxing_start == false) {
                is_muxing_start = true;
                ret = avcodec_send_packet(c, &out_pkt);
                if (ret < 0) {
                    printf("decode error:%d\n", ret);
                }
                pts = 0;
                AVRational r;
                r.num = framerate;
                r.den = 1;
                dts = framerate ? - c->has_b_frames * AV_TIME_BASE / av_q2d(r) : 0;
                extradata = av_packet_get_side_data(&out_pkt, AV_PKT_DATA_NEW_EXTRADATA,
                                            &extradata_size);
            }
            if (is_muxing_start) {
                if (key_frame) {
                    out_pkt.flags |= AV_PKT_FLAG_KEY;
                }
                if (next_dts == AV_NOPTS_VALUE) {
                    next_dts = dts;
                }
                if (next_pts == AV_NOPTS_VALUE) {
                    next_pts = pts;
                }
                dts = next_dts;
                AVRational r;
                r.num = framerate;
                r.den = 1;
                AVRational _framerate = framerate ? r : c->framerate;
                AVRational time_base_q;
                time_base_q.num = 1;
                time_base_q.den = AV_TIME_BASE;
                int64_t _next_dts = av_rescale_q(next_dts, time_base_q, av_inv_q(_framerate));
                next_dts = av_rescale_q(_next_dts + 1, av_inv_q(_framerate), time_base_q);
                pts = dts;
                next_pts = next_dts;
                out_pkt.dts = av_rescale_q(dts, TIME_BASE, mux_timebase);
                int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(r);
                out_pkt.duration = (double)calc_duration/(double)(av_q2d(in_time_base)*AV_TIME_BASE);
                write_packet(&out_pkt);
            } else {
                av_packet_unref(&out_pkt);
            }
        }
    }

    return true;
}

void CMp4Muxer::set_delay_time(int64_t delay)
{
    if (delay >= DEFAULT_DELAY_TIME) {
        delay_time = delay;
    }
}

int CMp4Muxer::dump_packets(int max_count, CMessage** pmsg, int& count)
{
    int ret = MP4_SUCCESS;
    CAutoMutex lock(&mutex);

    int nb_msgs = (int)msgs.size();
    if (nb_msgs <= 0 || max_count <= 0) {
        return ret;
    }

    count = min(max_count, nb_msgs);
    CMessage** omsgs = msgs.data();
    for (int i = 0; i < count; i++) {
        pmsg[i] = omsgs[i];
    }

    if (count >= nb_msgs) {
        msgs.clear();
    } else {
        msgs.erase(msgs.begin(), msgs.begin() + count);
    }

    return ret;
}

bool CMp4Muxer::create_multi_directory(void)
{
    const char* file_path = path.c_str();
    char temp[256] = {0};
    for (int i = 0; file_path[i] != 0; ++i)
	{
		if (file_path[i] == '\\' || file_path[i] == '/')
		{
			memcpy(temp, file_path, i+1);
			if (!is_directory_exsits(temp))
			{
				BOOL ret = CreateDirectory(temp, NULL);
				if (!ret)
				{
// 					int nRet = GetLastError();
// 					printf("Createdirectory %s error:%d", temp, nRet);
					return false;
				}
			}
		}
	}
	return true;
}

bool CMp4Muxer::is_directory_exsits(const char* file_path)
{
    if (_access(file_path,0) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
	return(GetFileAttributes(file_path) == FILE_ATTRIBUTE_DIRECTORY);
	HANDLE hReuslt = NULL;
	WIN32_FIND_DATA FindFileData = {0};
	hReuslt = FindFirstFile(file_path, &FindFileData);
	if   (hReuslt == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else
	{
		FindClose(hReuslt);
		return true;
	}
}

int CMp4Muxer::thread_muxing(LPVOID lParam)
{
    CMp4Muxer *muxer = (CMp4Muxer *)lParam;
    if (muxer) {
        return muxer->do_muxing();
    }
    return 0;
}

void CMp4Muxer::strat_muxing()
{
    thread.Start();
}

int CMp4Muxer::do_muxing()
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

int CMp4Muxer::segment_open()
{
    int ret = MP4_SUCCESS;
    if (current) {
        //warn
        safe_freep(current);
    }
    string file_name = "";
    CVorxTime tm;
    char stime[20];
    memset(stime, 0, sizeof(stime));
    tm.GetCurrentTime();
    tm.ToString(stime,sizeof(stime),false);
    char tmp[128];
    memset(tmp, 0, sizeof(tmp));
    _snprintf_s(tmp, sizeof(tmp) - 1, "%0.3d_%s", playid, stime);
    file_name = tmp;
    file_name += ".mp4";
    current = new CMp4Segment(path, file_name, width, height, framerate);
    return current->init_segment();
}

int CMp4Muxer::segment_close()
{
    int ret = MP4_SUCCESS;
    if (!current) {
        return MP4_ERROR;
    }
    segment_shrink();
    safe_freep(current);
    current = NULL;
    return ret;
}

int CMp4Muxer::reap_segment()
{
    int ret = MP4_SUCCESS;
    segment_close();
    segment_open();
    return ret;
}

int CMp4Muxer::segment_shrink(bool is_close)
{
    int ret = MP4_SUCCESS;
    if (!current) {
        return MP4_ERROR;
    }
    string filename = current->full_path;
    if (filename == "") {
        return MP4_ERROR;
    }
    refresh_period_list(filename);
    refresh_history_list(filename);
    return ret;
}

int CMp4Muxer::refresh_period_list(string filename)
{
    int ret = MP4_SUCCESS;
    vector<string> _segments_period;
    //update segments_period
    {
        CAutoMutex lock(&mutex_period);
        if (segments_period.empty() || segments_period.back() != filename) {
            segments_period.push_back(filename);
            // shrink the segments_period.
            int record_index = (int)segments_period.size() - record_period_window;
            for (int i = 0; i < record_index && !segments_period.empty(); i++) {
                segments_period.erase(segments_period.begin());
            }
            _segments_period.assign(segments_period.begin(), segments_period.end());
        }
    }
    if (!_segments_period.empty()) {

        //write index file
        ofstream ofs;
        locale loc = locale::global(locale(""));
        ofs.open(index_filename.c_str(), ios::out | ios::trunc);
        locale::global(loc);
        if (!ofs.is_open()) {
            return MP4_ERROR;
        }
        vector<string>::iterator it;
        for (it = _segments_period.begin(); it != _segments_period.end(); it++) {
            string mp4_file = *it;
            ofs << mp4_file.c_str() << endl;
        }
        ofs.close();
    }
    return ret;
}

int CMp4Muxer::refresh_history_list(string filename)
{
    int ret = MP4_SUCCESS;
    map<string, int> _segments_history;
    map<string, int>::iterator it;
    // the segments_period to remove
    vector<string> segment_to_remove;
    int len = current->get_duration() / 1000;//second
    //update segments_history
    {
        CAutoMutex lock(&mutex_history);
        segments_history[filename] = len;
        if (!segments_history.empty()) {
            int remove_index = (int)segments_history.size() - record_history_window;
            for (int i = 0; i < remove_index && !segments_history.empty(); i++) {
                string mp4_file = segments_history.begin()->first;
                segments_history.erase(segments_history.begin());
                segment_to_remove.push_back(mp4_file);
            }
            _segments_history = segments_history;
        }
    }
    if (!_segments_history.empty()) {
        // remove the mp4 file.
        for (int i = 0; i < (int)segment_to_remove.size(); i++) {
            string mp4_file = segment_to_remove[i];
            if (remove(mp4_file.c_str()) != 0) {
                printf("remove %s error\n", mp4_file.c_str());
            }
        }
        segment_to_remove.clear();

        //write index file
        ofstream ofs;
        locale loc = locale::global(locale(""));
        ofs.open(index_all_filename.c_str(), ios::out | ios::trunc);
        locale::global(loc);
        if (!ofs.is_open()) {
            return MP4_ERROR;
        }

        for (it = _segments_history.begin(); it != _segments_history.end(); it++) {
            string mp4_file = it->first;
            int len = it->second;
            char size[256];
            memset(size, 0, sizeof(size));
            _itoa_s(len, size, sizeof(size) - 1, 10);
            string ss = mp4_file + SEPARATOR_CHAR + (string)size;
            ofs << ss.c_str() << endl;
        }
        ofs.close();
    }
    return ret;
}


int CMp4Muxer::mp4_muxing(CMessage* msg)
{
    int ret = MP4_SUCCESS;
    if (!current) {
        return MP4_ERROR;
    }
    int64_t duration = current->get_duration();
    if (duration >= max_file_length && msg->pkt->flags == AV_PKT_FLAG_KEY) {
        ret = reap_segment();
    }
    if (ret != MP4_SUCCESS) {
        return ret;
    }
    current_dts = msg->pkt->dts;
    current->ffmpeg_muxing(msg);
    duration = current->get_duration();
    if (duration >= delay_time) {
        segment_shrink();
    }
    return ret;
}

void CMp4Muxer::init_segments_period()
{
    CAutoMutex lock(&mutex_period);
    ifstream ifs;
    ifs.open(index_filename.c_str(), ios::in);
    if (!ifs.is_open()) {
        return;
    }
    string line;
    while (getline(ifs, line)) {
        segments_period.push_back(line);
    }
}

void CMp4Muxer::init_segments_history()
{
    CAutoMutex lock(&mutex_history);
    ifstream ifs;
    ifs.open(index_all_filename.c_str(), ios::in);
    if (!ifs.is_open()) {
        return;
    }
    string line;
    while (getline(ifs, line)) {
        string::size_type pos;
        string filename;
        int len;
        if ((pos = line.find(SEPARATOR_CHAR)) != string::npos) {
            filename = line.substr(0, pos);
            string size = line.substr(pos + 1);
            len = atoi(size.c_str());
            segments_history[filename] = len;
        }
    }
}

int64_t CMp4Muxer::get_current_dts()
{
    return current_dts;
}

void CMp4Muxer::get_record_list(vector<string> &vec)
{
    CAutoMutex lock(&mutex_period);
    vec.assign(segments_period.begin(), segments_period.end());
    return;
}

void CMp4Muxer::get_record_history(map<string, int> &history)
{
    CAutoMutex lock(&mutex_history);
    history = segments_history;
    return;
}

