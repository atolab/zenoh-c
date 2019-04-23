#ifndef ZENOH_C_RECEIVE_LOOP_H_
#define ZENOH_C_RECEIVE_LOOP_H_ 

#include "zenoh/types.h"

int z_start_recv_loop(zenoh_t* z);

int z_stop_recv_loop(zenoh_t* z);

#endif /* ZENOH_C_RECEIVE_LOOP_H_ */