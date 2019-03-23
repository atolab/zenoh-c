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
  SCOUT,
  HELLO,
  OPEN,
  ACCEPT,
  CLOSE,
  ARRAY_UINT8,
  ARRAY_PROP,
  ERROR
};

typedef struct {
  enum result_kind tag;
  union {
    int error;
    z_vle_t vle;
    z_scout_t scout;
    z_hello_t hello;
    z_open_t open;
    z_accept_t accept;
    z_array_uint8_t a_uint8;    
  } value;
} z_result_t;

#define RESULT_IS(kind, result) (result.tag == kind)
#define Z_RESULT_VLE (value) (z_result_t){VLE, value} 

z_result_t z_error_result(int code);


#endif /* ZENOH_C_RESULT_H */