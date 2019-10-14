#ifndef ZENOH_C_PROPERTY_H_
#define ZENOH_C_PROPERTY_H_

#include "zenoh/iobuf.h"

typedef struct {
  z_vle_t id;
  z_array_uint8_t value;
} z_property_t;
Z_RESULT_DECLARE (z_property_t, property)

/*
 * Creates a new property with the given id and name. Notice that the ownership
 * for the name remains with the caller.
 */ 
z_property_t* z_property_make(z_vle_t id, z_array_uint8_t value);
z_property_t* z_property_make_from_str(z_vle_t id, char *value);
void z_property_free(z_property_t** p);

typedef struct {
    z_vle_t origin;
    z_vle_t period;
    z_vle_t duration;
} z_temporal_property_t;

Z_RESULT_DECLARE (z_temporal_property_t, temporal_property)

#endif /* ZENOH_C_PROPERTY_H_ */
