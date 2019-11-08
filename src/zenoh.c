#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <stdatomic.h>
#include "zenoh.h"
#include "zenoh/property.h"
#include "zenoh/recv_loop.h"
#include "zenoh/private/net.h"
#include "zenoh/private/msgcodec.h"
#include "zenoh/private/logging.h"
#include <sys/socket.h>

// -- Some refactoring will be done to support multiple platforms / transports

void default_on_disconnect(void *vz) { 
  z_zenoh_t *z = (z_zenoh_t*)vz;
  for (int i = 0; i < 3; ++i) {
    sleep(3);
    // Try to reconnect -- eventually we should scout here.
    // We should also re-do declarations.
    _Z_DEBUG("Tring to reconnect...\n");
    z_socket_result_t r_sock = _z_open_tx_session(strdup(z->locator));    
    if (r_sock.tag == Z_OK_TAG) {  
      z->sock = r_sock.value.socket;
      return;
    }
  }
}

char* auto_select_iface() {
  return ZENOH_LOCAL_HOST;
}

z_vec_t
_z_scout_loop(z_socket_t socket, const z_iobuf_t* sbuf, const struct sockaddr *dest, socklen_t salen, size_t tries) {    
  struct sockaddr *from  = (struct sockaddr*) malloc(2*sizeof(struct sockaddr_in*));
  socklen_t flen = 0;
  z_iobuf_t hbuf = z_iobuf_make(ZENOH_MAX_SCOUT_MSG_LEN);
  z_vec_t ls;
  ls.capacity_ = 0;
  ls.elem_ = 0;
  ls.length_ = 0;
  while (tries != 0) {    
    tries -= 1;    
    _z_send_dgram_to(socket, sbuf, dest, salen);      
    int len = _z_recv_dgram_from(socket, &hbuf, from, &flen);
    if (len > 0) {      
      int header = z_iobuf_read(&hbuf);
      if (_Z_MID(header) == _Z_HELLO) {
        _z_hello_result_t r_h = z_hello_decode(&hbuf);
        if (r_h.tag == Z_OK_TAG) {
          ls = r_h.value.hello.locators;                
        }
      } else {
        perror("Scouting loop received unexpected message\n");
      }
      z_iobuf_free(&hbuf);
      return ls;
    }
  }
  z_iobuf_free(&hbuf);
  return ls;
}

z_vec_t
z_scout(char* iface, size_t tries, size_t period) {  
  char *addr = iface;  
  if ((iface == 0) || (strcmp(iface,"auto") == 0)) {    
    addr = _z_select_scout_iface();    
  }
  
  z_iobuf_t sbuf = z_iobuf_make(ZENOH_MAX_SCOUT_MSG_LEN);
  _z_scout_t scout;
  scout.mask = _Z_SCOUT_BROKER;
  _z_scout_encode(&sbuf, &scout);
  z_socket_result_t r = _z_create_udp_socket(addr, 0, period);
  ASSERT_RESULT(r, "Unable to create scouting socket\n");
  // Scout first on local node.
  struct sockaddr_in *laddr = _z_make_socket_address(addr, ZENOH_SCOUT_PORT);
  struct sockaddr_in *maddr = _z_make_socket_address(ZENOH_SCOUT_MCAST_ADDR, ZENOH_SCOUT_PORT);
  socklen_t salen = sizeof(struct sockaddr_in);  
  // Scout on Localhost
  z_vec_t locs =  _z_scout_loop(r.value.socket, &sbuf, (struct sockaddr *)laddr, salen, tries);

  if (z_vec_length(&locs) == 0) {
    // We did not find broker on the local host, thus Scout in the LAN 
    locs = _z_scout_loop(r.value.socket, &sbuf, (struct sockaddr *)maddr, salen, tries);    
  }
  return locs;
}

