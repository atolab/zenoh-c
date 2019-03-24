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
  VLE,
  HELLO,
  ACCEPT,
  CLOSE,
  ARRAY_UINT8,
  ARRAY_PROP,
  ERROR
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
Z_RESULT_DECLARE (z_accept_t, accept);

#endif /* ZENOH_C_RESULT_H */