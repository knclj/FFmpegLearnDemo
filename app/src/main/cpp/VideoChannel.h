//
// Created by cailianjun on 6/6/21.
//

#ifndef FFMPEGLEARNDEMO_VIDEOCHANNEL_H
#define FFMPEGLEARNDEMO_VIDEOCHANNEL_H
#include "BaseChannel.h"
#include "AudioChannel.h"
extern "C"{
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
};
/**
 *
 */
typedef void(* RenderCallback)(uint8_t* data,int w,int h,int lineSize);

class VideoChannel : public BaseChannel{

private:
    pthread_t pthread_video_decode;
    pthread_t pthread_video_play;
    RenderCallback renderCallback;
    AudioChannel *audioChannel = 0;
    int fps;

public:
    VideoChannel(int streamIndex,AVCodecContext * codecContext,AVRational base_time,int fps);
    ~VideoChannel();
    void start();
    void stop();
    void video_play();
    void video_decode();
    void setRenderCallback(RenderCallback callback);
    void setAudioChannel(AudioChannel *audioChannel);
};


#endif //FFMPEGLEARNDEMO_VIDEOCHANNEL_H
