#include "zenoh.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <assert.h>
#include <uuid/uuid.h>
#include <time.h>

#include "zenoh/msg.h"
#include "zenoh/codec.h"

int z_send_buf(int sock, z_iobuf_t* buf) {
  int len =  z_iobuf_readable(buf);
  uint8_t *ptr = buf->buf;  
  int n = len;
  int wb;
  do {    
    wb = send(sock, ptr, n, 0);       
    if (wb == 0) return -1;
    n -= wb;
    ptr = ptr + (len - n);
  } while (n > 0);
  return 0;
}

int z_recv_n(int sock, z_iobuf_t* buf, size_t len) {    
  uint8_t *ptr = buf->buf;  
  int n = len;
  int rb;
  do {    
    rb = recv(sock, ptr, n, 0);       
    if (rb == 0) return -1;
    n -= rb;
    ptr = ptr + (len - n);
  } while (n > 0);
  return 0;
}

size_t 
z_send_msg(int sock, z_iobuf_t* buf, z_message_t* m) {
  printf(">> send_msg\n");
  z_iobuf_clear(buf);
  printf(">> \t z_message_encode\n");
  z_message_encode(buf, m);
  z_iobuf_t l_buf = z_iobuf_make(10);
  z_vle_t len =  z_iobuf_readable(buf);
  z_vle_encode(&l_buf, len); 
  z_send_buf(sock, &l_buf);
  z_iobuf_free(&l_buf);
  printf(">> Message encoded is %llu bytes", len);  
  uint8_t *ptr = buf->buf;  
  int n = len;
  int sb;
  do {   
    sb = send(sock, ptr, n, 0);       
    n -= sb;
    ptr = ptr + (len - n);
  } while ((n > 0) || (sb == 0));
  
  if (sb == 0) return 0; 
  else return len;
}


z_vle_result_t
z_recv_vle(int sock) {
  printf(">> recv_vle\n");
  z_vle_result_t r;
  uint8_t buf[10];
  int n;  
  int i = 0;
  do {
    n = recv(sock, &buf[i], 1, 0);
    printf(">> recv_vle [%d] : 0x%x\n", i, buf[i]);
    i++;        
  } while ((buf[i] > 0x7f) && (n != 0) && (i < 10));

  if (n == 0 || i > 10) {
    r.tag = Z_ERROR_TAG;
    r.value.error = Z_VLE_PARSE_ERROR;
    return r;
  }
  z_iobuf_t iobuf;
  iobuf.capacity = 10;
  iobuf.r_pos = 0;
  iobuf.w_pos = i;
  iobuf.buf = buf;
  printf(">> \tz_vle_decode\n");
  return z_vle_decode(&iobuf);
}

z_message_result_t
z_recv_msg(int sock, z_iobuf_t* buf) {   
  z_iobuf_clear(buf);
  printf(">> recv_msg\n"); 
  z_message_result_t r;
  r.tag = Z_ERROR_TAG;
  z_vle_result_t r_vle = z_recv_vle(sock);  
  ASSURE_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  size_t len = r_vle.value.vle;  
  printf(">> \t msg len = %zu\n", len); 
  if (z_iobuf_writable(buf) < len) {
    r.tag = Z_ERROR_TAG;
    r.value.error = Z_INSUFFICIENT_IOBUF_SIZE;
    return r;
  }
  z_recv_n(sock, buf, len);
  buf->r_pos = 0;
  buf->w_pos = len;
  printf(">> \t z_message_decode\n"); 
  return z_message_decode(buf);  
}

// -- Some refactoring will be done to support multiple platforms / transports

z_zenoh_result_t 
z_open(char* locator, on_disconnect_t *on_disconnect) {
  z_zenoh_result_t r; 
  srand(clock());

  r.tag = Z_OK_TAG;  
  char *tx = strtok(locator, "/");  
  assert(strcmp(tx, "tcp") == 0);
  char *addr = strtok(NULL, ":");  
  char *s_port = strtok(NULL, ":");    
  fflush(stdout);
  int port;
  sscanf(s_port, "%d", &port);    
  int sock;
  struct sockaddr_in serv_addr;  
  
  sock = socket(PF_INET, SOCK_STREAM, 0);
 
  if (sock < 0) {
    perror("ERROR Creating Socket");
    r.tag = Z_ERROR_TAG;
    r.value.error = Z_IO_ERROR;
    return r;
  }

  bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	if(inet_pton(AF_INET, addr, &serv_addr.sin_addr)<=0)
	{
		printf("\n inet_pton error occured\n");
		r.tag = Z_ERROR_TAG;
    r.value.error = Z_IO_ERROR;
    return r;
	}
	
	if( connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Error : Connect Failed \n");
    r.tag = Z_ERROR_TAG;
    r.value.error = Z_IO_ERROR;
    return r;		
	}

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

  z_send_msg(sock, &r.value.zenoh.wbuf, &msg);
  z_message_result_t r_msg = z_recv_msg(sock, &r.value.zenoh.rbuf);
  ASSERT_RESULT(r_msg, "Failed to receive accept")

  r.value.zenoh.sock = sock;
  r.value.zenoh.on_disconnect = on_disconnect;

  return r;
}

void z_close(zenoh_t* z) { }

z_vle_result_t 
z_declare_resource(zenoh_t *z, const char* resource) {
  z_message_t msg;
  z_message_result_t r_msg;
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
  ASSERT_RESULT(r_msg, "Failed to receive accept")
  if (Z_MID(r_msg.value.message.header) == Z_DECLARE) {    
    r_rid.value.vle = z->rid++;    
  } 
  else {
    r_rid.tag = Z_ERROR_TAG;
    r_rid.value.error = Z_RESOURCE_DECL_ERROR;
  }
  return r_rid;
}

int z_declare_subscriber(zenoh_t *z, z_vle_t rid,  z_sub_mode_t sm, subscriber_callback_t *callback) {
  return 0;
}
int 
z_declare_publisher(zenoh_t *z,  z_vle_t rid) {
  z_message_t msg;
  z_message_result_t r_msg;

  msg.header = Z_DECLARE;
  msg.payload.declare.sn = z->sn++;
  z_array_declaration_t decl = {2, (z_declaration_t*)malloc(2*sizeof(z_declaration_t))};

  
  decl.elem[1].header = Z_PUBLISHER_DECL;
  decl.elem[1].payload.pub.rid = rid;
  
  decl.elem[2].header = Z_COMMIT_DECL;
  decl.elem[2].payload.commit.cid = z->cid++;
  
  msg.payload.declare.declarations = decl;  
  z_send_msg(z->sock, &z->wbuf, &msg);  
  r_msg = z_recv_msg(z->sock, &z->rbuf);
  ASSERT_RESULT(r_msg, "Failed to receive accept")
  
  if (Z_MID(r_msg.value.message.header) == Z_DECLARE) {
    printf ("Declaration was accepted");    
    return 0;
  }
  else return -1;
  
}

void z_stream_data(zenoh_t *z, z_vle_t rid, const z_array_uint8_t *payload) { 
  z_message_t msg;
  msg.header = Z_STREAM_DATA;
  msg.payload.stream_data.rid = rid;    
  msg.payload.stream_data.payload = *payload;  
  msg.payload.stream_data.sn = z->sn++;
  z_send_msg(z->sock, &z->wbuf, &msg);      
}
void z_write_data(zenoh_t *z, const char* resource, const z_array_uint8_t *payload) { }

