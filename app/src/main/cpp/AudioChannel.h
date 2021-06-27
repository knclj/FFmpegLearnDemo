//
// Created by cailianjun on 6/6/21.
//

#ifndef FFMPEGLEARNDEMO_AUDIOCHANNEL_H
#define FFMPEGLEARNDEMO_AUDIOCHANNEL_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "BaseChannel.h"

extern "C"{
#include <libswresample/swresample.h>
};


class AudioChannel : public BaseChannel{

public:
    pthread_t pthread_audio_decode;
    pthread_t pthread_audio_play;

public:

    SLObjectItf audioEngine;
    SLEngineItf audioEngineInferface;
    SLObjectItf outputMix;
    SLEnvironmentalReverbItf outputMixInterface;
    SLObjectItf audioPlayer;
    SLPlayItf audioPlayerInteface;
    SLAndroidSimpleBufferQueueItf audioPlayerBuf;

    uint8_t * out_buffers = 0;
    int out_channals;
    int out_samplesRate;
    int out_samples_size;
    int out_bufers_size;
    SwrContext *swr_ctx;
    double audio_time;

    AudioChannel(int index, AVCodecContext *codeContext,AVRational base_time);
    ~AudioChannel();
    void start();
    void stop();

    void decode();

    void play();


    int getPCMSize();



};


#endif //FFMPEGLEARNDEMO_AUDIOCHANNEL_H
