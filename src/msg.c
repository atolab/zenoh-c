#include "zenoh/msg.h"

z_property_t* z_property_make(z_vle_t id, z_array_uint8_t value) {
  z_property_t* p = (z_property_t*)malloc(sizeof(z_property_t));
  p->id = id;
  p->value = value;  
  return p;
}
void z_property_free(z_property_t** p) {
  free((*p)->value.elem);
  free((*p));
  *p = 0;
  }
