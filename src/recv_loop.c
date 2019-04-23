#include <pthread.h>
#include <unistd.h>

#include "zenoh/recv_loop.h"
#include "zenoh/net.h"

typedef struct {
    pthread_t recv_loop;
    volatile int running;
} z_runtime_t;

void* z_recv_loop(void* arg) {
    zenoh_t *z = (zenoh_t*)arg;
    z_runtime_t *rt = (z_runtime_t*)z->runtime;
    while (rt->running) {
        z_message_p_result_t r = z_recv_msg(z->sock, &z->rbuf);
        if (r.tag == Z_OK_TAG) {
            switch (Z_MID(r.value.message->header)) {    
                case Z_DECLARE:
                    if (z_mvar_is_empty(z->reply_msg_mvar)) {                        
                        z_mvar_put(z->reply_msg_mvar, &r.value.message->payload.declare);
                    }            
                case Z_ACCEPT:
                    break;
                default:
                    break;
            }
        }

    }
}
int z_start_recv_loop(zenoh_t* z) { 
    z_runtime_t *zr = (z_runtime_t*)malloc(sizeof(z_runtime_t));    
    bzero(zr, sizeof(z_runtime_t));
    z->runtime = zr;    
    if (pthread_create(&(zr->recv_loop), 0, z_recv_loop, z) == 0) {
        zr->running = 1;
        return 0;
    } 
    return -1;
}

int z_stop_recv_loop(zenoh_t *z) { 
    z_message_t c;
    c.header = Z_CLOSE;
    c.payload.close.pid = z->pid;
    c.payload.close.reason = Z_PEER_CLOSE;
    z_send_msg(z->sock, &z->wbuf, &c);
    close(z->sock);
    ((z_runtime_t*)z->runtime)->running = 0;
    return 0;
}