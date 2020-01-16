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

#ifndef ZENOH_C_MVAR_H_
#define ZENOH_C_MVAR_H_

typedef void* z_mvar_t;

z_mvar_t *z_mvar_empty();
int z_mvar_is_empty(z_mvar_t *mv);

z_mvar_t *z_mvar_of(void *e);
void * z_mvar_get(z_mvar_t *mv);
void z_mvar_put(z_mvar_t *mv, void *e);

#endif /* ZENOH_C_MVAR_H_ */
