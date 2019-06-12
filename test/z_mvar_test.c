#include "zenoh/mvar.h"
#include <pthread.h>

void *sleep_and_fill(void *m) {
  z_mvar_t *mv = (z_mvar_t*)m;
  sleep(5);
  z_mvar_put(mv, "filled now!");
}
int main(int argc, char **argv) {
  z_mvar_t *mv = z_mvar_empty();
  char *msg = z_mvar_get(mv);
  printf("MVar content: %s", msg);
  return 0;
}

