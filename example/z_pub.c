#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"

int main(int argc, char **argv) {
  char *locator = "tcp/127.0.0.1:7447";
  if (argc > 1) {
    locator = argv[1];
  }

  z_zenoh_result_t r_z = z_open(locator, 0);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  zenoh_t z = r_z.value.zenoh;

  z_vle_result_t r_rid = z_declare_resource(&z, "/demo/sensor/temp");
  ASSERT_RESULT(r_rid, "Unable to register result")
  z_vle_t rid = r_rid.value.vle;

  if (z_declare_publisher(&z, rid) != 0) {
    printf("Unable to declare pub\n");
    return -1;
  }
  
  z_iobuf_t sdata = z_iobuf_make(256);
  char *str = "Hello World!";  
  z_string_encode(&sdata, str);
  z_array_uint8_t bs = z_iobuf_to_array(&sdata);    
  while (1) {
    z_stream_data(&z, rid, &bs);
    sleep(1);
  }

  return 0;
}
// int send_buf(int sock, z_iobuf_t* buf) {
//   int len =  z_iobuf_readable(buf);
//   uint8_t *ptr = buf->buf;  
//   int n = len;
//   int wb;
//   do {    
//     wb = send(sock, ptr, n, 0);       
//     if (wb == 0) return -1;
//     n -= wb;
//     ptr = ptr + (len - n);
//   } while (n > 0);
//   return 0;
// }

// int recv_n(int sock, z_iobuf_t* buf, size_t len) {    
//   uint8_t *ptr = buf->buf;  
//   int n = len;
//   int rb;
//   do {    
//     rb = recv(sock, ptr, n, 0);       
//     if (rb == 0) return -1;
//     n -= rb;
//     ptr = ptr + (len - n);
//   } while (n > 0);
//   return 0;
// }

// size_t 
// send_msg(int sock, z_iobuf_t* buf, z_message_t* m) {
//   printf(">> send_msg\n");
//   z_iobuf_clear(buf);
//   printf(">> \t z_message_encode\n");
//   z_message_encode(buf, m);
//   z_iobuf_t l_buf = z_iobuf_make(10);
//   z_vle_t len =  z_iobuf_readable(buf);
//   z_vle_encode(&l_buf, len); 
//   send_buf(sock, &l_buf);
//   z_iobuf_free(&l_buf);
//   printf(">> Message encoded is %llu bytes", len);  
//   uint8_t *ptr = buf->buf;  
//   int n = len;
//   int sb;
//   do {   
//     sb = send(sock, ptr, n, 0);       
//     n -= sb;
//     ptr = ptr + (len - n);
//   } while ((n > 0) || (sb == 0));
  
//   if (sb == 0) return 0; 
//   else return len;
// }


// z_vle_result_t
// recv_vle(int sock) {
//   printf(">> recv_vle\n");
//   z_vle_result_t r;
//   uint8_t buf[10];
//   int n;  
//   int i = 0;
//   do {
//     n = recv(sock, &buf[i], 1, 0);
//     printf(">> recv_vle [%d] : 0x%x\n", i, buf[i]);
//     i++;        
//   } while ((buf[i] > 0x7f) && (n != 0) && (i < 10));

//   if (n == 0 || i > 10) {
//     r.tag = Z_ERROR_TAG;
//     r.value.error = Z_VLE_PARSE_ERROR;
//     return r;
//   }
//   z_iobuf_t iobuf;
//   iobuf.capacity = 10;
//   iobuf.r_pos = 0;
//   iobuf.w_pos = i;
//   iobuf.buf = buf;
//   printf(">> \tz_vle_decode\n");
//   return z_vle_decode(&iobuf);
// }

