#include <pthread.h>
#include "zenoh/recv_loop.h"

void* pthread_recv_loop(void *arg){
    return z_recv_loop((z_zenoh_t*)arg);
}

int z_start_recv_loop(z_zenoh_t* z) { 
    pthread_t *thread = (pthread_t*)malloc(sizeof(pthread_t));
    bzero(thread, sizeof(pthread_t));
    z->thread = thread;
    if (pthread_create(thread, 0, pthread_recv_loop, z) != 0) {
        return -1;
    } 
    return 0;
}

int z_stop_recv_loop(z_zenoh_t *z) { 
    z->running = 0;
    return 0;
}
