#include "zenoh/property.h"
#include <string.h>

z_property_t* z_property_make(z_vle_t id, z_array_uint8_t value) {
  z_property_t* p = (z_property_t*)malloc(sizeof(z_property_t));
  p->id = id;
  p->value = value;  
  return p;
}

z_property_t* z_property_make_from_str(z_vle_t id, char *str) {
  z_property_t* p = (z_property_t*)malloc(sizeof(z_property_t));
  p->id = id;
  p->value.elem = (uint8_t *)str;
  p->value.length = strlen(str);
  return p;
}

void z_property_free(z_property_t** p) {
  free((*p));
  *p = 0;
  }
