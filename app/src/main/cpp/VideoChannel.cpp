//
// Created by cailianjun on 6/6/21.
//

#include "VideoChannel.h"


void  frame_sync_back(queue<AVFrame*> &q){
    if(!q.empty()){
       AVFrame * frame = q.front();
       BaseChannel::releaseAVFrame(&frame);
       q.pop();
    }
}

void pkt_sync_back(queue<AVPacket*> &q){
    while(!q.empty()){
        AVPacket * packet = q.front();
        if(packet->flags != AV_PKT_FLAG_KEY){
            //丢非关键帧
            BaseChannel::releaseAVPacket(&packet);
            q.pop();
        }else{
            break;
        }

    }
}


void * task_video_decode(void *args){
    VideoChannel * channel = static_cast<VideoChannel *>(args);
    channel->video_decode();
    return 0;
}

void * task_video_play(void * args){
    VideoChannel * channel = static_cast<VideoChannel *>(args);
    channel->video_play();
    return 0;
}

/**
 * 从原始包栈中取出数据包解压后放入缓冲栈进行播放
 */
void VideoChannel::start() {

    isPlaying = 1;

    packets.setWork(1);
    frames.setWork(1);

    pthread_create(&pthread_video_decode, 0, task_video_decode, this);
    pthread_create(&pthread_video_play, 0, task_video_play, this);

}

VideoChannel::VideoChannel(int streamIndex, AVCodecContext *codecContext,AVRational base_time,int fps) : BaseChannel(streamIndex,
                                                                                        codecContext,base_time),fps(fps) {
    packets.setSyncCallback(pkt_sync_back);
    frames.setSyncCallback(frame_sync_back);
}

VideoChannel::~VideoChannel() {

}

void VideoChannel::stop() {

}

void VideoChannel::video_play() {
    AVFrame * frame;
    uint8_t *dst_data[4];
    int dst_linesize[4];
    av_image_alloc(dst_data,
            dst_linesize,
            codecContext->width,
            codecContext->height,
            AV_PIX_FMT_RGBA,1);
   SwsContext* swsctx = sws_getContext(
           codecContext->width,
           codecContext->height,
            codecContext->pix_fmt,

            codecContext->width,
            codecContext->height,
                   AV_PIX_FMT_RGBA,
                   SWS_BILINEAR,
                   nullptr,
                   nullptr,
                   nullptr);
    while (isPlaying){
       int ret = frames.getQueueAndDel(frame);
       if(!ret){
           continue;
       }
       if(!isPlaying){
           break;
       }
       //YUV 数据->RGBA 数据
       sws_scale(swsctx,
               frame->data,
               frame->linesize,
               0,
               codecContext->height,
               dst_data,
               dst_linesize);
       //音视频同步
        //计算延时时间
        double extra_delay = frame->repeat_pict/(2*fps);
        double fps_delay = 1.0/fps;
        double real_delay = extra_delay + fps_delay;

       double video_time = frame->best_effort_timestamp*av_q2d(base_time);
       double audio_time = audioChannel->audio_time;
       double diff_time = video_time - audio_time;


       if(diff_time > 0){
           //视频更快，等待一小段时间，需要注意的是如果差值比较多的时候，只等待delay时间
           if(diff_time >1){
               av_usleep((real_delay * 2) * 1000000);
           }else{
               //差值小就等待差值时间
               av_usleep((real_delay + diff_time) * 1000000);
           }
       }
       if(diff_time < 0){
            //视频慢，视频需要快点，视频可以丢包
            //0.05 这是经验值
            if(fabs(diff_time) <= 0.05){
                frames.sync();
                continue;
            }
       }else{
           //达不到理想状态
       }

       //数据已经转行可现实数据，开始进行界面的绘制
       renderCallback(dst_data[0],frame->width,frame->height,dst_linesize[0]);
       av_frame_unref(frame);
       releaseAVFrame(&frame);
    }
    av_frame_unref(frame);
    releaseAVFrame(&frame);
    isPlaying= 0;
    av_free(&dst_data[0]);
    sws_freeContext(swsctx);
}

void VideoChannel::video_decode(){
    AVPacket *pkt = 0;

    while (isPlaying){
        if(frames.size() > 100){
            av_usleep(10*1000);
            continue;
        }
        int ret = packets.getQueueAndDel(pkt);
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
        AVFrame  *avFrame = av_frame_alloc();

        ret = avcodec_receive_frame(codecContext,avFrame);
        if(ret == AVERROR(EAGAIN)){
            // B帧  B帧参考前面成功  B帧参考后面失败   可能是P帧没有出来，再拿一次就行了
            continue;
        }else if(ret != 0){
            //失败 释放frame
            if(avFrame){
                releaseAVFrame(&avFrame);
            }
            break;
        }
        frames.insertToQueue(avFrame);
        av_packet_unref(pkt);
        releaseAVPacket(&pkt);
    }
    av_packet_unref(pkt);
    releaseAVPacket(&pkt);

}

void VideoChannel::setRenderCallback(RenderCallback callback) {

    this->renderCallback = callback;

}

void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {

    this->audioChannel = audioChannel;
}
