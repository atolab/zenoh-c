#ifndef ZENOH_C_NET_RECV_LOOP_H
#define ZENOH_C_NET_RECV_LOOP_H

#include "zenoh/net/types.h"

void* zn_recv_loop(zn_session_t* z);

int zn_running(zn_session_t* z);

int zn_start_recv_loop(zn_session_t* z);

int zn_stop_recv_loop(zn_session_t* z);

#endif /* ZENOH_C_NET_RECV_LOOP_H */
