#ifndef ZENOH_C_CODEC_H
#define ZENOH_C_CODEC_H

#include "zenoh/types.h"
#include "zenoh/property.h"
#include "zenoh/iobuf.h"

#define Z_DECLARE_ENCODE(name) \
void z_ ##name ##_encode(z_iobuf_t* buf, const z_ ##name ##_t* m)

#define Z_DECLARE_DECODE(name) \
z_ ##name ##_result_t z_ ##name ## _decode(z_iobuf_t* buf, uint8_t header); \
void z_ ## name ## _decode_na(z_iobuf_t* buf, uint8_t header, _z_ ##name ##_result_t *r)

#define Z_DECLARE_DECODE_NOH(name) \
z_ ##name ##_result_t z_ ##name ## _decode(z_iobuf_t* buf); \
void z_ ## name ## _decode_na(z_iobuf_t* buf, z_ ##name ##_result_t *r)

void z_vle_encode(z_iobuf_t* buf, z_vle_t v);
z_vle_result_t z_vle_decode(z_iobuf_t* buf);

void z_array_uint8_encode(z_iobuf_t* buf, const z_array_uint8_t* bs);
z_array_uint8_result_t z_array_uint8_decode(z_iobuf_t* buf);

void z_string_encode(z_iobuf_t* buf, const char* s);
z_string_result_t z_string_decode(z_iobuf_t* buf);


Z_DECLARE_ENCODE(property);
Z_DECLARE_DECODE_NOH(property);

Z_DECLARE_ENCODE(temporal_property);
Z_DECLARE_DECODE_NOH(temporal_property);

void z_properties_encode(z_iobuf_t *buf, const z_vec_t *ps);

#endif /* ZENOH_C_CODEC_H */

