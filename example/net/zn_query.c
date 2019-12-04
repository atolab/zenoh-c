#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"

#define MAX_LEN 256

void reply_handler(const zn_reply_value_t *reply, void *arg) {
  Z_UNUSED_ARG(arg);
  char str[MAX_LEN];
  switch (reply->kind) {
    case ZN_STORAGE_DATA: 
    case ZN_EVAL_DATA: 
      memcpy(&str, reply->data, reply->data_length < MAX_LEN ? reply->data_length : MAX_LEN - 1);
      str[reply->data_length < MAX_LEN ? reply->data_length : MAX_LEN - 1] = 0;
      switch (reply->kind) {
        case ZN_STORAGE_DATA: printf(">> [Reply handler] received -Storage Data- ('%s': '%s')\n", reply->rname, str);break;
        case ZN_EVAL_DATA:    printf(">> [Reply handler] received -Eval Data-    ('%s': '%s')\n", reply->rname, str);break;
      }
      break;
    case ZN_STORAGE_FINAL:
      printf(">> [Reply handler] received -Storage Final-\n");
      break;
    case ZN_EVAL_FINAL:
      printf(">> [Reply handler] received -Eval Final-\n");
      break;
    case ZN_REPLY_FINAL:
      printf(">> [Reply handler] received -Reply Final-\n");
      break;
  }
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

  printf("Sending Query '%s'...\n", uri);
  zn_query_dest_t dest_all = {ZN_ALL, 0};
  if (zn_query_wo(z, uri, "", reply_handler, NULL, dest_all, dest_all) != 0) {
    printf("Unable to query.\n");
    return -1;
  }

  sleep(1);

  zn_close(z);
  zn_stop_recv_loop(z);
  return 0;
}
