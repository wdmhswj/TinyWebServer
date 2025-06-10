#ifndef LOCKER_HPP
#define LOCKER_HPP

#include <exception>
#include <pthread.h>
#include <semaphore.h>

class sem {
public:
    sem() {
        if(sem_init(&sem_, 0, 0) != 0) {
            throw std::exception();
        }
    }

    sem(int num) {
        if(sem_init(&sem_, 0, num) != 0) {
            throw std::exception();
        }
    }

    ~sem() {
        sem_destroy(&sem_);
    }

    bool wait() {
        return sem_wait(&sem_) == 0;
    }

    bool post() {
        return sem_post(&sem_) == 0;
    }
    
private:
    sem_t sem_;    // 信号量
};

class locker {
public:
    locker() {
        if(pthread_mutex_init(&mutex_, nullptr) != 0) {
            throw std::exception();
        }
    }

    ~locker() {
        pthread_mutex_destroy(&mutex_);
    }

    bool lock() {
        return pthread_mutex_lock(&mutex_) == 0;
    }

    bool unlock() {
        return pthread_mutex_unlock(&mutex_) == 0;
    }

    pthread_mutex_t* get() {
        return &mutex_;
    }

private:
    pthread_mutex_t mutex_;  // 互斥锁
};

class cond {
public:
    cond() {
        if(pthread_cond_init(&cond_, nullptr) != 0) {
            throw std::exception();
        }
    }

    ~cond() {
        pthread_cond_destroy(&cond_);
    }

    bool wait(pthread_mutex_t* mutex) {
        return pthread_cond_wait(&cond_, mutex) == 0;
    }

    bool timewait(pthread_mutex_t* mutex, const struct timespec t) {
        return pthread_cond_timedwait(&cond_, mutex, &t) == 0;
    }

    bool signal() {
        return pthread_cond_signal(&cond_) == 0;
    }

    bool broadcast() {
        return pthread_cond_broadcast(&cond_) == 0;
    }
private:
    pthread_cond_t cond_;
};

#endif // LOCKER_HPP