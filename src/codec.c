#include "zenoh/codec.h"
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
    Z_DEBUG_VA("vle c = 0x%x\n",c);
    r.value.vle = r.value.vle | ((c & 0x7f) << i);
    Z_DEBUG_VA("current vle  = %zu\n",r.value.vle);
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

void z_iobuf_encode(z_iobuf_t *buf, const z_iobuf_t *bs) {
  z_vle_t len = z_iobuf_readable(bs);
  z_vle_encode(buf, len);
  z_iobuf_write_slice(buf, bs->buf, bs->r_pos, bs->w_pos);
}

z_iobuf_t z_iobuf_decode(z_iobuf_t *buf) {
  z_vle_result_t r_len = z_vle_decode(buf);
  ASSERT_RESULT(r_len, "Unable to decode iobuf");
  uint8_t *bs = z_iobuf_read_n(buf, r_len.value.vle);
  z_iobuf_t iob = z_iobuf_wrap_wo(bs, r_len.value.vle, 0, r_len.value.vle);
  return iob;
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
z_open_encode(z_iobuf_t* buf, const z_open_t* m) {
  z_iobuf_write(buf, m->version);  
  z_array_uint8_encode(buf, &(m->pid));
  z_vle_encode(buf, m->lease);
  z_vle_encode(buf, 0); // no locators
  // TODO: Encode properties if present
}

void
z_accept_decode_na(z_iobuf_t* buf, z_accept_result_t *r) {  

  r->tag = Z_OK_TAG;  

  z_array_uint8_result_t r_cpid = z_array_uint8_decode(buf);
  ASSURE_P_RESULT(r_cpid, r, Z_ARRAY_PARSE_ERROR) 

  z_array_uint8_result_t r_bpid = z_array_uint8_decode(buf);
  ASSURE_P_RESULT(r_bpid, r, Z_ARRAY_PARSE_ERROR)

  z_vle_result_t r_vle = z_vle_decode(buf);
  ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  
  // TODO: Decode Properties

  r->value.accept.client_pid = r_cpid.value.array_uint8;
  r->value.accept.broker_pid = r_bpid.value.array_uint8;
  r->value.accept.lease = r_vle.value.vle;
}

z_accept_result_t 
z_accept_decode(z_iobuf_t* buf) {
  z_accept_result_t r;
  z_accept_decode_na(buf, &r);
  return r;
}

void 
z_close_encode(z_iobuf_t* buf, const z_close_t* m) { 
  z_array_uint8_encode(buf, &(m->pid));
  z_iobuf_write(buf, m->reason);
}

void 
z_close_decode_na(z_iobuf_t* buf, z_close_result_t *r) {   
  r->tag = Z_OK_TAG;
  z_array_uint8_result_t ar =  z_array_uint8_decode(buf);
  ASSURE_P_RESULT(ar, r, Z_ARRAY_PARSE_ERROR)
  r->value.close.pid = ar.value.array_uint8;
  r->value.close.reason = z_iobuf_read(buf);  
}

z_close_result_t 
z_close_decode(z_iobuf_t* buf) { 
  z_close_result_t r;
  z_close_decode_na(buf, &r); 
  return r;
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

void 
z_sub_mode_encode(z_iobuf_t* buf, const z_sub_mode_t* m) {
  z_iobuf_write(buf, m->kind);
  switch (m->kind) {
    case Z_PERIODIC_PULL_MODE:
    case Z_PERIODIC_PUSH_MODE:
      z_temporal_property_encode(buf, &m->tprop);
      break;
    default:
      break;
  }
}

void
z_sub_mode_decode_na(z_iobuf_t* buf, z_sub_mode_result_t *r) {  
  z_temporal_property_result_t r_tp;
  r->tag = Z_OK_TAG;
  r->value.sub_mode.kind = z_iobuf_read(buf);
  switch (r->value.sub_mode.kind) {
    case Z_PERIODIC_PULL_MODE:
    case Z_PERIODIC_PUSH_MODE:      
      r_tp = z_temporal_property_decode(buf);
      ASSURE_P_RESULT(r_tp, r, Z_MESSAGE_PARSE_ERROR)
      r->value.sub_mode.tprop = r_tp.value.temporal_property;
      break; 
    default:
      break;
  }  
}

z_sub_mode_result_t 
z_sub_mode_decode(z_iobuf_t* buf) {
  z_sub_mode_result_t r;
  z_sub_mode_decode_na(buf, &r);
  return r;
}

void 
z_declaration_encode(z_iobuf_t *buf, z_declaration_t *d) {
  z_iobuf_write(buf, d->header);  
  switch (Z_MID(d->header)) {
    case Z_RESOURCE_DECL:      
      z_vle_encode(buf, d->payload.resource.rid);
      z_string_encode(buf, d->payload.resource.r_name);
      break;    
    case Z_PUBLISHER_DECL:
      z_vle_encode(buf, d->payload.pub.rid);
      break;
    case Z_STORAGE_DECL:
      z_vle_encode(buf, d->payload.storage.rid);
      break; 
    case Z_EVAL_DECL:
      z_vle_encode(buf, d->payload.eval.rid);
      break;  
    case Z_SUBSCRIBER_DECL:
      z_vle_encode(buf, d->payload.sub.rid);
      z_sub_mode_encode(buf, &d->payload.sub.sub_mode);
      break;
    case Z_FORGET_PUBLISHER_DECL:
      z_vle_encode(buf, d->payload.forget_pub.rid);
      break;
    case Z_FORGET_STORAGE_DECL:
      z_vle_encode(buf, d->payload.forget_sto.rid);
      break;
    case Z_FORGET_EVAL_DECL:
      z_vle_encode(buf, d->payload.forget_eval.rid);
      break;
    case Z_FORGET_SUBSCRIBER_DECL:
      z_vle_encode(buf, d->payload.forget_sub.rid);
      break;
    case Z_RESULT_DECL:  
      z_iobuf_write(buf, d->payload.result.cid);
      z_iobuf_write(buf, d->payload.result.status);
      break;
    case Z_COMMIT_DECL:
      z_iobuf_write(buf, d->payload.commit.cid);
      break;
    default:      
      break;
  }  
}

void 
z_declaration_decode_na(z_iobuf_t *buf, z_declaration_result_t *r) {
  z_vle_result_t r_vle;
  z_string_result_t r_str;
  z_sub_mode_result_t r_sm;
  r->tag = Z_OK_TAG;
  r->value.declaration.header = z_iobuf_read(buf); 
  switch (Z_MID(r->value.declaration.header)) {
    case Z_RESOURCE_DECL:      
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
      r_str = z_string_decode(buf);
      ASSURE_P_RESULT(r_str, r, Z_STRING_PARSE_ERROR)  
      r->value.declaration.payload.resource.rid = r_vle.value.vle;      
      r->value.declaration.payload.resource.r_name = r_str.value.string;
      break;    
    case Z_PUBLISHER_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)      
      r->value.declaration.payload.pub.rid = r_vle.value.vle;
      break;
    case Z_STORAGE_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)      
      r->value.declaration.payload.storage.rid = r_vle.value.vle;
      break; 
    case Z_EVAL_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)      
      r->value.declaration.payload.eval.rid = r_vle.value.vle;
      break;  
    case Z_SUBSCRIBER_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
      r_sm = z_sub_mode_decode(buf);
      ASSURE_P_RESULT(r_sm, r, Z_MESSAGE_PARSE_ERROR)
      r->value.declaration.payload.sub.rid = r_vle.value.vle;
      r->value.declaration.payload.sub.sub_mode = r_sm.value.sub_mode; 
      break;
    case Z_FORGET_PUBLISHER_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
      r->value.declaration.payload.forget_pub.rid = r_vle.value.vle;
      break;
    case Z_FORGET_STORAGE_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
      r->value.declaration.payload.forget_sto.rid = r_vle.value.vle;
      break;
    case Z_FORGET_EVAL_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
      r->value.declaration.payload.forget_eval.rid = r_vle.value.vle;
      break;
    case Z_FORGET_SUBSCRIBER_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
      r->value.declaration.payload.forget_sub.rid = r_vle.value.vle;
      break;
    case Z_RESULT_DECL:  
      r->value.declaration.payload.result.cid = z_iobuf_read(buf);
      r->value.declaration.payload.result.status = z_iobuf_read(buf);  
      break;
    case Z_COMMIT_DECL:
      r->value.declaration.payload.commit.cid = z_iobuf_read(buf);
      break;
    default:
      r->tag = Z_ERROR_TAG;
      r->value.error = Z_MESSAGE_PARSE_ERROR;
      return;
  }  
}
z_declaration_result_t
z_declaration_decode(z_iobuf_t *buf) {
  z_declaration_result_t r;
  z_declaration_decode_na(buf, &r);
  return r;
}
void 
z_declare_encode(z_iobuf_t* buf, const z_declare_t* m) { 
  z_vle_encode(buf, m->sn);
  unsigned int len = m->declarations.length;
  z_vle_encode(buf, len);  
  for (unsigned int i = 0; i < len; ++i) {    
    z_declaration_encode(buf, &m->declarations.elem[i]);
  }
}

