#ifndef ZENOH_C_RESULT_H
#define ZENOH_C_RESULT_H

#include "zenoh/msg.h"

#define VLE_PARSE_ERROR 0x01
#define ARRAY_PARSE_ERROR 0x02
#define PROPERTY_PARSE_ERROR 0x03
#define PROPERTIES_PARSE_ERROR 0x04
#define MESSAGE_PARSE_ERROR 0x05
#define UNEXPECTED_MESSAGE 0x06

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


#endif /* ZENOH_C_RESULT_H */