#pragma once
#ifndef _COMMON_H_
#define _COMMON_H_

#define MAX_PLAY_CHN 9

#include <iostream>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include <math.h>
#include "AutoFree.h"
using namespace std;

#include "Classes/VorxTimer.h"
#include "Classes/VorxTime.h"
#include "Classes/MutexInteger.h"
#include "Classes/VorxThread.h"
using namespace vfc;
#define PRINT_  0

extern "C"
{
    #include "ffmpeg/include/libavcodec/avcodec.h"
    #include "ffmpeg/include/libavformat/avformat.h"
    #include "ffmpeg/include/libavutil/channel_layout.h"
    #include "ffmpeg/include/libavutil/common.h"
    #include "ffmpeg/include/libavutil/imgutils.h"
    #include "ffmpeg/include/libavutil/imgutils.h"
    #include "ffmpeg/include/libavutil/opt.h"
    #include "ffmpeg/include/libavutil/mathematics.h"
    #include "ffmpeg/include/libavutil/samplefmt.h"
    #include "ffmpeg/include/libavutil/avutil.h"
	#include "ffmpeg/include/libswscale/swscale.h"
	#include "ffmpeg/include/libswresample/swresample.h"
	#include "ffmpeg/include/libavdevice/avdevice.h"
	#include "ffmpeg/include/libavfilter/avfilter.h"
	#include "ffmpeg/include/libavfilter/buffersink.h"
	#include "ffmpeg/include/libavfilter/buffersrc.h"
	#include "ffmpeg/include/libpostproc/postprocess.h"
};
#pragma comment(lib, "ffmpeg/lib/avcodec.lib")
#pragma comment(lib, "ffmpeg/lib/avformat.lib")
#pragma comment(lib, "ffmpeg/lib/avdevice.lib")
#pragma comment(lib, "ffmpeg/lib/avfilter.lib")
#pragma comment(lib, "ffmpeg/lib/avutil.lib")
#pragma comment(lib, "ffmpeg/lib/postproc.lib")
#pragma comment(lib, "ffmpeg/lib/swresample.lib")
#pragma comment(lib, "ffmpeg/lib/swscale.lib")

//#include "Classes/stdint.h"
#pragma comment(lib,"d3d/D3DVideoRender.lib")
#include <d3d9.h>
#pragma comment(lib,"d3d9.lib")

#define MAX_READ_PACKETS                    128
#define DEFAULT_DELAY_TIME                  7*1000//10s
#define MP4_SUCCESS                         0
#define MP4_ERROR                           1
#if 0
#define DEFAULT_MAX_FILE_LENGTH             30*60*1000//30min
#define DEFAULT_RECORD_PERIOD_TIME          2*60*60*1000//2h
#define DEFAULT_RECORD_HISTORY_TIME         24*60*60*1000//24h
#else
#define DEFAULT_MAX_FILE_LENGTH             10*1000//10s
#define DEFAULT_RECORD_PERIOD_TIME          2*60*1000//2min
#define DEFAULT_RECORD_HISTORY_TIME         5*60*60*1000//5min
#endif
#if _MSC_VER
#define snprintf _snprintf
#endif

extern AVRational in_time_base;
extern AVRational mux_timebase;
extern AVRational TIME_BASE;


// free the p and set to NULL.
// p must be a T*.
#define safe_freep(p) \
    if (p) { \
        delete p; \
        p = NULL; \
    } \
    (void)0
// please use the freepa(T[]) to free an array,
// or the behavior is undefined.
#define safe_freepa(pa) \
    if (pa) { \
        delete[] pa; \
        pa = NULL; \
    } \
    (void)0

#if 0
#define UINT64 unsigned long long
#define INT64 long long
#define BYTE unsigned char
#define ULONG unsigned long
#define UINT unsigned int
#define WORD unsigned short
#define DWORD unsigned long
#define LPCTSTR const char*
#define BOOL int
#define TRUE 1
#define FALSE 0
#define LPVOID void*
#define HANDLE void*
#define UINT32 unsigned long
#define INT32 long
#endif

