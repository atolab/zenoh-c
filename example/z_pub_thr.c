#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"



int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc > 1) {
    locator = argv[1];
  }

  Z_DEBUG_VA("Connecting to %s...\n", locator);
  z_zenoh_result_t r_z = z_open(locator, 0);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  zenoh_t z = r_z.value.zenoh;

  Z_DEBUG(">>>>> Declaring Resource...\n");
  z_vle_result_t r_rid = z_declare_resource(&z, "/home1");
  ASSERT_RESULT(r_rid, "Unable to register result")
  z_vle_t rid = r_rid.value.vle;

  Z_DEBUG("<<<<< DONE Declaring Resource...\n");

  Z_DEBUG(">>>>> Declaring Publisher...\n");
  if (z_declare_publisher(&z, rid) != 0) {
    Z_ERROR("Unable to declare pub for resource %llu\n", rid);
    return -1;
  }
  Z_DEBUG("<<<<< DONE Declaring Pub...\n");
  z_iobuf_t sdata = z_iobuf_make(256);
  char *str = "- Hello World -!";  
  z_string_encode(&sdata, str);
  z_array_uint8_t bs = z_iobuf_to_array(&sdata);    
  
  printf("Streaming Data...\n");
  while (1) {    
    // printf(".\n");
    z_compact_data(&z, rid, &bs);
    // sleep(1);
  }

  return 0;
}