z_zenoh_p_result_t 
z_open(char* locator, z_on_disconnect_t on_disconnect, const z_vec_t* ps) {
  z_zenoh_p_result_t r; 
  if (locator == 0) {
    z_vec_t locs = z_scout("auto", ZENOH_SCOUT_TRIES, ZENOH_SCOUT_TIMEOUT);
    if (z_vec_length(&locs) > 0) {
      locator = strdup((const char *)z_vec_get(&locs, 0));
    }
    else {
      perror("Unable do scout a zenoh router. Please make sure one is running on your network!\n");
      r.tag = Z_ERROR_TAG;
      r.value.error = Z_TX_CONNECTION_ERROR;
      return r;
    }
  }
  r.value.zenoh = 0;
  srand(clock());

  z_socket_result_t r_sock = _z_open_tx_session(locator);    
  if (r_sock.tag == Z_ERROR_TAG) {  
    r.tag = Z_ERROR_TAG;
    r.value.error = Z_IO_ERROR;
    return r;
  }
  
  r.tag = Z_OK_TAG;

  Z_ARRAY_S_DEFINE(uint8_t, pid, ZENOH_PID_LENGTH);   
  for (int i = 0; i < ZENOH_PID_LENGTH; ++i) 
    pid.elem[i] = rand() % 255;

  _z_message_t msg;

  msg.header = ps == 0 ? _Z_OPEN : _Z_OPEN | _Z_P_FLAG;
  msg.payload.open.version = ZENOH_PROTO_VERSION;
  msg.payload.open.pid = pid;
  msg.payload.open.lease = ZENOH_DEFAULT_LEASE;
  msg.properties = ps;

  _Z_DEBUG("Sending Open\n");
  z_iobuf_t wbuf = z_iobuf_make(ZENOH_WRITE_BUF_LEN); 
  z_iobuf_t rbuf = z_iobuf_make(ZENOH_READ_BUF_LEN); 
  _z_send_msg(r_sock.value.socket, &wbuf, &msg);
  z_iobuf_clear(&rbuf);
  _z_message_p_result_t r_msg = _z_recv_msg(r_sock.value.socket, &rbuf);
  
  if (r_msg.tag == Z_ERROR_TAG) {
    r.tag = Z_ERROR_TAG;    
    r.value.error = Z_FAILED_TO_OPEN_SESSION;
    z_iobuf_free(&wbuf);
    z_iobuf_free(&rbuf);
    return r;
  }

  r.value.zenoh = (z_zenoh_t *)malloc(sizeof(z_zenoh_t));
  r.value.zenoh->sock = r_sock.value.socket;
  r.value.zenoh->sn = 0;
  r.value.zenoh->cid = 0;
  r.value.zenoh->rid = 0;
  r.value.zenoh->eid = 0;
  r.value.zenoh->rbuf = rbuf;
  r.value.zenoh->wbuf = wbuf;
  r.value.zenoh->pid = pid;
  Z_ARRAY_S_COPY(uint8_t, r.value.zenoh->peer_pid, r_msg.value.message->payload.accept.broker_pid);
  r.value.zenoh->qid = 0;
  r.value.zenoh->locator = strdup(locator);
  r.value.zenoh->on_disconnect = on_disconnect != 0 ? on_disconnect : &default_on_disconnect;
  r.value.zenoh->declarations = z_list_empty;
  r.value.zenoh->subscriptions = z_list_empty;
  r.value.zenoh->storages = z_list_empty;
  r.value.zenoh->evals = z_list_empty;
  r.value.zenoh->replywaiters = z_list_empty;
  r.value.zenoh->reply_msg_mvar = z_mvar_empty();
  r.value.zenoh->remote_subs = z_i_map_make(DEFAULT_I_MAP_CAPACITY); 
  r.value.zenoh->running = 0;
  r.value.zenoh->thread = 0;
  _z_message_p_result_free(&r_msg);
  
  return r;
}

z_vec_t z_info(z_zenoh_t *z) {
  z_vec_t res = z_vec_make(3);
  z_property_t *pid = z_property_make(Z_INFO_PID_KEY, z->pid);
  z_array_uint8_t locator;
  locator.length = strlen(z->locator) + 1;
  locator.elem = (uint8_t *)z->locator;
  z_property_t *peer = z_property_make(Z_INFO_PEER_KEY, locator);
  z_property_t *peer_pid = z_property_make(Z_INFO_PEER_PID_KEY, z->peer_pid);

  z_vec_set(&res, Z_INFO_PID_KEY, pid);
  z_vec_set(&res, Z_INFO_PEER_KEY, peer);
  z_vec_set(&res, Z_INFO_PEER_PID_KEY, peer_pid);
  res.length_ = 3;

  return res;
}


int z_close(z_zenoh_t* z) {
  _z_message_t c;    
  c.header = _Z_CLOSE;
  c.payload.close.pid = z->pid;
  c.payload.close.reason = _Z_PEER_CLOSE;
  int rv = _z_send_msg(z->sock, &z->wbuf, &c);
  close(z->sock);    
  return rv;  
}

