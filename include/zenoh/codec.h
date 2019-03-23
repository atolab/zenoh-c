#ifndef ZENOH_C_CODEC_H
#define ZENOH_C_CODEC_H

#include "zenoh/msg.h"
#include "zenoh/result.h"

void z_vle_encode(z_iobuf_t* buf, z_vle_t v);
z_result_t z_vle_decode(z_iobuf_t* buf);

void z_array_uint8_encode(z_iobuf_t* buf, const z_array_uint8_t* bs);
z_result_t z_array_uint8_decode(z_iobuf_t* buf);

void z_open_encode(z_iobuf_t* buf, const z_open_t* m);
z_result_t z_accept_decode(z_iobuf_t* buf, uint8_t header);


#endif /* ZENOH_C_CODEC_H */