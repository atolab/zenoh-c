#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include "zenoh/net.h"
#include "zenoh.h"

// -- Some refactoring will be done to support multiple platforms / transports

void default_on_disconnect(void *vz) { 
  z_zenoh_t *z = (z_zenoh_t*)vz;
  for (int i = 0; i < 3; ++i) {
    sleep(3);
    // Try to reconnect -- eventually we should scout here.
    // We should also re-do declarations.
    Z_DEBUG("Tring to reconnect...\n");
    z_socket_result_t r_sock = open_tx_session(strdup(z->locator));    
    if (r_sock.tag == Z_OK_TAG) {  
      z->sock = r_sock.value.socket;
      return;
    }
  }
}

z_zenoh_p_result_t 
z_open(char* locator, on_disconnect_t *on_disconnect, const z_vec_t* ps) {
  z_zenoh_p_result_t r; 
  
  r.value.zenoh = (z_zenoh_t *)malloc(sizeof(z_zenoh_t));
  r.value.zenoh->locator = strdup(locator);
  srand(clock());

  z_socket_result_t r_sock = open_tx_session(locator);    
  if (r_sock.tag == Z_ERROR_TAG) {  
    r.tag = Z_ERROR_TAG;
    r.value.error = Z_IO_ERROR;
    return r;
  }
  

  r.tag = Z_OK_TAG;  
  r.value.zenoh->sock = r_sock.value.socket;
  r.value.zenoh->cid = 0;
  r.value.zenoh->sn = 0;
  r.value.zenoh->rbuf = z_iobuf_make(ZENOH_READ_BUF_LEN);  
  r.value.zenoh->wbuf = z_iobuf_make(ZENOH_WRITE_BUF_LEN);  
  r.value.zenoh->qid = 0;
  r.value.zenoh->rid = 0;  
  r.value.zenoh->subscriptions = 0;
  r.value.zenoh->declarations = 0;
  r.value.zenoh->reply_msg_mvar = 0;

  Z_ARRAY_S_MAKE(uint8_t, pid, ZENOH_PID_LENGTH);   
  for (int i = 0; i < ZENOH_PID_LENGTH; ++i) 
    pid.elem[i] = rand() % 255;  

  z_message_t msg;

  msg.header = ps == 0 ? Z_OPEN : Z_OPEN | Z_P_FLAG;
  msg.payload.open.version = ZENOH_PROTO_VERSION;
  msg.payload.open.pid = pid;
  msg.payload.open.lease = ZENOH_DEFAULT_LEASE;
  msg.properties = ps;

  Z_DEBUG("Sending Open\n");
  z_send_msg(r_sock.value.socket, &r.value.zenoh->wbuf, &msg);
  z_iobuf_clear(&r.value.zenoh->rbuf);
  z_message_p_result_t r_msg = z_recv_msg(r_sock.value.socket, &r.value.zenoh->rbuf);
  ASSERT_P_RESULT(r_msg, "Failed to receive accept");
  z_message_p_result_free(&r_msg);

  r.value.zenoh->sock = r_sock.value.socket;
  r.value.zenoh->pid = pid;  
  r.value.zenoh->declarations = z_list_empty;
  r.value.zenoh->subscriptions = z_list_empty;
  r.value.zenoh->replywaiters = z_list_empty;
  r.value.zenoh->reply_msg_mvar = z_mvar_empty();
  r.value.zenoh->remote_subs = z_i_map_make(DEFAULT_I_MAP_CAPACITY); 

  if (on_disconnect != 0)
    r.value.zenoh->on_disconnect = on_disconnect;  
  else 
    r.value.zenoh->on_disconnect = &default_on_disconnect;  
  
  return r;
}

z_zenoh_t * 
z_open_wup(char* locator, const char * uname, const char *passwd) {
  z_zenoh_p_result_t r;
  z_property_t user;
  z_property_t password;
  z_vec_t ps = z_vec_make(2);
  Z_ARRAY_S_MAKE(uint8_t, uid, 256);
  Z_ARRAY_S_MAKE(uint8_t, pwd, 256);
  if (uname != 0) {
    uid.elem = (uint8_t *)uname; 
    uid.length = strlen(uname);
    pwd.elem = (uint8_t *)passwd; 
    pwd.length = strlen(passwd);
  
    user.id = Z_USER_KEY;
    user.value = uid;
  
    password.id = Z_PASSWD_KEY;
    password.value = pwd;
  
    z_vec_append(&ps, &user);
    z_vec_append(&ps, &password);

    r = z_open(locator, 0, &ps);
  } else {
    r = z_open(locator, 0, 0);
  }
  if (r.tag == Z_ERROR_TAG)
    return 0;
  
  return r.value.zenoh;
}