// z_message_result_t
// recv_msg(int sock, z_iobuf_t* buf) {   
//   z_iobuf_clear(buf);
//   printf(">> recv_msg\n"); 
//   z_message_result_t r;
//   r.tag = Z_ERROR_TAG;
//   z_vle_result_t r_vle = recv_vle(sock);  
//   ASSURE_RESULT(r_vle, r, Z_VLE_PARSE_ERROR)
//   size_t len = r_vle.value.vle;  
//   printf(">> \t msg len = %zu\n", len); 
//   if (z_iobuf_writable(buf) < len) {
//     r.tag = Z_ERROR_TAG;
//     r.value.error = Z_INSUFFICIENT_IOBUF_SIZE;
//     return r;
//   }
//   recv_n(sock, buf, len);
//   buf->r_pos = 0;
//   buf->w_pos = len;
//   printf(">> \t z_message_decode\n"); 
//   return z_message_decode(buf);  
// }

// int main(int argc, char** argv) {  
//   char* broker =  "127.0.0.1";
//   if (argc > 1) {
//     broker = argv[1];
//   }
  
//   int sock;
//   struct sockaddr_in serv_addr;  
//   int port = 7447;
  
//   sock = socket(PF_INET, SOCK_STREAM, 0);
 
//   if (sock < 0) {
//     perror("ERROR Creating Socket");
//     exit(1);
//   }

//   bzero(&serv_addr, sizeof(serv_addr));
// 	serv_addr.sin_family = AF_INET;
// 	serv_addr.sin_port = htons(port);

// 	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
// 	{
// 		printf("\n inet_pton error occured\n");
// 		return 1;
// 	}
	
// 	if( connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
// 	{
// 		printf("\n Error : Connect Failed \n");
// 		return 1;
// 	}
  

//   z_iobuf_t wbuf = z_iobuf_make(1024);
//   z_iobuf_t rbuf = z_iobuf_make(1024);
//   z_vle_t sn = 0;
//   z_vle_t rid = 18;
 

//   Z_ARRAY_S_MAKE(uint8_t, pid, 4); 
//   pid.elem[0] = 0;
//   pid.elem[1] = 1;
//   pid.elem[2] = 2;
//   pid.elem[3] = 3;
//   z_message_t msg;

//   msg.header = Z_OPEN;
//   msg.payload.open.version = 0x01;
//   msg.payload.open.pid = pid;
//   msg.payload.open.lease = 100000;
//   msg.properties = 0;

//   send_msg(sock, &wbuf, &msg);
//   z_message_result_t r_msg = recv_msg(sock, &rbuf);
//   ASSERT_RESULT(r_msg, "Failed to receive accept")
  
//   msg.header = Z_DECLARE;
//   msg.payload.declare.sn = sn++;
//   z_array_declaration_t decl = {3, (z_declaration_t*)malloc(3*sizeof(z_declaration_t))};

//   decl.elem[0].header = Z_RESOURCE_DECL;
//   decl.elem[0].payload.resource.r_name = "/demo/sensor/temp";
//   decl.elem[0].payload.resource.rid = rid;
  
//   decl.elem[1].header = Z_PUBLISHER_DECL;
//   decl.elem[1].payload.pub.rid = rid;
  
//   decl.elem[2].header = Z_COMMIT_DECL;
//   decl.elem[2].payload.commit.cid = 1;
  
//   msg.payload.declare.declarations = decl;
//   z_iobuf_clear(&wbuf);
//   printf(">>> Sending Declare...\n");
//   send_msg(sock, &wbuf, &msg);
//   printf(">>> Waiting for message...\n");
//   r_msg = recv_msg(sock, &rbuf);
//   ASSERT_RESULT(r_msg, "Failed to receive accept")
//   if (Z_MID(r_msg.value.message.header) == Z_DECLARE) {
//     printf ("Declaration was accepted");
//   }

//   msg.header = Z_STREAM_DATA;
//   msg.payload.stream_data.rid = rid;
//   z_iobuf_t sdata = z_iobuf_make(256);
//   char *str = "Hello World!";  
//   z_string_encode(&sdata, str);
//   z_array_uint8_t bs = z_iobuf_to_array(&sdata);  
//   msg.payload.stream_data.payload = bs;
//   while (1) {
//     msg.payload.stream_data.sn = sn++;
//     send_msg(sock, &wbuf, &msg);
//     sleep(1);
//   }

//   return 0;
// }