//
// Created by cailianjun on 6/13/21.
//

#include "BaseChannel.h"

BaseChannel::BaseChannel(int index, AVCodecContext *codeContext,AVRational base_time):stream_index(index),codecContext(codeContext),base_time(base_time){
    packets.setReleaseCallback(releaseAVPacket);
    frames.setReleaseCallback(releaseAVFrame);
}

BaseChannel::~BaseChannel() {

    packets.clear();
    frames.clear();

}

