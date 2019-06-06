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

// z_zenoh_t*
// z_open_ptr(char* locator) {
//   z_zenoh_result_t  *rp = (z_zenoh_result_t*)malloc(sizeof(z_zenoh_result_t ));  
//   *rp = z_open(locator, 0);
//   if (rp->tag == Z_OK_TAG)
//     return &rp->value.zenoh;
//   else {
//     free (rp);
//     return 0;
//   }
// }


z_zenoh_p_result_t 
z_open(char* locator, on_disconnect_t *on_disconnect) {
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

  msg.header = Z_OPEN;
  msg.payload.open.version = ZENOH_PROTO_VERSION;
  msg.payload.open.pid = pid;
  msg.payload.open.lease = ZENOH_DEFAULT_LEASE;
  msg.properties = 0;

  Z_DEBUG("Sending Open\n");
  z_send_msg(r_sock.value.socket, &r.value.zenoh->wbuf, &msg);
  z_iobuf_clear(&r.value.zenoh->rbuf);
  z_message_p_result_t r_msg = z_recv_msg(r_sock.value.socket, &r.value.zenoh->rbuf);
  ASSERT_P_RESULT(r_msg, "Failed to receive accept");

  r.value.zenoh->sock = r_sock.value.socket;
  r.value.zenoh->pid = pid;  
  r.value.zenoh->declarations = z_list_empty;
  r.value.zenoh->subscriptions = z_list_empty;
  r.value.zenoh->replywaiters = z_list_empty;
  r.value.zenoh->reply_msg_mvar = z_mvar_empty();

  if (on_disconnect != 0)
    r.value.zenoh->on_disconnect = on_disconnect;  
  else 
    r.value.zenoh->on_disconnect = &default_on_disconnect;  
  
  return r;
}

void z_close(z_zenoh_t* z) { }

z_sub_p_result_t
z_declare_subscriber(z_zenoh_t *z, const char *resource,  z_sub_mode_t sm, subscriber_callback_t *callback) {
  z_sub_p_result_t r;
  r.tag = Z_OK_TAG;
  r.value.sub = (z_sub_t*)malloc(sizeof(z_sub_t));
  r.value.sub->z = z;
  r.value.sub->id = z_get_entity_id(z);

  z_message_t *msg = (z_message_t *)malloc(sizeof(z_message_t));
  z_message_p_result_t r_msg;
  z_message_p_result_init(&r_msg);

  msg->header = Z_DECLARE;
  msg->payload.declare.sn = z->sn++;
  int dnum = 3;
  z_array_declaration_t decl = {dnum, (z_declaration_t*)malloc(dnum*sizeof(z_declaration_t))};
  
  
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
  
  msg->payload.declare.declarations = decl;  
  z_send_msg(z->sock, &z->wbuf, msg);  
  z_register_res_decl(z, rid, resource);
  z_register_subscription(z, rid, callback);
  free(msg);
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

  z_message_t *msg = (z_message_t *)malloc(sizeof(z_message_t));
  z_message_p_result_t r_msg;
  z_message_p_result_init(&r_msg);

  msg->header = Z_DECLARE;
  msg->payload.declare.sn = z->sn++;
  int dnum = 3;
  z_array_declaration_t decl = {dnum, (z_declaration_t*)malloc(dnum*sizeof(z_declaration_t))};
    
  int rid = z_get_resource_id(z, resource);
  r.value.pub->rid = rid;
  
  decl.elem[0].header = Z_RESOURCE_DECL;
  decl.elem[0].payload.resource.r_name = (char*)resource;  
  decl.elem[0].payload.resource.rid = rid;
  
  decl.elem[1].header = Z_PUBLISHER_DECL;
  decl.elem[1].payload.pub.rid = rid;
  
  decl.elem[2].header = Z_COMMIT_DECL;
  decl.elem[2].payload.commit.cid = z->cid++;
  
  msg->payload.declare.declarations = decl;  
  z_send_msg(z->sock, &z->wbuf, msg);  
  z_register_res_decl(z, rid, resource);
  free(msg);
  // -- This will be refactored to use mvars
  return r;
#if 0
  r_msg = z_recv_msg(z->sock, &z->rbuf);
  ASSERT_P_RESULT(r_msg, "Failed to receive message");
  
  if (Z_MID(r_msg.value.message->header) == Z_DECLARE) {
    Z_DEBUG ("Declaration was accepted");    
    return 0;
  }
  else return -1;
#endif 
  
}