void
z_declare_decode_na(z_iobuf_t* buf, z_declare_result_t *r) { 
  z_declaration_result_t *r_decl;
  z_vle_result_t r_sn = z_vle_decode(buf);
  r->tag = Z_OK_TAG;
  ASSURE_P_RESULT(r_sn, r, Z_VLE_PARSE_ERROR)

  z_vle_result_t r_dlen = z_vle_decode(buf);
  ASSURE_P_RESULT(r_dlen, r, Z_VLE_PARSE_ERROR)
  size_t len = r_dlen.value.vle;
  r->value.declare.declarations.length = len;
  r->value.declare.declarations.elem = (z_declaration_t*)malloc(sizeof(z_declaration_t)*len);  
  
  r_decl = (z_declaration_result_t*)malloc(sizeof(z_declaration_result_t));
  for (unsigned int i = 0; i < len; ++i) {    
    z_declaration_decode_na(buf, r_decl);
    if (r_decl->tag != Z_ERROR_TAG) 
      r->value.declare.declarations.elem[i] = r_decl->value.declaration; 
    else {
      r->value.declare.declarations.length = 0;
      free(r->value.declare.declarations.elem);        
      free(r_decl);
      r->tag = Z_ERROR_TAG;
      r->value.error = Z_MESSAGE_PARSE_ERROR;
      return;
    }
  }
  free(r_decl);
}

