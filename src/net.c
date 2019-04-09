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

#include "zenoh/net.h"


int open_tx_session(char *locator) {
  printf("Connecting to: %s:\n", locator);
  char *tx = strtok(locator, "/");  
  assert(strcmp(tx, "tcp") == 0);
  char *addr = strtok(NULL, ":");  
  char *s_port = strtok(NULL, ":");    
  
  int port;
  sscanf(s_port, "%d", &port);    
  int sock;

  printf("Connecting to: %s:%d\n", addr, port);

  struct sockaddr_in serv_addr;  
  
  sock = socket(PF_INET, SOCK_STREAM, 0);
  int set = 1;
#if defined (ZENOH_MACOS)
  setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif
 
  if (sock < 0) {    
    return sock;
  }

  bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	if(inet_pton(AF_INET, addr, &serv_addr.sin_addr)<=0)
	{
    return -1;
	}
	
	if( connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    return -1;		
	
  return sock;
  
}

int z_send_buf(int sock, z_iobuf_t* buf) {
  int len =  z_iobuf_readable(buf);
  uint8_t *ptr = buf->buf;  
  int n = len;
  int wb;
  do {
    printf("Sending data on socket....\n");    
    wb = send(sock, ptr, n, MSG_NOSIGNAL);       
    printf("Socket returned: %d\n", wb);    
    if (wb <= 0) {
      printf("Broker closed connection.... [%d]\n", wb);
      return -1;
    }
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
  return z_send_buf(sock, buf);  
}

z_vle_result_t
z_recv_vle(int sock) {
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

z_message_p_result_t
z_recv_msg(int sock, z_iobuf_t* buf) {   
  z_iobuf_clear(buf);
  printf(">> recv_msg\n"); 
  z_message_p_result_t r;
  z_message_p_result_init(&r);
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