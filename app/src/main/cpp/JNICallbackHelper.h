
#ifndef FFMPEGLEARNDEMO_JNICALLBACKHELPER_H
#define FFMPEGLEARNDEMO_JNICALLBACKHELPER_H
#include <jni.h>
#include "util.h"
class JNICallbackHelper {

private:
    JavaVM *vm = 0;
    JNIEnv *env = 0;
    jobject  job;
    jmethodID  jmd_prepared;
    jmethodID  jerrorMethodId;
    jmethodID  jprogressMethodId;
public:
    JNICallbackHelper(JavaVM *javaVm,JNIEnv *env,jobject jobj);
    virtual ~JNICallbackHelper();
    void onPrepared(int thread_mode);
    void onError(int thread_mode,int error_code);
    void onProgress(int thread_mod,int progress);
};


#endif //FFMPEGLEARNDEMO_JNICALLBACKHELPER_H
