#pragma once
#pragma warning (disable:4819)

#ifndef _TZ_MP4_H_
#define _TZ_MP4_H_

#ifdef TZ_MP4_EXPORTS
#define TZ_MP4 _declspec(dllexport)
#else
#define TZ_MP4 _declspec(dllimport)
#endif
#ifdef __cplusplus
#include <string>
using namespace std;

extern "C"
{
    /*功能说明：初始化接口
     *参数�?     *[out]lPlayID：操作句柄，后面接口都通过改句柄操�?     *[in]sFilePath:录像存储路径
     *[in]w:图像�?     *[in]h:图像�?     *[in]frameRate:视频帧率
     *[return] true 成功  false 失败
    */
    TZ_MP4 bool  __stdcall  open_mp4(unsigned int &lPlayID,const char* sFilePath, unsigned int w,unsigned int h,unsigned int frameRate);
    /*功能说明：数据写入接�?     *参数�?     *[in]lPlayID：操作句�?     *[in]sData:录像数据
     *[in]nDateLen:录像数据长度
     *[return] true 成功  false 失败
    */
    TZ_MP4 bool __stdcall   write_frame(unsigned int lPlayID,const char* sData,unsigned int nDateLen);
    /*功能说明：获取时长接�?     *参数�?     *[in]lPlayID：操作句�?     *[out]ts:总时�?     *[out]cur_ts:当前时间
     *[return] true 成功  false 失败
    */
    TZ_MP4 bool __stdcall   play_ts(unsigned int lPlayID,unsigned int &ts,unsigned int &cur_ts);
    /*功能说明：播放接�?     *参数�?     *[in]lPlayID：操作句�?     *[in]hWnd:录像播放窗口句柄
     *[return] true 成功  false 失败
    */
    TZ_MP4 bool __stdcall   play_start(unsigned int lPlayID,unsigned int hWnd);
    /*功能说明：暂停播放接�?     *参数�?     *[in]lPlayID：操作句�?     *[return] true 成功  false 失败
    */
    TZ_MP4 bool __stdcall   play_pause(unsigned int lPlayID);
    /*功能说明：继续播放接�?     *参数�?     *[in]lPlayID：操作句�?     *[return] true 成功  false 失败
    */
    TZ_MP4 bool __stdcall   play_resume(unsigned int lPlayID);
    /*功能说明：下一帧播放接�?     *参数�?     *[in]lPlayID：操作句�?     *[return] true 成功  false 失败
    */
    TZ_MP4 bool __stdcall   play_step(unsigned int lPlayID);
    /*功能说明：上一帧播放接�?     *参数�?     *[in]lPlayID：操作句�?     *[return] true 成功  false 失败
    */
    TZ_MP4 bool __stdcall   play_step_prev(unsigned int lPlayID);

    /*功能说明：定位播放接�?     *参数�?     *[in]lPlayID：操作句�?     *[in]start_time:定位时间
     *[return] true 成功  false 失败
    */
    TZ_MP4 bool __stdcall   play_start_time(unsigned int lPlayID,unsigned int start_time);
    /*功能说明：开始另存为文件接口
     *参数�?     *[in]lPlayID：操作句�?     *[in]sSavePath:文件路径
     *[return] true 成功  false 失败
    */
    TZ_MP4 bool __stdcall   play_save_start(unsigned int lPlayID,const char* sSavePath);
    /*功能说明：结束另存为文件接口
     *参数�?     *[in]lPlayID：操作句�?     *[return] true 成功  false 失败
    */
    TZ_MP4 bool __stdcall   play_save_stop(unsigned int lPlayID);
    /*功能说明：倍速播放接�?     *参数�?     *[in]lPlayID：操作句�?     *[in]speed:速度
     *[return] true 成功  false 失败
    */
    TZ_MP4 bool __stdcall   play_speed(unsigned int lPlayID,int speed);
    /*功能说明：停止播放接�?     *参数�?     *[in]lPlayID：操作句�?     *[return] true 成功  false 失败
    */
    TZ_MP4 bool __stdcall   play_stop(unsigned int lPlayID);
    /*功能说明：反初始化接�?     *参数�?     *[in]lPlayID：操作句�?     *[return] true 成功  false 失败
    */
    TZ_MP4 bool __stdcall   close_mp4(unsigned int lPlayID);
};
#endif
#endif


