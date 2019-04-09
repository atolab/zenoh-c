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
    printf("Tring to reconnect...\n");
    int sock = open_tx_session(strdup(z->locator));
    if (sock > 0) {  
      z->sock = sock;
      return;
    }
  }
}


z_zenoh_result_t 
z_open(char* locator, on_disconnect_t *on_disconnect) {
  z_zenoh_result_t r; 
  r.value.zenoh.locator = strdup(locator);
  srand(clock());

  int sock = open_tx_session(locator);
  if (sock < 0) {
    r.tag = Z_ERROR_TAG;
    r.value.error = Z_IO_ERROR;
    return r;
  }

  r.tag = Z_OK_TAG;  
  r.value.zenoh.sock = sock;
  r.value.zenoh.cid = 0;
  r.value.zenoh.sn = 0;
  r.value.zenoh.rbuf = z_iobuf_make(ZENOH_READ_BUF_LEN);  
  r.value.zenoh.wbuf = z_iobuf_make(ZENOH_WRITE_BUF_LEN);  
  r.value.zenoh.rid = 0;  

  Z_ARRAY_S_MAKE(uint8_t, pid, ZENOH_PID_LENGTH);   
  for (int i = 0; i < ZENOH_PID_LENGTH; ++i) 
    pid.elem[i] = rand() % 255;  

  z_message_t msg;

  msg.header = Z_OPEN;
  msg.payload.open.version = ZENOH_PROTO_VERSION;
  msg.payload.open.pid = pid;
  msg.payload.open.lease = ZENOH_DEFAULT_LEASE;
  msg.properties = 0;

  printf("Sending Open\n");
  z_send_msg(sock, &r.value.zenoh.wbuf, &msg);
  z_message_p_result_t r_msg = z_recv_msg(sock, &r.value.zenoh.rbuf);
  ASSERT_P_RESULT(r_msg, "Failed to receive accept");

  r.value.zenoh.sock = sock;
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
  printf(">>> Sending Declare...\n");
  z_send_msg(z->sock, &z->wbuf, &msg);
  printf(">>> Waiting for message...\n");
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
}

int
z_declare_subscriber(zenoh_t *z, z_vle_t rid,  z_sub_mode_t sm, subscriber_callback_t *callback) {
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
  r_msg = z_recv_msg(z->sock, &z->rbuf);
  ASSERT_P_RESULT(r_msg, "Failed to receive message");
  
  if (Z_MID(r_msg.value.message->header) == Z_DECLARE) {
    printf ("Declaration was accepted");    
    return 0;
  }
  else return -1;
  
}

int z_compact_data(zenoh_t *z, z_vle_t rid, const z_array_uint8_t *payload) { 
  z_message_t msg;
  msg.header = Z_COMPACT_DATA;
  msg.payload.stream_data.rid = rid;    
  msg.payload.stream_data.payload = *payload;  
  msg.payload.stream_data.sn = z->sn++;
  if (z_send_msg(z->sock, &z->wbuf, &msg) == 0) 
    return 0;
  else
  {
    printf("Trying to reconnect....\n");
    z->on_disconnect(z);
    return z_send_msg(z->sock, &z->wbuf, &msg);
  }

}
int z_write_data(zenoh_t *z, const char* resource, const z_array_uint8_t *payload) { 
  return 0;
}