z_sub_p_result_t
z_declare_subscriber(z_zenoh_t *z, const char *resource,  const z_sub_mode_t *sm, z_data_handler_t data_handler, void *arg) {
  z_sub_p_result_t r;
  assert((sm->kind > 0) && (sm->kind <= 4));
  r.tag = Z_OK_TAG;
  r.value.sub = (z_sub_t*)malloc(sizeof(z_sub_t));
  r.value.sub->z = z;
  int id = _z_get_entity_id(z);
  r.value.sub->id = id;

  _z_message_t msg;
  msg.header = _Z_DECLARE;
  msg.payload.declare.sn = z->sn++;
  int dnum = 3;
  Z_ARRAY_S_DEFINE(_z_declaration_t, decl, dnum)
  
  int rid = _z_get_resource_id(z, resource);
  r.value.sub->rid = rid;

  decl.elem[0].header = _Z_RESOURCE_DECL;
  decl.elem[0].payload.resource.r_name = (char*)resource;  
  decl.elem[0].payload.resource.rid = rid;

  decl.elem[1].header = _Z_SUBSCRIBER_DECL;
  decl.elem[1].payload.sub.sub_mode = *sm;
  decl.elem[1].payload.sub.rid = rid;
  
  decl.elem[2].header = _Z_COMMIT_DECL;
  decl.elem[2].payload.commit.cid = z->cid++;
  
  msg.payload.declare.declarations = decl; 

  if (_z_send_msg(z->sock, &z->wbuf, &msg) != 0) {
      _Z_DEBUG("Trying to reconnect....\n");
      z->on_disconnect(z);
      _z_send_msg(z->sock, &z->wbuf, &msg);
  } 
  Z_ARRAY_S_FREE(decl);
  _z_register_res_decl(z, rid, resource);
  _z_register_subscription(z, rid, id, data_handler, arg);
  // -- This will be refactored to use mvars
  return r;
}

int z_undeclare_subscriber(z_sub_t *sub) {
  _z_unregister_subscription(sub);
  z_list_t * subs = _z_get_subscriptions_by_rid(sub->z, sub->rid);
  if(subs == z_list_empty) {
    _z_message_t msg;
    msg.header = _Z_DECLARE;
    msg.payload.declare.sn = sub->z->sn++;
    int dnum = 2;
    Z_ARRAY_S_DEFINE(_z_declaration_t, decl, dnum)
    
    decl.elem[0].header = _Z_FORGET_SUBSCRIBER_DECL;
    decl.elem[0].payload.forget_sub.rid = sub->rid;
    
    decl.elem[1].header = _Z_COMMIT_DECL;
    decl.elem[1].payload.commit.cid = sub->z->cid++;
    
    msg.payload.declare.declarations = decl; 

    if (_z_send_msg(sub->z->sock, &sub->z->wbuf, &msg) != 0) {
        _Z_DEBUG("Trying to reconnect....\n");
        sub->z->on_disconnect(sub->z);
        _z_send_msg(sub->z->sock, &sub->z->wbuf, &msg);
    } 
    Z_ARRAY_S_FREE(decl);
  }
  z_list_free(&subs);
  // TODO free subscription
  return 0;
}

z_sto_p_result_t
z_declare_storage(z_zenoh_t *z, const char *resource, z_data_handler_t data_handler, z_query_handler_t query_handler, void *arg) {
  z_sto_p_result_t r;
  r.tag = Z_OK_TAG;
  r.value.sto = (z_sto_t*)malloc(sizeof(z_sto_t));
  r.value.sto->z = z;
  z_vle_t id = _z_get_entity_id(z);
  r.value.sto->id = id;

  _z_message_t msg;
  msg.header = _Z_DECLARE;
  msg.payload.declare.sn = z->sn++;
  int dnum = 3;
  Z_ARRAY_S_DEFINE(_z_declaration_t, decl, dnum)
  
  int rid = _z_get_resource_id(z, resource);
  r.value.sto->rid = rid;

  decl.elem[0].header = _Z_RESOURCE_DECL;
  decl.elem[0].payload.resource.r_name = (char*)resource;  
  decl.elem[0].payload.resource.rid = rid;

  decl.elem[1].header = _Z_STORAGE_DECL;
  decl.elem[1].payload.storage.rid = rid;
  
  decl.elem[2].header = _Z_COMMIT_DECL;
  decl.elem[2].payload.commit.cid = z->cid++;
  
  msg.payload.declare.declarations = decl;  
  if (_z_send_msg(z->sock, &z->wbuf, &msg) != 0) {
      _Z_DEBUG("Trying to reconnect....\n");
      z->on_disconnect(z);
      _z_send_msg(z->sock, &z->wbuf, &msg);
  } 
  Z_ARRAY_S_FREE(decl);
  _z_register_res_decl(z, rid, resource);
  _z_register_storage(z, rid, id, data_handler, query_handler, arg);
  // -- This will be refactored to use mvars
  return r;
}

