#ifndef ZENOH_C_NET_COLLECTION_H
#define ZENOH_C_NET_COLLECTION_H

#include "zenoh/collection.h"

#define ZN_ARRAY_DECLARE(T) \
typedef struct { \
  unsigned int length; \
  T* elem; \
} zn_array_##T;

#define ZN_ARRAY_P_DECLARE(T) \
typedef struct { \
  unsigned int length; \
  T** elem; \
} zn_array_p_##T;

#define ZN_ARRAY_DECLARE_ZN_TYPE(T) \
typedef struct { \
  unsigned int length; \
  zn_##T* elem; \
} zn_array_##T;

#define ZN_ARRAY_P_DECLARE_ZN_TYPE(T) \
typedef struct { \
  unsigned int length; \
  zn_##T** elem; \
} zn_array_p_##T;

#define ZN_ARRAY_S_DEFINE(T, arr, len) \
zn_array_##T arr = {len, (T*)malloc(len*sizeof(T))};

#define ZN_ARRAY_P_S_DEFINE(T, arr, len) \
zn_array_p_##T arr = {len, (T**)malloc(len*sizeof(T*))};

#define ZN_ARRAY_S_INIT(T, arr, len) \
arr.length = len; \
arr.elem =  (T*)malloc(len*sizeof(T));

#define ZN_ARRAY_P_S_INIT(T, arr, len) \
arr.length = len; \
arr.elem =  (T**)malloc(len*sizeof(T*));

#define ZN_ARRAY_H_INIT(T, arr, len) \
arr->length = len; \
arr->elem =  (T*)malloc(len*sizeof(T))

#define ZN_ARRAY_S_COPY(T, dst, src) \
dst.length = src.length; \
dst.elem = (T*)malloc(dst.length*sizeof(T)); \
memcpy(dst.elem, src.elem, dst.length);

#define ZN_ARRAY_H_COPY(T, dst, src) \
dst->length = src->length; \
dst->elem =  (T*)malloc(dst->length*sizeof(T)); \
memcpy(dst->elem, src->elem, dst->length);

#define ZN_ARRAY_S_ZN_TYPE_DEFINE(T, arr, len) \
zn_array_##T arr = {len, (z_##T*)malloc(len*sizeof(z_##T))};


#define ZN_ARRAY_H_DEFINE(T, arr, len) \
zn_array_##T * arr = (zn_array_##T*)malloc(sizeof(zn_array_##T)); \
arr->length = len; \
arr->elem = (T*)malloc(len*sizeof(T));

#define ZN_ARRAY_H_ZN_TYPE_DEFINE(T, arr, len) \
zn_array_##T * arr = (zn_array_##T*)malloc(sizeof(zn_array_##T)); \
arr->length = len; \
arr->elem = (z_##T*)malloc(len*sizeof(zn_##T));


#define ZN_ARRAY_S_FREE(arr) \
free(arr.elem); \
arr.elem = 0; \
arr.length = 0;

#define ZN_ARRAY_H_FREE(arr) \
free(arr->elem); \
arr->elem = 0; \
arr->length = 0

#endif /* ZENOH_C_NET_COLLECTION_H */
