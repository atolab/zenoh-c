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

#ifndef ZENOH_C_NET_CONFIG_H
#define ZENOH_C_NET_CONFIG_H

#define ZENOH_NET_READ_BUF_LEN 65563
#define ZENOH_NET_WRITE_BUF_LEN 65563
#define ZENOH_NET_PID_LENGTH 8
#define ZENOH_NET_PROTO_VERSION 0x01
#define ZENOH_NET_DEFAULT_LEASE 10000

#define ZENOH_NET_SCOUT_MCAST_ADDR "239.255.0.1"
#define ZENOH_NET_LOCAL_HOST "127.0.0.1"
#define ZENOH_NET_MAX_SCOUT_MSG_LEN 1024
#define ZENOH_NET_SCOUT_PORT 7447
#define ZENOH_NET_SCOUT_TRIES 3
#define ZENOH_NET_SCOUT_TIMEOUT 100000

#define ZENOH_NET_TRANSPORT_TCP_IP 1
// #define ZENOH_NET_TRANSPORT_BLE 1

#endif /* ZENOH_C_NET_CONFIG_H */
