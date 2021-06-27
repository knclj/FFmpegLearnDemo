#include <jni.h>
#include <string>
#include "NativePlayer.h"
#include "JNICallbackHelper.h"
#include <android/native_window_jni.h>

extern "C" {
#include "ffmpeg/include/libavutil/avutil.h"
}

NativePlayer *player = 0;
JavaVM *vm = 0;
ANativeWindow *window = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void rendFrame(uint8_t *src_data,int w,int h,int src_lineSize){
    pthread_mutex_lock(&mutex);

    if(!window){
        pthread_mutex_unlock(&mutex);
        return;
    }
    ANativeWindow_setBuffersGeometry(window,w,h,WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer  windowBuffer;
    //是否能锁定
    if(ANativeWindow_lock(window,&windowBuffer,0)){
        ANativeWindow_release(window);
        window = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }
    //正常情况下开始渲染数据，一行一行渲染
    uint8_t *dst_data = static_cast<uint8_t *>(windowBuffer.bits);
    int dst_linesize = windowBuffer.stride*4;
    //拷贝数据到目的的缓冲中
    for (int i = 0; i < windowBuffer.height; ++i) {
        memcpy(dst_data+i*dst_linesize,src_data+i*src_lineSize,dst_linesize);
    }

    //post开始绘制
    ANativeWindow_unlockAndPost(window);

    pthread_mutex_unlock(&mutex);
}

jint JNI_OnLoad(JavaVM *javaVm,void *args){
    ::vm = javaVm;

    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_clj_ffmpeglearndemo_NativePlayer_stringFromJNI(JNIEnv *env, jclass clazz) {
    std::string hello = "当前的FFmpeg的版本是:";
    hello.append(av_version_info());
    return env->NewStringUTF(hello.c_str());
}



extern "C"
JNIEXPORT void JNICALL
Java_com_clj_ffmpeglearndemo_NativePlayer_prepareNative(JNIEnv *env, jobject thiz,
                                                        jstring data_source) {
    jboolean  isCopy;
    const char*  dataSourceChar =  env->GetStringUTFChars(data_source,&isCopy);
    auto * jniCallbackHelper = new JNICallbackHelper(vm,env,thiz);
    player = new NativePlayer(dataSourceChar,jniCallbackHelper);
    player->setRenderCallback(rendFrame);
    player->prepare();
    env->ReleaseStringUTFChars(data_source,dataSourceChar);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_clj_ffmpeglearndemo_NativePlayer_startNative(JNIEnv *env, jobject thiz) {
    if(player){
        player->start();
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_clj_ffmpeglearndemo_NativePlayer_stopNative(JNIEnv *env, jobject thiz) {
    // TODO: implement stopNative()
}

extern "C"
JNIEXPORT void JNICALL
Java_com_clj_ffmpeglearndemo_NativePlayer_releaseNative(JNIEnv *env, jobject thiz) {
    // TODO: implement releaseNative()
}extern "C"
JNIEXPORT void JNICALL
Java_com_clj_ffmpeglearndemo_NativePlayer_setSufaceNative(JNIEnv *env, jobject thiz, jobject suface) {
    pthread_mutex_lock(&mutex);
    if(window){
        ANativeWindow_release(window);
        window = 0;
    }
    //获取一个window
    //然后将数据填充到window进行渲染界面
    window = ANativeWindow_fromSurface(env,suface);
    pthread_mutex_unlock(&mutex);
}