int z_undeclare_storage(z_sto_t *sto) {
  _z_unregister_storage(sto);
  z_list_t * stos = _z_get_storages_by_rid(sto->z, sto->rid);
  if(stos == z_list_empty) {
    _z_message_t msg;
    msg.header = _Z_DECLARE;
    msg.payload.declare.sn = sto->z->sn++;
    int dnum = 2;
    Z_ARRAY_S_DEFINE(_z_declaration_t, decl, dnum)
    
    decl.elem[0].header = _Z_FORGET_STORAGE_DECL;
    decl.elem[0].payload.forget_sto.rid = sto->rid;
    
    decl.elem[1].header = _Z_COMMIT_DECL;
    decl.elem[1].payload.commit.cid = sto->z->cid++;
    
    msg.payload.declare.declarations = decl; 

    if (_z_send_msg(sto->z->sock, &sto->z->wbuf, &msg) != 0) {
        _Z_DEBUG("Trying to reconnect....\n");
        sto->z->on_disconnect(sto->z);
        _z_send_msg(sto->z->sock, &sto->z->wbuf, &msg);
    } 
    Z_ARRAY_S_FREE(decl);
  }
  z_list_free(&stos);
  // TODO free storages
  return 0;
}

z_eval_p_result_t
z_declare_eval(z_zenoh_t *z, const char *resource, z_query_handler_t query_handler, void *arg) {
  z_eval_p_result_t r;
  r.tag = Z_OK_TAG;
  r.value.eval = (z_eva_t*)malloc(sizeof(z_eva_t));
  r.value.eval->z = z;
  z_vle_t id = _z_get_entity_id(z);
  r.value.eval->id = id;

  _z_message_t msg;
  msg.header = _Z_DECLARE;
  msg.payload.declare.sn = z->sn++;
  int dnum = 3;
  Z_ARRAY_S_DEFINE(_z_declaration_t, decl, dnum)
  
  int rid = _z_get_resource_id(z, resource);
  r.value.eval->rid = rid;

  decl.elem[0].header = _Z_RESOURCE_DECL;
  decl.elem[0].payload.resource.r_name = (char*)resource;  
  decl.elem[0].payload.resource.rid = rid;

  decl.elem[1].header = _Z_EVAL_DECL;
  decl.elem[1].payload.eval.rid = rid;
  
  decl.elem[2].header = _Z_COMMIT_DECL;
  decl.elem[2].payload.commit.cid = z->cid++;
  
  msg.payload.declare.declarations = decl;  
  if (_z_send_msg(z->sock, &z->wbuf, &msg) != 0) {
      _Z_DEBUG("Trying to reconnect....\n");
      z->on_disconnect(z);
      _z_send_msg(z->sock, &z->wbuf, &msg);
  } 
  Z_ARRAY_S_FREE(decl);
  _z_register_res_decl(z, rid, resource);
  _z_register_eval(z, rid, id, query_handler, arg);
  // -- This will be refactored to use mvars
  return r;
}

int z_undeclare_eval(z_eva_t *eval) {
  _z_unregister_eval(eval);
  z_list_t * evals = _z_get_evals_by_rid(eval->z, eval->rid);
  if(evals == z_list_empty) {
    _z_message_t msg;
    msg.header = _Z_DECLARE;
    msg.payload.declare.sn = eval->z->sn++;
    int dnum = 2;
    Z_ARRAY_S_DEFINE(_z_declaration_t, decl, dnum)
    
    decl.elem[0].header = _Z_FORGET_EVAL_DECL;
    decl.elem[0].payload.forget_eval.rid = eval->rid;
    
    decl.elem[1].header = _Z_COMMIT_DECL;
    decl.elem[1].payload.commit.cid = eval->z->cid++;
    
    msg.payload.declare.declarations = decl; 

    if (_z_send_msg(eval->z->sock, &eval->z->wbuf, &msg) != 0) {
        _Z_DEBUG("Trying to reconnect....\n");
        eval->z->on_disconnect(eval->z);
        _z_send_msg(eval->z->sock, &eval->z->wbuf, &msg);
    } 
    Z_ARRAY_S_FREE(decl);
  }
  z_list_free(&evals);
  // TODO free evals
  return 0;
}

