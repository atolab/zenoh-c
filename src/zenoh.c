#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include "zenoh/net.h"
#include "zenoh.h"

// -- Some refactoring will be done to support multiple platforms / transports

void default_on_disconnect(void *vz) { 
  zenoh_t *z = (zenoh_t*)vz;
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

zenoh_t*
z_open_ptr(char* locator) {
  z_zenoh_result_t  *rp = (z_zenoh_result_t*)malloc(sizeof(z_zenoh_result_t ));  
  *rp = z_open(locator, 0);
  if (rp->tag == Z_OK_TAG)
    return &rp->value.zenoh;
  else {
    free (rp);
    return 0;
  }
}


z_zenoh_result_t 
z_open(char* locator, on_disconnect_t *on_disconnect) {
  z_zenoh_result_t r; 
  r.value.zenoh.locator = strdup(locator);
  srand(clock());

  z_socket_result_t r_sock = open_tx_session(locator);    
  if (r_sock.tag == Z_ERROR_TAG) {  
    r.tag = Z_ERROR_TAG;
    r.value.error = Z_IO_ERROR;
    return r;
  }
  

  r.tag = Z_OK_TAG;  
  r.value.zenoh.sock = r_sock.value.socket;
  r.value.zenoh.cid = 0;
  r.value.zenoh.sn = 0;
  r.value.zenoh.rbuf = z_iobuf_make(ZENOH_READ_BUF_LEN);  
  r.value.zenoh.wbuf = z_iobuf_make(ZENOH_WRITE_BUF_LEN);  
  r.value.zenoh.rid = 0;  
  r.value.zenoh.subscriptions = 0;
  r.value.zenoh.declarations = 0;
  r.value.zenoh.reply_msg_mvar = 0;

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
  z_send_msg(r_sock.value.socket, &r.value.zenoh.wbuf, &msg);
  z_iobuf_clear(&r.value.zenoh.rbuf);
  z_message_p_result_t r_msg = z_recv_msg(r_sock.value.socket, &r.value.zenoh.rbuf);
  ASSERT_P_RESULT(r_msg, "Failed to receive accept");

  r.value.zenoh.sock = r_sock.value.socket;
  r.value.zenoh.pid = pid;  
  r.value.zenoh.declarations = z_list_empty;
  r.value.zenoh.subscriptions = z_list_empty;
  r.value.zenoh.reply_msg_mvar = z_mvar_empty();

  if (on_disconnect != 0)
    r.value.zenoh.on_disconnect = on_disconnect;  
  else 
    r.value.zenoh.on_disconnect = &default_on_disconnect;  
  
  return r;
}

void z_close(zenoh_t* z) { }

z_vle_result_t 
z_declare_resource(zenoh_t *z, const char* resource) {
  z_message_t msg;
  z_message_p_result_t r_msg;
  z_message_p_result_init(&r_msg);
  z_vle_result_t r_rid;
  r_rid.tag = Z_OK_TAG;
  msg.header = Z_DECLARE;
  msg.payload.declare.sn = z->sn++;
  z_array_declaration_t decl = {2, (z_declaration_t*)malloc(2*sizeof(z_declaration_t))};

  decl.elem[0].header = Z_RESOURCE_DECL;
  decl.elem[0].payload.resource.r_name = resource;  
  decl.elem[0].payload.resource.rid = z->rid;
    
  decl.elem[1].header = Z_COMMIT_DECL;
  decl.elem[1].payload.commit.cid = z->cid++;
  
  msg.payload.declare.declarations = decl;
  z_iobuf_clear(&z->wbuf);
  Z_DEBUG(">>> Sending Declare...\n");
  z_send_msg(z->sock, &z->wbuf, &msg);
  // This will be refactored to use mvars
  r_rid.value.vle = z->rid++;      
  z_register_res_decl(z, r_rid.value.vle, resource);
  return r_rid;

// -- This will be refactored to use mvars
#if 0
  Z_DEBUG(">>> Waiting for message...\n");
  r_msg = z_recv_msg(z->sock, &z->rbuf);
  ASSERT_P_RESULT(r_msg, "Failed to receive accept");
  if (Z_MID(r_msg.value.message->header) == Z_DECLARE) {    
    r_rid.value.vle = z->rid++;    
  } 
  else if (Z_MID(r_msg.value.message->header) == Z_CLOSE) {
    r_rid.tag = Z_ERROR_TAG;
    r_rid.value.error = r_msg.value.message->payload.close.reason;
  }

  else {
    r_rid.tag = Z_ERROR_TAG;
    r_rid.value.error = Z_RESOURCE_DECL_ERROR;
  }
  return r_rid;
#endif 
}

int z_declare_resource_ir(zenoh_t *z, const char* resource) {
  z_vle_result_t r = z_declare_resource(z, resource);
  if (r.tag == Z_OK_TAG)
    return r.value.vle;
  else 
    return -1;
}
int
z_declare_subscriber(zenoh_t *z, z_vle_t rid,  z_sub_mode_t sm, subscriber_callback_t *callback) {
  z_message_t *msg = (z_message_t *)malloc(sizeof(z_message_t));
  z_message_p_result_t r_msg;
  z_message_p_result_init(&r_msg);

  msg->header = Z_DECLARE;
  msg->payload.declare.sn = z->sn++;
  z_array_declaration_t decl = {2, (z_declaration_t*)malloc(2*sizeof(z_declaration_t))};
  
  decl.elem[0].header = Z_SUBSCRIBER_DECL;
  decl.elem[0].payload.sub.sub_mode = sm;
  decl.elem[0].payload.sub.rid = rid;
  
  decl.elem[1].header = Z_COMMIT_DECL;
  decl.elem[1].payload.commit.cid = z->cid++;
  
  msg->payload.declare.declarations = decl;  
  z_send_msg(z->sock, &z->wbuf, msg);  
  z_register_subscription(z, rid, callback);
  free(msg);
  // -- This will be refactored to use mvars
  return 0;
}

int 
z_declare_publisher(zenoh_t *z,  z_vle_t rid) {
  z_message_t *msg = (z_message_t *)malloc(sizeof(z_message_t));
  z_message_p_result_t r_msg;
  z_message_p_result_init(&r_msg);

  msg->header = Z_DECLARE;
  msg->payload.declare.sn = z->sn++;
  z_array_declaration_t decl = {2, (z_declaration_t*)malloc(2*sizeof(z_declaration_t))};

  
  decl.elem[0].header = Z_PUBLISHER_DECL;
  decl.elem[0].payload.pub.rid = rid;
  
  decl.elem[1].header = Z_COMMIT_DECL;
  decl.elem[1].payload.commit.cid = z->cid++;
  
  msg->payload.declare.declarations = decl;  
  z_send_msg(z->sock, &z->wbuf, msg);  
  free(msg);
  // -- This will be refactored to use mvars
  return 0;
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

int z_stream_compact_data(zenoh_t *z, z_vle_t rid, const z_iobuf_t *payload) { 
  z_message_t msg;
  msg.header = Z_COMPACT_DATA;
  msg.payload.compact_data.rid = rid;    
  msg.payload.compact_data.payload = *payload;  
  msg.payload.compact_data.sn = z->sn++;
  if (z_send_msg(z->sock, &z->wbuf, &msg) == 0) 
    return 0;
  else
  {
    Z_DEBUG("Trying to reconnect....\n");
    z->on_disconnect(z);
    return z_send_msg(z->sock, &z->wbuf, &msg);
  }
}

int 
z_stream_data(zenoh_t *z, z_vle_t rid, const z_iobuf_t *payload_header) {
  z_message_t msg;
  msg.header = Z_STREAM_DATA;
  msg.payload.stream_data.rid = rid;      
  msg.payload.stream_data.sn = z->sn++;
  msg.payload.stream_data.payload_header = *payload_header;
  if (z_send_msg(z->sock, &z->wbuf, &msg) == 0) 
    return 0;
  else
  {
    Z_DEBUG("Trying to reconnect....\n");
    z->on_disconnect(z);
    return z_send_msg(z->sock, &z->wbuf, &msg);
  }
}

int 
z_stream_data_wo(zenoh_t *z, z_vle_t rid, const z_iobuf_t *data, uint8_t encoding, uint8_t kind) {
  z_payload_header_t ph;
  ph.flags = Z_ENCODING | Z_KIND;
  Z_DEBUG_VA("z_stream_data_wo with flags: 0x%x\n", ph.flags);
  Z_DEBUG_VA("z_stream_data_wo data has r_pos = %d w_pos = %d capacity = %d\n", data->r_pos, data->w_pos, data->capacity);
  ph.encoding = encoding;
  ph.kind = kind;
  ph.payload = *data;
  z_iobuf_t buf = z_iobuf_make(z_iobuf_readable(data) + 32 );
  z_payload_header_encode(&buf, &ph);
  int rv = z_stream_data(z, rid, &buf);
  z_iobuf_free(&buf);
  return rv;
}

int z_write_data(zenoh_t *z, const char* resource, const z_iobuf_t *payload_header) { 
  z_message_t msg;
  msg.header = Z_WRITE_DATA;
  msg.payload.write_data.rname = resource;      
  msg.payload.stream_data.sn = z->sn++;
  msg.payload.stream_data.payload_header = *payload_header;
  if (z_send_msg(z->sock, &z->wbuf, &msg) == 0) 
    return 0;
  else
  {
    Z_DEBUG("Trying to reconnect....\n");
    z->on_disconnect(z);
    return z_send_msg(z->sock, &z->wbuf, &msg);
  }
}

