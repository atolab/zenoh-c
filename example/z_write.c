#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"

int main(int argc, char **argv) {
  char *uri = "/demo/example/zenoh-c-write";
  if (argc > 1) {
    uri = argv[1];
  }
  char *value = "Write from C!";
  if (argc > 2) {
    value = argv[2];
  }
  char *locator = 0;
  if (argc > 3) {
    locator = argv[3];
  }

  printf("Openning session...\n");
  z_zenoh_p_result_t r_z = z_open(locator, 0, 0);
  ASSERT_RESULT(r_z, "Unable to open session.\n")
  z_zenoh_t *z = r_z.value.zenoh;

  printf("Writing Data ('%s': '%s')...\n", uri, value);
  z_write_data(z, uri, (const unsigned char *)value, strlen(value));

  z_close(z);
  return 0;
}
