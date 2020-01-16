/*
 * Copyright (c) 2014, 2020 Contributors to the Eclipse Foundation
 *
 * See the NOTICE file(s) distributed with this work for additional
 * information regarding copyright ownership.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
 * which is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
 *
 * Contributors: Julien Enoch, ADLINK Technology Inc.
 * Initial implementation of Eclipse Zenoh.
 */

#ifndef ZENOH_C_PROPERTY_H
#define ZENOH_C_PROPERTY_H

#include "zenoh/iobuf.h"
#include "zenoh/net/result.h"

typedef struct {
  z_vle_t id;
  z_uint8_array_t value;
} zn_property_t;

ZN_RESULT_DECLARE (zn_property_t, property)

/*
 * Creates a new property with the given id and name. Notice that the ownership
 * for the name remains with the caller.
 */
zn_property_t* zn_property_make(z_vle_t id, z_uint8_array_t value);
zn_property_t* zn_property_make_from_str(z_vle_t id, char *value);
void zn_property_free(zn_property_t** p);

typedef struct {
    z_vle_t origin;
    z_vle_t period;
    z_vle_t duration;
} zn_temporal_property_t;

ZN_RESULT_DECLARE (zn_temporal_property_t, temporal_property)

#endif /* ZENOH_C_PROPERTY_H */
