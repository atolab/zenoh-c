#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"

void listener(const z_resource_id_t *rid, const unsigned char *data, size_t length, const z_data_info_t *info, void *arg) {
  Z_UNUSED_ARG_2(info, arg);
  char str[length + 1];
  memcpy(&str, data, length);
  str[length] = 0;
  if (rid->kind == Z_INT_RES_ID) 
    printf(">> [Subscription listener] Received (#%zu: '%s')\n", rid->id.rid, str);
  else
    printf(">> [Subscription listener] Received ('%s': '%s')\n", rid->id.rname, str);
}

int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc > 1) {
    locator = argv[1];
  }
  char *uri = "/demo/example/**";
  if (argc > 2) {
    uri = argv[2];
  }

  printf("Connecting to %s...\n", locator);
  z_zenoh_p_result_t r_z = z_open_wup(locator, "user", "password");
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  z_zenoh_t *z = r_z.value.zenoh;
  z_start_recv_loop(z);

  printf("Declaring Subscriber on '%s'...\n", uri);
  z_sub_mode_t sm;
  sm.kind = Z_PUSH_MODE;
  z_sub_p_result_t r = z_declare_subscriber(z, uri, &sm, listener, NULL);
  ASSERT_P_RESULT(r,"Unable to declare subscriber\n");
  z_sub_t *sub = r.value.sub;

  while (1) { 
    sleep(60);
  }

  z_undeclare_subscriber(sub);
  z_close(z);
  z_stop_recv_loop(z);
  return 0;
}