z_declare_result_t
z_declare_decode(z_iobuf_t* buf) { 
  z_declare_result_t r;
  z_declare_decode_na(buf, &r);
  return r;
}

void 
z_compact_data_encode(z_iobuf_t* buf, const z_compact_data_t* m) {
  z_vle_encode(buf, m->sn);
  z_vle_encode(buf, m->rid);
  z_iobuf_encode(buf, &m->payload);    
}

void
z_compact_data_decode_na(z_iobuf_t* buf, z_compact_data_result_t *r) {  
  r->tag = Z_OK_TAG;
  z_vle_result_t r_vle = z_vle_decode(buf);
  ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  
  r->value.compact_data.sn = r_vle.value.vle;
  r_vle = z_vle_decode(buf);
  ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  r->value.compact_data.rid = r_vle.value.vle;    
  r->value.compact_data.payload = z_iobuf_decode(buf);
}

z_compact_data_result_t 
z_compact_data_decode(z_iobuf_t* buf) {
  z_compact_data_result_t r;
  z_compact_data_decode_na(buf, &r);
  return r;
}

void z_payload_header_encode(z_iobuf_t *buf, const z_payload_header_t *ph) {
  uint8_t flags = ph->flags;
  Z_DEBUG_VA("z_payload_header_encode flags = 0x%x\n", flags);
  z_iobuf_write(buf, flags);
  if (flags & Z_SRC_ID) {
    Z_DEBUG("Encoding Z_SRC_ID\n");
    z_iobuf_write_slice(buf, (uint8_t*)ph->src_id, 0, 16);
  }
  if (flags & Z_SRC_SN) {
    Z_DEBUG("Encoding Z_SRC_SN\n");
    z_vle_encode(buf, ph->src_sn);    
  }
  if (flags & Z_BRK_ID) {
    Z_DEBUG("Encoding Z_BRK_ID\n");
    z_iobuf_write_slice(buf, (uint8_t*)ph->brk_id, 0, 16);
  }
  if (flags & Z_BRK_SN) {
    Z_DEBUG("Encoding Z_BRK_SN\n");
    z_vle_encode(buf, ph->brk_sn);
  }
  if (flags & Z_KIND) {
    Z_DEBUG("Encoding Z_KIND\n");
    z_vle_encode(buf, ph->kind);
  }    
  if (flags & Z_ENCODING) {
    Z_DEBUG("Encoding Z_ENCODING\n");
    z_vle_encode(buf, ph->encoding);
  }

  z_iobuf_encode(buf, &ph->payload);
}

