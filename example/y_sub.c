#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"
#include "yaks.h"

void listener(z_resource_id_t rid, z_iobuf_t data, z_data_info_t info) {    
  z_string_result_t r_s = z_string_decode(&data);        
  if (r_s.tag == Z_OK_TAG) {
    if (rid.kind == Z_INT_RES_ID) 
      printf(">>: (%llu, %s)\n", rid.id.rid, r_s.value.string);
    else
      printf(">>: (%s, %s)\n", rid.id.rname, r_s.value.string);
    free(r_s.value.string);
  } else {
    printf(">>: Error decoding string\n");
  }
  z_iobuf_free(&data);
}

int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  char *se = "/demo/hello/*";
  if (argc > 1) {
    se = argv[1];  
  } 
  if (argc > 2) {
    locator = argv[1];
  }

  printf("Connecting to %s...\n", locator);
  z_zenoh_p_result_t r_z = z_open(locator, 0);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  z_zenoh_t *z = r_z.value.zenoh;

  z_start_recv_loop(z);
  printf("Declaring Resource: %s\n", se);
  y_subscribe(z, se, listener);
  sleep(60000);
  return 0;
}