#include "threadpool.hpp"

template <typename T>
threadpool<T>::threadpool(int actor_model, connection_pool* connPool, int thread_num, int max_request) {

}
template <typename T>
threadpool<T>::~threadpool() {

}

template <typename T>
bool threadpool<T>::append(T* request, int state) {
    
}

template <typename T>
bool threadpool<T>::append_p(T* request) {

}

template <typename T>
void* threadpool<T>::worker(void* arg) {

}

template <typename T>
void threadpool<T>::run() {
    
}