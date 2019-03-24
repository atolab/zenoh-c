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
  
  z_iobuf_t* wbuf = z_iobuf_make(1024);
  z_iobuf_t* rbuf = z_iobuf_make(1024);

  Z_ARRAY_S_MAKE(uint8_t, pid, 4); 
  pid.elem[0] = 0;
  pid.elem[1] = 1;
  pid.elem[2] = 2;
  pid.elem[3] = 3;
  z_open_t m_open = {Z_OPEN, 0x01, pid, 100000,  0};

  z_vle_encode(wbuf, 10);
  z_open_encode(wbuf, &m_open);
  printf("Encoded Open:\n");
  for (int i = 0; i < wbuf->w_pos; ++i) {
    printf("0x%x ", wbuf->buf[i]);
  }
  printf("\n");
  int n =  z_iobuf_readable(wbuf);
  printf("Sending Open of %d bytes\n", n);
  
  int sb = send(sock, wbuf->buf, n, 0);   
  printf("Sent  %d bytes\n", sb);  

  int rb = recv(sock, rbuf->buf, 1, 0);
  printf("Received %d bytes\n", rb);  
  rbuf->w_pos = rb;
  z_vle_result_t r_len = z_vle_decode(rbuf);
  if (r_len.tag == VLE) {
    printf("Message Length: %llu\n", r_len.value.vle);   
    z_iobuf_clear(rbuf);
    int rb = recv(sock, rbuf->buf, r_len.value.vle, 0);
    printf("Received %d bytes\n", rb);  
    rbuf->w_pos = rb;
    uint8_t h = z_iobuf_read(rbuf);
    printf("Message header: 0x%x\n", h);
    z_accept_result_t ra = z_accept_decode(rbuf, h);
    if (ra.tag == ACCEPT) {
      printf("Session successfully opened\n");   
    }
  }

  
  sleep(2);  
  return 0;
}