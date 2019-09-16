#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"

#define MAX_LEN 256

void reply_handler(const z_reply_value_t *reply, void *arg) {
  Z_UNUSED_ARG(arg);
  char str[MAX_LEN];
  switch (reply->kind) {
    case Z_STORAGE_DATA: 
    case Z_EVAL_DATA: 
      memcpy(&str, reply->data, reply->data_length < MAX_LEN ? reply->data_length : MAX_LEN - 1);
      str[reply->data_length < MAX_LEN ? reply->data_length : MAX_LEN - 1] = 0;
      switch (reply->kind) {
        case Z_STORAGE_DATA: printf(">> [Reply handler] received -Storage Data- ('%s': '%s')\n", reply->rname, str);break;
        case Z_EVAL_DATA:    printf(">> [Reply handler] received -Eval Data-    ('%s': '%s')\n", reply->rname, str);break;
      }
      break;
    case Z_STORAGE_FINAL:
      printf(">> [Reply handler] received -Storage Final-\n");
      break;
    case Z_EVAL_FINAL:
      printf(">> [Reply handler] received -Eval Final-\n");
      break;
    case Z_REPLY_FINAL:
      printf(">> [Reply handler] received -Reply Final-\n");
      break;
  }
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
  z_zenoh_p_result_t r_z = z_open(locator, 0, 0);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  z_zenoh_t *z = r_z.value.zenoh;
  z_start_recv_loop(z);

  printf("Sending Query '%s'...\n", uri);
  z_query_dest_t dest_all = {Z_ALL, 0};
  if (z_query_wo(z, uri, "", reply_handler, NULL, dest_all, dest_all) != 0) {
    printf("Unable to query\n");
    return -1;
  }

  sleep(1);

  z_close(z);
  z_stop_recv_loop(z);
  return 0;
}
