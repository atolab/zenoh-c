#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"



int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc > 1) {
    locator = argv[1];
  }

  printf("Connecting to %s...\n", locator);
  z_zenoh_result_t r_z = z_open(locator, 0);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  zenoh_t z = r_z.value.zenoh;

  printf("Declaring Resource...\n");
  z_vle_result_t r_rid = z_declare_resource(&z, "/demo/hello");
  ASSERT_RESULT(r_rid, "Unable to register result")
  z_vle_t rid = r_rid.value.vle;

  printf("Declaring Publisher...\n");
  if (z_declare_publisher(&z, rid) != 0) {
    printf("Unable to declare pub\n");
    return -1;
  }
  
  z_iobuf_t sdata = z_iobuf_make(256);
  char *str = "Hello World!";  
  z_string_encode(&sdata, str);
  
  z_payload_header_t ph;
  ph.flags = 0;
  printf("Streaming Data...\n");
  while (1) {    
    z_stream_data(&z, rid, sdata);   
    sleep(1);
  }

  return 0;
}