#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"

void query_handler(const char *rname, const char *predicate, z_replies_sender_t send_replies, void *query_handle, void *arg) {
  printf(">> [Query handler] Handling '%s?%s'\n", rname, predicate);

  char *data = "Eval from C!";
  z_resource_t resource;
  resource.rname = arg;
  resource.data = (const unsigned char *)data;
  resource.length = strlen(data);
  resource.encoding = 0;
  resource.kind = 0;
  z_resource_t *p_resource = &resource;
  z_array_p_resource_t replies = {1, &p_resource};

  send_replies(query_handle, replies);
}

int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc > 1) {
    locator = argv[1];
  }
  char *uri = "/demo/example/zenoh-c-eval";
  if (argc > 2) {
    uri = argv[2];
  }

  printf("Connecting to %s...\n", locator);
  z_zenoh_p_result_t r_z = z_open(locator, 0, 0);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  z_zenoh_t *z = r_z.value.zenoh;
  z_start_recv_loop(z);

  printf("Declaring Eval on '%s'...\n", uri);
  z_eval_p_result_t r = z_declare_eval(z, uri, query_handler, uri);
  ASSERT_P_RESULT(r, "Unable to declare eval\n");  
  z_eva_t *eval = r.value.eval;

  char c = 0;
  while (c != 'q') {
    c = fgetc(stdin);
  }

  z_undeclare_eval(eval);
  z_close(z);
  z_stop_recv_loop(z);
  return 0;
}
