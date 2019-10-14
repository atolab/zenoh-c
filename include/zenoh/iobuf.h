#ifndef ZENOH_C_IOBUF_H_
#define ZENOH_C_IOBUF_H_

#include <stdlib.h>
#include "zenoh/result.h"
#include "zenoh/collection.h"

typedef  size_t  z_vle_t;
Z_RESULT_DECLARE (z_vle_t, vle)

Z_ARRAY_DECLARE(uint8_t)
Z_RESULT_DECLARE (z_array_uint8_t, array_uint8)

typedef struct {
  unsigned int r_pos;
  unsigned int w_pos;
  unsigned int capacity;
  unsigned char* buf;
} z_iobuf_t;

z_iobuf_t z_iobuf_make(unsigned int capacity);
z_iobuf_t z_iobuf_wrap(unsigned char *buf, unsigned int capacity);
z_iobuf_t z_iobuf_wrap_wo(unsigned char *buf, unsigned int capacity, unsigned int rpos, unsigned int wpos);

void z_iobuf_free(z_iobuf_t* buf);
unsigned int z_iobuf_readable(const z_iobuf_t* buf);
unsigned int z_iobuf_writable(const z_iobuf_t* buf);
void z_iobuf_write(z_iobuf_t* buf, unsigned char b);
void z_iobuf_write_slice(z_iobuf_t* buf, const uint8_t *bs, unsigned int offset, unsigned int length);
void z_iobuf_write_bytes(z_iobuf_t* buf, const unsigned char *bs, unsigned int length);
uint8_t z_iobuf_read(z_iobuf_t* buf);
uint8_t* z_iobuf_read_n(z_iobuf_t* buf, unsigned int length);
uint8_t* z_iobuf_read_to_n(z_iobuf_t* buf, unsigned char* dest, unsigned int length);
void z_iobuf_put(z_iobuf_t* buf, unsigned char b, unsigned int pos);
uint8_t z_iobuf_get(z_iobuf_t* buf, unsigned int pos);
void z_iobuf_clear(z_iobuf_t *buf);
z_array_uint8_t z_iobuf_to_array(z_iobuf_t* buf);
void z_iobuf_compact(z_iobuf_t *buf);

#endif /* ZENOH_C_IOBUF_H_ */
