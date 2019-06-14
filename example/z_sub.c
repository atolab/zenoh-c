#include <stdio.h>
#include <unistd.h>
#include "zenoh.h"
#include "zenoh/recv_loop.h"
void listener(z_resource_id_t rid, const unsigned char *data, size_t length, z_data_info_t info) {    
  z_iobuf_t buf = z_iobuf_wrap_wo((unsigned char *)data, length, 0, length);
  z_string_result_t r_s = z_string_decode(&buf);        
  if (r_s.tag == Z_OK_TAG) {
    if (rid.kind == Z_INT_RES_ID) 
      printf(">>: (%zu, %s)\n", rid.id.rid, r_s.value.string);
    else
      printf(">>: (%s, %s)\n", rid.id.rname, r_s.value.string);
    free(r_s.value.string);
  }
}

int main(int argc, char **argv) {
  Z_ARRAY_S_MAKE(uint8_t, uid, 32);
  Z_ARRAY_S_MAKE(uint8_t, pwd, 32);
  uid.elem = (uint8_t *)"user"; 
  uid.length = strlen("user");
  pwd.elem = (uint8_t *)"password"; 
  pwd.length = strlen("password");
  z_property_t user;
  user.id = Z_USER_KEY;
  user.value = uid;
  z_property_t password;
  password.id = Z_PASSWD_KEY;
  password.value = pwd;
  z_vec_t ps = z_vec_make(2);
  z_vec_append(&ps, &user);
  z_vec_append(&ps, &password);
  char *locator = strdup("tcp/127.0.0.1:7447");
  if (argc > 1) {
    locator = argv[1];
  }

  printf("Connecting to %s...\n", locator);
  z_zenoh_p_result_t r_z = z_open(locator, 0, &ps);
  ASSERT_RESULT(r_z, "Unable to open session with broker")
  z_zenoh_t *z = r_z.value.zenoh;

  z_start_recv_loop(z);
  printf("Declaring Resource...\n");  

  printf("Declaring Subscriber...\n");
  z_sub_mode_t sm;
  sm.kind = Z_PUSH_MODE;
  z_sub_p_result_t r = z_declare_subscriber(z, "/demo/hello/*", sm, listener);
  ASSERT_P_RESULT(r,"Unable to declare pub\n");
  
  sleep(60000);
  return 0;
}