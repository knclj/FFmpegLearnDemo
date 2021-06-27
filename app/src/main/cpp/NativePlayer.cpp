#include "NativePlayer.h"

NativePlayer::NativePlayer(const char *data_source, JNICallbackHelper* helper) : helper(helper) {
    this->data_source = new char[strlen(data_source)+1];
    strcpy(this->data_source,data_source);
    this->helper = helper;
}

void * task_prepare(void *args){
    auto *player = (NativePlayer *)args;
    player->prepare_();
    return 0;
}

//子线程调用
void NativePlayer::prepare_() {
    //准备ffmpeg 框架数据
    formatContext = avformat_alloc_context();

    AVDictionary* avDictionary = 0;
    av_dict_set(&avDictionary,"timeout","5000000",0);

    int r = avformat_open_input(&formatContext,data_source,0,&avDictionary);

    av_dict_free(&avDictionary);
    if(r){
        LOGD("the open input path %s, error code:%d,the error string:%s",data_source,r,av_err2str(r));
        this->helper->onError(THREAD_CHILD,FFMPEG_CAN_NOT_OPEN_URL);
        return;
    }

    r = avformat_find_stream_info(formatContext,0);
    if(r < 0){
        this->helper->onError(THREAD_CHILD,FFMPEG_CAN_NOT_FIND_STREAMS);
        return;
    }

    for (int i = 0; i < formatContext->nb_streams; ++i) {
       AVStream *stream = formatContext->streams[i];
       AVCodecParameters*  parameters = stream->codecpar;
       AVCodec *codec = avcodec_find_decoder(parameters->codec_id);
       if(!codec){
           if(this->helper){
               helper->onError(THREAD_CHILD,FFMPEG_FIND_DECODER_FAIL);
           }
       }
       AVCodecContext *codecContext = avcodec_alloc_context3(codec);
       if(!codecContext){
           this->helper->onError(THREAD_CHILD,FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
           return;
       }
      r = avcodec_parameters_to_context(codecContext,parameters);
       if(r < 0){
           this->helper->onError(THREAD_CHILD,FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
           return;
       }
      r = avcodec_open2(codecContext,codec,0);
       if(r < 0){
           this->helper->onError(THREAD_CHILD,FFMPEG_OPEN_DECODER_FAIL);
           return;
       }

       AVRational base_time = stream->time_base;

       if(codec->type == AVMEDIA_TYPE_VIDEO){

           if(stream->disposition & AV_DISPOSITION_ATTACHED_PIC){
               continue;
           }
            AVRational fps_rational = stream->avg_frame_rate;
           int fps = av_q2d(fps_rational);
            videoChannel = new VideoChannel(i,codecContext,base_time,fps);
            videoChannel->setRenderCallback(renderCallback);
       }else if(codec->type == AVMEDIA_TYPE_AUDIO){
            audioChannel = new AudioChannel(i, codecContext,base_time);
       }
    }//for end
    if(!videoChannel && !audioChannel) {
        this->helper->onError(THREAD_CHILD,FFMPEG_NOMEDIA);
        return;
    }

    if (helper) {
        helper->onPrepared(THREAD_CHILD);
    }
}

NativePlayer::~NativePlayer() {
    if(data_source){
        delete data_source;
        data_source = nullptr;
    }
}

void NativePlayer::prepare() {
    //主线程，需要子线程工作
    pthread_create(&pid_prepare,0,task_prepare, this);
}

void *startTask(void *parms){
    NativePlayer* player = static_cast<NativePlayer*>(parms) ;
    player->start_();
    return 0;
}

void NativePlayer::start() {
    isPlaying = 1;
    if(videoChannel){
        videoChannel->setAudioChannel(audioChannel);
        videoChannel->start();
    }

    if(audioChannel){
        audioChannel->start();
    }

    pthread_create(&pid_start,0,startTask,this);
}

/**
 * 子线程调用 把视频的压缩包取出来，放到队列中
 */
void NativePlayer::start_() {

    while (isPlaying){
        //解码过快导致内存占满
        if(videoChannel && videoChannel->packets.size() > 100){
            av_usleep(10*1000);
            continue;
        }
        if(audioChannel && audioChannel->packets.size() > 100){
            av_usleep(10*1000);
            continue;
        }
        AVPacket *packet = av_packet_alloc();
        int ret = av_read_frame(this->formatContext,packet);
        if(!ret){
            if(videoChannel != nullptr && videoChannel->stream_index == packet->stream_index){
                videoChannel->packets.insertToQueue(packet);
            }else if(audioChannel != nullptr && audioChannel->stream_index == packet->stream_index){
                audioChannel->packets.insertToQueue(packet);
            }

        }else if(ret == AVERROR_EOF){
            //读文件结束.要告诉队列读取文件数据结束
            //文件结束后，需要判断解码包是否解码结束
            if(videoChannel->packets.empty() && audioChannel->packets.empty()){
                break;
            }
        }else{
            break;
        }
    }
    isPlaying = 0;
    videoChannel->stop();
    audioChannel->stop();

}

void NativePlayer::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}