enum Tz_PixelFormat {
    TZ_PIX_FMT_NONE= -1,
    TZ_PIX_FMT_YUV420P,   ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    TZ_PIX_FMT_YUYV422,   ///< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
    TZ_PIX_FMT_RGB24,     ///< packed RGB 8:8:8, 24bpp, RGBRGB...
    TZ_PIX_FMT_BGR24,     ///< packed RGB 8:8:8, 24bpp, BGRBGR...
    TZ_PIX_FMT_YUV422P,   ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    TZ_PIX_FMT_YUV444P,   ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    TZ_PIX_FMT_YUV410P,   ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    TZ_PIX_FMT_YUV411P,   ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
    TZ_PIX_FMT_GRAY8,     ///<        Y        ,  8bpp
    TZ_PIX_FMT_MONOWHITE, ///<        Y        ,  1bpp, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb
    TZ_PIX_FMT_MONOBLACK, ///<        Y        ,  1bpp, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb
    TZ_PIX_FMT_PAL8,      ///< 8 bit with PIX_FMT_RGB32 palette
    TZ_PIX_FMT_YUVJ420P,  ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of PIX_FMT_YUV420P and setting color_range
    TZ_PIX_FMT_YUVJ422P,  ///< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in favor of PIX_FMT_YUV422P and setting color_range
    TZ_PIX_FMT_YUVJ444P,  ///< planar YUV 4:4:4, 24bpp, full scale (JPEG), deprecated in favor of PIX_FMT_YUV444P and setting color_range
    TZ_PIX_FMT_XVMC_MPEG2_MC,///< XVideo Motion Acceleration via common packet passing
    TZ_PIX_FMT_XVMC_MPEG2_IDCT,
    TZ_PIX_FMT_UYVY422,   ///< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
    TZ_PIX_FMT_UYYVYY411, ///< packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
    TZ_PIX_FMT_BGR8,      ///< packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
    TZ_PIX_FMT_BGR4,      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1B 2G 1R(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    TZ_PIX_FMT_BGR4_BYTE, ///< packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
    TZ_PIX_FMT_RGB8,      ///< packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
    TZ_PIX_FMT_RGB4,      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    TZ_PIX_FMT_RGB4_BYTE, ///< packed RGB 1:2:1,  8bpp, (msb)1R 2G 1B(lsb)
    TZ_PIX_FMT_NV12,      ///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    TZ_PIX_FMT_NV21,      ///< as above, but U and V bytes are swapped

    TZ_PIX_FMT_ARGB,      ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
    TZ_PIX_FMT_RGBA,      ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
    TZ_PIX_FMT_ABGR,      ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
    TZ_PIX_FMT_BGRA,      ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...

    TZ_PIX_FMT_GRAY16BE,  ///<        Y        , 16bpp, big-endian
    TZ_PIX_FMT_GRAY16LE,  ///<        Y        , 16bpp, little-endian
    TZ_PIX_FMT_YUV440P,   ///< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
    TZ_PIX_FMT_YUVJ440P,  ///< planar YUV 4:4:0 full scale (JPEG), deprecated in favor of PIX_FMT_YUV440P and setting color_range
    TZ_PIX_FMT_YUVA420P,  ///< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
    TZ_PIX_FMT_VDPAU_H264,///< H.264 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    TZ_PIX_FMT_VDPAU_MPEG1,///< MPEG-1 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    TZ_PIX_FMT_VDPAU_MPEG2,///< MPEG-2 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    TZ_PIX_FMT_VDPAU_WMV3,///< WMV3 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    TZ_PIX_FMT_VDPAU_VC1, ///< VC-1 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    TZ_PIX_FMT_RGB48BE,   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as big-endian
    TZ_PIX_FMT_RGB48LE,   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as little-endian

    TZ_PIX_FMT_RGB565BE,  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), big-endian
    TZ_PIX_FMT_RGB565LE,  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), little-endian
    TZ_PIX_FMT_RGB555BE,  ///< packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), big-endian, most significant bit to 0
    TZ_PIX_FMT_RGB555LE,  ///< packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), little-endian, most significant bit to 0

    TZ_PIX_FMT_BGR565BE,  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), big-endian
    TZ_PIX_FMT_BGR565LE,  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), little-endian
    TZ_PIX_FMT_BGR555BE,  ///< packed BGR 5:5:5, 16bpp, (msb)1A 5B 5G 5R(lsb), big-endian, most significant bit to 1
    TZ_PIX_FMT_BGR555LE,  ///< packed BGR 5:5:5, 16bpp, (msb)1A 5B 5G 5R(lsb), little-endian, most significant bit to 1

    TZ_PIX_FMT_VAAPI_MOCO, ///< HW acceleration through VA API at motion compensation entry-point, Picture.data[3] contains a vaapi_render_state struct which contains macroblocks as well as various fields extracted from headers
    TZ_PIX_FMT_VAAPI_IDCT, ///< HW acceleration through VA API at IDCT entry-point, Picture.data[3] contains a vaapi_render_state struct which contains fields extracted from headers
    TZ_PIX_FMT_VAAPI_VLD,  ///< HW decoding through VA API, Picture.data[3] contains a vaapi_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers

    TZ_PIX_FMT_YUV420P16LE,  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    TZ_PIX_FMT_YUV420P16BE,  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    TZ_PIX_FMT_YUV422P16LE,  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    TZ_PIX_FMT_YUV422P16BE,  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    TZ_PIX_FMT_YUV444P16LE,  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    TZ_PIX_FMT_YUV444P16BE,  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    TZ_PIX_FMT_VDPAU_MPEG4,  ///< MPEG4 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    TZ_PIX_FMT_DXVA2_VLD,    ///< HW decoding through DXVA2, Picture.data[3] contains a LPDIRECT3DSURFACE9 pointer

    TZ_PIX_FMT_RGB444LE,  ///< packed RGB 4:4:4, 16bpp, (msb)4A 4R 4G 4B(lsb), little-endian, most significant bits to 0
    TZ_PIX_FMT_RGB444BE,  ///< packed RGB 4:4:4, 16bpp, (msb)4A 4R 4G 4B(lsb), big-endian, most significant bits to 0
    TZ_PIX_FMT_BGR444LE,  ///< packed BGR 4:4:4, 16bpp, (msb)4A 4B 4G 4R(lsb), little-endian, most significant bits to 1
    TZ_PIX_FMT_BGR444BE,  ///< packed BGR 4:4:4, 16bpp, (msb)4A 4B 4G 4R(lsb), big-endian, most significant bits to 1
    TZ_PIX_FMT_GRAY8A,    ///< 8bit gray, 8bit alpha
    TZ_PIX_FMT_BGR48BE,   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as big-endian
    TZ_PIX_FMT_BGR48LE,   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as little-endian

    //the following 10 formats have the disadvantage of needing 1 format for each bit depth, thus
    //If you want to support multiple bit depths, then using PIX_FMT_YUV420P16* with the bpp stored seperately
    //is better
    TZ_PIX_FMT_YUV420P9BE, ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    TZ_PIX_FMT_YUV420P9LE, ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    TZ_PIX_FMT_YUV420P10BE,///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    TZ_PIX_FMT_YUV420P10LE,///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    TZ_PIX_FMT_YUV422P10BE,///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    TZ_PIX_FMT_YUV422P10LE,///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    TZ_PIX_FMT_YUV444P9BE, ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    TZ_PIX_FMT_YUV444P9LE, ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    TZ_PIX_FMT_YUV444P10BE,///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    TZ_PIX_FMT_YUV444P10LE,///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian

    TZ_PIX_FMT_NB,        ///< number of pixel formats, DO NOT USE THIS if you want to link with shared libav* because the number of formats might differ between versions
};

