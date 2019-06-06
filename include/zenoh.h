#ifndef ZENOH_C_H_DEFINED_
#define ZENOH_C_H_DEFINED_

#include "zenoh/config.h"
#include "zenoh/msg.h"
#include "zenoh/codec.h"



z_zenoh_p_result_t 
z_open(char* locator, on_disconnect_t *on_disconnect);

z_sub_p_result_t 
z_declare_subscriber(z_zenoh_t *z, const char* resource, z_sub_mode_t sm, subscriber_callback_t *callback);

z_pub_p_result_t 
z_declare_publisher(z_zenoh_t *z, const char *resource);

int z_stream_compact_data(z_pub_t *pub, const z_iobuf_t *payload);
int z_stream_data(z_pub_t *pub, const z_iobuf_t *payload);
int z_write_data(z_zenoh_t *z, const char* resource, const z_iobuf_t *payload);

int z_stream_data_wo(z_pub_t *pub, const z_iobuf_t *payload, uint8_t encoding, uint8_t kind);
int z_write_data_wo(z_zenoh_t *z, const char* resource, const z_iobuf_t *payload, uint8_t encoding, uint8_t kind);

int z_query(z_zenoh_t *z, const char* resource, const char* predicate, reply_callback_t *callback);

#endif /* ZENOH_C_H_DEFINED_ */
