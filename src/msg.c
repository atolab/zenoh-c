#include "zenoh/msg.h"

z_property_t* make_z_property(vle_t id, const char* name) {
  z_property_t* p = (z_property_t*)malloc(sizeof(z_property_t));
  p->id = id;
  p->name = strdup(name);
  return p;
}
void free_z_property(z_property_t* p) {
  free(p->name);
  free(p);
  }
