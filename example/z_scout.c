#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"

int main() {    
  printf("Scouting...\n");
  z_vec_t locs = z_scout("auto", 10, 500000);  
  if (z_vec_length(&locs) > 0) {
    for (unsigned int i = 0; i < z_vec_length(&locs); ++i) {
      printf("Locator: %s\n", (char*)z_vec_get(&locs, i));
    }
  } else {
    printf("Did not find any zenoh routers.\n");
  }
  return 0;
}