z_pub_p_result_t 
z_declare_publisher(z_zenoh_t *z, const char *resource) {
  z_pub_p_result_t r;
  r.tag = Z_OK_TAG;
  r.value.pub = (z_pub_t*)malloc(sizeof(z_pub_t));
  r.value.pub->z = z;
  r.value.pub->id = _z_get_entity_id(z);

  _z_message_t msg;
  msg.header = _Z_DECLARE;
  msg.payload.declare.sn = z->sn++;
  int dnum = 3;
  Z_ARRAY_S_DEFINE(_z_declaration_t, decl, dnum)
    
  int rid = _z_get_resource_id(z, resource);
  r.value.pub->rid = rid;
  
  decl.elem[0].header = _Z_RESOURCE_DECL;
  decl.elem[0].payload.resource.r_name = (char*)resource;  
  decl.elem[0].payload.resource.rid = rid;
  
  decl.elem[1].header = _Z_PUBLISHER_DECL;
  decl.elem[1].payload.pub.rid = rid;
  
  decl.elem[2].header = _Z_COMMIT_DECL;
  decl.elem[2].payload.commit.cid = z->cid++;
  
  msg.payload.declare.declarations = decl;  
  if (_z_send_msg(z->sock, &z->wbuf, &msg) != 0) {
      _Z_DEBUG("Trying to reconnect....\n");
      z->on_disconnect(z);
      _z_send_msg(z->sock, &z->wbuf, &msg);
  }
  Z_ARRAY_S_FREE(decl);
  _z_register_res_decl(z, rid, resource);
  // -- This will be refactored to use mvars
  return r;  
}

int z_undeclare_publisher(z_pub_t *pub) {
  _z_message_t msg;
  msg.header = _Z_DECLARE;
  msg.payload.declare.sn = pub->z->sn++;
  int dnum = 2;
  Z_ARRAY_S_DEFINE(_z_declaration_t, decl, dnum)
  
  decl.elem[0].header = _Z_FORGET_PUBLISHER_DECL;
  decl.elem[0].payload.forget_pub.rid = pub->rid;
  
  decl.elem[1].header = _Z_COMMIT_DECL;
  decl.elem[1].payload.commit.cid = pub->z->cid++;
  
  msg.payload.declare.declarations = decl; 

  if (_z_send_msg(pub->z->sock, &pub->z->wbuf, &msg) != 0) {
      _Z_DEBUG("Trying to reconnect....\n");
      pub->z->on_disconnect(pub->z);
      _z_send_msg(pub->z->sock, &pub->z->wbuf, &msg);
  } 
  Z_ARRAY_S_FREE(decl);

  // TODO manage multi publishers
  return 0;
}

int z_stream_compact_data(z_pub_t *pub, const unsigned char *data, size_t length) { 
  const char *rname = _z_get_resource_name(pub->z, pub->rid);
  z_resource_id_t rid;
  _z_sub_t *sub;
  _z_sto_t *sto;
  z_list_t *subs = z_list_empty;
  z_list_t *stos = z_list_empty;
  z_list_t *xs;
  if (rname != 0) {
    subs = _z_get_subscriptions_by_rname(pub->z, rname);
    stos = _z_get_storages_by_rname(pub->z, rname);
    rid.kind = Z_STR_RES_ID;
    rid.id.rname = (char *)rname;
  } else {
    subs = _z_get_subscriptions_by_rid(pub->z, pub->rid);
    rid.kind = Z_INT_RES_ID;
    rid.id.rid = pub->rid;
  }

  if (subs != 0 || stos != 0) {    
    z_data_info_t info;
    bzero(&info, sizeof(z_data_info_t));
    
    if(subs != 0) {
      xs = subs;
      while (xs != z_list_empty) {
        sub = z_list_head(xs);
        sub->data_handler(&rid, data, length, &info, sub->arg);
        xs = z_list_tail(xs);
      }
      z_list_free(&subs); 
    }
    
    if(stos != 0) {
      xs = stos;
      while (xs != z_list_empty) {
        sto = z_list_head(xs);
        sto->data_handler(&rid, data, length, &info, sto->arg);
        xs = z_list_tail(xs);
      }
      z_list_free(&stos); 
    }
  }

  if (_z_matching_remote_sub(pub->z, pub->rid) == 1) {
    _z_message_t msg;
    msg.header = _Z_COMPACT_DATA;  
    msg.payload.compact_data.rid = pub->rid;    
    msg.payload.compact_data.payload = z_iobuf_wrap_wo((unsigned char *)data, length, 0, length);
    msg.payload.compact_data.sn = pub->z->sn++;
    if (_z_send_large_msg(pub->z->sock, &pub->z->wbuf, &msg, length + 128) == 0)
      return 0;
    else
    {
      _Z_DEBUG("Trying to reconnect....\n");
      pub->z->on_disconnect(pub->z);
      return _z_send_large_msg(pub->z->sock, &pub->z->wbuf, &msg, length + 128);
    }
  }
  return 0;
}


