#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"

int main(int argc, char **argv) {
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc > 1) {
    locator = argv[1];
  }
  char *uri = "/demo/example/zenoh-c-stream";
  if (argc > 2) {
    uri = argv[2];
  }
  char *value = "Stream from C!";
  if (argc > 3) {
    value = argv[3];
  }

  printf("Connecting to %s...\n", locator);
  z_zenoh_p_result_t r_z = z_open_wup(locator, "user", "password");
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  z_zenoh_t *z = r_z.value.zenoh;
  z_start_recv_loop(z);  
  
  printf("Declaring Publisher on '%s'...\n", uri);
  z_pub_p_result_t r = z_declare_publisher(z, uri);
  ASSERT_P_RESULT(r, "Unable to declare pub\n");  
  z_pub_t *pub = r.value.pub;

  char buf[256];
  for(int idx = 0; 1; ++idx) {
    sleep(1);
    sprintf(buf, "[%4d] %s", idx, value);
    printf("Streaming Data ('%s': '%s')...\n", uri, buf);
    z_stream_data(pub, (const unsigned char *)buf, strlen(buf));
  }

  z_undeclare_publisher(pub);
  z_close(z);
  z_stop_recv_loop(z);
  return 0;
}