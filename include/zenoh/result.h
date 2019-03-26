#ifndef ZENOH_C_RESULT_H
#define ZENOH_C_RESULT_H

#include "zenoh/msg.h"

#define Z_VLE_PARSE_ERROR 0x01
#define Z_ARRAY_PARSE_ERROR 0x02
#define Z_STRING_PARSE_ERROR 0x03
#define Z_PROPERTY_PARSE_ERROR 0x04
#define Z_PROPERTIES_PARSE_ERROR 0x05
#define Z_MESSAGE_PARSE_ERROR 0x06
#define Z_INSUFFICIENT_IOBUF_SIZE 0x07
#define Z_UNEXPECTED_MESSAGE 0x7f

enum result_kind {
  Z_VLE_TAG,
  Z_STRING_TAG,
  Z_ARRAY_UINT8_TAG,
  Z_ARRAY_PROP_TAG,
  Z_HELLO_TAG,
  Z_ACCEPT_TAG,
  Z_CLOSE_TAG,
  Z_DECLARE_TAG,
  Z_RES_DECL_TAG,
  Z_PUB_DECL_TAG,
  Z_TEMP_PROPERTY_TAG,
  Z_SUB_MODE_TAG,
  Z_SUB_DECL_TAG,
  Z_COMMIT_TAG,
  Z_RESULT_TAG,
  Z_STREAM_DATA_TAG,
  Z_ERROR_TAG
};

#define Z_RESULT_DECLARE(type, name) \
typedef struct { \
  enum result_kind tag; \
  union { \
    type  name; \
    int error; \
  } value;\
} z_ ## name ## _result_t

#define Z_P_RESULT_DECLARE(type, name) \
typedef struct { \
  enum result_kind tag; \
  union { \
    type * name; \
    int error; \
  } value;\
} z_ ## name ## _result_t

#define ASSURE_RESULT(in_r, out_r, e) \
  if (in_r.tag == Z_ERROR_TAG) { \
    out_r.tag = Z_ERROR_TAG; \
    out_r.value.error = e; \
    return out_r; \
  }

Z_RESULT_DECLARE (z_vle_t, vle);
Z_RESULT_DECLARE (z_array_uint8_t, array_uint8);
Z_RESULT_DECLARE (char*, string);

Z_RESULT_DECLARE (z_accept_t, accept);
Z_RESULT_DECLARE (z_close_t, close);
Z_RESULT_DECLARE (z_declare_t, declare);
Z_RESULT_DECLARE (z_res_decl_t, res_decl);
Z_RESULT_DECLARE (z_pub_decl_t, pub_decl);
Z_RESULT_DECLARE (z_temporal_property_t, temporal_property);
Z_RESULT_DECLARE (z_sub_mode_t, sub_mode);
Z_RESULT_DECLARE (z_sub_decl_t, sub_decl);
Z_RESULT_DECLARE (z_commit_decl_t, commit);
Z_RESULT_DECLARE (z_result_decl_t, result_decl);
Z_RESULT_DECLARE (z_stream_data_t, stream_data);

typedef struct {
  enum result_kind tag;

  union { 
    z_accept_result_t accept;
    z_close_result_t close;
    z_declare_result_t declare;
    z_stream_data_result_t stream_data;
    uint8_t error;
  } value;
} z_message_result_t;

#endif /* ZENOH_C_RESULT_H */