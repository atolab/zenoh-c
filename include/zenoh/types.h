#ifndef ZENOH_C_TYPES_H_
#define ZENOH_C_TYPES_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "zenoh/collection.h"

typedef  uint64_t z_vle_t;

Z_ARRAY_DECLARE(uint8_t)

typedef struct {
  unsigned int r_pos;
  unsigned int w_pos;
  unsigned int capacity;
  uint8_t* buf;
} z_iobuf_t;

z_iobuf_t z_iobuf_make(unsigned int capacity);
void z_iobuf_free(z_iobuf_t* buf);
unsigned int z_iobuf_readable(const z_iobuf_t* buf);
unsigned int z_iobuf_writable(const z_iobuf_t* buf);
void z_iobuf_write(z_iobuf_t* buf, uint8_t b);
void z_iobuf_write_n(z_iobuf_t* buf, uint8_t* bs, unsigned int offset, unsigned int length);
uint8_t z_iobuf_read(z_iobuf_t* buf);
uint8_t* z_iobuf_read_n(z_iobuf_t* buf, unsigned int length);
uint8_t* z_iobuf_read_to_n(z_iobuf_t* buf, uint8_t* dest, unsigned int length);
void z_iobuf_put(z_iobuf_t* buf, uint8_t b, unsigned int pos);
uint8_t z_iobuf_get(z_iobuf_t* buf, unsigned int pos);
void z_iobuf_clear(z_iobuf_t *buf);
z_array_uint8_t z_iobuf_to_array(z_iobuf_t* buf);

typedef void on_disconnect_t(void);

typedef struct {
  int sock;
  z_vle_t sn;
  z_vle_t cid;
  z_vle_t rid;
  z_iobuf_t wbuf;
  z_iobuf_t rbuf;
  z_array_uint8_t pid;
  on_disconnect_t *on_disconnect;
} zenoh_t;

typedef struct {
  int z;
} z_sub_t;

typedef struct {
  int z;
} z_pub_t;

typedef void subscriber_callback_t(z_vle_t rid, z_array_uint8_t data);

#endif /* ZENOH_C_TYPES_H_ */ 