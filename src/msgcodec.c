#include "zenoh/private/msgcodec.h"
#include "zenoh/codec.h"
#include "zenoh/private/logging.h"
#include <stdio.h>

void _z_payload_encode(z_iobuf_t *buf, const z_iobuf_t *bs) {
  z_iobuf_write_slice(buf, bs->buf, bs->r_pos, bs->w_pos);
}

z_iobuf_t _z_payload_decode(z_iobuf_t *buf) {
  z_vle_t len = z_iobuf_readable(buf);
  uint8_t *bs = z_iobuf_read_n(buf, len);
  z_iobuf_t iob = z_iobuf_wrap_wo(bs, len, 0, len);
  return iob;
}

void 
_z_open_encode(z_iobuf_t* buf, const _z_open_t* m) {
  z_iobuf_write(buf, m->version);  
  z_array_uint8_encode(buf, &(m->pid));
  z_vle_encode(buf, m->lease);
  z_vle_encode(buf, 0); // no locators
  // TODO: Encode properties if present
}

void
_z_accept_decode_na(z_iobuf_t* buf, _z_accept_result_t *r) {  

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

_z_accept_result_t 
_z_accept_decode(z_iobuf_t* buf) {
  _z_accept_result_t r;
  _z_accept_decode_na(buf, &r);
  return r;
}

void 
_z_close_encode(z_iobuf_t* buf, const _z_close_t* m) { 
  z_array_uint8_encode(buf, &(m->pid));
  z_iobuf_write(buf, m->reason);
}

void 
_z_close_decode_na(z_iobuf_t* buf, _z_close_result_t *r) {   
  r->tag = Z_OK_TAG;
  z_array_uint8_result_t ar =  z_array_uint8_decode(buf);
  ASSURE_P_RESULT(ar, r, Z_ARRAY_PARSE_ERROR)
  r->value.close.pid = ar.value.array_uint8;
  r->value.close.reason = z_iobuf_read(buf);  
}

_z_close_result_t 
_z_close_decode(z_iobuf_t* buf) { 
  _z_close_result_t r;
  _z_close_decode_na(buf, &r); 
  return r;
}

void 
_z_sub_mode_encode(z_iobuf_t* buf, const z_sub_mode_t* m) {
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
_z_sub_mode_decode_na(z_iobuf_t* buf, z_sub_mode_result_t *r) {  
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
_z_sub_mode_decode(z_iobuf_t* buf) {
  z_sub_mode_result_t r;
  _z_sub_mode_decode_na(buf, &r);
  return r;
}

void 
_z_declaration_encode(z_iobuf_t *buf, _z_declaration_t *d) {
  z_iobuf_write(buf, d->header);  
  switch (_Z_MID(d->header)) {
    case _Z_RESOURCE_DECL:      
      z_vle_encode(buf, d->payload.resource.rid);
      z_string_encode(buf, d->payload.resource.r_name);
      break;    
    case _Z_PUBLISHER_DECL:
      z_vle_encode(buf, d->payload.pub.rid);
      break;
    case _Z_STORAGE_DECL:
      z_vle_encode(buf, d->payload.storage.rid);
      break; 
    case _Z_EVAL_DECL:
      z_vle_encode(buf, d->payload.eval.rid);
      break;  
    case _Z_SUBSCRIBER_DECL:
      z_vle_encode(buf, d->payload.sub.rid);
      _z_sub_mode_encode(buf, &d->payload.sub.sub_mode);
      break;
    case _Z_FORGET_PUBLISHER_DECL:
      z_vle_encode(buf, d->payload.forget_pub.rid);
      break;
    case _Z_FORGET_STORAGE_DECL:
      z_vle_encode(buf, d->payload.forget_sto.rid);
      break;
    case _Z_FORGET_EVAL_DECL:
      z_vle_encode(buf, d->payload.forget_eval.rid);
      break;
    case _Z_FORGET_SUBSCRIBER_DECL:
      z_vle_encode(buf, d->payload.forget_sub.rid);
      break;
    case _Z_RESULT_DECL:  
      z_iobuf_write(buf, d->payload.result.cid);
      z_iobuf_write(buf, d->payload.result.status);
      break;
    case _Z_COMMIT_DECL:
      z_iobuf_write(buf, d->payload.commit.cid);
      break;
    default:      
      break;
  }  
}

void 
_z_declaration_decode_na(z_iobuf_t *buf, _z_declaration_result_t *r) {
  z_vle_result_t r_vle;
  z_string_result_t r_str;
  z_sub_mode_result_t r_sm;
  r->tag = Z_OK_TAG;
  r->value.declaration.header = z_iobuf_read(buf); 
  switch (_Z_MID(r->value.declaration.header)) {
    case _Z_RESOURCE_DECL:      
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
      r_str = z_string_decode(buf);
      ASSURE_P_RESULT(r_str, r, Z_STRING_PARSE_ERROR)  
      r->value.declaration.payload.resource.rid = r_vle.value.vle;      
      r->value.declaration.payload.resource.r_name = r_str.value.string;
      break;    
    case _Z_PUBLISHER_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)      
      r->value.declaration.payload.pub.rid = r_vle.value.vle;
      break;
    case _Z_STORAGE_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)      
      r->value.declaration.payload.storage.rid = r_vle.value.vle;
      break; 
    case _Z_EVAL_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)      
      r->value.declaration.payload.eval.rid = r_vle.value.vle;
      break;  
    case _Z_SUBSCRIBER_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
      r_sm = _z_sub_mode_decode(buf);
      ASSURE_P_RESULT(r_sm, r, Z_MESSAGE_PARSE_ERROR)
      r->value.declaration.payload.sub.rid = r_vle.value.vle;
      r->value.declaration.payload.sub.sub_mode = r_sm.value.sub_mode; 
      break;
    case _Z_FORGET_PUBLISHER_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
      r->value.declaration.payload.forget_pub.rid = r_vle.value.vle;
      break;
    case _Z_FORGET_STORAGE_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
      r->value.declaration.payload.forget_sto.rid = r_vle.value.vle;
      break;
    case _Z_FORGET_EVAL_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
      r->value.declaration.payload.forget_eval.rid = r_vle.value.vle;
      break;
    case _Z_FORGET_SUBSCRIBER_DECL:
      r_vle = z_vle_decode(buf);
      ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
      r->value.declaration.payload.forget_sub.rid = r_vle.value.vle;
      break;
    case _Z_RESULT_DECL:  
      r->value.declaration.payload.result.cid = z_iobuf_read(buf);
      r->value.declaration.payload.result.status = z_iobuf_read(buf);  
      break;
    case _Z_COMMIT_DECL:
      r->value.declaration.payload.commit.cid = z_iobuf_read(buf);
      break;
    default:
      r->tag = _Z_ERROR_TAG;
      r->value.error = Z_MESSAGE_PARSE_ERROR;
      return;
  }  
}
_z_declaration_result_t
_z_declaration_decode(z_iobuf_t *buf) {
  _z_declaration_result_t r;
  _z_declaration_decode_na(buf, &r);
  return r;
}
void 
_z_declare_encode(z_iobuf_t* buf, const _z_declare_t* m) { 
  z_vle_encode(buf, m->sn);
  unsigned int len = m->declarations.length;
  z_vle_encode(buf, len);  
  for (unsigned int i = 0; i < len; ++i) {    
    _z_declaration_encode(buf, &m->declarations.elem[i]);
  }
}

