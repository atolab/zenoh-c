#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"

char *hexdump(z_array_uint8_t array) {
  char *res = malloc((array.length*2+1)*sizeof(char));
  for(unsigned int i = 0; i < array.length; ++i){
    sprintf(res + 2*i, "%02x", array.elem[i]);
  }
  res[array.length*2+1] = 0;
  return res;
}

int main(int argc, char **argv) {
  char *locator = 0;
  if (argc > 1) {
    locator = argv[1];
  }

  z_vec_t ps = z_vec_make(2);
  z_vec_append(&ps, z_property_make_from_str(Z_USER_KEY, "user"));
  z_vec_append(&ps, z_property_make_from_str(Z_PASSWD_KEY, "password"));

  printf("Openning session...\n");
  z_zenoh_p_result_t r_z = z_open(locator, 0, &ps);
  ASSERT_RESULT(r_z, "Unable to open session.\n")
  z_zenoh_t *z = r_z.value.zenoh;

  z_vec_t info = z_info(z);
  printf("LOCATOR :  %s\n", ((z_property_t *)z_vec_get(&info, Z_INFO_PEER_KEY))->value.elem);
  printf("PID :      %s\n", hexdump(((z_property_t *)z_vec_get(&info, Z_INFO_PID_KEY))->value));
  printf("PEER PID : %s\n", hexdump(((z_property_t *)z_vec_get(&info, Z_INFO_PEER_PID_KEY))->value));

  z_vec_free(&info);

  z_close(z);
  return 0;
}
