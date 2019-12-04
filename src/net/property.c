#include "zenoh/net/property.h"
#include <string.h>

zn_property_t* zn_property_make(z_vle_t id, z_array_uint8_t value) {
  zn_property_t* p = (zn_property_t*)malloc(sizeof(zn_property_t));
  p->id = id;
  p->value = value;  
  return p;
}

zn_property_t* zn_property_make_from_str(z_vle_t id, char *str) {
  zn_property_t* p = (zn_property_t*)malloc(sizeof(zn_property_t));
  p->id = id;
  p->value.elem = (uint8_t *)str;
  p->value.length = strlen(str);
  return p;
}

void zn_property_free(zn_property_t** p) {
  free((*p));
  *p = 0;
  }
