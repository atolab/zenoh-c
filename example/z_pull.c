#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"

#define MAX_LEN 256

void data_handler(const z_resource_id_t *rid, const unsigned char *data, size_t length, const z_data_info_t *info, void *arg) {
  Z_UNUSED_ARG_2(info, arg);
  char str[MAX_LEN];
  memcpy(&str, data, length < MAX_LEN ? length : MAX_LEN - 1);
  str[length < MAX_LEN ? length : MAX_LEN - 1] = 0;
  if (rid->kind == Z_INT_RES_ID) 
    printf(">> [Subscription listener] Received (#%zu: '%s')\n", rid->id.rid, str);
  else
    printf(">> [Subscription listener] Received ('%s': '%s')\n", rid->id.rname, str);
}

int main(int argc, char **argv) {
  char *uri = "/demo/example/**";
  if (argc > 1) {
    uri = argv[1];
  }
  char *locator = 0;
  if (argc > 2) {
    locator = argv[2];
  }

  printf("Openning session...\n");
  z_zenoh_p_result_t r_z = z_open(locator, 0, 0);
  ASSERT_RESULT(r_z, "Unable to open session.\n")
  z_zenoh_t *z = r_z.value.zenoh;
  z_start_recv_loop(z);

  printf("Declaring Subscriber on '%s'...\n", uri);
  z_sub_mode_t sm;
  sm.kind = Z_PULL_MODE;
  z_sub_p_result_t r = z_declare_subscriber(z, uri, &sm, data_handler, NULL);
  ASSERT_P_RESULT(r,"Unable to declare subscriber.\n");
  z_sub_t *sub = r.value.sub;

  printf("Press <enter> to pull data...\n");
  char c = 0;
  while (c != 'q') {
    c = fgetc(stdin);
    z_pull(sub);
  }

  z_undeclare_subscriber(sub);
  z_close(z);
  z_stop_recv_loop(z);
  return 0;
}
