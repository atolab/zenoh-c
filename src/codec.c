#include "zenoh/codec.h"
#include "zenoh/property.h"
#include "zenoh/private/logging.h"
#include <stdio.h>

void 
z_vle_encode(z_iobuf_t* buf, z_vle_t v) {
  while (v > 0x7f) {
    uint8_t c = (v & 0x7f) | 0x80;
    z_iobuf_write(buf, (uint8_t)c);
    v = v >> 7;  
  }
  z_iobuf_write(buf, (uint8_t)v);
}

z_vle_result_t 
z_vle_decode(z_iobuf_t* buf) {
  z_vle_result_t r;
  r.tag = Z_OK_TAG;
  r.value.vle = 0;

  uint8_t c;  
  int i = 0;
  do {
    c = z_iobuf_read(buf);
    _Z_DEBUG_VA("vle c = 0x%x\n",c);
    r.value.vle = r.value.vle | ((c & 0x7f) << i);
    _Z_DEBUG_VA("current vle  = %zu\n",r.value.vle);
    i += 7;
  } while (c > 0x7f); 
  return r;
} 

void 
z_array_uint8_encode(z_iobuf_t* buf, const z_array_uint8_t* bs) {
  z_vle_encode(buf, bs->length);
  z_iobuf_write_slice(buf, bs->elem, 0,  bs->length);
}

void
z_array_uint8_decode_na(z_iobuf_t* buf, z_array_uint8_result_t *r) {  
  r->tag = Z_OK_TAG;
  z_vle_result_t r_vle = z_vle_decode(buf);
  ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  r->value.array_uint8.length = (unsigned int)r_vle.value.vle;  
  r->value.array_uint8.elem = z_iobuf_read_n(buf, r->value.array_uint8.length); 
}

z_array_uint8_result_t 
z_array_uint8_decode(z_iobuf_t* buf) {  
  z_array_uint8_result_t r;
  z_array_uint8_decode_na(buf, &r);
  return r;  
}

void 
z_string_encode(z_iobuf_t* buf, const char* s) {
  size_t len = strlen(s);
  z_vle_encode(buf, len);
  // Note that this does not put the string terminator on the wire.
  z_iobuf_write_slice(buf, (uint8_t*)s, 0, len);
}

z_string_result_t 
z_string_decode(z_iobuf_t* buf) {
  z_string_result_t r;
  r.tag = Z_OK_TAG;
  z_vle_result_t vr = z_vle_decode(buf);
  ASSURE_RESULT(vr, r, Z_VLE_PARSE_ERROR)
  size_t len = vr.value.vle;
  // Allocate space for the string terminator.
  char* s = (char*)malloc(len + 1);
  s[len] = '\0';
  z_iobuf_read_to_n(buf, (uint8_t*)s, len);
  r.value.string = s;
  return r;
}

void z_property_encode(z_iobuf_t* buf, const z_property_t* m) {
  z_vle_encode(buf, m->id);
  z_array_uint8_encode(buf, &m->value);
}

void z_property_decode_na(z_iobuf_t* buf, z_property_result_t *r) {
  z_vle_result_t r_vle;
  z_array_uint8_result_t r_a8;
  r->tag = Z_OK_TAG;
  r_vle = z_vle_decode(buf);
  ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);
  z_array_uint8_decode_na(buf, &r_a8);
  ASSURE_P_RESULT(r_a8, r, Z_ARRAY_PARSE_ERROR);
  r->value.property.id = r_vle.value.vle;
  r->value.property.value = r_a8.value.array_uint8;
}
z_property_result_t z_property_decode(z_iobuf_t* buf) {
  z_property_result_t r;
  z_property_decode_na(buf, &r);
  return r;  
}

void z_properties_encode(z_iobuf_t *buf, const z_vec_t *ps) {
  z_property_t *p;
  z_vle_t l = z_vec_length(ps);
  z_vle_encode(buf, l);
  for (unsigned int i = 0; i < l; ++i) {    
    p = (z_property_t *)z_vec_get(ps, i);    
    z_property_encode(buf, p);
  }
}

void 
z_temporal_property_encode(z_iobuf_t* buf, const z_temporal_property_t* tp) { 
  z_vle_encode(buf, tp->origin);
  z_vle_encode(buf, tp->period);
  z_vle_encode(buf, tp->duration);
}

void
z_temporal_property_decode_na(z_iobuf_t* buf, z_temporal_property_result_t *r) {  
  r->tag = Z_OK_TAG;
  z_vle_result_t r_origin = z_vle_decode(buf);
  ASSURE_P_RESULT(r_origin, r, Z_VLE_PARSE_ERROR)
  z_vle_result_t r_period = z_vle_decode(buf);
  ASSURE_P_RESULT(r_period, r, Z_VLE_PARSE_ERROR)
  z_vle_result_t r_duration = z_vle_decode(buf);
  ASSURE_P_RESULT(r_duration, r, Z_VLE_PARSE_ERROR)

  r->value.temporal_property.origin = r_origin.value.vle;
  r->value.temporal_property.period = r_period.value.vle;
  r->value.temporal_property.duration = r_duration.value.vle;  
}

z_temporal_property_result_t 
z_temporal_property_decode(z_iobuf_t* buf) {
  z_temporal_property_result_t r;
  z_temporal_property_decode_na(buf, &r);
  return r;
}
