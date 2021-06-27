#include "JNICallbackHelper.h"

JNICallbackHelper::JNICallbackHelper(JavaVM *javaVm, JNIEnv *env, jobject jobj) {
    this->vm = javaVm;
    this->env = env;
    this->job = env->NewGlobalRef(jobj);
    jclass clazz = env->GetObjectClass(jobj);
    jmd_prepared = env->GetMethodID(clazz,"onPrepared","()V");
    jerrorMethodId  = env->GetMethodID(clazz,"onError","(I)V");
}

JNICallbackHelper::~JNICallbackHelper() {

    this->vm = 0;
    this->env->DeleteGlobalRef(job);
    this->job = 0;
    this->env = 0;

}

void JNICallbackHelper::onPrepared(int thread_mode) {

    if(thread_mode == THREAD_MAIN){
        env->CallVoidMethod(job,jmd_prepared);
    } else if(thread_mode == THREAD_CHILD){
        JNIEnv * childEnv;
        this->vm->AttachCurrentThread(&childEnv,0);
        childEnv->CallVoidMethod(job,jmd_prepared);
        this->vm->DetachCurrentThread();
    }

}

void JNICallbackHelper::onError(int thread_mode, int error_code) {
    if(thread_mode == THREAD_MAIN){
        env->CallVoidMethod(job,jerrorMethodId,error_code);
    } else if(thread_mode == THREAD_CHILD){
        JNIEnv * childEnv;
        this->vm->AttachCurrentThread(&childEnv,0);
        childEnv->CallVoidMethod(job,jerrorMethodId,error_code);
        this->vm->DetachCurrentThread();
    }
}