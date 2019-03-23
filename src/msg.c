#include "zenoh/msg.h"

z_property_t* z_property_make(z_vle_t id, const char* name) {
  z_property_t* p = (z_property_t*)malloc(sizeof(z_property_t));
  p->id = id;
  p->name = strdup(name);
  return p;
}
void z_property_free(z_property_t** p) {
  free((*p)->name);
  free((*p));
  *p = 0;
  }
