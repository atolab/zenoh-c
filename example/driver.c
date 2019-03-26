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

#include "zenoh/msg.h"
#include "zenoh/codec.h"

int send_buf(int sock, z_iobuf_t* buf) {
  int len =  z_iobuf_readable(buf);
  uint8_t ptr = buf->buf;  
  int n = len;
  int wb;
  do {    
    wb = send(sock, ptr, n, 0);       
    if (wb = 0) return -1;
    n -= wb;
    ptr = ptr + (len - n);
  } while (n > 0);
  return 0;
}

int recv_n(int sock, z_iobuf_t* buf, size_t len) {    
  uint8_t ptr = buf->buf;  
  int n = len;
  int rb;
  do {    
    rb = recv(sock, ptr, n, 0);       
    if (rb = 0) return -1;
    n -= rb;
    ptr = ptr + (len - n);
  } while (n > 0);
  return 0;
}

size_t send_msg(int sock, z_iobuf_t* buf, z_message_t* m) {
  z_iobuf_clear(buf);
  z_encode_message(buf, m);
  int len =  z_iobuf_readable(buf);
  uint8_t ptr = buf->buf;  
  int n = len;
  do {    
    n -= send(sock, ptr, n, 0);       
    ptr = ptr + (len - n);
  } while (n > 0);
}


z_vle_result_t
recv_vle(int sock) {
  z_vle_result_t r;
  uint8_t buf[10];
  int n;
  int c = 0;
  int i = 0;
  do {
    n = recv(sock, &buf[i], 1, 0);
    i++;    
  } while ((buf[i] > 0x7f) || (n == 0) || (i > 10));

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
  return z_vle_decode(&iobuf);
}
z_message_result_t
recv_msg(int sock, z_iobuf_t* buf) {
  z_message_result_t r; 
  
  z_vle_result_t r_vle = recv_vle(sock);
  ASSURE_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  size_t len = r_vle.value.vle;
  if (z_iobuf_length(buf) < len) {
    r.tag = Z_ERROR_TAG;
    r.value.error = Z_INSUFFICIENT_IOBUF_SIZE;
    return r;
  }
  recv_n(sock, buf->buf, len);
  buf->r_pos = 0;
  buf->w_pos = len;

  
  return r;
}

int main(int argc, char** argv) {
  char* broker =  "127.0.0.1";
  if (argc > 1) {
    broker = argv[1];
  }
  
  int sock;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  int port = 7447;
  
  sock = socket(PF_INET, SOCK_STREAM, 0);
 
  if (sock < 0) {
    perror("ERROR Creating Socket");
    exit(1);
  }

  bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
	{
		printf("\n inet_pton error occured\n");
		return 1;
	}
	
	if( connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Error : Connect Failed \n");
		return 1;
	}
  
  z_iobuf_t wbuf = z_iobuf_make(1024);
  z_iobuf_t rbuf = z_iobuf_make(1024);

  Z_ARRAY_S_MAKE(uint8_t, pid, 4); 
  pid.elem[0] = 0;
  pid.elem[1] = 1;
  pid.elem[2] = 2;
  pid.elem[3] = 3;
  z_open_t m_open = {Z_OPEN, 0x01, pid, 100000,  0};

  z_vle_encode(&wbuf, 10);
  z_open_encode(&wbuf, &m_open);
  printf("Encoded Open:\n");
  for (int i = 0; i < wbuf.w_pos; ++i) {
    printf("0x%x ", wbuf.buf[i]);
  }
  printf("\n");
  int n =  z_iobuf_readable(&wbuf);
  printf("Sending Open of %d bytes\n", n);
  
  int sb = send(sock, wbuf.buf, n, 0);   
  printf("Sent  %d bytes\n", sb);  

  int rb = recv(sock, rbuf.buf, 1, 0);
  printf("Received %d bytes\n", rb);  
  rbuf.w_pos = rb;
  z_vle_result_t r_len = z_vle_decode(&rbuf);
  if (r_len.tag == Z_VLE_TAG) {
    printf("Message Length: %llu\n", r_len.value.vle);   
    z_iobuf_clear(&rbuf);
    int rb = recv(sock, rbuf.buf, r_len.value.vle, 0);
    printf("Received %d bytes\n", rb);  
    rbuf.w_pos = rb;
    uint8_t h = z_iobuf_read(&rbuf);
    printf("Message header: 0x%x\n", h);
    z_accept_result_t ra = z_accept_decode(&rbuf, h);
    if (ra.tag == Z_ACCEPT_TAG) {
      printf("Session successfully opened\n");   
      printf("Declaring publisher\n");   
      z_res_decl_t rdecl = {Z_RESOURCE_DECL, 0, "/demo/sensor/temp"};
      z_pub_decl_t pdecl = {Z_PUBLISHER_DECL, 0};
      z_commit_decl_t cdecl = {Z_COMMIT_DECL, 0};
      z_vec_t ds = z_vec_make(3);
      z_vec_append(&ds, &rdecl);
      z_vec_append(&ds, &pdecl);
      z_vec_append(&ds, &cdecl);
      z_declare_t d = {Z_DECLARE, 0, ds};
      

      
    }
  }

  
  sleep(2);  
  return 0;
}