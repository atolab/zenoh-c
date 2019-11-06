#ifndef ZENOH_C_NET_H_
#define ZENOH_C_NET_H_

#include "zenoh/types.h"
#include "zenoh/private/msg.h"
#include <sys/socket.h>
#include <netinet/in.h>

Z_RESULT_DECLARE (z_socket_t, socket)

z_socket_result_t _z_open_tx_session(const char *locator);

struct sockaddr_in * 
_z_make_socket_address(const char* addr, int port);

z_socket_result_t 
_z_create_udp_socket(const char *addr, int port, int recv_timeout);

int _z_recv_dgram_from(z_socket_t sock, z_iobuf_t* buf, struct sockaddr* from, socklen_t *salen);

int _z_send_dgram_to(z_socket_t sock, const z_iobuf_t* buf, const struct sockaddr *dest,  socklen_t salen);

int _z_send_buf(z_socket_t sock, const z_iobuf_t* buf);

int _z_recv_buf(z_socket_t sock, z_iobuf_t *buf);

int _z_recv_n(z_socket_t sock, uint8_t* buf, size_t len);

size_t _z_send_msg(z_socket_t sock, z_iobuf_t* buf, _z_message_t* m);

size_t _z_send_large_msg(z_socket_t sock, z_iobuf_t* buf, _z_message_t* m, unsigned int max_len);

z_vle_result_t _z_recv_vle(z_socket_t sock);

_z_message_p_result_t _z_recv_msg(z_socket_t sock, z_iobuf_t* buf);
void _z_recv_msg_na(z_socket_t sock, z_iobuf_t* buf, _z_message_p_result_t *r);

#endif /* ZENOH_C_NET_H_ */
