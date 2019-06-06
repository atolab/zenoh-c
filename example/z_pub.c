#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"



int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc > 1) {
    locator = argv[1];
  }

  printf("Connecting to %s...\n", locator);
  z_zenoh_p_result_t r_z = z_open(locator, 0);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  z_zenoh_t *z = r_z.value.zenoh;

  
  printf("Declaring Publisher...\n");
  z_pub_p_result_t r = z_declare_publisher(z, "/demo/hello/alpha");
  ASSERT_P_RESULT(r, "Unable to declare pub\n");
  
  z_pub_t *pub = r.value.pub;

  z_iobuf_t sdata = z_iobuf_make(512);
  char *str = "Hello World!";  
  z_string_encode(&sdata, str);
  
  z_iobuf_t phbuf = z_iobuf_make(512);
  z_payload_header_t ph;
  ph.flags = 0;
  ph.payload = sdata;
  z_payload_header_encode(&phbuf, &ph);
  printf("Streaming Data...\n");
  while (1) {    
    z_stream_data(pub, &phbuf);   
    sleep(1);
  }

  return 0;
}