#ifndef ZENOH_C_RECV_LOOP_H_
#define ZENOH_C_RECV_LOOP_H_

#include "zenoh/types.h"

void* z_recv_loop(z_zenoh_t* z);

int z_running(z_zenoh_t* z);

int z_start_recv_loop(z_zenoh_t* z);

int z_stop_recv_loop(z_zenoh_t* z);

#endif /* ZENOH_C_RECV_LOOP_H_ */
