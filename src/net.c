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
  Z_DEBUG_VA("Connecting to: %s:\n", locator);
  char *tx = strtok(locator, "/");  
  assert(strcmp(tx, "tcp") == 0);
  char *addr = strtok(NULL, ":");  
  char *s_port = strtok(NULL, ":");    
  
  int port;
  sscanf(s_port, "%d", &port);    
  int sock;

  Z_DEBUG_VA("Connecting to: %s:%d\n", addr, port);

  struct sockaddr_in serv_addr;  
  
  sock = socket(PF_INET, SOCK_STREAM, 0);

#if (ZENOH_MACOS == 1)
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
    Z_DEBUG("Sending data on socket....\n");    
  #if (ZENOH_LINUX == 1)
    wb = send(sock, ptr, n, MSG_NOSIGNAL);       
  #else
    wb = send(sock, ptr, n, 0);       
  #endif 
    Z_DEBUG_VA("Socket returned: %d\n", wb);    
    if (wb <= 0) {
      Z_DEBUG_VA("Broker closed connection.... [%d]\n", wb);
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
  Z_DEBUG(">> send_msg\n");
  z_iobuf_clear(buf);
  Z_DEBUG(">> \t z_message_encode\n");
  z_message_encode(buf, m);
  z_iobuf_t l_buf = z_iobuf_make(10);
  z_vle_t len =  z_iobuf_readable(buf);
  z_vle_encode(&l_buf, len); 
  z_send_buf(sock, &l_buf);
  z_iobuf_free(&l_buf);
  Z_DEBUG_VA(">> Message encoded is %llu bytes", len);    
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
    Z_DEBUG_VA(">> recv_vle [%d] : 0x%x\n", i, buf[i]);
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
  Z_DEBUG(">> \tz_vle_decode\n");
  return z_vle_decode(&iobuf);
}

z_message_p_result_t
z_recv_msg(int sock, z_iobuf_t* buf) {   
  z_iobuf_clear(buf);
  Z_DEBUG(">> recv_msg\n"); 
  z_message_p_result_t r;
  z_message_p_result_init(&r);
  r.tag = Z_ERROR_TAG;
  z_vle_result_t r_vle = z_recv_vle(sock);  
  ASSURE_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  size_t len = r_vle.value.vle;  
  Z_DEBUG_VA(">> \t msg len = %zu\n", len); 
  if (z_iobuf_writable(buf) < len) {
    r.tag = Z_ERROR_TAG;
    r.value.error = Z_INSUFFICIENT_IOBUF_SIZE;
    return r;
  }
  z_recv_n(sock, buf, len);
  buf->r_pos = 0;
  buf->w_pos = len;
  Z_DEBUG(">> \t z_message_decode\n"); 
  return z_message_decode(buf);  
}