void
_z_declare_decode_na(z_iobuf_t* buf, _z_declare_result_t *r) { 
  _z_declaration_result_t *r_decl;
  z_vle_result_t r_sn = z_vle_decode(buf);
  r->tag = Z_OK_TAG;
  ASSURE_P_RESULT(r_sn, r, Z_VLE_PARSE_ERROR)

  z_vle_result_t r_dlen = z_vle_decode(buf);
  ASSURE_P_RESULT(r_dlen, r, Z_VLE_PARSE_ERROR)
  size_t len = r_dlen.value.vle;
  r->value.declare.declarations.length = len;
  r->value.declare.declarations.elem = (_z_declaration_t*)malloc(sizeof(_z_declaration_t)*len);  
  
  r_decl = (_z_declaration_result_t*)malloc(sizeof(_z_declaration_result_t));
  for (unsigned int i = 0; i < len; ++i) {    
    _z_declaration_decode_na(buf, r_decl);
    if (r_decl->tag != _Z_ERROR_TAG) 
      r->value.declare.declarations.elem[i] = r_decl->value.declaration; 
    else {
      r->value.declare.declarations.length = 0;
      free(r->value.declare.declarations.elem);        
      free(r_decl);
      r->tag = _Z_ERROR_TAG;
      r->value.error = Z_MESSAGE_PARSE_ERROR;
      return;
    }
  }
  free(r_decl);
}

_z_declare_result_t
_z_declare_decode(z_iobuf_t* buf) { 
  _z_declare_result_t r;
  _z_declare_decode_na(buf, &r);
  return r;
}

void 
_z_compact_data_encode(z_iobuf_t* buf, const _z_compact_data_t* m) {
  z_vle_encode(buf, m->sn);
  z_vle_encode(buf, m->rid);
  _z_payload_encode(buf, &m->payload);
}

