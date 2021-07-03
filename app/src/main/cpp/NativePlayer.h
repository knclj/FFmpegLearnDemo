
#ifndef FFMPEGLEARNDEMO_NATIVEPLAYER_H
#define FFMPEGLEARNDEMO_NATIVEPLAYER_H

#include <cstring>
#include <pthread.h>
#include "JNICallbackHelper.h"
#include "util.h"
#include "VideoChannel.h"
#include "AudioChannel.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
};
class NativePlayer {
private:
    char *data_source = 0;
    pthread_t  pid_prepare;
    pthread_t  pid_start;
    AVFormatContext* formatContext;
    JNICallbackHelper* helper = 0;
    VideoChannel* videoChannel = 0;
    AudioChannel* audioChannel = 0;
    bool isPlaying;
    RenderCallback renderCallback;
    int duration = 0;
    pthread_mutex_t seek_mutex;


public:
    NativePlayer(const char *data_source,JNICallbackHelper* callbackHelper);
    ~NativePlayer();
    void prepare();
    void start();
    void prepare_();
    void start_();
    void setRenderCallback(RenderCallback callback);


    void stop();

    int getDuration();

    void seek(int progress);
};


#endif //FFMPEGLEARNDEMO_NATIVEPLAYER_H
