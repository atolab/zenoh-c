#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"
void listener(z_resource_id_t rid, z_iobuf_t data, z_data_info_t info) {    
  z_string_result_t r_s = z_string_decode(&data);        
  if (r_s.tag == Z_OK_TAG) {
    if (rid.kind == Z_INT_RES_ID) 
      printf(">>: (%llu, %s)\n", rid.id.rid, r_s.value.string);
    else
      printf(">>: (%s, %s)\n", rid.id.rname, r_s.value.string);
    free(r_s.value.string);
  }
  z_iobuf_free(&data);
}

int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc > 1) {
    locator = argv[1];
  }

  printf("Connecting to %s...\n", locator);
  z_zenoh_result_t r_z = z_open(locator, 0);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  zenoh_t z = r_z.value.zenoh;

  z_start_recv_loop(&z);
  printf("Declaring Resource...\n");
  z_vle_result_t r_rid = z_declare_resource(&z, "/demo/hello/hello");
  r_rid = z_declare_resource(&z, "/demo/hello/hello/lello");
  r_rid = z_declare_resource(&z, "/demo/hello/*");
  ASSERT_RESULT(r_rid, "Unable to register result")
  z_vle_t rid = r_rid.value.vle;

  printf("Declaring Subscriber...\n");
  z_sub_mode_t sm;
  sm.kind = Z_PUSH_MODE;
  if (z_declare_subscriber(&z, rid, sm, listener) != 0) {
    printf("Unable to declare pub\n");
    return -1;
  }
  sleep(60000);
  return 0;
}