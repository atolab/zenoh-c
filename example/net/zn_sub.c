#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"

#define MAX_LEN 256

void data_handler(const zn_resource_id_t *rid, const unsigned char *data, size_t length, const zn_data_info_t *info, void *arg) {
  Z_UNUSED_ARG_2(info, arg);
  char str[MAX_LEN];
  memcpy(&str, data, length < MAX_LEN ? length : MAX_LEN - 1);
  str[length < MAX_LEN ? length : MAX_LEN - 1] = 0;
  if (rid->kind == ZN_INT_RES_ID) 
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
  zn_session_p_result_t r_z = zn_open(locator, 0, 0);
  ASSERT_RESULT(r_z, "Unable to open session.\n")
  zn_session_t *z = r_z.value.session;
  zn_start_recv_loop(z);

  printf("Declaring Subscriber on '%s'...\n", uri);
  zn_sub_mode_t sm;
  sm.kind = ZN_PUSH_MODE;
  zn_sub_p_result_t r = zn_declare_subscriber(z, uri, &sm, data_handler, NULL);
  ASSERT_P_RESULT(r,"Unable to declare subscriber.\n");
  zn_sub_t *sub = r.value.sub;

  char c = 0;
  while (c != 'q') {
    c = fgetc(stdin);
  }

  zn_undeclare_subscriber(sub);
  zn_close(z);
  zn_stop_recv_loop(z);
  return 0;
}