void 
z_payload_header_decode_na(z_iobuf_t *buf, z_payload_header_result_t *r) {  
  z_vle_result_t r_vle;
  uint8_t flags = z_iobuf_read(buf);
  Z_DEBUG_VA("Payload header flags: 0x%x\n", flags);

  if (flags & Z_SRC_ID) {
    Z_DEBUG("Decoding Z_SRC_ID\n");
    z_iobuf_read_to_n(buf, r->value.payload_header.src_id, 16);
  }

  if (flags & Z_T_STAMP) {          
    r_vle = z_vle_decode(buf);        
    ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);
    r->value.payload_header.tstamp.time = r_vle.value.vle;
    memcpy(r->value.payload_header.tstamp.clock_id, buf->buf + buf->r_pos, 16);
    buf->r_pos += 16;
  }
    
  if (flags & Z_SRC_SN) { 
    Z_DEBUG("Decoding Z_SRC_SN\n");
    r_vle = z_vle_decode(buf);
    ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);
    r->value.payload_header.src_sn = r_vle.value.vle;
  }

  if (flags & Z_BRK_ID) {
    Z_DEBUG("Decoding Z_BRK_ID\n");
    z_iobuf_read_to_n(buf, r->value.payload_header.brk_id, 16);
  }

  if (flags & Z_BRK_SN) {
    Z_DEBUG("Decoding Z_BRK_SN\n");
    r_vle = z_vle_decode(buf);
    ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);
    r->value.payload_header.brk_sn = r_vle.value.vle;
  }

  if (flags & Z_KIND) {
    Z_DEBUG("Decoding Z_KIND\n");
    r_vle = z_vle_decode(buf);
    ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);
    r->value.payload_header.kind = r_vle.value.vle;
  }

  if (flags & Z_ENCODING) {
    Z_DEBUG("Decoding Z_ENCODING\n");
    r_vle = z_vle_decode(buf);
    ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);
    r->value.payload_header.encoding = r_vle.value.vle;
    Z_DEBUG("Done Decoding Z_ENCODING\n");
  }
  
  r->value.payload_header.flags = flags;
  r->value.payload_header.payload = z_iobuf_decode(buf);
  r->tag = Z_OK_TAG;
}

z_payload_header_result_t 
z_payload_header_decode(z_iobuf_t *buf) {  
  z_payload_header_result_t r;
  z_payload_header_decode_na(buf, &r);
  return r;
}

void 
z_stream_data_encode(z_iobuf_t *buf, const z_stream_data_t* m) {
  z_vle_encode(buf, m->sn);
  z_vle_encode(buf, m->rid);
  z_vle_t len = z_iobuf_readable(&m->payload_header);
  z_vle_encode(buf, len);  
  z_iobuf_write_slice(buf, m->payload_header.buf, m->payload_header.r_pos, m->payload_header.w_pos);
}

void z_stream_data_decode_na(z_iobuf_t *buf, z_stream_data_result_t *r) {
  r->tag = Z_OK_TAG;
  z_vle_result_t r_vle = z_vle_decode(buf);
  ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);
  r->value.stream_data.sn = r_vle.value.vle;

  r_vle = z_vle_decode(buf);
  ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);
  r->value.stream_data.rid = r_vle.value.vle;

  r_vle = z_vle_decode(buf);
  ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);  
  uint8_t *ph = z_iobuf_read_n(buf, r_vle.value.vle);
  r->value.stream_data.payload_header = z_iobuf_wrap_wo(ph, r_vle.value.vle, 0, r_vle.value.vle);
  r->value.stream_data.payload_header.w_pos = r_vle.value.vle;
}

z_stream_data_result_t
z_stream_data_decode(z_iobuf_t *buf) {
  z_stream_data_result_t r;
  z_stream_data_decode_na(buf, &r);
  return r;
}

void
z_write_data_encode(z_iobuf_t *buf, const z_write_data_t* m) {
  z_vle_encode(buf, m->sn);
  z_string_encode(buf, m->rname);
  z_vle_t len = z_iobuf_readable(&m->payload_header);
  z_vle_encode(buf, len);  
  z_iobuf_write_slice(buf, m->payload_header.buf, m->payload_header.r_pos, m->payload_header.w_pos);    
}


