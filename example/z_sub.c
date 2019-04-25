#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"

void listener(uint8_t mid, z_vle_t rid, z_iobuf_t data) {  
  z_payload_header_result_t r;
  z_string_result_t r_s;
  switch (mid) {
    case Z_STREAM_DATA:      
      z_payload_header_decode_na(&data, &r);
      if (r.tag == Z_OK_TAG) {        
        r_s = z_string_decode(&r.value.payload_header.payload);
        if (r_s.tag == Z_OK_TAG) {
          printf(">>: %s\n", r_s.value.string);
        }
        z_iobuf_free(&r.value.payload_header.payload);
        z_iobuf_free(&data);
      }

      break;
    case Z_COMPACT_DATA:      
      r_s = z_string_decode(&data);
      if (r_s.tag == Z_OK_TAG) {
        printf(">>: %s\n", r_s.value.string);
      }
      break;
    default:
      printf(">>> Got unsupported data message with id %d!\n", mid);
      break;
  }
  
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
  z_vle_result_t r_rid = z_declare_resource(&z, "/demo/hello");
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