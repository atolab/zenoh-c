#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"

int main() {    
  z_vec_t locs = z_scout("auto", 2, 500000);  
  for (unsigned int i = 0; i < z_vec_length(&locs); ++i) {
    printf("Locator: %s\n", (char*)z_vec_get(&locs, i));
  }
  return 0;
}
