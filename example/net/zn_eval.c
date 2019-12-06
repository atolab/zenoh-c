#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"

void query_handler(const char *rname, const char *predicate, zn_replies_sender_t send_replies, void *query_handle, void *arg) {
  printf(">> [Query handler] Handling '%s?%s'\n", rname, predicate);

  char *data = "Eval from C!";
  zn_resource_t resource;
  resource.rname = arg;
  resource.data = (const unsigned char *)data;
  resource.length = strlen(data);
  resource.encoding = 0;
  resource.kind = 0;
  zn_resource_t *p_resource = &resource;
  zn_resource_p_array_t replies = {1, &p_resource};

  send_replies(query_handle, replies);
}

int main(int argc, char **argv) {
  char *uri = "/demo/example/zenoh-c-eval";
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

  printf("Declaring Eval on '%s'...\n", uri);
  zn_eval_p_result_t r = zn_declare_eval(z, uri, query_handler, uri);
  ASSERT_P_RESULT(r, "Unable to declare eval.\n");  
  zn_eva_t *eval = r.value.eval;

  char c = 0;
  while (c != 'q') {
    c = fgetc(stdin);
  }

  zn_undeclare_eval(eval);
  zn_close(z);
  zn_stop_recv_loop(z);
  return 0;
}
