#include "zenoh/codec.h"
#include <stdio.h>

void z_vle_encode(z_iobuf_t* buf, z_vle_t v) {
  while (v > 0x7f) {
    uint8_t c = (v & 0x7f) | 0x80;
    z_iobuf_write(buf, (uint8_t)c);
    v = v >> 7;  
  }
  z_iobuf_write(buf, (uint8_t)v);
}

z_vle_result_t z_vle_decode(z_iobuf_t* buf) {
  z_vle_result_t r;
  r.tag = VLE;
  r.value.vle = 0;

  uint8_t c;  
  int i = 0;
  do {
    c = z_iobuf_read(buf);
    printf("vle c = 0x%x\n",c);
    r.value.vle = r.value.vle | ((c & 0x7f) << i);
    printf("current vle  = %llu\n",r.value.vle);
    i += 7;
  } while (c > 0x7f); 
  return r;
} 

void z_array_uint8_encode(z_iobuf_t* buf, const z_array_uint8_t* bs) {
  z_vle_encode(buf, bs->length);
  z_iobuf_write_n(buf, bs->elem, 0,  bs->length);
}

z_array_uint8_result_t z_array_uint8_decode(z_iobuf_t* buf) {
  printf("z_array_uint8_decode\n");
  z_array_uint8_result_t r;
  r.tag = ARRAY_UINT8;
  z_vle_result_t r_vle = z_vle_decode(buf);
  if (r_vle.tag != VLE) {
    r.tag = ERROR;
    r.value.error = VLE_PARSE_ERROR;
    return r;
  }  
  r.value.array_uint8.length = (unsigned int)r_vle.value.vle;  
  r.value.array_uint8.elem = z_iobuf_read_n(buf, r.value.array_uint8.length); 
  return r;
}

void z_open_encode(z_iobuf_t* buf, const z_open_t* m) {
  z_iobuf_write(buf, m->header);
  z_iobuf_write(buf, m->version);  
  z_array_uint8_encode(buf, &(m->pid));
  z_vle_encode(buf, m->lease);
  // TODO: Encode properties if present
}

z_accept_result_t z_accept_decode(z_iobuf_t* buf, uint8_t header) {
  printf("z_accept_decode\n");
  z_accept_result_t r;
  r.tag = ACCEPT;  
  r.value.accept.header = header;
  printf("Array decode 1 \n");
  z_array_uint8_result_t r_cpid = z_array_uint8_decode(buf);
  if (r_cpid.tag != ARRAY_UINT8) {
    r.tag = ERROR;
    r.value.error = ARRAY_PARSE_ERROR;
    return r;
  }
  printf("Array decode 2\n");
  z_array_uint8_result_t r_bpid = z_array_uint8_decode(buf);
  if (r_bpid.tag != ARRAY_UINT8) {
    r.tag = ERROR;
    r.value.error = ARRAY_PARSE_ERROR;
    return r;
  }

  z_vle_result_t r_vle = z_vle_decode(buf);
  if (r_vle.tag != VLE) {
    r.tag = ERROR;
    r.value.error = VLE_PARSE_ERROR;
    return r;
  }
 
  // TODO: Decode Properties

  r.value.accept.client_pid = r_cpid.value.array_uint8;
  r.value.accept.broker_pid = r_bpid.value.array_uint8;
  r.value.accept.lease = r_vle.value.vle;

  return r;
}