int 
z_stream_data_wo(z_pub_t *pub, const unsigned char *data, size_t length, uint8_t encoding, uint8_t kind) {
  const char *rname = _z_get_resource_name(pub->z, pub->rid);
  z_resource_id_t rid;
  _z_sub_t *sub;
  _z_sto_t *sto;
  z_list_t *subs = z_list_empty;
  z_list_t *stos = z_list_empty;
  z_list_t *xs;
  if (rname != 0) {
    subs = _z_get_subscriptions_by_rname(pub->z, rname);
    stos = _z_get_storages_by_rname(pub->z, rname);
    rid.kind = Z_STR_RES_ID;
    rid.id.rname = (char *)rname;
  } else {
    subs = _z_get_subscriptions_by_rid(pub->z, pub->rid);
    rid.kind = Z_INT_RES_ID;
    rid.id.rid = pub->rid;
  }

  if (subs != 0 || stos != 0) {    
    z_data_info_t info;
    info.flags = _Z_ENCODING | _Z_KIND;
    info.encoding = encoding;
    info.kind = kind;  
    
    if(subs != 0) {
      xs = subs;
      while (xs != z_list_empty) {
        sub = z_list_head(xs);
        sub->data_handler(&rid, data, length, &info, sub->arg);
        xs = z_list_tail(xs);
      }
      z_list_free(&subs); 
    }
    
    if(stos != 0) {
      xs = stos;
      while (xs != z_list_empty) {
        sto = z_list_head(xs);
        sto->data_handler(&rid, data, length, &info, sto->arg);
        xs = z_list_tail(xs);
      }
      z_list_free(&stos); 
    }
  }

  if (_z_matching_remote_sub(pub->z, pub->rid) == 1) {
    _z_payload_header_t ph;
    ph.flags = _Z_ENCODING | _Z_KIND;
    ph.encoding = encoding;
    ph.kind = kind;
    ph.payload = z_iobuf_wrap_wo((unsigned char *)data, length, 0, length);
    z_iobuf_t buf = z_iobuf_make(length + 32 );
    _z_payload_header_encode(&buf, &ph);

    _z_message_t msg;
    msg.header = _Z_STREAM_DATA;
    // No need to take ownership, avoid strdup.
    msg.payload.stream_data.rid = pub->rid;      
    msg.payload.stream_data.sn = pub->z->sn++;
    msg.payload.stream_data.payload_header = buf;
    if (_z_send_large_msg(pub->z->sock, &pub->z->wbuf, &msg, length + 128) == 0) {
      z_iobuf_free(&buf);
      return 0;
    }
    else
    {
      _Z_DEBUG("Trying to reconnect....\n");
      pub->z->on_disconnect(pub->z);
      int rv = _z_send_large_msg(pub->z->sock, &pub->z->wbuf, &msg, length + 128);
      z_iobuf_free(&buf);
      return rv;
    }
  } else {
    _Z_DEBUG_VA("No remote subscription matching for rid = %zu\n", pub->rid);
  }
    
  return 0;
}

int z_stream_data(z_pub_t *pub, const unsigned char *data, size_t length) {
  return z_stream_data_wo(pub, data, length, 0, 0); 
}

int z_write_data_wo(z_zenoh_t *z, const char* resource, const unsigned char *payload, size_t length, uint8_t encoding, uint8_t kind) { 
  z_list_t *subs = _z_get_subscriptions_by_rname(z, resource);
  z_list_t *stos = _z_get_storages_by_rname(z, resource);
  _z_sub_t *sub;
  _z_sto_t *sto;
  z_resource_id_t rid;
  rid.kind = Z_STR_RES_ID;
  rid.id.rname = (char *)resource;
  z_data_info_t info;
  info.flags = _Z_ENCODING | _Z_KIND;
  info.encoding = encoding;
  info.kind = kind;  
  while (subs != 0) {
    sub = z_list_head(subs);
    sub->data_handler(&rid, payload, length, &info, sub->arg);
    subs = z_list_tail(subs);
  }
  while (stos != 0) {
    sto = z_list_head(stos);
    sto->data_handler(&rid, payload, length, &info, sto->arg);
    stos = z_list_tail(stos);
  }
  _z_payload_header_t ph;
  ph.flags = _Z_ENCODING | _Z_KIND;
  ph.encoding = encoding;
  ph.kind = kind;
  ph.payload = z_iobuf_wrap_wo((unsigned char *)payload, length, 0, length);
  z_iobuf_t buf = z_iobuf_make(length + 32 );
  _z_payload_header_encode(&buf, &ph);

  _z_message_t msg;
  msg.header = _Z_WRITE_DATA;
  // No need to take ownership, avoid strdup.
  msg.payload.write_data.rname = (char*) resource;      
  msg.payload.write_data.sn = z->sn++;
  msg.payload.write_data.payload_header = buf;
  if (_z_send_large_msg(z->sock, &z->wbuf, &msg, length + 128) == 0) {
    z_iobuf_free(&buf);
    return 0;
  }
  else { 
    _Z_DEBUG("Trying to reconnect....\n");
    z->on_disconnect(z);
    int rv = _z_send_large_msg(z->sock, &z->wbuf, &msg, length + 128);
    z_iobuf_free(&buf);
    return rv;    
  }
  
}

