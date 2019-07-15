#include "zenoh.h"
#include "yaks.h"


int y_put(z_zenoh_t *z, const char *path, const unsigned char *data, size_t length, int encoding) {  
  return z_write_data_wo(z, path, data, length, encoding, Y_PUT);  
}

int y_remove(z_zenoh_t *z, const char *path, int encoding) {
  Z_UNUSED_ARG(z);
  Z_UNUSED_ARG(path);
  Z_UNUSED_ARG(encoding);
  return 0;
}

z_sub_p_result_t 
y_subscribe(z_zenoh_t *z, const char *selector, subscriber_callback_t callback, void *arg) {
  Z_DEBUG_VA(">>> Creating Yaks sub for %s\n", selector);      
  z_sub_mode_t sm;
  sm.kind = Z_PUSH_MODE;
  return z_declare_subscriber(z, selector, &sm, callback, arg);
  
}
