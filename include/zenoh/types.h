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

#define Z_STORAGE_DATA 0
#define Z_STORAGE_FINAL 1
#define Z_REPLY_FINAL 2

#ifndef ZENOH_C_SWIG
typedef  size_t  z_vle_t;
/*------------------ Temporal Properties ------------------*/
typedef struct {
    z_vle_t origin;
    z_vle_t period;
    z_vle_t duration;
} z_temporal_property_t;

typedef struct {  
  uint8_t kind;
  z_temporal_property_t tprop;
} z_sub_mode_t;

#endif /* ZENOH_C_SWIG  */

Z_ARRAY_DECLARE(uint8_t)
typedef struct {
  unsigned int r_pos;
  unsigned int w_pos;
  unsigned int capacity;
  unsigned char* buf;
} z_iobuf_t;

z_iobuf_t z_iobuf_make(unsigned int capacity);
z_iobuf_t z_iobuf_wrap(unsigned char *buf, unsigned int capacity);
z_iobuf_t z_iobuf_wrap_wo(unsigned char *buf, unsigned int capacity, unsigned int rpos, unsigned int wpos);

void z_iobuf_free(z_iobuf_t* buf);
unsigned int z_iobuf_readable(const z_iobuf_t* buf);
unsigned int z_iobuf_writable(const z_iobuf_t* buf);
void z_iobuf_write(z_iobuf_t* buf, unsigned char b);
void z_iobuf_write_slice(z_iobuf_t* buf, const uint8_t *bs, unsigned int offset, unsigned int length);
void z_iobuf_write_bytes(z_iobuf_t* buf, const unsigned char *bs, unsigned int length);
uint8_t z_iobuf_read(z_iobuf_t* buf);
uint8_t* z_iobuf_read_n(z_iobuf_t* buf, unsigned int length);
uint8_t* z_iobuf_read_to_n(z_iobuf_t* buf, unsigned char* dest, unsigned int length);
void z_iobuf_put(z_iobuf_t* buf, unsigned char b, unsigned int pos);
uint8_t z_iobuf_get(z_iobuf_t* buf, unsigned int pos);
void z_iobuf_clear(z_iobuf_t *buf);
z_array_uint8_t z_iobuf_to_array(z_iobuf_t* buf);
void z_iobuf_compact(z_iobuf_t *buf);

#ifndef ZENOH_C_SWIG

typedef struct {
  unsigned int flags;
  // TODO: Add support for timestamp
  // unsigned long long timestamp;
  unsigned short encoding;
  unsigned short kind;  
} z_data_info_t;

typedef struct {
  char kind;
  const unsigned char *stoid; 
  size_t stoid_length; 
  z_vle_t rsn;
  const char* rname;
  const unsigned char *data;
  size_t data_length;
  z_data_info_t info;
} z_reply_value_t;

typedef union {  
  z_vle_t rid;
  char *rname;
} z_res_id_t;

typedef struct {
  int kind;
  z_res_id_t id; 
} z_resource_id_t;

typedef void (*z_reply_callback_t)(const z_reply_value_t *reply, void *arg);

typedef void (*subscriber_callback_t)(const z_resource_id_t *rid, const unsigned char *data, size_t length, z_data_info_t info, void *arg);

typedef struct {
  const char* rname;
  const unsigned char *data;
  size_t length;
  unsigned short encoding;
  unsigned short kind; 
} z_resource_t;

Z_ARRAY_DECLARE(z_resource_t)

typedef z_array_z_resource_t (*query_handler_t)(const char *rname, const char *predicate, void *arg);
typedef void (*replies_cleaner_t)(z_array_z_resource_t replies, void *arg);

#endif /* ZENOH_C_SWIG */
typedef void (*on_disconnect_t)(void *z);

typedef struct {
  z_socket_t sock;
  z_vle_t sn;
  z_vle_t cid;
  z_vle_t rid;
  z_vle_t eid;
  z_iobuf_t wbuf;
  z_iobuf_t rbuf;
  z_array_uint8_t pid;
  z_vle_t qid;
  char *locator;
  on_disconnect_t on_disconnect;
  z_list_t *declarations;
  z_list_t *subscriptions;
  z_list_t *storages;
  z_list_t *replywaiters;
  z_i_map_t *remote_subs;
  z_mvar_t *reply_msg_mvar;
  void *runtime;
} z_zenoh_t;


#ifndef ZENOH_C_SWIG
typedef struct {
  z_zenoh_t *z;
  z_vle_t rid;
  z_vle_t id;
} z_sub_t;

typedef struct {
  z_zenoh_t *z;
  z_vle_t rid;
  z_vle_t id;
} z_sto_t;

typedef struct {
  z_zenoh_t *z;
  z_vle_t rid;
  z_vle_t id;
} z_pub_t;
#endif /* ZENOH_C_SWIG */

typedef struct {   
  z_vle_t rid;
  char* r_name;
} z_res_decl_t;


typedef struct {  
  char *rname;
  z_vle_t rid;
  subscriber_callback_t callback;
  void *arg;
}  z_subscription_t;

typedef struct {  
  char *rname;
  z_vle_t rid;
  subscriber_callback_t callback;
  query_handler_t handler;
  replies_cleaner_t cleaner;
  void *arg;
}  z_storage_t;

typedef struct {  
  z_vle_t qid;
  z_reply_callback_t callback;
  void *arg;
} z_replywaiter_t;

z_vle_t z_get_entity_id(z_zenoh_t *z);
z_vle_t z_get_resource_id(z_zenoh_t *z, const char *rname);
int z_register_res_decl(z_zenoh_t *z, z_vle_t rid, const char *rname);
z_res_decl_t *z_get_res_decl_by_rid(z_zenoh_t *z, z_vle_t rid);
z_res_decl_t *z_get_res_decl_by_rname(z_zenoh_t *z, const char *rname);


void z_register_subscription(z_zenoh_t *z, z_vle_t rid,  subscriber_callback_t callback, void *arg);
const char * z_get_resource_name(z_zenoh_t *z, z_vle_t rid);
z_list_t * z_get_subscriptions_by_rid(z_zenoh_t *z, z_vle_t rid);
z_list_t * z_get_subscriptions_by_rname(z_zenoh_t *z, const char *rname);

void z_register_storage(z_zenoh_t *z, z_vle_t rid, subscriber_callback_t callback, query_handler_t handler, replies_cleaner_t cleaner, void *arg);
z_list_t * z_get_storages_by_rname(z_zenoh_t *z, const char *rname);

int z_matching_remote_sub(z_zenoh_t *z, z_vle_t rid);

void z_register_query(z_zenoh_t *z, z_vle_t qid, z_reply_callback_t callback, void *arg);
z_replywaiter_t *z_get_query(z_zenoh_t *z, z_vle_t qid);

#endif /* ZENOH_C_TYPES_H_ */ 