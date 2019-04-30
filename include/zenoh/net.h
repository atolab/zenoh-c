#ifndef ZENOH_C_NET_H_
#define ZENOH_C_NET_H_

#include "zenoh/codec.h"

int open_tx_session(char *locator);

int z_send_buf(int sock, z_iobuf_t* buf);

int z_recv_n(int sock, z_iobuf_t* buf, size_t len);

size_t z_send_msg(int sock, z_iobuf_t* buf, z_message_t* m);

z_vle_result_t z_recv_vle(int sock);

z_message_p_result_t z_recv_msg(int sock, z_iobuf_t* buf);
void z_recv_msg_na(int sock, z_iobuf_t* buf, z_message_p_result_t *r);

#endif /* ZENOH_C_NET_H_ */