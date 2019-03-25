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
  r.tag = Z_VLE_TAG;
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
  r.tag = Z_ARRAY_UINT8_TAG;
  z_vle_result_t r_vle = z_vle_decode(buf);
  if (r_vle.tag != Z_VLE_TAG) {
    r.tag = Z_ERROR_TAG;
    r.value.error = VLE_PARSE_ERROR;
    return r;
  }  
  r.value.array_uint8.length = (unsigned int)r_vle.value.vle;  
  r.value.array_uint8.elem = z_iobuf_read_n(buf, r.value.array_uint8.length); 
  return r;
}

void z_string_encode(z_iobuf_t* buf, const char* s) {
  size_t len = strlen(s);
  z_vle_encode(buf, len);
  // Note that this does not put the string terminator on the wire.
  z_iobuf_write_n(buf, (uint8_t*)s, 0, len);
}

z_string_result_t z_string_decode(z_iobuf_t* buf) {
  z_string_result_t r;
  r.tag = Z_STRING_TAG;
  z_vle_result_t vr = z_vle_decode(buf);
  if (vr.tag == Z_ERROR_TAG) {
    r.tag = Z_ERROR_TAG;
    return r;
  }
  size_t len = vr.value.vle;
  // Allocate space for the string terminator.
  char* s = (char*)malloc(len + 1);
  s[len] = '\0';
  z_iobuf_read_to_n(buf, (uint8_t*)s, len);
  r.value.string = s;
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
  r.tag = Z_ACCEPT_TAG;  
  r.value.accept.header = header;
  printf("Array decode 1 \n");
  z_array_uint8_result_t r_cpid = z_array_uint8_decode(buf);
  if (r_cpid.tag != Z_ARRAY_UINT8_TAG) {
    r.tag = Z_ERROR_TAG;
    r.value.error = ARRAY_PARSE_ERROR;
    return r;
  }
  printf("Array decode 2\n");
  z_array_uint8_result_t r_bpid = z_array_uint8_decode(buf);
  if (r_bpid.tag != Z_ARRAY_UINT8_TAG) {
    r.tag = Z_ERROR_TAG;
    r.value.error = ARRAY_PARSE_ERROR;
    return r;
  }

  z_vle_result_t r_vle = z_vle_decode(buf);
  if (r_vle.tag != Z_VLE_TAG) {
    r.tag = Z_ERROR_TAG;
    r.value.error = VLE_PARSE_ERROR;
    return r;
  }
 
  // TODO: Decode Properties

  r.value.accept.client_pid = r_cpid.value.array_uint8;
  r.value.accept.broker_pid = r_bpid.value.array_uint8;
  r.value.accept.lease = r_vle.value.vle;

  return r;
}


void z_close_encode(z_iobuf_t* buf, const z_close_t* m) { 
  z_iobuf_write(buf, m->header);
  z_array_uint8_encode(buf, &(m->pid));
  z_iobuf_write(buf, m->reason);
}

z_close_result_t z_close_decode(z_iobuf_t* buf, uint8_t header) { 
  z_close_result_t r;
  r.tag = Z_CLOSE_TAG;
  r.value.close.header = z_iobuf_read(buf);
  z_array_uint8_result_t ar =  z_array_uint8_decode(buf);
  if (ar.tag == Z_ERROR_TAG) {
    r.tag = Z_ERROR_TAG;
    r.value.error = ARRAY_PARSE_ERROR;
    return r;
  }
  r.value.close.pid = ar.value.array_uint8;
  r.value.close.reason = z_iobuf_read(buf);
  return r;
}

void z_res_decl_encode(z_iobuf_t* buf, const z_res_decl_t* m) { 
  z_iobuf_write(buf, m->header);
  z_vle_encode(buf, m->rid);
  z_string_encode(buf, m->r_name);
  // TODO: Encode Properties
}
void z_pub_decl_encode(z_iobuf_t* buf, const z_pub_decl_t* m) {
  z_iobuf_write(buf, m->header);
  z_vle_encode(buf, m->rid);
  // TODO: Encode Properties    
}
void z_storage_decl_encode(z_iobuf_t* buf, const z_storage_decl_t* m) { 
    z_iobuf_write(buf, m->header);
  z_vle_encode(buf, m->rid);
  // TODO: Encode Properties    
}
void z_temporal_property_encode(z_iobuf_t* buf, const z_temporal_property_t* m) { }
void z_sub_mode_encode(z_iobuf_t* buf, const z_sub_mode_t* m) {}
void z_sub_decl_encode(z_iobuf_t* buf, const z_sub_decl_t* m) {}
void z_commit_decl_encode(z_iobuf_t* buf, const z_commit_decl_t* m) { }
z_result_decl_result_t z_result_decl_decode(z_iobuf_t* buf, uint8_t header) { 
  z_result_decl_result_t r;
  r.tag = Z_RES_DECL_TAG;
  return r;
}
z_stream_data_result_t z_stream_data_decode(z_iobuf_t* buf, uint8_t header) {
  z_stream_data_result_t r;
  r.tag = Z_STREAM_DATA_TAG;
  return r;
}
void z_stream_data_encode(z_iobuf_t* buf, const z_stream_data_t* m) {}

void z_declare_encode(z_iobuf_t* buf, const z_declare_t* m) { }