int z_stream_compact_data(z_pub_t *pub, const z_iobuf_t *payload) { 
  z_message_t msg;
  msg.header = Z_COMPACT_DATA;
  
  msg.payload.compact_data.rid = pub->rid;    
  msg.payload.compact_data.payload = *payload;  
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

int 
z_stream_data_with_header(z_pub_t *pub, const z_iobuf_t *payload_header) {
  z_message_t msg;
  msg.header = Z_STREAM_DATA;
  msg.payload.stream_data.rid = pub->rid;      
  msg.payload.stream_data.sn = pub->z->sn++;
  msg.payload.stream_data.payload_header = *payload_header;
  if (z_send_msg(pub->z->sock, &pub->z->wbuf, &msg) == 0) 
    return 0;
  else
  {
    Z_DEBUG("Trying to reconnect....\n");
    pub->z->on_disconnect(pub->z);
    return z_send_msg(pub->z->sock, &pub->z->wbuf, &msg);
  }
}


int 
z_stream_data_wo(z_pub_t *pub, const z_iobuf_t *data, uint8_t encoding, uint8_t kind) {
  z_payload_header_t ph;
  ph.flags = Z_ENCODING | Z_KIND;
  ph.encoding = encoding;
  ph.kind = kind;
  ph.payload = *data;
  z_iobuf_t buf = z_iobuf_make(z_iobuf_readable(data) + 32 );
  z_payload_header_encode(&buf, &ph);
  int rv = z_stream_data(pub, &buf);
  z_iobuf_free(&buf);
  return rv;
}

int z_stream_data(z_pub_t *pub, const z_iobuf_t *data) {
  return z_stream_data_wo(pub, data,  0, 0); 
}

int z_write_data_wo(z_zenoh_t *z, const char* resource, const z_iobuf_t *payload, uint8_t encoding, uint8_t kind) { 
  z_payload_header_t ph;
  ph.flags = Z_ENCODING | Z_KIND;
  ph.encoding = encoding;
  ph.kind = kind;
  ph.payload = *payload;
  z_iobuf_t buf = z_iobuf_make(z_iobuf_readable(payload) + 32 );
  z_payload_header_encode(&buf, &ph);

  z_message_t msg;
  msg.header = Z_WRITE_DATA;
  // No need to take ownership, avoid strdup.
  msg.payload.write_data.rname = (char*) resource;      
  msg.payload.stream_data.sn = z->sn++;
  msg.payload.stream_data.payload_header = buf;
  if (z_send_msg(z->sock, &z->wbuf, &msg) == 0) 
    return 0;
  else
  {
    Z_DEBUG("Trying to reconnect....\n");
    z->on_disconnect(z);
    return z_send_msg(z->sock, &z->wbuf, &msg);
  }
}

int z_write_data(z_zenoh_t *z, const char* resource, const z_iobuf_t *payload) {
  return z_write_data_wo(z, resource, payload, 0, 0);
}
int z_query(z_zenoh_t *z, const char* resource, const char* predicate, reply_callback_t *callback) { 
  z_message_t *msg = (z_message_t *)malloc(sizeof(z_message_t));
  z_message_p_result_t r_msg;
  z_message_p_result_init(&r_msg);

  msg->header = Z_QUERY;
  msg->payload.query.pid = z->pid;
  msg->payload.query.qid = z->qid++;
  // No need to take ownership --  avoid strdup.
  msg->payload.query.rname = (char *)resource;
  msg->payload.query.predicate = (char *)predicate;
  
  z_send_msg(z->sock, &z->wbuf, msg);  
  z_register_query(z, msg->payload.query.qid, callback);
  free(msg);
  // -- This will be refactored to use mvars
  return 0;
}
