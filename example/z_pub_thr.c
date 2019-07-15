#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"

#include "zenoh/codec.h"

int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc < 2) {
    printf("USAGE:\n\tz_pub_thr <payload-size> [<zenoh-locator>]\n\n");
    exit(-1);
  }
  size_t len = atoi(argv[1]);  
  printf("Running throughput test for payload of %zu bytes\n", len);
  if (argc > 2) {
    locator = argv[2];
  }  

  z_iobuf_t data = z_iobuf_make(len + 8);
  z_vle_encode(&data, len);
  for (unsigned int i = 0; i < len; ++i) 
    z_iobuf_write(&data, i%10);
  
  
  z_zenoh_p_result_t r_z = z_open(locator, 0, 0);
  z_zenoh_t *z = r_z.value.zenoh;
  z_start_recv_loop(z);  

  z_pub_p_result_t rp = z_declare_publisher(z, "/test/thr");
  ASSERT_P_RESULT(rp, "Unable to declare publisher");
  z_pub_t *pub = rp.value.pub;
    
  while (1) {      
    z_stream_data(pub, data.buf, z_iobuf_readable(&data));    
  }

  return 0;
}
