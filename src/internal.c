#include <assert.h>
#include "zenoh/rname.h"
#include "zenoh/private/internal.h"
#include "zenoh/private/logging.h"

z_vle_t _z_get_entity_id(z_zenoh_t *z) {
  return z->eid++;
}

z_vle_t _z_get_resource_id(z_zenoh_t *z, const char *rname) {
  _z_res_decl_t *rd_brn = _z_get_res_decl_by_rname(z, rname);
  if (rd_brn == 0) {
    z_vle_t rid = z->rid++;
    while (_z_get_res_decl_by_rid(z, rid) != 0) {
      rid++;
    }
    z->rid = rid;
    return rid;
  }
  else return rd_brn->rid;
}

int _z_register_res_decl(z_zenoh_t *z, z_vle_t rid, const char *rname) {
  _Z_DEBUG_VA(">>> Allocating res decl for (%zu,%s)\n", rid, rname);
  _z_res_decl_t *rd_bid = _z_get_res_decl_by_rid(z, rid);
  _z_res_decl_t *rd_brn = _z_get_res_decl_by_rname(z, rname);

  if (rd_bid == 0 && rd_brn == 0) {  
    _z_res_decl_t *rdecl = (_z_res_decl_t *) malloc(sizeof(_z_res_decl_t));
    rdecl->rid = rid;
    rdecl->r_name = strdup(rname); 
    z->declarations = z_list_cons(z->declarations, rdecl);   
    return 0;
  } 
  else if (rd_bid == rd_brn) 
    return 0;
  else return 1;  
}

_z_res_decl_t *_z_get_res_decl_by_rid(z_zenoh_t *z, z_vle_t rid) {
  if (z->declarations == 0) {
    return 0;
  }
  else {
    _z_res_decl_t *decl = (_z_res_decl_t *)z_list_head(z->declarations);
    z_list_t *decls = z_list_tail(z->declarations);
    while (decls != 0 && decl->rid != rid) {      
      decl = z_list_head(decls);
      decls = z_list_tail(decls);  
    }    
    if (decl->rid == rid) return decl;
    else return 0;
  }
}

_z_res_decl_t *_z_get_res_decl_by_rname(z_zenoh_t *z, const char *rname) {
  if (z->declarations == 0) {
    return 0;
  } else {
    _z_res_decl_t *decl = (_z_res_decl_t *)z_list_head(z->declarations);
    z_list_t *decls = z_list_tail(z->declarations);

    while (decls != 0 && strcmp(decl->r_name, rname) != 0) {      
      decl = z_list_head(decls);
      decls = z_list_tail(decls);  
    }    
    if (strcmp(decl->r_name, rname) == 0) return decl;
    else return 0;
  }
}

void _z_register_subscription(z_zenoh_t *z, z_vle_t rid, z_vle_t id, z_data_handler_t data_handler, void *arg) {
  _z_sub_t *sub = (_z_sub_t *) malloc(sizeof(_z_sub_t));
  sub->rid = rid;
  sub->id = id;
  _z_res_decl_t *decl = _z_get_res_decl_by_rid(z, rid);
  assert(decl != 0);
  sub->rname = strdup(decl->r_name);
  sub->data_handler = data_handler;
  sub->arg = arg;
  z->subscriptions = z_list_cons(z->subscriptions, sub);
}

int sub_predicate(void *elem, void *arg) {
  z_sub_t *s = (z_sub_t *)arg;
  _z_sub_t *sub = (_z_sub_t *)elem;
  if(sub->id == s->id) {
    return 1;
  } else {
    return 0;
  }
}

void _z_unregister_subscription(z_sub_t *s) {
  s->z->subscriptions = z_list_remove(s->z->subscriptions, sub_predicate, s);
}

const char *_z_get_resource_name(z_zenoh_t *z, z_vle_t rid) {
  z_list_t *ds = z->declarations;
  _z_res_decl_t *d;
  while (ds != z_list_empty) {
    d = z_list_head(ds);
    if (d->rid == rid) {
      return d->r_name;
    }
    ds = z_list_tail(ds);
  }
  return 0;
}

z_list_t *
_z_get_subscriptions_by_rid(z_zenoh_t *z, z_vle_t rid) {
  z_list_t *subs = z_list_empty;
  if (z->subscriptions == 0) {
    return subs;
  }  
  else {
    _z_sub_t *sub = 0;
    z_list_t *subs = z->subscriptions;
    z_list_t *xs = z_list_empty;
    do {      
      sub = (_z_sub_t *)z_list_head(subs);
      subs = z_list_tail(subs);            
      if (sub->rid == rid) {        
        xs = z_list_cons(xs, sub);
      }       
    } while (subs != 0);          
    return xs;
  }
}

z_list_t *
_z_get_subscriptions_by_rname(z_zenoh_t *z, const char *rname) {
  z_list_t *subs = z_list_empty;
  if (z->subscriptions == 0) {
    return subs;
  }  
  else {
    _z_sub_t *sub = 0;
    z_list_t *subs = z->subscriptions;
    z_list_t *xs = z_list_empty;
    do {      
      sub = (_z_sub_t *)z_list_head(subs);
      subs = z_list_tail(subs);            
      if (intersect(sub->rname, (char *)rname)) {        
        xs = z_list_cons(xs, sub);
      }       
    } while (subs != 0);          
    return xs;
  }
}

