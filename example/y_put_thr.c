#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "yaks.h"

#include "zenoh/codec.h"

int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc > 1) {
    locator = argv[2];
  }

  printf("Connecting to %s...\n", locator);
  z_zenoh_p_result_t r_z = z_open(locator, 0, 0);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  z_zenoh_t *z = r_z.value.zenoh;

  
  z_iobuf_t data = z_iobuf_make(256);
  char *str = "Hello World!";  
  z_string_encode(&data, str);
  size_t len = z_iobuf_readable(&data);
  
  
  printf("Streaming Data...\n");
  while (1) {              
    y_put(z, "/perf/put/thr", data.buf, len, Y_RAW_ENC);     
  }

  return 0;
}
