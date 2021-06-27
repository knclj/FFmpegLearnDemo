//
// Created by cailianjun on 6/6/21.
//

#include "AudioChannel.h"

AudioChannel::AudioChannel(int index, AVCodecContext *codeContext,AVRational base_time) : BaseChannel(index, codeContext,base_time) {

    out_channals = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_samplesRate = 44100;
    out_samples_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

    out_bufers_size = out_samplesRate*out_samples_size*out_channals;
    out_buffers = static_cast<uint8_t *>(malloc(out_bufers_size));


    /**
      * @param s               existing Swr context if available, or NULL if not
 * @param out_ch_layout   output channel layout (AV_CH_LAYOUT_*)
 * @param out_sample_fmt  output sample format (AV_SAMPLE_FMT_*).
 * @param out_sample_rate output sample rate (frequency in Hz)
 * @param in_ch_layout    input channel layout (AV_CH_LAYOUT_*)
 * @param in_sample_fmt   input sample format (AV_SAMPLE_FMT_*).
 * @param in_sample_rate  input sample rate (frequency in Hz)
 * @param log_offset      logging level offset
 * @param log_ctx         parent logging context, can be NULL
     */
 swr_ctx =    swr_alloc_set_opts(0,
            AV_CH_LAYOUT_STEREO,
            AV_SAMPLE_FMT_S16,
            out_samplesRate,

            codeContext->channel_layout,
            codeContext->sample_fmt,
            codeContext->sample_rate,
            0,0);
 swr_init(swr_ctx);
}

AudioChannel::~AudioChannel() {

}

void *task_audio_decode(void *args){

    AudioChannel * channel = static_cast<AudioChannel *>(args);

    channel->decode();
    return nullptr;
}
void *task_audio_play(void *args){
    AudioChannel * channel = static_cast<AudioChannel *>(args);
    channel->play();
    return nullptr;
}



void AudioChannel::start() {
    isPlaying = true;

    packets.setWork(1);
    frames.setWork(1);

    pthread_create(&pthread_audio_decode, 0, task_audio_decode, this);
    pthread_create(&pthread_audio_play, 0, task_audio_play, this);
}

void AudioChannel::stop() {

}

/**
 * 子线程调用开始解码
 */
void AudioChannel::decode() {

    AVPacket * pkt;
    while (isPlaying){

        if(frames.size() >100){
            av_usleep(10*100);
            continue;
        }

       int ret =  packets.getQueueAndDel(pkt);
        if(!isPlaying){
            break;
        }
        if(!ret){
            continue;
        }
        ret = avcodec_send_packet(codecContext,pkt);

        if(ret){
            break;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext,frame);
        if(ret == AVERROR(EAGAIN)){
            continue;
        }else if(ret != 0){
            break;
        }
        frames.insertToQueue(frame);
        av_packet_unref(pkt);
        releaseAVPacket(&pkt);
    }//while end.
    av_packet_unref(pkt);
    releaseAVPacket(&pkt);
}

void audioPlayCallback(SLAndroidSimpleBufferQueueItf bq,void *args){
    AudioChannel* audioChannel = static_cast<AudioChannel*>(args);
    int pcmSize =  audioChannel->getPCMSize();
    (*bq)->Enqueue(bq,audioChannel->out_buffers,pcmSize);
}

