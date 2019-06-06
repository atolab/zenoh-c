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

  z_iobuf_t p_buf = z_iobuf_make(256);
  z_iobuf_t ph_buf = z_iobuf_make(512);
  char *str = "- Hello World -!";  
  z_string_encode(&p_buf, str);  

  z_payload_header_t ph;
  ph.flags = 0;
  ph.payload = p_buf;
  z_payload_header_encode(&ph_buf, &ph);
  
  while (1) {      
    z_stream_data(pub, &ph_buf);    
  }

  return 0;
}