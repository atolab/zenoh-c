#include <pthread.h>
#include "zenoh/net/recv_loop.h"

void* pthread_recv_loop(void *arg){
    return zn_recv_loop((zn_session_t*)arg);
}

int zn_start_recv_loop(zn_session_t* z) { 
    pthread_t *thread = (pthread_t*)malloc(sizeof(pthread_t));
    bzero(thread, sizeof(pthread_t));
    z->thread = thread;
    if (pthread_create(thread, 0, pthread_recv_loop, z) != 0) {
        return -1;
    } 
    return 0;
}

int zn_stop_recv_loop(zn_session_t *z) { 
    z->running = 0;
    return 0;
}
