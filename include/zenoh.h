#ifndef ZENOH_C_H_DEFINED_
#define ZENOH_C_H_DEFINED_

#include "zenoh/config.h"
#include "zenoh/msg.h"
#include "zenoh/codec.h"


z_zenoh_result_t 
z_open(char* locator, on_disconnect_t *on_disconnect);

zenoh_t*
z_open_ptr(char* locator);

void z_close(zenoh_t* z);
z_vle_result_t z_declare_resource(zenoh_t *z, const char* resource);
int z_declare_subscriber(zenoh_t *z, z_vle_t rid, z_sub_mode_t sm, subscriber_callback_t *callback);
int z_declare_publisher(zenoh_t *z, z_vle_t rid);

int z_stream_compact_data(zenoh_t *z, z_vle_t rid, const z_iobuf_t *payload);
int z_stream_data(zenoh_t *z, z_vle_t rid, const z_iobuf_t *payload_header);

int z_write_data(zenoh_t *z, const char* resource, const z_iobuf_t *payload_header);

#endif /* ZENOH_C_H_DEFINED_ */
