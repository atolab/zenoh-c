#include "zenoh/types.h"
#include "zenoh/rname.h"
#include <assert.h>
#include <stdio.h>


int count_occurences(const char *src, const char *tok) {
  char *s, *tofree, *token;
  s = tofree = strdup(src);
  
  int count = 0;  
  while ((token = strsep(&s, tok)) != NULL) 
    count += 1;      
  free(tofree);
  return count;
}

char *replace_with(const char *src, const char *match, const char *rep) {
  int n = count_occurences(src, match);
  if (n == 0) 
    return strdup(src);

  int a = strlen(match);
  int b = strlen(rep);
  int len = strlen(src);

  len = len + (n * b) - (n * a) +1;
  char *r = (char *)malloc(len);
  r[0] = '\0';
  char *s, *tofree, *token;
  tofree = s = strdup(src);  

  while ((token = strsep(&s, match)) != 0) {
    strcat(r, token);
    strcat(r, rep);    
  }
  free(tofree);
  return r;
}

z_iobuf_t z_iobuf_wrap_wo(unsigned char *buf, unsigned int capacity, unsigned int rpos, unsigned int wpos) {
  assert(rpos <= capacity && wpos <= capacity);
  z_iobuf_t iobuf;
  iobuf.r_pos = rpos;
  iobuf.w_pos = wpos; 
  iobuf.capacity = capacity;
  iobuf.buf = buf;  
  return iobuf;
}

