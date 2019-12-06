#ifndef ZENOH_C_NET_COLLECTION_H
#define ZENOH_C_NET_COLLECTION_H

#include "zenoh/collection.h"

#define ZN_ARRAY_DECLARE(name) ARRAY_DECLARE(zn_##name##_t, name, zn_)
#define _ZN_ARRAY_DECLARE(name) ARRAY_DECLARE(_zn_##name##_t, name, _zn_)

#define ZN_ARRAY_P_DECLARE(name) ARRAY_P_DECLARE(zn_##name##_t, name, zn_)
#define _ZN_ARRAY_P_DECLARE(name) ARRAY_P_DECLARE(_zn_##name##_t, name, _zn_)

#define ZN_ARRAY_S_DEFINE(name, arr, len) ARRAY_S_DEFINE(zn_##name##_t, name, zn_, arr, len)
#define _ZN_ARRAY_S_DEFINE(name, arr, len) ARRAY_S_DEFINE(_zn_##name##_t, name, _zn_, arr, len)

#define ZN_ARRAY_P_S_DEFINE(name, arr, len) ARRAY_P_S_DEFINE(zn_##name##_t, name, zn_, arr, len)
#define _ZN_ARRAY_P_S_DEFINE(name, arr, len) ARRAY_P_S_DEFINE(_zn_##name##_t, name, _zn_, arr, len)

#define ZN_ARRAY_S_INIT(name, arr, len) ARRAY_S_INIT(zn_##name##_t, arr, len)
#define _ZN_ARRAY_S_INIT(name, arr, len) ARRAY_S_INIT(_zn_##name##_t, arr, len)

#define ZN_ARRAY_P_S_INIT(name, arr, len) ARRAY_P_S_INIT(zn_##name##_t, arr, len)
#define _ZN_ARRAY_P_S_INIT(name, arr, len) ARRAY_P_S_INIT(_zn_##name##_t, arr, len)

#define ZN_ARRAY_H_INIT(name, arr, len) ARRAY_H_INIT(zn_##name##_t, arr, len)
#define _ZN_ARRAY_H_INIT(name, arr, len) ARRAY_H_INIT(_zn_##name##_t, arr, len)

#define ZN_ARRAY_S_COPY(name, arr, len) ARRAY_S_COPY(zn_##name##_t, arr, len)
#define _ZN_ARRAY_S_COPY(name, arr, len) ARRAY_S_COPY(_zn_##name##_t, arr, len)

#define ZN_ARRAY_H_COPY(name, arr, len) ARRAY_H_COPY(zn_##name##_t, arr, len)
#define _ZN_ARRAY_H_COPY(name, arr, len) ARRAY_H_COPY(_zn_##name##_t, arr, len)

#endif /* ZENOH_C_NET_COLLECTION_H */
