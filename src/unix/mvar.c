#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "zenoh/mvar.h"

typedef struct {
    void *elem;
    int full;
    pthread_mutex_t mtx;
    pthread_cond_t can_put;
    pthread_cond_t can_get;
} z_posix_mvar_t;

z_mvar_t *z_mvar_empty() {
    z_posix_mvar_t *mv = (z_posix_mvar_t *)malloc(sizeof(z_posix_mvar_t));
    bzero(mv, sizeof(z_posix_mvar_t));
    pthread_mutex_init(&mv->mtx, 0);
    pthread_cond_init(&mv->can_get, 0);
    pthread_cond_init(&mv->can_put, 0);
    return (z_mvar_t *)mv;
}

int z_mvar_is_empty(z_mvar_t *zmv) {
    z_posix_mvar_t * mv = (z_posix_mvar_t *)zmv;
    return mv->full == 0;
}

z_mvar_t *z_mvar_of(void *e) {
    z_posix_mvar_t *mv = (z_posix_mvar_t *)z_mvar_empty();
    mv->elem = e;
    mv->full = 1;
    return (z_mvar_t *)mv;    
}

void * z_mvar_get(z_mvar_t *zmv) {
    z_posix_mvar_t * mv = (z_posix_mvar_t *)zmv;
    do {
        pthread_mutex_lock(&mv->mtx);

        if (mv->full) {
            mv->full = 0;
            void *e = mv->elem;
            mv->elem = 0;
            pthread_mutex_unlock(&mv->mtx);
            pthread_cond_signal(&mv->can_put);        
            return e;
        } else {
            pthread_cond_wait(&mv->can_get, &mv->mtx);
        }
    } while (1);
} 

void z_mvar_put(z_mvar_t * zmv, void *e) {
    z_posix_mvar_t * mv = (z_posix_mvar_t *)zmv;

    do {
        pthread_mutex_lock(&mv->mtx);
        if (mv->full) {
            pthread_cond_wait(&mv->can_put, &mv->mtx);
        } else {
            mv->elem = e;
            mv->full = 1;
            pthread_mutex_unlock(&mv->mtx);
            pthread_cond_signal(&mv->can_get);
            return;            
        }
    } while (1);
}
