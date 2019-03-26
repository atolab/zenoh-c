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

void 
z_array_uint8_encode(z_iobuf_t* buf, const z_array_uint8_t* bs) {
  z_vle_encode(buf, bs->length);
  z_iobuf_write_n(buf, bs->elem, 0,  bs->length);
}

z_array_uint8_result_t 
z_array_uint8_decode(z_iobuf_t* buf) {
  printf("z_array_uint8_decode\n");
  z_array_uint8_result_t r;
  r.tag = Z_ARRAY_UINT8_TAG;
  z_vle_result_t r_vle = z_vle_decode(buf);
  ASSURE_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  r.value.array_uint8.length = (unsigned int)r_vle.value.vle;  
  r.value.array_uint8.elem = z_iobuf_read_n(buf, r.value.array_uint8.length); 
  return r;
}

void 
z_string_encode(z_iobuf_t* buf, const char* s) {
  size_t len = strlen(s);
  z_vle_encode(buf, len);
  // Note that this does not put the string terminator on the wire.
  z_iobuf_write_n(buf, (uint8_t*)s, 0, len);
}

z_string_result_t 
z_string_decode(z_iobuf_t* buf) {
  z_string_result_t r;
  r.tag = Z_STRING_TAG;
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

void 
z_open_encode(z_iobuf_t* buf, const z_open_t* m) {
  z_iobuf_write(buf, m->header);
  z_iobuf_write(buf, m->version);  
  z_array_uint8_encode(buf, &(m->pid));
  z_vle_encode(buf, m->lease);
  // TODO: Encode properties if present
}

z_accept_result_t 
z_accept_decode(z_iobuf_t* buf, uint8_t header) {
  z_accept_result_t r;
  r.tag = Z_ACCEPT_TAG;  
  r.value.accept.header = header;

  z_array_uint8_result_t r_cpid = z_array_uint8_decode(buf);
  ASSURE_RESULT(r_cpid, r, Z_ARRAY_PARSE_ERROR) 

  z_array_uint8_result_t r_bpid = z_array_uint8_decode(buf);
  ASSURE_RESULT(r_bpid, r, Z_ARRAY_PARSE_ERROR)

  z_vle_result_t r_vle = z_vle_decode(buf);
  ASSURE_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  
  // TODO: Decode Properties

  r.value.accept.client_pid = r_cpid.value.array_uint8;
  r.value.accept.broker_pid = r_bpid.value.array_uint8;
  r.value.accept.lease = r_vle.value.vle;

  return r;
}


void 
z_close_encode(z_iobuf_t* buf, const z_close_t* m) { 
  z_iobuf_write(buf, m->header);
  z_array_uint8_encode(buf, &(m->pid));
  z_iobuf_write(buf, m->reason);
}

z_close_result_t 
z_close_decode(z_iobuf_t* buf, uint8_t header) { 
  z_close_result_t r;
  r.tag = Z_CLOSE_TAG;
  r.value.close.header = z_iobuf_read(buf);
  z_array_uint8_result_t ar =  z_array_uint8_decode(buf);
  ASSURE_RESULT(ar, r, Z_ARRAY_PARSE_ERROR)
  r.value.close.pid = ar.value.array_uint8;
  r.value.close.reason = z_iobuf_read(buf);
  return r;
}

void 
z_res_decl_encode(z_iobuf_t* buf, const z_res_decl_t* m) { 
  z_iobuf_write(buf, m->header);
  z_vle_encode(buf, m->rid);
  z_string_encode(buf, m->r_name);
  // TODO: Encode Properties
}

z_res_decl_result_t 
z_res_decl_decode(z_iobuf_t* buf, uint8_t header) {
  z_res_decl_result_t r;
  r.tag = Z_RES_DECL_TAG;
  z_vle_result_t r_vle = z_vle_decode(buf);
  ASSURE_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  z_string_result_t r_str = z_string_decode(buf);
  ASSURE_RESULT(r_str, r, Z_STRING_PARSE_ERROR)  
  r.value.res_decl.header = header;
  r.value.res_decl.rid = r_vle.value.vle;
  r.value.res_decl.r_name = r_str.value.string;
  return r;
}

void 
z_pub_decl_encode(z_iobuf_t* buf, const z_pub_decl_t* m) {
  z_iobuf_write(buf, m->header);
  z_vle_encode(buf, m->rid);
  // TODO: Encode Properties    
}
void 
z_storage_decl_encode(z_iobuf_t* buf, const z_storage_decl_t* m) { 
  z_iobuf_write(buf, m->header);
  z_vle_encode(buf, m->rid);
  // TODO: Encode Properties    
}
void 
z_temporal_property_encode(z_iobuf_t* buf, const z_temporal_property_t* tp) { 
  z_vle_encode(buf, tp->origin);
  z_vle_encode(buf, tp->period);
  z_vle_encode(buf, tp->duration);
}

z_temporal_property_result_t 
z_temporal_property_decode(z_iobuf_t* buf) {
  z_temporal_property_result_t r;
  r.tag = Z_TEMP_PROPERTY_TAG;
  z_vle_result_t r_origin = z_vle_decode(buf);
  ASSURE_RESULT(r_origin, r, Z_VLE_PARSE_ERROR)
  z_vle_result_t r_period = z_vle_decode(buf);
  ASSURE_RESULT(r_period, r, Z_VLE_PARSE_ERROR)
  z_vle_result_t r_duration = z_vle_decode(buf);
  ASSURE_RESULT(r_duration, r, Z_VLE_PARSE_ERROR)

  r.value.temporal_property.origin = r_origin.value.vle;
  r.value.temporal_property.period = r_period.value.vle;
  r.value.temporal_property.duration = r_duration.value.vle;
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

z_sub_mode_result_t 
z_sub_mode_decode(z_iobuf_t* buf) {
  z_sub_mode_result_t r;
  z_temporal_property_result_t r_tp;
  r.tag = Z_SUB_MODE_TAG;
  r.value.sub_mode.kind = z_iobuf_read(buf);
  switch (r.value.sub_mode.kind) {
    case Z_PERIODIC_PULL_MODE:
    case Z_PERIODIC_PUSH_MODE:      
      r_tp = z_temporal_property_decode(buf);
      ASSURE_RESULT(r_tp, r, Z_MESSAGE_PARSE_ERROR)
      r.value.sub_mode.tprop = r_tp.value.temporal_property;
      break; 
    default:
      break;
  }
  return r;
}


void 
z_sub_decl_encode(z_iobuf_t* buf, const z_sub_decl_t* m) {
  z_iobuf_write(buf, m->header);
  z_vle_encode(buf, m->rid);
  z_sub_mode_encode(buf, &m->sub_mode);
  // TODO: Encode Properties    
}

z_sub_decl_result_t 
z_sub_decl_decode(z_iobuf_t* buf, uint8_t header) {
  z_sub_decl_result_t r;
  r.tag = Z_SUB_DECL_TAG;
  z_vle_result_t r_rid = z_vle_decode(buf);
  ASSURE_RESULT(r_rid, r, Z_VLE_PARSE_ERROR)
  z_sub_mode_result_t r_sm = z_sub_mode_decode(buf);
  ASSURE_RESULT(r_sm, r, Z_MESSAGE_PARSE_ERROR)
  r.value.sub_decl.rid = r_rid.value.vle;
  r.value.sub_decl.sub_mode = r_sm.value.sub_mode;
  return r;
}

void 
z_commit_decl_encode(z_iobuf_t* buf, const z_commit_decl_t* m) { 
  z_iobuf_write(buf, m->header);
  z_iobuf_write(buf, m->cid);
}

z_result_decl_result_t 
z_result_decl_decode(z_iobuf_t* buf, uint8_t header) { 
  z_result_decl_result_t r;
  r.tag = Z_RES_DECL_TAG;
  r.value.result_decl.header = header;
  r.value.result_decl.cid = z_iobuf_read(buf);
  r.value.result_decl.status = z_iobuf_read(buf);
  return r;
}

void 
z_declare_encode(z_iobuf_t* buf, const z_declare_t* m) { 
  z_iobuf_write(buf, m->header);
  z_vle_encode(buf, m->sn);
  unsigned int len = z_vec_length(&m->declarations);
  int i;
  for (i = 0; i < len; ++i) {
    z_message_t* d = (z_message_t*)z_vec_get(&m->declarations, i);
    uint8_t did = Z_MID(m->header);
    switch (did) {
      case Z_RESOURCE_DECL: 
        z_res_decl_encode(buf, (z_res_decl_t*)d);
        break;
      case Z_PUBLISHER_DECL:
        z_pub_decl_encode(buf, (z_pub_decl_t*)d);
        break;
      case Z_SUBSCRIBER_DECL:
        z_sub_decl_encode(buf, (z_sub_decl_t*)d);
        break;
      case Z_STORAGE_DECL:
        z_storage_decl_encode(buf, (z_storage_decl_t*)d);
        break;
      case Z_COMMIT_DECL:
        z_commit_decl_encode(buf, (z_commit_decl_t*)d);
        break;
      default:
        printf("WARNING: Trying to encode unknown declaration!\n");
        break;
    }
  }

}


z_declare_result_t
z_declare_decode(z_iobuf_t* buf, uint8_t header) { 
  z_declare_result_t r;
  
  z_vle_result_t r_sn = z_vle_decode(buf);
  ASSURE_RESULT(r_sn, r, Z_VLE_PARSE_ERROR)

  z_vle_result_t r_dlen = z_vle_decode(buf);
  ASSURE_RESULT(r_dlen, r, Z_VLE_PARSE_ERROR)
  z_vec_t vec = z_vec_make(r_dlen.value.vle);
  int i;
  for (i = 0; i < r_dlen.value.vle; ++i) {
    uint8_t h = z_iobuf_read(buf);
    switch (Z_MID(h)) {
       case Z_RESOURCE_DECL: 
        
        z_res_decl_encode(buf, (z_res_decl_t*)d);
        break;
      case Z_PUBLISHER_DECL:
        z_pub_decl_encode(buf, (z_pub_decl_t*)d);
        break;
      case Z_SUBSCRIBER_DECL:
        z_sub_decl_encode(buf, (z_sub_decl_t*)d);
        break;
      case Z_STORAGE_DECL:
        z_storage_decl_encode(buf, (z_storage_decl_t*)d);
        break;
      case Z_COMMIT_DECL:
        z_commit_decl_encode(buf, (z_commit_decl_t*)d);
        break;
      default:
        printf("WARNING: Trying to encode unknown declaration!\n");
        break;
    }
    }
  }
  unsigned int len = z_vec_length(&m->declarations);
  int i;
  for (i = 0; i < len; ++i) {
    z_message_t* d = (z_message_t*)z_vec_get(&m->declarations, i);
    uint8_t did = Z_MID(m->header);
    switch (did) {
      case Z_RESOURCE_DECL: 
        z_res_decl_encode(buf, (z_res_decl_t*)d);
        break;
      case Z_PUBLISHER_DECL:
        z_pub_decl_encode(buf, (z_pub_decl_t*)d);
        break;
      case Z_SUBSCRIBER_DECL:
        z_sub_decl_encode(buf, (z_sub_decl_t*)d);
        break;
      case Z_STORAGE_DECL:
        z_storage_decl_encode(buf, (z_storage_decl_t*)d);
        break;
      case Z_COMMIT_DECL:
        z_commit_decl_encode(buf, (z_commit_decl_t*)d);
        break;
      default:
        printf("WARNING: Trying to encode unknown declaration!\n");
        break;
    }
  }

}

void 
z_stream_data_encode(z_iobuf_t* buf, const z_stream_data_t* m) {
  z_iobuf_write(buf, m->header);
  z_vle_encode(buf, m->sn);
  z_vle_encode(buf, m->rid);
  z_array_uint8_encode(buf, &m->payload);
}

z_stream_data_result_t 
z_stream_data_decode(z_iobuf_t* buf, uint8_t header) {
  z_stream_data_result_t r;
  r.tag = Z_STREAM_DATA_TAG;
  r.value.stream_data.header = header;
  z_vle_result_t r_vle = z_vle_decode(buf);
  ASSURE_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  
  r.value.stream_data.sn = r_vle.value.vle;
  r_vle = z_vle_decode(buf);
  ASSURE_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  
  r.value.stream_data.rid = r_vle.value.vle;  
  z_array_uint8_result_t r_au8 = z_array_uint8_decode(buf);
  ASSURE_RESULT(r_au8, r, Z_ARRAY_PARSE_ERROR)
  return r;
}


void 
encode_message(z_iobuf_t* buf, const z_message_t* m) {
  uint8_t mid = Z_MID(m->header);
  switch (mid) {
    case Z_STREAM_DATA:
      z_stream_data_encode(buf, (z_stream_data_t*)m);
      break;
    case Z_OPEN:
      z_open_encode(buf, (z_open_t*)m);
      break;
    case Z_CLOSE:
      z_close_encode(buf, (z_close_t*)m);
      break;
    case Z_DECLARE:
      z_declare_encode(buf, (z_declare_t*)m);
      break;
    default:
      printf("WARNING: Trying to encode message with unknown ID(%d)", mid); 
  }
}

z_message_result_t
decode_message(z_iobuf_t* buf) {
  z_message_result_t r; 
  uint8_t h = z_iobuf_read(buf);
  switch (Z_MID(h)) {
    case Z_STREAM_DATA:
      r.tag = Z_STREAM_DATA_TAG;
      r.value.stream_data = 
        z_stream_data_decode(buf, h);
      break;
    case Z_ACCEPT:
      r.tag = Z_ACCEPT_TAG;
      r.value.accept = z_accept_decode(buf, h);
      break;
    case Z_CLOSE:
      r.tag = Z_CLOSE_TAG;
      r.value.close = z_close_decode(buf, h);
      break;
    case Z_DECLARE:
      r.tag = Z_DECLARE_TAG;
      r.value.declare = z_declare_decode(buf, h);
      break;
    default:
      printf("WARNING: Trying to encode message with unknown ID(%d)", mid); 
  }
  return r;
}