enum HEVCNALUnitType {
	HEVC_NAL_TRAIL_N    = 0,
	HEVC_NAL_TRAIL_R    = 1,
	HEVC_NAL_TSA_N      = 2,
	HEVC_NAL_TSA_R      = 3,
	HEVC_NAL_STSA_N     = 4,
	HEVC_NAL_STSA_R     = 5,
	HEVC_NAL_RADL_N     = 6,
	HEVC_NAL_RADL_R     = 7,
	HEVC_NAL_RASL_N     = 8,
	HEVC_NAL_RASL_R     = 9,
	HEVC_NAL_VCL_N10    = 10,
	HEVC_NAL_VCL_R11    = 11,
	HEVC_NAL_VCL_N12    = 12,
	HEVC_NAL_VCL_R13    = 13,
	HEVC_NAL_VCL_N14    = 14,
	HEVC_NAL_VCL_R15    = 15,
	HEVC_NAL_BLA_W_LP   = 16,
	HEVC_NAL_BLA_W_RADL = 17,
	HEVC_NAL_BLA_N_LP   = 18,
	HEVC_NAL_IDR_W_RADL = 19,
	HEVC_NAL_IDR_N_LP   = 20,
	HEVC_NAL_CRA_NUT    = 21,
	HEVC_NAL_IRAP_VCL22 = 22,
	HEVC_NAL_IRAP_VCL23 = 23,
	HEVC_NAL_RSV_VCL24  = 24,
	HEVC_NAL_RSV_VCL25  = 25,
	HEVC_NAL_RSV_VCL26  = 26,
	HEVC_NAL_RSV_VCL27  = 27,
	HEVC_NAL_RSV_VCL28  = 28,
	HEVC_NAL_RSV_VCL29  = 29,
	HEVC_NAL_RSV_VCL30  = 30,
	HEVC_NAL_RSV_VCL31  = 31,
	HEVC_NAL_VPS        = 32,
	HEVC_NAL_SPS        = 33,
	HEVC_NAL_PPS        = 34,
	HEVC_NAL_AUD        = 35,
	HEVC_NAL_EOS_NUT    = 36,
	HEVC_NAL_EOB_NUT    = 37,
	HEVC_NAL_FD_NUT     = 38,
	HEVC_NAL_SEI_PREFIX = 39,
	HEVC_NAL_SEI_SUFFIX = 40,
};
#endif
