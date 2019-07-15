#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"
#include "yaks.h"

#include "zenoh/codec.h"

void listener(const z_resource_id_t *rid, const unsigned char *data, size_t length, const z_data_info_t *info, void *arg) {    
  Z_UNUSED_ARG_2(info, arg);  
  z_iobuf_t buf = z_iobuf_wrap((unsigned char *)data, length);
  z_string_result_t r_s = z_string_decode(&buf);        
  if (r_s.tag == Z_OK_TAG) {
    if (rid->kind == Z_INT_RES_ID) 
      printf(">>: (%zu, %s)\n", rid->id.rid, r_s.value.string);
    else
      printf(">>: (%s, %s)\n", rid->id.rname, r_s.value.string);
    free(r_s.value.string);
  } else {
    printf(">>: Error decoding string\n");
  }
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
  z_zenoh_p_result_t r_z = z_open(locator, 0, 0);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  z_zenoh_t *z = r_z.value.zenoh;

  z_start_recv_loop(z);
  printf("Declaring Subscriber: %s\n", se);
  y_subscribe(z, se, listener, NULL);
  sleep(60000);
  return 0;
}