void z_close(z_zenoh_t* z) { }

z_sub_p_result_t
z_declare_subscriber(z_zenoh_t *z, const char *resource,  z_sub_mode_t sm, subscriber_callback_t *callback) {
  z_sub_p_result_t r;
  r.tag = Z_OK_TAG;
  r.value.sub = (z_sub_t*)malloc(sizeof(z_sub_t));
  r.value.sub->z = z;
  r.value.sub->id = z_get_entity_id(z);

  z_message_t msg;
  msg.header = Z_DECLARE;
  msg.payload.declare.sn = z->sn++;
  int dnum = 3;
  Z_ARRAY_S_MAKE(z_declaration_t, decl, dnum)
  
  int rid = z_get_resource_id(z, resource);
  r.value.sub->rid = rid;

  decl.elem[0].header = Z_RESOURCE_DECL;
  decl.elem[0].payload.resource.r_name = (char*)resource;  
  decl.elem[0].payload.resource.rid = rid;

  decl.elem[1].header = Z_SUBSCRIBER_DECL;
  decl.elem[1].payload.sub.sub_mode = sm;
  decl.elem[1].payload.sub.rid = rid;
  
  decl.elem[2].header = Z_COMMIT_DECL;
  decl.elem[2].payload.commit.cid = z->cid++;
  
  msg.payload.declare.declarations = decl; 

  if (z_send_msg(z->sock, &z->wbuf, &msg) != 0) {
      Z_DEBUG("Trying to reconnect....\n");
      z->on_disconnect(z);
      z_send_msg(z->sock, &z->wbuf, &msg);
  } 
  Z_ARRAY_S_FREE(decl);
  z_register_res_decl(z, rid, resource);
  z_register_subscription(z, rid, callback);
  // -- This will be refactored to use mvars
  return r;
}


z_sto_p_result_t
z_declare_storage(z_zenoh_t *z, const char *resource, subscriber_callback_t *callback, query_handler_t *handler, replies_cleaner_t *cleaner) {
  z_sto_p_result_t r;
  r.tag = Z_OK_TAG;
  r.value.sto = (z_sto_t*)malloc(sizeof(z_sto_t));
  r.value.sto->z = z;
  r.value.sto->id = z_get_entity_id(z);

  z_message_t msg;
  msg.header = Z_DECLARE;
  msg.payload.declare.sn = z->sn++;
  int dnum = 3;
  Z_ARRAY_S_MAKE(z_declaration_t, decl, dnum)
  
  int rid = z_get_resource_id(z, resource);
  r.value.sto->rid = rid;

  decl.elem[0].header = Z_RESOURCE_DECL;
  decl.elem[0].payload.resource.r_name = (char*)resource;  
  decl.elem[0].payload.resource.rid = rid;

  decl.elem[1].header = Z_STORAGE_DECL;
  decl.elem[1].payload.storage.rid = rid;
  
  decl.elem[2].header = Z_COMMIT_DECL;
  decl.elem[2].payload.commit.cid = z->cid++;
  
  msg.payload.declare.declarations = decl;  
  if (z_send_msg(z->sock, &z->wbuf, &msg) != 0) {
      Z_DEBUG("Trying to reconnect....\n");
      z->on_disconnect(z);
      z_send_msg(z->sock, &z->wbuf, &msg);
  } 
  Z_ARRAY_S_FREE(decl);
  z_register_res_decl(z, rid, resource);
  z_register_storage(z, rid, callback, handler, cleaner);
  // -- This will be refactored to use mvars
  return r;
}

