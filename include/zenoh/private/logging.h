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

#ifndef ZENOH_C_LOGGING_H_
#define ZENOH_C_LOGGING_H_

#if (ZENOH_DEBUG == 2)
#include <stdio.h>

#define _Z_DEBUG(x) printf(x)
#define _Z_DEBUG_VA(x, ...) printf(x, __VA_ARGS__)
#define _Z_ERROR(x, ...) printf(x, __VA_ARGS__)
#elif (ZENOH_DEBUG == 1)
#include "zenoh/types.h"

#define _Z_ERROR(x, ...) printf(x, __VA_ARGS__)
#define _Z_DEBUG_VA(x, ...) (void)(_z_dummy_arg)
#define _Z_DEBUG(x) (void)(_z_dummy_arg)
#elif (ZENOH_DEBUG == 0)
#include "zenoh/types.h"

#define _Z_DEBUG(x) (void)(_z_dummy_arg)
#define _Z_DEBUG_VA(x, ...) (void)(_z_dummy_arg)
#define _Z_ERROR(x, ...) (void)(_z_dummy_arg)
#endif

#endif /* ZENOH_C_LOGGING_H_ */
