#ifndef ZENOH_C_CODEC_H
#define ZENOH_C_CODEC_H

#include "zenoh/msg.h"
#include "zenoh/result.h"

#define DECLARE_MSG_ENCODE(name) \
void z_ ##name ##_encode(z_iobuf_t* buf, const z_ ##name ##_t* m)

#define DECLARE_MSG_DECODE(name) \
z_ ##name ##_result_t z_ ##name ## _decode(z_iobuf_t* buf, uint8_t header); \
void  z_ ## name ## _decode_na(z_iobuf_t* buf, uint8_t header, z_ ##name ##_result_t *r)

#define DECLARE_MSG_DECODE_NOH(name) \
z_ ##name ##_result_t z_ ##name ## _decode(z_iobuf_t* buf)

#define DECLARE_MSG_CODEC(name)\
  DECLARE_MSG_DECODE(name); \
  DECLARE_MSG_ENCODE(name)


void z_vle_encode(z_iobuf_t* buf, z_vle_t v);
z_vle_result_t z_vle_decode(z_iobuf_t* buf);

void z_array_uint8_encode(z_iobuf_t* buf, const z_array_uint8_t* bs);
z_array_uint8_result_t z_array_uint8_decode(z_iobuf_t* buf);

void z_string_encode(z_iobuf_t* buf, const char* s);
z_string_result_t z_string_decode(z_iobuf_t* buf);


DECLARE_MSG_ENCODE(open);
DECLARE_MSG_DECODE(accept);

DECLARE_MSG_ENCODE(close);
DECLARE_MSG_DECODE(close);

DECLARE_MSG_ENCODE(declare);

// DECLARE_MSG_ENCODE(res_decl);
// DECLARE_MSG_DECODE(res_decl);

// DECLARE_MSG_ENCODE(pub_decl);
// DECLARE_MSG_DECODE(pub_decl);

// DECLARE_MSG_ENCODE(storage_decl);

DECLARE_MSG_ENCODE(temporal_property);
DECLARE_MSG_DECODE_NOH(temporal_property);

DECLARE_MSG_ENCODE(sub_mode);
DECLARE_MSG_DECODE_NOH(sub_mode);

// DECLARE_MSG_ENCODE(sub_decl);
// DECLARE_MSG_DECODE(sub_decl);

// DECLARE_MSG_ENCODE(commit_decl);
// DECLARE_MSG_DECODE(commit_decl);

// DECLARE_MSG_ENCODE(result_decl);
// DECLARE_MSG_DECODE(result_decl);

DECLARE_MSG_ENCODE(compact_data);
DECLARE_MSG_DECODE(compact_data);

DECLARE_MSG_ENCODE(payload_header);
DECLARE_MSG_DECODE_NOH(payload_header);

DECLARE_MSG_ENCODE(stream_data);
DECLARE_MSG_DECODE(stream_data);

void z_message_encode(z_iobuf_t* buf, const z_message_t* m);
z_message_p_result_t z_message_decode(z_iobuf_t* buf);
#endif /* ZENOH_C_CODEC_H */