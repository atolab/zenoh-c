#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"



int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc > 1) {
    locator = argv[1];
  }

  z_zenoh_p_result_t r_z = z_open(locator, 0);
  z_zenoh_t *z = r_z.value.zenoh;
  
  z_pub_p_result_t rp = z_declare_publisher(z, "/home1");
  ASSERT_P_RESULT(rp, "Unable to declare publisher");
  z_pub_t *pub = rp.value.pub;

  z_iobuf_t data = z_iobuf_make(256);  
  char *str = "- Hello World -!";  
  z_string_encode(&data, str);  

  
  
  while (1) {      
    z_stream_data(pub, data.buf, z_iobuf_readable(&data));    
  }

  return 0;
}