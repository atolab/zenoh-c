#ifndef ZENOH_C_PROPERTY_H
#define ZENOH_C_PROPERTY_H

#include "zenoh/iobuf.h"
#include "zenoh/net/result.h"

typedef struct {
  z_vle_t id;
  z_array_uint8_t value;
} zn_property_t;

ZN_RESULT_DECLARE (zn_property_t, property)

/*
 * Creates a new property with the given id and name. Notice that the ownership
 * for the name remains with the caller.
 */ 
zn_property_t* zn_property_make(z_vle_t id, z_array_uint8_t value);
zn_property_t* zn_property_make_from_str(z_vle_t id, char *value);
void zn_property_free(zn_property_t** p);

typedef struct {
    z_vle_t origin;
    z_vle_t period;
    z_vle_t duration;
} zn_temporal_property_t;

ZN_RESULT_DECLARE (zn_temporal_property_t, temporal_property)

#endif /* ZENOH_C_PROPERTY_H */
