#ifndef ZENOH_C_IOBUF_H
#define ZENOH_C_IOBUF_H

#include "zenoh/types.h"

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

#endif /* ZENOH_C_IOBUF_H */