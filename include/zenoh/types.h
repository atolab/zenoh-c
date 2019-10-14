#ifndef ZENOH_C_TYPES_H_
#define ZENOH_C_TYPES_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "zenoh/config.h"
#include "zenoh/result.h"
#include "zenoh/collection.h"
#include "zenoh/property.h"
#include "zenoh/iobuf.h"
#include "zenoh/mvar.h"

extern const int _z_dummy_arg;

# define Z_UNUSED_ARG(z) (void)(z)
# define Z_UNUSED_ARG_2(z1, z2) (void)(z1); (void)(z2)
# define Z_UNUSED_ARG_3(z1, z2, z3) (void)(z1); (void)(z2); (void)(z3)
# define Z_UNUSED_ARG_4(z1, z2, z3, z4) (void)(z1); (void)(z2); (void)(z3); (void)(z4)
# define Z_UNUSED_ARG_5(z1, z2, z3, z4, z5) (void)(z1); (void)(z2); (void)(z3); (void)(z4); (void)(z5)

#if (ZENOH_LINUX ==1) || (ZENOH_MACOS == 1) 
#include "zenoh/private/unix/types.h"
#elif (ZENOH_CONTIKI == 1)
#include "zenoh/private/contiki/types.h"
#endif 

#define Z_PUSH_MODE 0x01
#define Z_PULL_MODE 0x02
#define Z_PERIODIC_PUSH_MODE 0x03
#define Z_PERIODIC_PULL_MODE 0x04

#define Z_INT_RES_ID 0
#define Z_STR_RES_ID 1

#define Z_DEST_STORAGES_KEY 0x10
#define Z_DEST_EVALS_KEY 0x11

#define Z_STORAGE_DATA 0
#define Z_STORAGE_FINAL 1
#define Z_EVAL_DATA 2
#define Z_EVAL_FINAL 3
#define Z_REPLY_FINAL 4

#define Z_BEST_MATCH 0
#define Z_COMPLETE 1
#define Z_ALL 2
#define Z_NONE 3

typedef struct {  
  uint8_t kind;
  z_temporal_property_t tprop;
} z_sub_mode_t;
Z_RESULT_DECLARE (z_sub_mode_t, sub_mode)

typedef struct {
  uint8_t clock_id[16];
  z_vle_t time;
} z_timestamp_t;

typedef struct {
  unsigned int flags;  
  z_timestamp_t tstamp;
  uint8_t encoding;
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

typedef void (*z_reply_handler_t)(const z_reply_value_t *reply, void *arg);

typedef void (*z_data_handler_t)(const z_resource_id_t *rid, const unsigned char *data, size_t length, const z_data_info_t *info, void *arg);

typedef struct {
  const char* rname;
  const unsigned char *data;
  size_t length;
  unsigned short encoding;
  unsigned short kind;
  void *context;
} z_resource_t;

Z_ARRAY_P_DECLARE_Z_TYPE(resource_t)

typedef void (*z_replies_sender_t)(void* query_handle, z_array_resource_t replies);
typedef void (*z_query_handler_t)(const char *rname, const char *predicate, z_replies_sender_t send_replies, void *query_handle, void *arg);
typedef void (*z_on_disconnect_t)(void *z);

typedef struct {
  z_socket_t sock;
  z_vle_t sn;
  z_vle_t cid;
  z_vle_t rid;
  z_vle_t eid;
  z_iobuf_t wbuf;
  z_iobuf_t rbuf;
  z_array_uint8_t pid;
  z_array_uint8_t peer_pid;
  z_vle_t qid;
  char *locator;
  z_on_disconnect_t on_disconnect;
  z_list_t *declarations;
  z_list_t *subscriptions;
  z_list_t *storages;
  z_list_t *evals;
  z_list_t *replywaiters;
  z_i_map_t *remote_subs;
  z_mvar_t *reply_msg_mvar;
  volatile int running;
  void *thread;
} z_zenoh_t;


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

typedef struct {
  z_zenoh_t *z;
  z_vle_t rid;
  z_vle_t id;
} z_eva_t;

Z_P_RESULT_DECLARE(z_zenoh_t, zenoh)
Z_P_RESULT_DECLARE(z_sub_t, sub)
Z_P_RESULT_DECLARE(z_sto_t, sto)
Z_P_RESULT_DECLARE(z_pub_t, pub)
Z_P_RESULT_DECLARE(z_eva_t, eval)

typedef struct {
  uint8_t kind;
  uint8_t nb;
} z_query_dest_t;

#endif /* ZENOH_C_TYPES_H_ */ 
