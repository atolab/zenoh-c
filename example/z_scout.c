#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"

int main() {
  z_scout("auto", 2, 500000);
  return 0;
}
