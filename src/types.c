#include "zenoh/types.h"
#include <assert.h>
#include <stdio.h>

z_iobuf_t 
z_iobuf_wrap(uint8_t *buf, unsigned int capacity) {
  z_iobuf_t iobuf;
  iobuf.r_pos = 0;
  iobuf.w_pos = 0; 
  iobuf.capacity = capacity;
  iobuf.buf = buf;  
  return iobuf;
}

z_iobuf_t z_iobuf_make(unsigned int capacity) {
  return z_iobuf_wrap((uint8_t*)malloc(capacity), capacity);  
}


void z_iobuf_free(z_iobuf_t* buf) {  
  buf->r_pos = 0;
  buf->w_pos = 0;
  buf->capacity = 0;
  free(buf->buf);  
  buf = 0;
}

unsigned int z_iobuf_readable(const z_iobuf_t* iob) {
  return iob->w_pos - iob->r_pos;
}
unsigned int z_iobuf_writable(const z_iobuf_t* iob) {
  return iob->capacity - iob->w_pos;
}

void z_iobuf_write(z_iobuf_t* iob, uint8_t b) {
  assert(iob->w_pos < iob->capacity);
  iob->buf[iob->w_pos++] = b; 
}

void z_iobuf_write_n(z_iobuf_t* iob, const uint8_t* bs, unsigned int offset, unsigned int length) {
  assert(z_iobuf_writable(iob) >= length); 
  memcpy(iob->buf + iob->w_pos, bs + offset, length);
  iob->w_pos += length;
}

uint8_t z_iobuf_read(z_iobuf_t* iob) {
  Z_DEBUG_VA("r_pos = %d - w_pos = %d\n", iob->r_pos , iob->w_pos);
  assert(iob->r_pos < iob->w_pos);
  return iob->buf[iob->r_pos++]; 
}


uint8_t* z_iobuf_read_to_n(z_iobuf_t* iob, uint8_t* dst, unsigned int length) {
  assert(z_iobuf_readable(iob) >= length);  
  memcpy(dst, iob->buf + iob->r_pos, length);
  iob->r_pos += length;
  return dst;
  
}

uint8_t* z_iobuf_read_n(z_iobuf_t* iob, unsigned int length) {  
  uint8_t* dst = (uint8_t*)malloc(length);
  return z_iobuf_read_to_n(iob, dst, length);  
}


void z_iobuf_put(z_iobuf_t* iob, uint8_t b, unsigned int pos) {
  assert(pos < iob->capacity);
  iob->buf[pos] = b;
}

uint8_t z_iobuf_sget(z_iobuf_t* iob, unsigned int pos) {
  assert(pos < iob->capacity);
  return iob->buf[pos];
}

void z_iobuf_clear(z_iobuf_t *buf) {
  buf->r_pos = 0;
  buf->w_pos = 0;
}

z_array_uint8_t z_iobuf_to_array(z_iobuf_t* buf) {
  z_array_uint8_t a = {z_iobuf_readable(buf), &buf->buf[buf->r_pos]};
  return a;
}



void z_register_subscription(zenoh_t *z, z_vle_t rid,  subscriber_callback_t *callback) {
  z_subscription_t *sub = (z_subscription_t *) malloc(sizeof(z_subscription_t));
  sub->rid = rid;
  sub->callback = callback;
  z->subscriptions = z_list_cons(z->subscriptions, sub);
}

z_subscription_t *z_get_subscription(zenoh_t *z, z_vle_t rid) {
  if (z->subscriptions == 0) {
    printf(">>> Empty subscription set");
    return 0;
  }
  else {
    z_subscription_t *sub = (z_subscription_t *)z_list_head(z->subscriptions);
    z_list_t *subs = z_list_tail(z->subscriptions);
    while (subs != 0 && sub->rid != rid) {      
      sub = z_list_head(subs);
      subs = z_list_tail(subs);  
    }    
    if (sub->rid == rid) return sub;
    else return 0;
  }
}