void
_z_compact_data_decode_na(z_iobuf_t* buf, _z_compact_data_result_t *r) {  
  r->tag = Z_OK_TAG;
  z_vle_result_t r_vle = z_vle_decode(buf);
  ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  
  r->value.compact_data.sn = r_vle.value.vle;
  r_vle = z_vle_decode(buf);
  ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  r->value.compact_data.rid = r_vle.value.vle;    
  r->value.compact_data.payload = _z_payload_decode(buf);
}

_z_compact_data_result_t 
_z_compact_data_decode(z_iobuf_t* buf) {
  _z_compact_data_result_t r;
  _z_compact_data_decode_na(buf, &r);
  return r;
}

void _z_payload_header_encode(z_iobuf_t *buf, const _z_payload_header_t *ph) {
  uint8_t flags = ph->flags;
  _Z_DEBUG_VA("z_payload_header_encode flags = 0x%x\n", flags);
  z_iobuf_write(buf, flags);
  if (flags & _Z_SRC_ID) {
    _Z_DEBUG("Encoding _Z_SRC_ID\n");
    z_iobuf_write_slice(buf, (uint8_t*)ph->src_id, 0, 16);
  }
  if (flags & _Z_SRC_SN) {
    _Z_DEBUG("Encoding _Z_SRC_SN\n");
    z_vle_encode(buf, ph->src_sn);    
  }
  if (flags & _Z_BRK_ID) {
    _Z_DEBUG("Encoding _Z_BRK_ID\n");
    z_iobuf_write_slice(buf, (uint8_t*)ph->brk_id, 0, 16);
  }
  if (flags & _Z_BRK_SN) {
    _Z_DEBUG("Encoding _Z_BRK_SN\n");
    z_vle_encode(buf, ph->brk_sn);
  }
  if (flags & _Z_KIND) {
    _Z_DEBUG("Encoding _Z_KIND\n");
    z_vle_encode(buf, ph->kind);
  }    
  if (flags & _Z_ENCODING) {
    _Z_DEBUG("Encoding _Z_ENCODING\n");
    z_vle_encode(buf, ph->encoding);
  }

  _z_payload_encode(buf, &ph->payload);
}

void 
_z_payload_header_decode_na(z_iobuf_t *buf, _z_payload_header_result_t *r) {  
  z_vle_result_t r_vle;
  uint8_t flags = z_iobuf_read(buf);
  _Z_DEBUG_VA("Payload header flags: 0x%x\n", flags);

  if (flags & _Z_SRC_ID) {
    _Z_DEBUG("Decoding _Z_SRC_ID\n");
    z_iobuf_read_to_n(buf, r->value.payload_header.src_id, 16);
  }

  if (flags & _Z_T_STAMP) {          
    _Z_DEBUG("Decoding _Z_T_STAMP\n");
    r_vle = z_vle_decode(buf);        
    ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);
    r->value.payload_header.tstamp.time = r_vle.value.vle;
    memcpy(r->value.payload_header.tstamp.clock_id, buf->buf + buf->r_pos, 16);
    buf->r_pos += 16;
  }
    
  if (flags & _Z_SRC_SN) { 
    _Z_DEBUG("Decoding _Z_SRC_SN\n");
    r_vle = z_vle_decode(buf);
    ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);
    r->value.payload_header.src_sn = r_vle.value.vle;
  }

  if (flags & _Z_BRK_ID) {
    _Z_DEBUG("Decoding _Z_BRK_ID\n");
    z_iobuf_read_to_n(buf, r->value.payload_header.brk_id, 16);
  }

  if (flags & _Z_BRK_SN) {
    _Z_DEBUG("Decoding _Z_BRK_SN\n");
    r_vle = z_vle_decode(buf);
    ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);
    r->value.payload_header.brk_sn = r_vle.value.vle;
  }

  if (flags & _Z_KIND) {
    _Z_DEBUG("Decoding _Z_KIND\n");
    r_vle = z_vle_decode(buf);
    ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);
    r->value.payload_header.kind = r_vle.value.vle;
  }

  if (flags & _Z_ENCODING) {
    _Z_DEBUG("Decoding _Z_ENCODING\n");
    r_vle = z_vle_decode(buf);
    ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);
    r->value.payload_header.encoding = r_vle.value.vle;
    _Z_DEBUG("Done Decoding _Z_ENCODING\n");
  }
  
  _Z_DEBUG("Decoding payload\n");
  r->value.payload_header.flags = flags;
  r->value.payload_header.payload = _z_payload_decode(buf);
  r->tag = Z_OK_TAG;
}