z_iobuf_t 
z_iobuf_wrap(uint8_t *buf, unsigned int capacity) {
  return z_iobuf_wrap_wo(buf, capacity, 0, 0);  
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

void z_iobuf_write_slice(z_iobuf_t* iob, const uint8_t* bs, unsigned int offset, unsigned int length) {
  assert(z_iobuf_writable(iob) >= length); 
  memcpy(iob->buf + iob->w_pos, bs + offset, length);
  iob->w_pos += length;
}

void z_iobuf_write_bytes(z_iobuf_t* iob, const unsigned char *bs, unsigned int length) {
  assert(z_iobuf_writable(iob) >= length); 
  memcpy(iob->buf + iob->w_pos, bs, length);
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

z_vle_t z_get_entity_id(z_zenoh_t *z) {
  return z->eid++;
}

z_vle_t z_get_resource_id(z_zenoh_t *z, const char *rname) {
  z_res_decl_t *rd_brn = z_get_res_decl_by_rname(z, rname);
  if (rd_brn == 0) {
    z_vle_t rid = z->rid++;
    while (z_get_res_decl_by_rid(z, rid) != 0) {
      rid++;
    }
    z->rid = rid;
    return rid;
  }
  else return rd_brn->rid;
}

int z_register_res_decl(z_zenoh_t *z, z_vle_t rid, const char *rname) {
  Z_DEBUG_VA(">>> Allocating res decl for (%zu,%s)\n", rid, rname);
  z_res_decl_t *rd_bid = z_get_res_decl_by_rid(z, rid);
  z_res_decl_t *rd_brn = z_get_res_decl_by_rname(z, rname);

  if (rd_bid == 0 && rd_brn == 0) {  
    z_res_decl_t *rdecl = (z_res_decl_t *) malloc(sizeof(z_res_decl_t));
    rdecl->rid = rid;
    rdecl->r_name = strdup(rname); 
    z->declarations = z_list_cons(z->declarations, rdecl);   
    return 0;
  } 
  else if (rd_bid == rd_brn) 
    return 0;
  else return 1;  
}

z_res_decl_t *z_get_res_decl_by_rid(z_zenoh_t *z, z_vle_t rid) {
  if (z->declarations == 0) {
    return 0;
  }
  else {
    z_res_decl_t *decl = (z_res_decl_t *)z_list_head(z->declarations);
    z_list_t *decls = z_list_tail(z->declarations);
    while (decls != 0 && decl->rid != rid) {      
      decl = z_list_head(decls);
      decls = z_list_tail(decls);  
    }    
    if (decl->rid == rid) return decl;
    else return 0;
  }
}

z_res_decl_t *z_get_res_decl_by_rname(z_zenoh_t *z, const char *rname) {
  if (z->declarations == 0) {
    return 0;
  } else {
    z_res_decl_t *decl = (z_res_decl_t *)z_list_head(z->subscriptions);
    z_list_t *decls = z_list_tail(z->declarations);

    while (decls != 0 && strcmp(decl->r_name, rname) != 0) {      
      decls = z_list_head(decls);
      decls = z_list_tail(decls);  
    }    
    if (strcmp(decl->r_name, rname) == 0) return decl;
    else return 0;
  }
}


void z_register_subscription(z_zenoh_t *z, z_vle_t rid, subscriber_callback_t *callback) {
  z_subscription_t *sub = (z_subscription_t *) malloc(sizeof(z_subscription_t));
  sub->rid = rid;
  z_res_decl_t *decl = z_get_res_decl_by_rid(z, rid);
  assert(decl != 0);
  sub->rname = strdup(decl->r_name);
  sub->callback = callback;
  z->subscriptions = z_list_cons(z->subscriptions, sub);
}

const char *z_get_resource_name(z_zenoh_t *z, z_vle_t rid) {
  z_list_t *ds = z->declarations;
  z_res_decl_t *d;
  while (ds != z_list_empty) {
    d = z_list_head(ds);
    if (d->rid == rid) {
      return d->r_name;
    }
    ds = z_list_tail(ds);
  }
  return 0;
}
z_list_t *z_get_subscriptions_by_rid(z_zenoh_t *z, z_vle_t rid) {
  if (z->subscriptions == z_list_empty) {
    return z_list_empty;
  }
  else {
    z_list_t *subs = z->subscriptions;
    z_list_t *msubs = z_list_empty;
    z_subscription_t *sub;
    
    while (subs != z_list_empty) {      
      sub = z_list_head(subs);
      if (sub->rid == rid) {
        z_list_cons(msubs, sub);
      }
      subs = z_list_tail(subs);  
    }    
    return msubs;    
  }
}

z_list_t *
z_get_subscriptions_by_rname(z_zenoh_t *z, const char *rname) {
  z_list_t *subs = z_list_empty;
  if (z->subscriptions == 0) {
    return subs;
  }  
  else {
    z_subscription_t *sub = 0;
    z_list_t *subs = z->subscriptions;
    z_list_t *xs = z_list_empty;
    do {      
      sub = (z_subscription_t *)z_list_head(subs);
      subs = z_list_tail(subs);            
      if (intersect(sub->rname, (char *)rname)) {        
        xs = z_list_cons(xs, sub);
      }       
    } while (subs != 0);          
    return xs;
  }
}

int z_matching_remote_sub(z_zenoh_t *z, z_vle_t rid) {
  return z_i_map_get(z->remote_subs, rid) != 0 ? 1 : 0;   
}

void z_register_query(z_zenoh_t *z, z_vle_t qid, z_reply_callback_t *callback) {
  z_replywaiter_t *rw = (z_replywaiter_t *) malloc(sizeof(z_replywaiter_t));
  rw->qid = qid;
  rw->callback = callback;
  z->replywaiters = z_list_cons(z->replywaiters, rw);
}

z_replywaiter_t *z_get_query(z_zenoh_t *z, z_vle_t qid) {
  if (z->replywaiters == 0) {
    printf(">>> No reply waiters");
    return 0;
  }
  else {
    z_replywaiter_t *rw = (z_replywaiter_t *)z_list_head(z->replywaiters);
    z_list_t *rws = z_list_tail(z->replywaiters);
    while (rws != 0 && rw->qid != qid) {      
      rw = z_list_head(rws);
      rws = z_list_tail(rws);  
    }    
    if (rw->qid == qid) return rw;
    else return 0;
  }
}