int z_write_data(z_zenoh_t *z, const char* resource, const unsigned char *payload, size_t length) {
  return z_write_data_wo(z, resource, payload, length, 0, 0);
}

int z_pull(z_sub_t *sub) {
  _z_message_t msg;
  msg.header = _Z_PULL | _Z_F_FLAG | _Z_S_FLAG;
  msg.payload.pull.sn = sub->z->sn++;
  msg.payload.pull.id = sub->rid;
  if (_z_send_msg(sub->z->sock, &sub->z->wbuf, &msg) != 0) {
      _Z_DEBUG("Trying to reconnect....\n");
      sub->z->on_disconnect(sub->z);
      return _z_send_msg(sub->z->sock, &sub->z->wbuf, &msg);
  }
  return 0;
}

typedef struct {
  z_zenoh_t *z;
  char *resource;
  char *predicate;
  z_reply_handler_t reply_handler;
  void *arg;
  z_query_dest_t dest_storages;
  z_query_dest_t dest_evals;
  atomic_int nb_qhandlers;
  atomic_flag sent_final;
} local_query_handle_t;

void send_local_replies(void* query_handle, z_array_p_resource_t replies, char eval){
  unsigned int i;
  z_reply_value_t rep;
  local_query_handle_t *handle = (local_query_handle_t*)query_handle;
  for(i = 0; i < replies.length; ++i) {
    if(eval){
      rep.kind = Z_EVAL_DATA;
    }else{
      rep.kind = Z_STORAGE_DATA;
    }
    rep.srcid = handle->z->pid.elem;
    rep.srcid_length = handle->z->pid.length;
    rep.rsn = i;
    rep.rname = replies.elem[i]->rname;
    rep.info.flags = _Z_ENCODING | _Z_KIND;
    rep.info.encoding = replies.elem[i]->encoding;
    rep.info.kind = replies.elem[i]->kind;
    rep.data = replies.elem[i]->data;
    rep.data_length = replies.elem[i]->length;
    handle->reply_handler(&rep, handle->arg);
  }
  bzero(&rep, sizeof(z_reply_value_t));
  if(eval){
    rep.kind = Z_EVAL_FINAL;
  }else{
    rep.kind = Z_STORAGE_FINAL;
  }
  rep.srcid = handle->z->pid.elem;
  rep.srcid_length = handle->z->pid.length;
  rep.rsn = i;
  handle->reply_handler(&rep, handle->arg);

  atomic_fetch_sub(&handle->nb_qhandlers, 1);
  if(handle->nb_qhandlers <= 0 && !atomic_flag_test_and_set(&handle->sent_final))
  {
    _z_message_t msg;
    msg.header = _Z_QUERY;
    msg.payload.query.pid = handle->z->pid;
    msg.payload.query.qid = handle->z->qid++;
    msg.payload.query.rname = handle->resource;
    msg.payload.query.predicate = handle->predicate;

    if(handle->dest_storages.kind != Z_BEST_MATCH || handle->dest_evals.kind != Z_BEST_MATCH) {
      msg.header = _Z_QUERY | _Z_P_FLAG;
      z_vec_t ps = z_vec_make(2);
      if(handle->dest_storages.kind != Z_BEST_MATCH) {
        z_property_t dest_storages_prop = {Z_DEST_STORAGES_KEY, {1, (uint8_t*)&handle->dest_storages.kind}};
        z_vec_append(&ps, &dest_storages_prop);
      }
      if(handle->dest_evals.kind != Z_BEST_MATCH) {
        z_property_t dest_evals_prop = {Z_DEST_EVALS_KEY, {1, (uint8_t*)&handle->dest_evals.kind}};
        z_vec_append(&ps, &dest_evals_prop);
      }
      msg.properties = &ps;
    }
    
    _z_register_query(handle->z, msg.payload.query.qid, handle->reply_handler, handle->arg);
    if (_z_send_msg(handle->z->sock, &handle->z->wbuf, &msg) != 0) {
      _Z_DEBUG("Trying to reconnect....\n");
      handle->z->on_disconnect(handle->z);
      _z_send_msg(handle->z->sock, &handle->z->wbuf, &msg);
    }
    free(handle);
  }
}