z_pub_p_result_t 
z_declare_publisher(z_zenoh_t *z, const char *resource) {
  z_pub_p_result_t r;
  r.tag = Z_OK_TAG;
  r.value.pub = (z_pub_t*)malloc(sizeof(z_pub_t));
  r.value.pub->z = z;
  r.value.pub->id = z_get_entity_id(z);

  z_message_t msg;
  msg.header = Z_DECLARE;
  msg.payload.declare.sn = z->sn++;
  int dnum = 3;
  Z_ARRAY_S_MAKE(z_declaration_t, decl, dnum)
    
  int rid = z_get_resource_id(z, resource);
  r.value.pub->rid = rid;
  
  decl.elem[0].header = Z_RESOURCE_DECL;
  decl.elem[0].payload.resource.r_name = (char*)resource;  
  decl.elem[0].payload.resource.rid = rid;
  
  decl.elem[1].header = Z_PUBLISHER_DECL;
  decl.elem[1].payload.pub.rid = rid;
  
  decl.elem[2].header = Z_COMMIT_DECL;
  decl.elem[2].payload.commit.cid = z->cid++;
  
  msg.payload.declare.declarations = decl;  
  if (z_send_msg(z->sock, &z->wbuf, &msg) != 0) {
      Z_DEBUG("Trying to reconnect....\n");
      z->on_disconnect(z);
      z_send_msg(z->sock, &z->wbuf, &msg);
  }
  Z_ARRAY_S_FREE(decl);
  z_register_res_decl(z, rid, resource);
  // -- This will be refactored to use mvars
  return r;
#if 0
  z_message_p_result_t r_msg;
  r_msg = z_recv_msg(z->sock, &z->rbuf);
  ASSERT_P_RESULT(r_msg, "Failed to receive message");
  
  if (Z_MID(r_msg.value.message->header) == Z_DECLARE) {
    Z_DEBUG ("Declaration was accepted");
    z_message_p_result_free(&r_msg);
    return 0;
  }
  z_message_p_result_free(&r_msg);
  return -1;
#endif 
  
}

int z_stream_compact_data(z_pub_t *pub, const unsigned char *data, size_t length) { 
  const char *rname = z_get_resource_name(pub->z, pub->rid);
  z_resource_id_t rid;
  z_subscription_t *sub;
  z_list_t *subs = z_list_empty;
  z_list_t *xs;
  if (rname != 0) {
    subs = z_get_subscriptions_by_rname(pub->z, rname);
    rid.kind = Z_STR_RES_ID;
    rid.id.rname = (char *)rname;
  } else {
    subs = z_get_subscriptions_by_rid(pub->z, pub->rid);
    rid.kind = Z_INT_RES_ID;
    rid.id.rid = pub->rid;
  }
    
  if (subs != z_list_empty) {
    z_data_info_t info;
    info.flags = Z_ENCODING | Z_KIND;
    info.encoding = 0;
    info.kind = 0;  
    xs = subs;
    while (xs != z_list_empty) {
      sub = z_list_head(xs);
      sub->callback(rid, data, length, info);
      xs = z_list_tail(xs);
    }
    z_list_free(&subs);  
  }
  if (z_matching_remote_sub(pub->z, pub->rid) == 1) {
    z_message_t msg;
    msg.header = Z_COMPACT_DATA;  
    msg.payload.compact_data.rid = pub->rid;    
    msg.payload.compact_data.payload = z_iobuf_wrap((unsigned char *)data, length);  
    msg.payload.compact_data.sn = pub->z->sn++;
    if (z_send_msg(pub->z->sock, &pub->z->wbuf, &msg) == 0) 
      return 0;
    else
    {
      Z_DEBUG("Trying to reconnect....\n");
      pub->z->on_disconnect(pub->z);
      return z_send_msg(pub->z->sock, &pub->z->wbuf, &msg);
    }
  }
  return 0;
}


