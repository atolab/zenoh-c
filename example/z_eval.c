#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"

void query_handler(const char *rname, const char *predicate, replies_sender_t send_replies, void *query_handle, void *arg) {
  Z_UNUSED_ARG(predicate);
  printf("Handling Query: %s\n", rname);

  z_resource_t resource;
  resource.rname = arg;
  resource.data = (const unsigned char *)"\13EVAL_RESULT";
  resource.length = 13;
  resource.encoding = 0;
  resource.kind = 0;
  z_resource_t *p_resource = &resource;
  z_array_resource_t replies = {1, &p_resource};

  send_replies(query_handle, replies);
}

int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc > 1) {
    locator = argv[1];
  }
  char *uri="/demo/eval";
  if (argc > 2) {
    uri = argv[2];
  }

  printf("Connecting to %s...\n", locator);
  z_zenoh_p_result_t r_z = z_open(locator, 0, 0);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  z_zenoh_t *z = r_z.value.zenoh;
  z_start_recv_loop(z);

  printf("Declaring Eval: %s\n", uri);
  z_declare_eval(z, uri, query_handler, uri);

  while (1) { 
    sleep(1);
  }
  return 0;
}