void z_write_data_decode_na(z_iobuf_t *buf, z_write_data_result_t *r) {
  r->tag = Z_OK_TAG;
  z_string_result_t r_str;
  z_vle_result_t r_vle = z_vle_decode(buf);
  ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);
  r->value.write_data.sn = r_vle.value.vle;

  r_str = z_string_decode(buf);
  ASSURE_P_RESULT(r_str, r, Z_STRING_PARSE_ERROR);
  r->value.write_data.rname = r_str.value.string;
  Z_DEBUG_VA("Decoding write data for resource %s\n", r_str.value.string);
  r_vle = z_vle_decode(buf);
  ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);  
  uint8_t *ph = z_iobuf_read_n(buf, r_vle.value.vle);
  r->value.write_data.payload_header = z_iobuf_wrap_wo(ph, r_vle.value.vle, 0, r_vle.value.vle);
  r->value.write_data.payload_header.w_pos = r_vle.value.vle;
}

z_write_data_result_t
z_write_data_decode(z_iobuf_t *buf) {
  z_write_data_result_t r;
  z_write_data_decode_na(buf, &r);
  return r;
}

void
z_query_encode(z_iobuf_t *buf, const z_query_t* m) {
  z_array_uint8_encode(buf, &(m->pid));
  z_vle_encode(buf, m->qid);
  z_string_encode(buf, m->rname);
  z_string_encode(buf, m->predicate);
  // TODO : encode properties 
}

void z_query_decode_na(z_iobuf_t *buf, z_query_result_t *r) {
  r->tag = Z_OK_TAG;

  z_array_uint8_result_t r_pid = z_array_uint8_decode(buf);
  ASSURE_P_RESULT(r_pid, r, Z_ARRAY_PARSE_ERROR)
  r->value.query.pid = r_pid.value.array_uint8;

  z_vle_result_t r_qid = z_vle_decode(buf);
  ASSURE_P_RESULT(r_qid, r, Z_VLE_PARSE_ERROR)
  r->value.query.qid = r_qid.value.vle;

  z_string_result_t r_str = z_string_decode(buf);
  ASSURE_P_RESULT(r_str, r, Z_STRING_PARSE_ERROR);
  r->value.query.rname = r_str.value.string;

  r_str = z_string_decode(buf);
  ASSURE_P_RESULT(r_str, r, Z_STRING_PARSE_ERROR);
  r->value.query.predicate = r_str.value.string;

  // TODO : decode properties
}

z_query_result_t
z_query_decode(z_iobuf_t *buf) {
  z_query_result_t r;
  z_query_decode_na(buf, &r);
  return r;
}

void
z_reply_encode(z_iobuf_t *buf, const z_reply_t* m, uint8_t header) {
  z_array_uint8_encode(buf, &(m->qpid));
  z_vle_encode(buf, m->qid);

  if(header & Z_F_FLAG) {
    z_array_uint8_encode(buf, &(m->stoid));
    z_vle_encode(buf, m->rsn);
    z_string_encode(buf, m->rname);
    z_vle_t len = z_iobuf_readable(&m->payload_header);
    z_vle_encode(buf, len);  
    z_iobuf_write_slice(buf, m->payload_header.buf, m->payload_header.r_pos, m->payload_header.w_pos); 
  }
}

void z_reply_decode_na(z_iobuf_t *buf, uint8_t header, z_reply_result_t *r) {
  r->tag = Z_OK_TAG;

  z_array_uint8_result_t r_qpid = z_array_uint8_decode(buf);
  ASSURE_P_RESULT(r_qpid, r, Z_ARRAY_PARSE_ERROR)
  r->value.reply.qpid = r_qpid.value.array_uint8;

  z_vle_result_t r_qid = z_vle_decode(buf);
  ASSURE_P_RESULT(r_qid, r, Z_VLE_PARSE_ERROR)
  r->value.reply.qid = r_qid.value.vle;

  if (header & Z_F_FLAG)
  {
    z_array_uint8_result_t r_stoid = z_array_uint8_decode(buf);
    ASSURE_P_RESULT(r_stoid, r, Z_ARRAY_PARSE_ERROR)
    r->value.reply.stoid = r_stoid.value.array_uint8;

    z_vle_result_t r_vle = z_vle_decode(buf);
    ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
    r->value.reply.rsn = r_vle.value.vle;

    z_string_result_t r_str = z_string_decode(buf);
    ASSURE_P_RESULT(r_str, r, Z_STRING_PARSE_ERROR);
    r->value.reply.rname = r_str.value.string;

    r_vle = z_vle_decode(buf);
    ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR); 
    uint8_t *ph = z_iobuf_read_n(buf, r_vle.value.vle);
    r->value.reply.payload_header = z_iobuf_wrap_wo(ph, r_vle.value.vle, 0, r_vle.value.vle);
    r->value.reply.payload_header.w_pos = r_vle.value.vle;
  }
}

