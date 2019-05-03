#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"



int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc > 1) {
    locator = argv[1];
  }

  z_zenoh_result_t r_z = z_open(locator, 0);
  zenoh_t z = r_z.value.zenoh;

  z_vle_result_t r_rid = z_declare_resource(&z, "/home1");
  z_vle_t rid = r_rid.value.vle;

  if (z_declare_publisher(&z, rid) != 0) {
    printf("Unable to declare pub for resource %llu\n", rid);
    return -1;
  }  
  z_iobuf_t sdata = z_iobuf_make(256);
  char *str = "- Hello World -!";  
  z_string_encode(&sdata, str);  
  
  while (1) {      
    z_compact_data(&z, rid, &sdata);    
  }

  return 0;
}