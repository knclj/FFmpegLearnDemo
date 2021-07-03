#ifndef FFMPEGLEARNDEMO_SAFEQUEUE_H
#define FFMPEGLEARNDEMO_SAFEQUEUE_H

#include <queue>
#include <pthread.h>

using namespace std;

template<typename T>
class SafeQueue {
private:
    typedef void (*ReleaseCallback)(T *);

    typedef void (*SynCallBack)(queue<T> &);
    queue<T> queue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int work;
    ReleaseCallback releaseCallback;
    SynCallBack  synCallBack;

public:
    SafeQueue(){
        pthread_mutex_init(&mutex,0);
        pthread_cond_init(&cond,0);
    }
    ~SafeQueue(){
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }
    void insertToQueue(T value) {
        pthread_mutex_lock(&mutex);
        if(work){
            queue.push(value);
            pthread_cond_signal(&cond);
        }else{
            if(this->releaseCallback){
                this->releaseCallback(&value);
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    /**
     *
     * @param value
     * @return int  0 failed
     */
    int getQueueAndDel(T &value) {
        int ret = 0;
        if(!work){
            return ret;
        }
        pthread_mutex_lock(&mutex);

        while(work && queue.empty()){
            pthread_cond_wait(&cond,&mutex);
        }

        //有数据
        if(!queue.empty()){
            value = queue.front();
            queue.pop();
            ret = 1;
        }

        pthread_mutex_unlock(&mutex);
        return ret;
    }
    void setWork(int work) {
        pthread_mutex_lock(&mutex);
        this->work = work;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    int empty() {
        return queue.empty();
    }
    int size() {
        return queue.size();
    }
    void clear() {
        pthread_mutex_lock(&mutex);
        int size = queue.size();
        for (int i = 0; i < size; ++i) {
            T value =  queue.front();
            if(this->releaseCallback){
                this->releaseCallback(&value);
            }
            queue.pop();
        }
        pthread_mutex_unlock(&mutex);
    }
    void setReleaseCallback(SafeQueue::ReleaseCallback callback) {
        this->releaseCallback = callback;
    }

    void setSyncCallback(SynCallBack callBack){
        this->synCallBack =callBack;
    }

    void sync() {

        pthread_mutex_lock(&mutex);
        synCallBack(queue);
        pthread_mutex_unlock(&mutex);

    }
};




#endif //FFMPEGLEARNDEMO_SAFEQUEUE_H
