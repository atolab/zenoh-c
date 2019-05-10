#ifndef ZENOH_C_TYPES_H_
#define ZENOH_C_TYPES_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "zenoh/config.h"
#include "zenoh/collection.h"
#include "zenoh/mvar.h"

#if (ZENOH_DEBUG == 2)
#include <stdio.h>

#define Z_DEBUG(x) printf(x)
#define Z_DEBUG_VA(x, ...) printf(x, __VA_ARGS__) 
#define Z_ERROR(x, ...) printf(x, __VA_ARGS__) 
#elif (ZENOH_DEBUG == 1)
#define Z_ERROR(x, ...) printf(x, __VA_ARGS__) 
#define Z_DEBUG_VA(x, ...) 
#define Z_DEBUG(x) 
#elif (ZENOH_DEBUG == 0)
#define Z_DEBUG(x) 
#define Z_DEBUG_VA(x, ...) 
#define Z_ERROR(x, ...) 
#endif 


#if (ZENOH_LINUX ==1) || (ZENOH_MACOS == 1) 
#include "zenoh/unix/types.h"
#elif (ZENOH_CONTIKI == 1)
#include "zenoh/contiki/types.h"
#endif 

#define Z_INT_RES_ID 0
#define Z_STR_RES_ID 1

typedef  unsigned long long  z_vle_t;

Z_ARRAY_DECLARE(uint8_t)

typedef struct {
  unsigned int r_pos;
  unsigned int w_pos;
  unsigned int capacity;
  uint8_t* buf;
} z_iobuf_t;

z_iobuf_t z_iobuf_make(unsigned int capacity);
z_iobuf_t z_iobuf_wrap(uint8_t *buf, unsigned int capacity);

void z_iobuf_free(z_iobuf_t* buf);
unsigned int z_iobuf_readable(const z_iobuf_t* buf);
unsigned int z_iobuf_writable(const z_iobuf_t* buf);
void z_iobuf_write(z_iobuf_t* buf, uint8_t b);
void z_iobuf_write_n(z_iobuf_t* buf, const uint8_t *bs, unsigned int offset, unsigned int length);
uint8_t z_iobuf_read(z_iobuf_t* buf);
uint8_t* z_iobuf_read_n(z_iobuf_t* buf, unsigned int length);
uint8_t* z_iobuf_read_to_n(z_iobuf_t* buf, uint8_t* dest, unsigned int length);
void z_iobuf_put(z_iobuf_t* buf, uint8_t b, unsigned int pos);
uint8_t z_iobuf_get(z_iobuf_t* buf, unsigned int pos);
void z_iobuf_clear(z_iobuf_t *buf);
z_array_uint8_t z_iobuf_to_array(z_iobuf_t* buf);


typedef void on_disconnect_t(void *z);

typedef struct {
  z_socket_t sock;
  z_vle_t sn;
  z_vle_t cid;
  z_vle_t rid;
  z_iobuf_t wbuf;
  z_iobuf_t rbuf;
  z_array_uint8_t pid;
  char *locator;
  on_disconnect_t *on_disconnect;
  z_list_t *declarations;
  z_list_t *subscriptions;
  z_mvar_t *reply_msg_mvar;
  void *runtime;
} zenoh_t;

typedef struct {   
  z_vle_t rid;
  const char* r_name;  
} z_res_decl_t;

typedef struct {
  int z;
} z_sub_t;

typedef struct {
  int z;
} z_pub_t;

typedef union {  
  z_vle_t rid;
  const char *rname;
} z_res_id_t;

typedef struct {
  int kind;
  z_res_id_t id; 
} z_resource_id_t;

typedef struct {
  unsigned int flags;
  // TODO: Add support for timestamp
  // unsigned long long timestamp;
  unsigned short encoding;
  unsigned short kind;  
} z_data_info_t;

typedef void subscriber_callback_t(z_resource_id_t rid, z_iobuf_t data, z_data_info_t info);

typedef struct {  
  char *rname;
  z_vle_t rid;
  subscriber_callback_t *callback;
} z_subscription_t;

void z_register_res_decl(zenoh_t *z, z_vle_t rid, const char *rname);
z_res_decl_t *z_get_res_decl_by_rid(zenoh_t *z, z_vle_t rid);
z_res_decl_t *z_get_res_decl_by_rname(zenoh_t *z, const char *rname);


void z_register_subscription(zenoh_t *z, z_vle_t rid,  subscriber_callback_t *callback);
z_subscription_t *z_get_subscription_by_rid(zenoh_t *z, z_vle_t rid);
z_subscription_t *z_get_subscription_by_rname(zenoh_t *z, const char *rname);


#endif /* ZENOH_C_TYPES_H_ */ 