void _z_register_storage(z_zenoh_t *z, z_vle_t rid, z_vle_t id, z_data_handler_t data_handler, z_query_handler_t query_handler, void *arg) {
  _z_sto_t *sto = (_z_sto_t *) malloc(sizeof(_z_sto_t));
  sto->rid = rid;
  sto->id = id;
  _z_res_decl_t *decl = _z_get_res_decl_by_rid(z, rid);
  assert(decl != 0);
  sto->rname = strdup(decl->r_name);
  sto->data_handler = data_handler;
  sto->query_handler = query_handler;
  sto->arg = arg;
  z->storages = z_list_cons(z->storages, sto);
}

int sto_predicate(void *elem, void *arg) {
  z_sto_t *s = (z_sto_t *)arg;
  _z_sto_t *sto = (_z_sto_t *)elem;
  if(sto->id == s->id) {
    return 1;
  } else {
    return 0;
  }
}

void _z_unregister_storage(z_sto_t *s) {
  s->z->storages = z_list_remove(s->z->storages, sto_predicate, s);
}

z_list_t *
_z_get_storages_by_rid(z_zenoh_t *z, z_vle_t rid) {
  z_list_t *stos = z_list_empty;
  if (z->storages == 0) {
    return stos;
  }
  else {
    _z_sto_t *sto = 0;
    z_list_t *stos = z->storages;
    z_list_t *xs = z_list_empty;
    do {
      sto = (_z_sto_t *)z_list_head(stos);
      stos = z_list_tail(stos);
      if (sto->rid == rid) {
        xs = z_list_cons(xs, sto);
      }
    } while (stos != 0);
    return xs;
  }
}

z_list_t *
_z_get_storages_by_rname(z_zenoh_t *z, const char *rname) {
  z_list_t *stos = z_list_empty;
  if (z->storages == 0) {
    return stos;
  }
  else {
    _z_sto_t *sto = 0;
    z_list_t *stos = z->storages;
    z_list_t *xs = z_list_empty;
    do {
      sto = (_z_sto_t *)z_list_head(stos);
      stos = z_list_tail(stos);
      if (intersect(sto->rname, (char *)rname)) {
        xs = z_list_cons(xs, sto);
      }
    } while (stos != 0);
    return xs;
  }
}

void _z_register_eval(z_zenoh_t *z, z_vle_t rid, z_vle_t id, z_query_handler_t query_handler, void *arg) {
  _z_eva_t *eval = (_z_eva_t *) malloc(sizeof(_z_eva_t));
  eval->rid = rid;
  eval->id = id;
  _z_res_decl_t *decl = _z_get_res_decl_by_rid(z, rid);
  assert(decl != 0);
  eval->rname = strdup(decl->r_name);
  eval->query_handler = query_handler;
  eval->arg = arg;
  z->evals = z_list_cons(z->evals, eval);
}

int eva_predicate(void *elem, void *arg) {
  z_eva_t *e = (z_eva_t *)arg;
  _z_eva_t *eval = (_z_eva_t *)elem;
  if(eval->id == e->id) {
    return 1;
  } else {
    return 0;
  }
}

void _z_unregister_eval(z_eva_t *e) {
  e->z->evals = z_list_remove(e->z->evals, sto_predicate, e);
}

z_list_t *
_z_get_evals_by_rid(z_zenoh_t *z, z_vle_t rid) {
  z_list_t *evals = z_list_empty;
  if (z->evals == 0) {
    return evals;
  }
  else {
    _z_eva_t *eval = 0;
    z_list_t *evals = z->evals;
    z_list_t *xs = z_list_empty;
    do {
      eval = (_z_eva_t *)z_list_head(evals);
      evals = z_list_tail(evals);
      if (eval->rid == rid) {
        xs = z_list_cons(xs, eval);
      }
    } while (evals != 0);
    return xs;
  }
}

z_list_t *
_z_get_evals_by_rname(z_zenoh_t *z, const char *rname) {
  z_list_t *evals = z_list_empty;
  if (z->evals == 0) {
    return evals;
  }
  else {
    _z_eva_t *eval = 0;
    z_list_t *evals = z->evals;
    z_list_t *xs = z_list_empty;
    do {
      eval = (_z_eva_t *)z_list_head(evals);
      evals = z_list_tail(evals);
      if (intersect(eval->rname, (char *)rname)) {
        xs = z_list_cons(xs, eval);
      }
    } while (evals != 0);
    return xs;
  }
}

int _z_matching_remote_sub(z_zenoh_t *z, z_vle_t rid) {
  return z_i_map_get(z->remote_subs, rid) != 0 ? 1 : 0;   
}

void _z_register_query(z_zenoh_t *z, z_vle_t qid, z_reply_handler_t reply_handler, void *arg) {
  _z_replywaiter_t *rw = (_z_replywaiter_t *) malloc(sizeof(_z_replywaiter_t));
  rw->qid = qid;
  rw->reply_handler = reply_handler;
  rw->arg = arg;
  z->replywaiters = z_list_cons(z->replywaiters, rw);
}

_z_replywaiter_t *_z_get_query(z_zenoh_t *z, z_vle_t qid) {
  if (z->replywaiters == 0) {
    return 0;
  }
  else {
    _z_replywaiter_t *rw = (_z_replywaiter_t *)z_list_head(z->replywaiters);
    z_list_t *rws = z_list_tail(z->replywaiters);
    while (rws != 0 && rw->qid != qid) {      
      rw = z_list_head(rws);
      rws = z_list_tail(rws);  
    }    
    if (rw->qid == qid) return rw;
    else return 0;
  }
}