void send_local_storage_replies(void* query_handle, z_array_p_resource_t replies){
  send_local_replies(query_handle, replies, 0);
}

void send_local_eval_replies(void* query_handle, z_array_p_resource_t replies){
  send_local_replies(query_handle, replies, 1);
}

int z_query_wo(z_zenoh_t *z, const char* resource, const char* predicate, z_reply_handler_t reply_handler, void *arg, z_query_dest_t dest_storages, z_query_dest_t dest_evals) { 
  z_list_t *stos = dest_storages.kind == Z_NONE ? 0 : _z_get_storages_by_rname(z, resource);
  z_list_t *evals = dest_evals.kind == Z_NONE ? 0 : _z_get_evals_by_rname(z, resource);
  if(stos != 0 || evals != 0)
  {
    _z_sto_t *sto;
    _z_eva_t *eval;
    z_list_t *lit;

    local_query_handle_t *handle = malloc(sizeof(local_query_handle_t));
    handle->z = z;
    handle->resource = (char*)resource;
    handle->predicate = (char*)predicate;
    handle->reply_handler = reply_handler;
    handle->arg = arg;
    handle->dest_storages = dest_storages;
    handle->dest_evals = dest_evals;

    int nb_qhandlers = 0;
    if(stos != 0){nb_qhandlers += z_list_len(stos);}
    if(evals != 0){nb_qhandlers += z_list_len(evals);}
    atomic_init(&handle->nb_qhandlers, nb_qhandlers);
    atomic_flag_clear(&handle->sent_final);

    if(stos != 0){
      lit = stos;
      while (lit != 0) {
        sto = z_list_head(lit);
        sto->query_handler(resource, predicate, send_local_storage_replies, handle, sto->arg);
        lit = z_list_tail(lit);
      }
      z_list_free(&stos);
    }

    if(evals != 0){
      lit = evals;
      while (lit != 0) {
        eval = z_list_head(lit);
        eval->query_handler(resource, predicate, send_local_eval_replies, handle, eval->arg);
        lit = z_list_tail(lit);
      }
      z_list_free(&evals);
    }
  }else{
    _z_message_t msg;
    msg.header = _Z_QUERY;
    msg.payload.query.pid = z->pid;
    msg.payload.query.qid = z->qid++;
    msg.payload.query.rname = (char *)resource;
    msg.payload.query.predicate = (char *)predicate;

    if(dest_storages.kind != Z_BEST_MATCH || dest_evals.kind != Z_BEST_MATCH) {
      msg.header = _Z_QUERY | _Z_P_FLAG;
      z_vec_t ps = z_vec_make(2);
      if(dest_storages.kind != Z_BEST_MATCH) {
        z_property_t dest_storages_prop = {Z_DEST_STORAGES_KEY, {1, (uint8_t*)&dest_storages.kind}};
        z_vec_append(&ps, &dest_storages_prop);
      }
      if(dest_evals.kind != Z_BEST_MATCH) {
        z_property_t dest_evals_prop = {Z_DEST_EVALS_KEY, {1, (uint8_t*)&dest_evals.kind}};
        z_vec_append(&ps, &dest_evals_prop);
      }
      msg.properties = &ps;
    }
    
    _z_register_query(z, msg.payload.query.qid, reply_handler, arg);
    if (_z_send_msg(z->sock, &z->wbuf, &msg) != 0) {
      _Z_DEBUG("Trying to reconnect....\n");
      z->on_disconnect(z);
      _z_send_msg(z->sock, &z->wbuf, &msg);
    }
  }
  return 0;
}

int z_query(z_zenoh_t *z, const char* resource, const char* predicate, z_reply_handler_t reply_handler, void *arg) { 
  z_query_dest_t best_match = {Z_BEST_MATCH, 0};
  return z_query_wo(z, resource, predicate, reply_handler, arg, best_match, best_match); 
}
