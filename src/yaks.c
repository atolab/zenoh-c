#include "zenoh.h"
#include "yaks.h"



int y_put(zenoh_t *z, const char *path, const z_iobuf_t *data, int encoding) {
  z_payload_header_t ph;
  ph.flags = Z_ENCODING | Z_KIND;
  ph.encoding = encoding;
  ph.kind = Y_PUT;
  ph.payload = *data;
  return z_write_data(z, path, &ph);
}

int y_remove(zenoh_t *z, const char *path, int encoding) {
  return 0;
}

int y_subscribe(zenoh_t *z, const char *selector, subscriber_callback_t *callback) {
  z_vle_result_t r_rid  = z_declare_resource(z, selector);
  if (r_rid.tag == Z_ERROR_TAG)
    return -1;
  z_sub_mode_t sm;
  sm.kind = Z_PUSH_MODE;
  return z_declare_subscriber(z, r_rid.value.vle, sm, callback);  
}
