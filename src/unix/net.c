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

#include "zenoh.h"
#include "zenoh/net.h"

z_socket_result_t
open_tx_session(const char *locator) {
  z_socket_result_t r;
  r.tag = Z_OK_TAG;
  char * l = strdup(locator);
  Z_DEBUG_VA("Connecting to: %s:\n", locator);
  char *tx = strtok(l, "/");  
  assert(strcmp(tx, "tcp") == 0);
  char *addr = strdup(strtok(NULL, ":"));  
  char *s_port = strtok(NULL, ":");    
  
  int port;
  sscanf(s_port, "%d", &port);    
  

  Z_DEBUG_VA("Connecting to: %s:%d\n", addr, port);
  free(l);
  struct sockaddr_in serv_addr;  
  
  r.value.socket = socket(PF_INET, SOCK_STREAM, 0);

  if (r.value.socket < 0) {    
    r.tag = Z_ERROR_TAG;
    r.value.error = r.value.socket;
    r.value.socket = 0;
    return r;
  }

#if (ZENOH_MACOS == 1)
  setsockopt(r.value.socket, SOL_SOCKET, SO_NOSIGPIPE, (void *)0, sizeof(int));
#endif
  

  bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	if(inet_pton(AF_INET, addr, &serv_addr.sin_addr)<=0)
	{
    r.tag = Z_ERROR_TAG;
    r.value.error = Z_INVALID_ADDRESS_ERROR;
    r.value.socket = 0;
    free(addr);
    return r;
	} 
  
  free(addr);

	
	if( connect(r.value.socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    r.tag = Z_ERROR_TAG;
    r.value.error = Z_TX_CONNECTION_ERROR;
    r.value.socket = 0;
    return r;
  }
    	
  return r;
  
}

size_t z_iovs_len(struct iovec* iov, int iovcnt) {
  size_t len = 0;
  for (int i = 0; i < iovcnt; ++i) 
    len += iov[i].iov_len;
  return len;
}

int z_compute_remaining(struct iovec* iov, int iovcnt, size_t sent) {  
  size_t idx = 0;
  int i = 0;
  while (idx + iov[i].iov_len <= sent) {    
    idx += sent;
    i += 1;
  } 
  int j = 0; 
  if (idx + iov[i].iov_len > sent) {
    iov[0].iov_base = iov[i].iov_base + (sent - idx - iov[i].iov_len );
    j = 1;
    while (i < iovcnt) {
      iov[j] = iov[i];
      j++;
      i++;
    }
  }
  return j;
}

int z_send_iovec(z_socket_t sock, struct iovec* iov, int iovcnt) {
  size_t len = 0;
  for (int i = 0; i < iovcnt; ++i) 
    len += iov[i].iov_len;
  
  size_t n = writev(sock, iov, iovcnt);
  Z_DEBUG_VA("z_send_iovec sent %zu of %zu bytes \n", n, len);
  while (n < len) {
    iovcnt = z_compute_remaining(iov, iovcnt, n);
    len = z_iovs_len(iov, iovcnt);    
    n = writev(sock, iov, iovcnt);
    Z_DEBUG_VA("z_send_iovec sent %zu of %zu bytes \n", n, len);
    if (n < 0) 
      return -1;    
  }
  return 0;
}
int z_send_buf(z_socket_t sock, z_iobuf_t* buf) {
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

int z_recv_n(z_socket_t sock, z_iobuf_t* buf, size_t len) {    
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
z_send_msg(z_socket_t sock, z_iobuf_t* buf, z_message_t* m) {
  Z_DEBUG(">> send_msg\n");
  z_iobuf_clear(buf);
  Z_DEBUG(">> \t z_message_encode\n");
  z_message_encode(buf, m);
  z_iobuf_t l_buf = z_iobuf_make(10);
  z_vle_t len =  z_iobuf_readable(buf);
  z_vle_encode(&l_buf, len); 
  struct iovec iov[2];
  iov[0].iov_len = z_iobuf_readable(&l_buf);
  iov[0].iov_base = l_buf.buf;
  iov[1].iov_len = len;
  iov[1].iov_base = buf->buf;
  
  return z_send_iovec(sock, iov, 2);
  // z_send_buf(sock, &l_buf);
  // z_iobuf_free(&l_buf);
  // Z_DEBUG_VA(">> Message encoded is %llu bytes", len);    
  // return z_send_buf(sock, buf);  
}

z_vle_result_t
z_recv_vle(z_socket_t sock) {
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

void
z_recv_msg_na(z_socket_t sock, z_iobuf_t* buf, z_message_p_result_t *r) {   
  z_iobuf_clear(buf);
  Z_DEBUG(">> recv_msg\n");   
  r->tag = Z_OK_TAG;
  z_vle_result_t r_vle = z_recv_vle(sock);  
  ASSURE_P_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  size_t len = r_vle.value.vle;  
  Z_DEBUG_VA(">> \t msg len = %zu\n", len); 
  if (z_iobuf_writable(buf) < len) {
    r->tag = Z_ERROR_TAG;
    r->value.error = Z_INSUFFICIENT_IOBUF_SIZE;    
    return;
  }
  z_recv_n(sock, buf, len);
  buf->r_pos = 0;
  buf->w_pos = len;
  Z_DEBUG(">> \t z_message_decode\n"); 
  z_message_decode_na(buf, r);  
}

z_message_p_result_t
z_recv_msg(z_socket_t sock, z_iobuf_t* buf) {   
  z_message_p_result_t r;
  z_message_p_result_init(&r);
  z_recv_msg_na(sock, buf, &r);
  return r;
  // z_iobuf_clear(buf);
  // Z_DEBUG(">> recv_msg\n"); 
  // z_message_p_result_t r;
  // z_message_p_result_init(&r);
  // r.tag = Z_ERROR_TAG;
  // z_vle_result_t r_vle = z_recv_vle(sock);  
  // ASSURE_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
  // size_t len = r_vle.value.vle;  
  // Z_DEBUG_VA(">> \t msg len = %zu\n", len); 
  // if (z_iobuf_writable(buf) < len) {
  //   r.tag = Z_ERROR_TAG;
  //   r.value.error = Z_INSUFFICIENT_IOBUF_SIZE;
  //   return r;
  // }
  // z_recv_n(sock, buf, len);
  // buf->r_pos = 0;
  // buf->w_pos = len;
  // Z_DEBUG(">> \t z_message_decode\n"); 
  // return z_message_decode(buf);  
}