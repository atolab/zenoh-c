#ifndef ZENOH_C_MSGCODEC_H
#define ZENOH_C_MSGCODEC_H

#include "zenoh/result.h"
#include "zenoh/types.h"
#include "zenoh/property.h"
#include "zenoh/codec.h"
#include "zenoh/private/msg.h"

#define _Z_DECLARE_ENCODE(name) \
void _z_ ##name ##_encode(z_iobuf_t* buf, const _z_ ##name ##_t* m)

#define _Z_DECLARE_DECODE(name) \
_z_ ##name ##_result_t z_ ##name ## _decode(z_iobuf_t* buf, uint8_t header); \
void _z_ ## name ## _decode_na(z_iobuf_t* buf, uint8_t header, _z_ ##name ##_result_t *r)

#define _Z_DECLARE_DECODE_NOH(name) \
_z_ ##name ##_result_t z_ ##name ## _decode(z_iobuf_t* buf); \
void _z_ ## name ## _decode_na(z_iobuf_t* buf, _z_ ##name ##_result_t *r)

Z_DECLARE_ENCODE(sub_mode);
Z_DECLARE_DECODE_NOH(sub_mode);

_Z_DECLARE_ENCODE(scout);
_Z_DECLARE_DECODE_NOH(scout);

_Z_DECLARE_ENCODE(hello);
_Z_DECLARE_DECODE_NOH(hello);


_Z_DECLARE_ENCODE(open);
_Z_DECLARE_DECODE_NOH(accept);

_Z_DECLARE_ENCODE(close);
_Z_DECLARE_DECODE_NOH(close);

_Z_DECLARE_ENCODE(declare);
_Z_DECLARE_DECODE_NOH(declare);

_Z_DECLARE_ENCODE(compact_data);
_Z_DECLARE_DECODE_NOH(compact_data);

_Z_DECLARE_ENCODE(payload_header);
_Z_DECLARE_DECODE_NOH(payload_header);

_Z_DECLARE_ENCODE(stream_data);
_Z_DECLARE_DECODE_NOH(stream_data);

_Z_DECLARE_ENCODE(write_data);
_Z_DECLARE_DECODE_NOH(write_data);

_Z_DECLARE_ENCODE(query);
_Z_DECLARE_DECODE_NOH(query);

void _z_message_encode(z_iobuf_t* buf, const _z_message_t* m);
_z_message_p_result_t z_message_decode(z_iobuf_t* buf);
void _z_message_decode_na(z_iobuf_t* buf, _z_message_p_result_t *r);

#endif /* ZENOH_C_MSGCODEC_H */