int 
z_stream_data_wo(z_pub_t *pub, const unsigned char *data, size_t length, uint8_t encoding, uint8_t kind) {
  const char *rname = z_get_resource_name(pub->z, pub->rid);
  z_resource_id_t rid;
  z_subscription_t *sub;
  z_list_t *subs = z_list_empty;
  z_list_t *xs;
  if (rname != 0) {
    subs = z_get_subscriptions_by_rname(pub->z, rname);
    rid.kind = Z_STR_RES_ID;
    rid.id.rname = (char *)rname;
  } else {
    subs = z_get_subscriptions_by_rid(pub->z, pub->rid);
    rid.kind = Z_INT_RES_ID;
    rid.id.rid = pub->rid;
  }

  if (subs != 0) {    
    z_data_info_t info;
    info.flags = Z_ENCODING | Z_KIND;
    info.encoding = encoding;
    info.kind = kind;  
    xs = subs;
    while (xs != z_list_empty) {
      sub = z_list_head(xs);
      sub->callback(rid, data, length, info);
      xs = z_list_tail(xs);
    }
    z_list_free(&subs);    
  }
  if (z_matching_remote_sub(pub->z, pub->rid) == 1) {
    z_payload_header_t ph;
    ph.flags = Z_ENCODING | Z_KIND;
    ph.encoding = encoding;
    ph.kind = kind;
    ph.payload = z_iobuf_wrap_wo((unsigned char *)data, length, 0, length);
    z_iobuf_t buf = z_iobuf_make(length + 32 );
    z_payload_header_encode(&buf, &ph);

    z_message_t msg;
    msg.header = Z_STREAM_DATA;
    // No need to take ownership, avoid strdup.
    msg.payload.stream_data.rid = pub->rid;      
    msg.payload.stream_data.sn = pub->z->sn++;
    msg.payload.stream_data.payload_header = buf;
    if (z_send_msg(pub->z->sock, &pub->z->wbuf, &msg) == 0) {
      z_iobuf_free(&buf);
      return 0;
    }
    else
    {
      Z_DEBUG("Trying to reconnect....\n");
      pub->z->on_disconnect(pub->z);
      int rv = z_send_msg(pub->z->sock, &pub->z->wbuf, &msg);
      z_iobuf_free(&buf);
      return rv;
    }
  } else {
    printf("No remote subscription matching for rid = %zu\n", pub->rid);
  }
    
  return 0;
}

int z_stream_data(z_pub_t *pub, const unsigned char *data, size_t length) {
  return z_stream_data_wo(pub, data, length, 0, 0); 
}

int z_write_data_wo(z_zenoh_t *z, const char* resource, const unsigned char *payload, size_t length, uint8_t encoding, uint8_t kind) { 
  z_list_t *subs = z_get_subscriptions_by_rname(z, resource);
  z_subscription_t *sub;
  z_resource_id_t rid;
  rid.kind = Z_STR_RES_ID;
  rid.id.rname = (char *)resource;
  z_data_info_t info;
  info.flags = Z_ENCODING | Z_KIND;
  info.encoding = encoding;
  info.kind = kind;  
  while (subs != 0) {
    sub = z_list_head(subs);
    sub->callback(rid, payload, length, info);
    subs = z_list_tail(subs);
  }
  z_payload_header_t ph;
  ph.flags = Z_ENCODING | Z_KIND;
  ph.encoding = encoding;
  ph.kind = kind;
  ph.payload = z_iobuf_wrap((unsigned char *)payload, length);
  z_iobuf_t buf = z_iobuf_make(length + 32 );
  z_payload_header_encode(&buf, &ph);

  z_message_t msg;
  msg.header = Z_WRITE_DATA;
  // No need to take ownership, avoid strdup.
  msg.payload.write_data.rname = (char*) resource;      
  msg.payload.write_data.sn = z->sn++;
  msg.payload.write_data.payload_header = buf;
  if (z_send_msg(z->sock, &z->wbuf, &msg) == 0) {
    z_iobuf_free(&buf);
    return 0;
  }
  else { 
    Z_DEBUG("Trying to reconnect....\n");
    z->on_disconnect(z);
    int rv = z_send_msg(z->sock, &z->wbuf, &msg);
    z_iobuf_free(&buf);
    return rv;    
  }
  
}

int z_write_data(z_zenoh_t *z, const char* resource, const unsigned char *payload, size_t length) {
  return z_write_data_wo(z, resource, payload, length, 0, 0);
}

int z_query(z_zenoh_t *z, const char* resource, const char* predicate, z_reply_callback_t *callback) { 
  z_message_t msg;
  msg.header = Z_QUERY;
  msg.payload.query.pid = z->pid;
  msg.payload.query.qid = z->qid++;
  msg.payload.query.rname = (char *)resource;
  msg.payload.query.predicate = (char *)predicate;
  
  if (z_send_msg(z->sock, &z->wbuf, &msg) != 0) {
    Z_DEBUG("Trying to reconnect....\n");
    z->on_disconnect(z);
    z_send_msg(z->sock, &z->wbuf, &msg);
  }
  z_register_query(z, msg.payload.query.qid, callback);
  return 0;
}