void AudioChannel::play() {

    SLresult result;

    //创建引擎
    result = slCreateEngine(&audioEngine,0,0,0,0,0);

    if(result != SL_RESULT_SUCCESS){
        LOGE("play result failed")
        return;
    }
    //初始化
    result = (*audioEngine)->Realize(audioEngine,SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS){
        LOGE("audioEngine realize failed")
        return;
    }
    //获取引擎接口
    result = (*audioEngine)->GetInterface(audioEngine,SL_IID_ENGINE,&audioEngineInferface);
    if(result != SL_RESULT_SUCCESS){
        LOGE("GetInterface realize failed")
        return;
    }

    if(audioEngineInferface){
        LOGI("audioEngineInferface create success")
    }else{
        LOGE("audioEngineInferface create failed")
    }

    //接口设置混响音器
    result = (*audioEngineInferface)->CreateOutputMix(audioEngineInferface,&outputMix,0,0,0);
    if(result != SL_RESULT_SUCCESS){
        LOGE("audioEngine realize failed")
        return;
    }
    result = (*outputMix)->Realize(outputMix,SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS){
        LOGE("audioEngine realize failed")
        return;
    }

    result = (*outputMix)->GetInterface(outputMix,SL_IID_ENVIRONMENTALREVERB,&outputMixInterface);

    if(result == SL_RESULT_SUCCESS){
        LOGE("outputMixInterface realize sucess");
        // 设置混响 ： 默认。
//        SL_I3DL2_ENVIRONMENT_PRESET_ROOM: 室内
//        SL_I3DL2_ENVIRONMENT_PRESET_AUDITORIUM : 礼堂 等
        const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
        (*outputMixInterface)->SetEnvironmentalReverbProperties(outputMixInterface,&settings);

    }

    //创建播放器
    //播放器缓冲队列
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,10};

    /**
     * 	SLuint32 		formatType;
	SLuint32 		numChannels;
	SLuint32 		samplesPerSec;
	SLuint32 		bitsPerSample;
	SLuint32 		containerSize;
	SLuint32 		channelMask;
	SLuint32		endianness;
     */
    SLDataFormat_PCM format_pcm ={
        SL_DATAFORMAT_PCM,
        2,
        SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT,
        SL_BYTEORDER_LITTLEENDIAN
    };

    SLDataSource audioSrc = {&loc_bufq,&format_pcm};

    SLDataLocator_OutputMix loc_OutputMix = {SL_DATALOCATOR_OUTPUTMIX,outputMix};
    SLDataSink audioSink = {&loc_OutputMix,NULL};

    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    result =  (*audioEngineInferface)->CreateAudioPlayer(
            audioEngineInferface,
            &audioPlayer,
            &audioSrc,
            &audioSink,
            1,
            ids,
            req);

    if(result != SL_RESULT_SUCCESS){
        LOGE("CreateAudioPlayer failed")
        return;
    }

    result = (*audioPlayer)->Realize(audioPlayer,SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS){
        LOGE("AudioPlayer Realize failed")
        return;
    }

    result =  (*audioPlayer)->GetInterface(audioPlayer,SL_IID_PLAY,&audioPlayerInteface);


    LOGI("CreateAudioPlayer success")

    result = (*audioPlayer)->GetInterface(audioPlayer,SL_IID_BUFFERQUEUE,&audioPlayerBuf);
    if(result != SL_RESULT_SUCCESS){
        LOGE("AudioPlayer SL_IID_BUFFERQUEUE failed")
        return;
    }

    (*audioPlayerBuf)->RegisterCallback(audioPlayerBuf, audioPlayCallback, this);

    (*audioPlayerInteface)->SetPlayState(audioPlayerInteface,SL_PLAYSTATE_PLAYING);

    //调用启动播放
    audioPlayCallback(audioPlayerBuf,this);

}

int AudioChannel::getPCMSize() {
    int format_data_pcm = 0;
    AVFrame  *frame;
    if(isPlaying){
      int ret =  frames.getQueueAndDel(frame);
      if(!ret){
          return format_data_pcm;
      }
     int dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx,frame->sample_rate)+frame->nb_samples,
              out_samplesRate,
              frame->sample_rate,
              AV_ROUND_UP);
     int sample_per_channel = swr_convert(swr_ctx, &out_buffers, dst_nb_samples,
                  (const uint8_t**)(frame->data), frame->nb_samples);
      format_data_pcm = sample_per_channel*out_samples_size*out_channals;
    }

    audio_time = frame->best_effort_timestamp*av_q2d(base_time);

    av_frame_unref(frame);
    releaseAVFrame(&frame);
    return format_data_pcm;
}