z_reply_result_t 
z_reply_decode(z_iobuf_t *buf, uint8_t header) {  
  z_reply_result_t r;
  z_reply_decode_na(buf, header, &r);
  return r;
}

void 
z_message_encode(z_iobuf_t* buf, const z_message_t* m) {
  z_iobuf_write(buf, m->header);
  uint8_t mid = Z_MID(m->header);
  switch (mid) {
    case Z_COMPACT_DATA:
      z_compact_data_encode(buf, &m->payload.compact_data);
      break;    
    case Z_STREAM_DATA:
      z_stream_data_encode(buf, &m->payload.stream_data); 
      break;
    case Z_WRITE_DATA:
      z_write_data_encode(buf, &m->payload.write_data);
      break;
    case Z_QUERY:
      z_query_encode(buf, &m->payload.query);
      break;
    case Z_REPLY:
      z_reply_encode(buf, &m->payload.reply, m->header);
      break;
    case Z_OPEN:
      z_open_encode(buf, &m->payload.open);
      break;
    case Z_CLOSE:
      z_close_encode(buf, &m->payload.close);
      break;
    case Z_DECLARE:
      z_declare_encode(buf, &m->payload.declare);
      break;
    default:
      Z_ERROR("WARNING: Trying to encode message with unknown ID(%d)\n", mid); 
      return;
  }
  if (m->header & Z_P_FLAG ) {
    z_properties_encode(buf, m->properties);  
  }
}

void
z_message_decode_na(z_iobuf_t* buf, z_message_p_result_t* r) {
  z_compact_data_result_t r_cd;  
  z_stream_data_result_t r_sd;
  z_write_data_result_t r_wd;
  z_query_result_t r_q;
  z_reply_result_t r_r;
  z_accept_result_t r_a;
  z_close_result_t r_c;
  z_declare_result_t r_d;

  uint8_t h = z_iobuf_read(buf);
  r->tag = Z_OK_TAG;
  r->value.message->header = h;
  
  uint8_t mid = Z_MID(h);
  switch (mid) {
    case Z_COMPACT_DATA:
      r->tag = Z_OK_TAG;
      r_cd = z_compact_data_decode(buf);
      ASSURE_P_RESULT(r_cd, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.compact_data = r_cd.value.compact_data;
      break;
    case Z_STREAM_DATA:
      r->tag = Z_OK_TAG;
      r_sd = z_stream_data_decode(buf);
      ASSURE_P_RESULT(r_sd, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.stream_data = r_sd.value.stream_data;
      break;
    case Z_WRITE_DATA:
      r->tag = Z_OK_TAG;
      r_wd = z_write_data_decode(buf);
      ASSURE_P_RESULT(r_wd, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.write_data = r_wd.value.write_data;
      break;  
    case Z_QUERY:
      r->tag = Z_OK_TAG;
      r_q = z_query_decode(buf);
      ASSURE_P_RESULT(r_q, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.query = r_q.value.query;
      break;  
    case Z_REPLY:      
      r->tag = Z_OK_TAG;
      r_r = z_reply_decode(buf, h);
      ASSURE_P_RESULT(r_r, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.reply = r_r.value.reply;      
      break;  
    case Z_ACCEPT:
      r->tag = Z_OK_TAG;
      r_a = z_accept_decode(buf);
      ASSURE_P_RESULT(r_a, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.accept = r_a.value.accept;
      break;
    case Z_CLOSE:
      r->tag = Z_OK_TAG;
      r_c = z_close_decode(buf);
      ASSURE_P_RESULT(r_c, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.close = r_c.value.close;
      break;
    case Z_DECLARE:
      r->tag = Z_OK_TAG;
      r_d = z_declare_decode(buf);
      ASSURE_P_RESULT(r_d, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.declare = r_d.value.declare;
      break;
    default:
      r->tag = Z_ERROR_TAG;
      r->value.error = Z_MESSAGE_PARSE_ERROR;
      Z_ERROR("WARNING: Trying to decode message with unknown ID(%d)\n", mid); 

  }
}

z_message_p_result_t
z_message_decode(z_iobuf_t* buf) {
  z_message_p_result_t r;
  z_message_p_result_init(&r); 
  z_message_decode_na(buf, &r);
  return r;
}