_z_payload_header_result_t 
_z_payload_header_decode(z_iobuf_t *buf) {  
  _z_payload_header_result_t r;
  _z_payload_header_decode_na(buf, &r);
  return r;
}

void 
_z_stream_data_encode(z_iobuf_t *buf, const _z_stream_data_t* m) {
  z_vle_encode(buf, m->sn);
  z_vle_encode(buf, m->rid);
  z_vle_t len = z_iobuf_readable(&m->payload_header);
  z_vle_encode(buf, len);  
  z_iobuf_write_slice(buf, m->payload_header.buf, m->payload_header.r_pos, m->payload_header.w_pos);
}

void _z_stream_data_decode_na(z_iobuf_t *buf, _z_stream_data_result_t *r) {
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

_z_stream_data_result_t
_z_stream_data_decode(z_iobuf_t *buf) {
  _z_stream_data_result_t r;
  _z_stream_data_decode_na(buf, &r);
  return r;
}

void
_z_write_data_encode(z_iobuf_t *buf, const _z_write_data_t* m) {
  z_vle_encode(buf, m->sn);
  z_string_encode(buf, m->rname);
  z_vle_t len = z_iobuf_readable(&m->payload_header);
  z_vle_encode(buf, len);  
  z_iobuf_write_slice(buf, m->payload_header.buf, m->payload_header.r_pos, m->payload_header.w_pos);    
}


void _z_write_data_decode_na(z_iobuf_t *buf, _z_write_data_result_t *r) {
  r->tag = Z_OK_TAG;
  z_string_result_t r_str;
  z_vle_result_t r_vle = z_vle_decode(buf);
  ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);
  r->value.write_data.sn = r_vle.value.vle;

  r_str = z_string_decode(buf);
  ASSURE_P_RESULT(r_str, r, Z_STRING_PARSE_ERROR);
  r->value.write_data.rname = r_str.value.string;
  _Z_DEBUG_VA("Decoding write data for resource %s\n", r_str.value.string);
  r_vle = z_vle_decode(buf);
  ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR);  
  uint8_t *ph = z_iobuf_read_n(buf, r_vle.value.vle);
  r->value.write_data.payload_header = z_iobuf_wrap_wo(ph, r_vle.value.vle, 0, r_vle.value.vle);
  r->value.write_data.payload_header.w_pos = r_vle.value.vle;
}

_z_write_data_result_t
_z_write_data_decode(z_iobuf_t *buf) {
  _z_write_data_result_t r;
  _z_write_data_decode_na(buf, &r);
  return r;
}

void
_z_pull_encode(z_iobuf_t *buf, const _z_pull_t* m) {
  z_vle_encode(buf, m->sn);
  z_vle_encode(buf, m->id);
}

void
_z_query_encode(z_iobuf_t *buf, const _z_query_t* m) {
  z_array_uint8_encode(buf, &(m->pid));
  z_vle_encode(buf, m->qid);
  z_string_encode(buf, m->rname);
  z_string_encode(buf, m->predicate);
}

void _z_query_decode_na(z_iobuf_t *buf, _z_query_result_t *r) {
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
}

_z_query_result_t
_z_query_decode(z_iobuf_t *buf) {
  _z_query_result_t r;
  _z_query_decode_na(buf, &r);
  return r;
}

void
_z_reply_encode(z_iobuf_t *buf, const _z_reply_t* m, uint8_t header) {
  z_array_uint8_encode(buf, &(m->qpid));
  z_vle_encode(buf, m->qid);

  if(header & _Z_F_FLAG) {
    z_array_uint8_encode(buf, &(m->stoid));
    z_vle_encode(buf, m->rsn);
    z_string_encode(buf, m->rname);
    z_vle_t len = z_iobuf_readable(&m->payload_header);
    z_vle_encode(buf, len);  
    z_iobuf_write_slice(buf, m->payload_header.buf, m->payload_header.r_pos, m->payload_header.w_pos); 
  }
}

