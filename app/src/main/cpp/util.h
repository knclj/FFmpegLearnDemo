//
// Created by cailianjun on 6/6/21.
//

#ifndef FFMPEGLEARNDEMO_UTIL_H
#define FFMPEGLEARNDEMO_UTIL_H
#define THREAD_MAIN 1 // 主线程
#define THREAD_CHILD 2 // 子线程

#include <android/log.h>
#include <queue>
#include <pthread.h>
// 错误代码
// 打不开视频
#define FFMPEG_CAN_NOT_OPEN_URL 1
// 找不到流媒体
#define FFMPEG_CAN_NOT_FIND_STREAMS 2
// 找不到解码器
#define FFMPEG_FIND_DECODER_FAIL 3
// 无法根据解码器创建上下文
#define FFMPEG_ALLOC_CODEC_CONTEXT_FAIL 4
// 根据流信息 配置上下文参数失败
#define FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL 6
// 打开解码器失败
#define FFMPEG_OPEN_DECODER_FAIL 7
// 没有音视频
#define FFMPEG_NOMEDIA 8

#define TAG "NativePlayer"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__);
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__);
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG,  __VA_ARGS__);



#endif //FFMPEGLEARNDEMO_UTIL_H
