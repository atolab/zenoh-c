#include "zenoh/types.h"
#include <assert.h>
#include <stdio.h>


int count_occurences(const char *src, const char *tok) {
  char *s = strdup(src);
  char *token = strtok(s, tok);
  int count = 0;
  int len = strlen(src);
  while (token != 0 && strlen(token) < len) {
    count += 1;
    token = strtok(NULL, tok);
  }
  return count;
}

char *replace_with(const char *src, const char *match, const char *rep) {
  int n = count_occurences(src, match);
  int a = strlen(match);
  int b = strlen(rep);
  int len = strlen(src);

  len = len + (n * b) - (n * a);
  char *r = (char *)malloc(len);
  r[0] = '\0';
  char *s = strdup(src);
  char *token = strtok(s, match);

  while (token != NULL) {
    strcat(r, token);
    strcat(r, rep);
    token = strtok(NULL, match);
  }
  free(s);
  return r;
}

char *resource_to_regex(const char *res) {
  char *s_star = "*";    
  char star = '*';
  char *re_star = "([a-zA-Z0-9_-]+)";
  char *re_2star = "([a-zA-Z0-9_-]+(/[a-zA-Z0-9_-])*)";

  int n = count_occurences(res, s_star);  
  int k = strlen(re_2star);  
  int len = strlen(res);

  int nlen = len + (n * (k-1));
  char *r = (char *)malloc(nlen);

  r[0] = '\0';
  char *s = strdup(res);
  char *token = strtok(s, s_star);

  int idx = 0;
  strcat(r, "^(");
  while (token != NULL) {
    idx += strlen(token)+1;
    strcat(r, token);    
    if (idx < len) {
      if (res[idx] == star) {
        idx += 1;
        strcat(r, re_2star);
      } else{        
        strcat(r, re_star);      
      }    
    } else if (idx == len) {
      strcat(r, re_star);
    }
    token = strtok(NULL, s_star);
  }
  strcat(r, ")$");
  return r;
}

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

void z_register_res_decl(zenoh_t *z, z_vle_t rid, const char *rname) {
  z_res_decl_t *rdecl = (z_res_decl_t *) malloc(sizeof(z_res_decl_t));
  rdecl->rid = rid;
  rdecl->r_name = strdup(rname);  
  char *regex = resource_to_regex(rname);
  Z_DEBUG_VA(">>> Registering declaration %s as regex %s", rname, regex);
  regcomp(&rdecl->re, regex, REG_ICASE | REG_EXTENDED);
  free(regex);
  z->declarations = z_list_cons(z->declarations, rdecl);
}

z_res_decl_t *z_get_res_decl_by_rid(zenoh_t *z, z_vle_t rid) {
  if (z->declarations == 0) {
    printf(">>> Empty declaration set");
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
z_res_decl_t *z_get_res_decl_by_rname(zenoh_t *z, const char *rname) {
  if (z->declarations == 0) {
    printf(">>> Empty declarations set");
    return 0;
  }
  else {
    z_res_decl_t *decl = (z_res_decl_t *)z_list_head(z->subscriptions);
    z_list_t *decls = z_list_tail(z->declarations);

    while (decls != 0 && regexec(&decl->re, rname, 0, NULL, 0) != 0) {      
      decls = z_list_head(decls);
      decls = z_list_tail(decls);  
    }    
    if (regexec(&decl->re, rname, 0, NULL, 0) == 0) return decl;
    else return 0;
  }
}


void z_register_subscription(zenoh_t *z, z_vle_t rid, subscriber_callback_t *callback) {
  z_subscription_t *sub = (z_subscription_t *) malloc(sizeof(z_subscription_t));
  sub->rid = rid;
  z_res_decl_t *decl = z_get_res_decl_by_rid(z, rid);
  assert(decl != 0);
  sub->rname = strdup(decl->r_name);
  char *regex = resource_to_regex(decl->r_name);
  regcomp(&sub->re, regex, REG_ICASE | REG_EXTENDED);
  free(regex);
  sub->callback = callback;
  z->subscriptions = z_list_cons(z->subscriptions, sub);
}

z_subscription_t *z_get_subscription_by_rid(zenoh_t *z, z_vle_t rid) {
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

z_subscription_t *z_get_subscription_by_rname(zenoh_t *z, const char *rname) {
  if (z->subscriptions == 0) {
    printf(">>> Empty subscription set");
    return 0;
  }
  else {
    z_subscription_t *sub = (z_subscription_t *)z_list_head(z->subscriptions);
    z_list_t *subs = z_list_tail(z->subscriptions);
    while (subs != 0 && regexec(&sub->re, rname, 0, NULL, 0) != 0) {      
      sub = z_list_head(subs);
      subs = z_list_tail(subs);  
    }    
    if (regexec(&sub->re, rname, 0, NULL, 0) == 0) return sub;
    else return 0;
  }
}


void z_register_query(zenoh_t *z, z_vle_t qid, reply_callback_t *callback) {
  z_replywaiter_t *rw = (z_replywaiter_t *) malloc(sizeof(z_replywaiter_t));
  rw->qid = qid;
  rw->callback = callback;
  z->replywaiters = z_list_cons(z->replywaiters, rw);
}

z_replywaiter_t *z_get_query(zenoh_t *z, z_vle_t qid) {
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
