#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "yaks.h"


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
  
  z_iobuf_t sdata = z_iobuf_make(256);
  char *str = "Hello World!";  
  z_string_encode(&sdata, str);
  
  z_iobuf_t phbuf = z_iobuf_make(256);
  z_payload_header_t ph;
  ph.flags = 0;
  ph.payload = sdata;
  z_payload_header_encode(&phbuf, &ph);
  printf("Streaming Data...\n");
  int i = 0;
  while (1) {    
    char *key = (char*)malloc(256);
    sprintf(key, "/demo/hello/%d", i);
    i = (i + 1)%10;
    y_put(&z, key, &sdata, Y_RAW_ENC); 
    free(key);  
    sleep(1);
  }

  return 0;
}