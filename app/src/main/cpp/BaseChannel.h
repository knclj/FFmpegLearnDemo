//
// Created by cailianjun on 6/13/21.
//

#ifndef FFMPEGLEARNDEMO_BASECHANNEL_H
#define FFMPEGLEARNDEMO_BASECHANNEL_H
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/time.h>
};
#include "util.h"
#include "SafeQueue.h"

class BaseChannel {

public:
    int stream_index;
    AVCodecContext *codecContext;
    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;
    bool isPlaying;
    AVRational  base_time;
    BaseChannel(int index,AVCodecContext *codeContext,AVRational base_time);
    virtual ~BaseChannel();

    static void releaseAVPacket(AVPacket **p){
        if(p){
            av_packet_free(p);
            *p = 0;
        }

    }

    static void releaseAVFrame(AVFrame **f){
        if(f){
            av_frame_free(f);
            *f = 0;
        }
    }

};


#endif //FFMPEGLEARNDEMO_BASECHANNEL_H
