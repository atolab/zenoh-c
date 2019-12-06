#ifndef ZENOH_C_CODEC_H
#define ZENOH_C_CODEC_H

#include "zenoh/iobuf.h"
#include "zenoh/types.h"
#include "zenoh/net/types.h"
#include "zenoh/net/property.h"

void z_vle_encode(z_iobuf_t* buf, z_vle_t v);
z_vle_result_t z_vle_decode(z_iobuf_t* buf);

void z_uint8_array_encode(z_iobuf_t* buf, const z_uint8_array_t* bs);
z_uint8_array_result_t z_uint8_array_decode(z_iobuf_t* buf);

void z_string_encode(z_iobuf_t* buf, const char* s);
z_string_result_t z_string_decode(z_iobuf_t* buf);

#endif /* ZENOH_C_CODEC_H */

