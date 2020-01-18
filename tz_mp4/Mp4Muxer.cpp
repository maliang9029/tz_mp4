#include "Mp4Muxer.h"
#include <io.h>

CMessage::CMessage()
{
    data = NULL;
    size = 0;
}

CMessage::~CMessage()
{
    safe_freepa(data);
    size = 0;
}

void CMessage::create(char* buf, int len)
{
    data = new char[len];
    memcpy(data, buf, len);
    size = len;
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

CMp4Segment::CMp4Segment()
{
}


CMp4Segment::~CMp4Segment()
{
}

double CMp4Segment::get_duration()
{
    return duration;
}

void CMp4Segment::update_duration(double segment_duration)
{
    return;
}

CMp4Muxer::CMp4Muxer()
{
}


CMp4Muxer::~CMp4Muxer()
{
    msgs.free();
    msgs.clear();
}

bool CMp4Muxer::write_packet(const char* data, unsigned int len)
{
    CAutoMutex lock(&mutex);
    CMessage* msg = new CMessage;
    msg->create((char *)data, len);
    msgs.push_back(msg);
    return true;
}

int CMp4Muxer::dump_packets(int max_count, CMessage** pmsg, int& count)
{
    int ret = MP4_ERROR_SUCCESS;
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

int CMp4Muxer::do_muxing()
{
    return 1;
}

int CMp4Muxer::segment_open()
{
    int ret = MP4_ERROR_SUCCESS;
    return ret;
}

int CMp4Muxer::segment_close()
{
    int ret = MP4_ERROR_SUCCESS;
    return ret;
}

int CMp4Muxer::reap_segment()
{
    int ret = MP4_ERROR_SUCCESS;
    return ret;
}

int CMp4Muxer::segment_shrink()
{
    int ret = MP4_ERROR_SUCCESS;
    return ret;
}

int CMp4Muxer::refresh_file_list()
{
    int ret = MP4_ERROR_SUCCESS;
    return ret;
}

int CMp4Muxer::mp4_muxing()
{
    int ret = MP4_ERROR_SUCCESS;
    return ret;
}