void _z_reply_decode_na(z_iobuf_t *buf, uint8_t header, _z_reply_result_t *r) {
  r->tag = Z_OK_TAG;

  z_array_uint8_result_t r_qpid = z_array_uint8_decode(buf);
  ASSURE_P_RESULT(r_qpid, r, Z_ARRAY_PARSE_ERROR)
  r->value.reply.qpid = r_qpid.value.array_uint8;

  z_vle_result_t r_qid = z_vle_decode(buf);
  ASSURE_P_RESULT(r_qid, r, Z_VLE_PARSE_ERROR)
  r->value.reply.qid = r_qid.value.vle;

  if (header & _Z_F_FLAG)
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

_z_reply_result_t 
_z_reply_decode(z_iobuf_t *buf, uint8_t header) {  
  _z_reply_result_t r;
  _z_reply_decode_na(buf, header, &r);
  return r;
}

void 
_z_message_encode(z_iobuf_t* buf, const _z_message_t* m) {
  z_iobuf_write(buf, m->header);
  uint8_t mid = _Z_MID(m->header);
  switch (mid) {
    case _Z_COMPACT_DATA:
      _z_compact_data_encode(buf, &m->payload.compact_data);
      break;    
    case _Z_STREAM_DATA:
      _z_stream_data_encode(buf, &m->payload.stream_data); 
      break;
    case _Z_WRITE_DATA:
      _z_write_data_encode(buf, &m->payload.write_data);
      break;
    case _Z_PULL:
      _z_pull_encode(buf, &m->payload.pull);
      break;
    case _Z_QUERY:
      _z_query_encode(buf, &m->payload.query);
      break;
    case _Z_REPLY:
      _z_reply_encode(buf, &m->payload.reply, m->header);
      break;
    case _Z_OPEN:
      _z_open_encode(buf, &m->payload.open);
      if (m->header & _Z_P_FLAG ) {
        z_properties_encode(buf, m->properties);
      }
      break;
    case _Z_CLOSE:
      _z_close_encode(buf, &m->payload.close);
      break;
    case _Z_DECLARE:
      _z_declare_encode(buf, &m->payload.declare);
      break;
    default:
      _Z_ERROR("WARNING: Trying to encode message with unknown ID(%d)\n", mid); 
      return;
  }
}

void
_z_message_decode_na(z_iobuf_t* buf, _z_message_p_result_t* r) {
  _z_compact_data_result_t r_cd;  
  _z_stream_data_result_t r_sd;
  _z_write_data_result_t r_wd;
  _z_query_result_t r_q;
  _z_reply_result_t r_r;
  _z_accept_result_t r_a;
  _z_close_result_t r_c;
  _z_declare_result_t r_d;

  uint8_t h = z_iobuf_read(buf);
  r->tag = Z_OK_TAG;
  r->value.message->header = h;
  
  uint8_t mid = _Z_MID(h);
  switch (mid) {
    case _Z_COMPACT_DATA:
      r->tag = Z_OK_TAG;
      r_cd = _z_compact_data_decode(buf);
      ASSURE_P_RESULT(r_cd, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.compact_data = r_cd.value.compact_data;
      break;
    case _Z_STREAM_DATA:
      r->tag = Z_OK_TAG;
      r_sd = _z_stream_data_decode(buf);
      ASSURE_P_RESULT(r_sd, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.stream_data = r_sd.value.stream_data;
      break;
    case _Z_WRITE_DATA:
      r->tag = Z_OK_TAG;
      r_wd = _z_write_data_decode(buf);
      ASSURE_P_RESULT(r_wd, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.write_data = r_wd.value.write_data;
      break;  
    case _Z_QUERY:
      r->tag = Z_OK_TAG;
      r_q = _z_query_decode(buf);
      ASSURE_P_RESULT(r_q, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.query = r_q.value.query;
      break;  
    case _Z_REPLY:      
      r->tag = Z_OK_TAG;
      r_r = _z_reply_decode(buf, h);
      ASSURE_P_RESULT(r_r, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.reply = r_r.value.reply;      
      break;  
    case _Z_ACCEPT:
      r->tag = Z_OK_TAG;
      r_a = _z_accept_decode(buf);
      ASSURE_P_RESULT(r_a, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.accept = r_a.value.accept;
      break;
    case _Z_CLOSE:
      r->tag = Z_OK_TAG;
      r_c = _z_close_decode(buf);
      ASSURE_P_RESULT(r_c, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.close = r_c.value.close;
      break;
    case _Z_DECLARE:
      r->tag = Z_OK_TAG;
      r_d = _z_declare_decode(buf);
      ASSURE_P_RESULT(r_d, r, Z_MESSAGE_PARSE_ERROR)
      r->value.message->payload.declare = r_d.value.declare;
      break;
    default:
      r->tag = _Z_ERROR_TAG;
      r->value.error = Z_MESSAGE_PARSE_ERROR;
      _Z_ERROR("WARNING: Trying to decode message with unknown ID(%d)\n", mid); 

  }
}

_z_message_p_result_t
_z_message_decode(z_iobuf_t* buf) {
  _z_message_p_result_t r;
  _z_message_p_result_init(&r); 
  _z_message_decode_na(buf, &r);
  return r;
}
