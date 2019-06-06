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

  printf("Declaring Resource...\n");
  
  z_iobuf_t data = z_iobuf_make(256);
  char *str = "Hello World!";  
  z_string_encode(&data, str);
    
  printf("Streaming Data...\n");
  while (1) {    
    z_write_data(z, "/demo/hello/1", data.buf, z_iobuf_readable(&data));   
    sleep(1);
  }

  return 0;
}