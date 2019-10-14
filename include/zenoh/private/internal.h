#ifndef ZENOH_C_INTERNAL_H_
#define ZENOH_C_INTERNAL_H_

#include "zenoh/types.h"

typedef struct {   
  z_vle_t rid;
  char* r_name;
} _z_res_decl_t;

z_vle_t _z_get_entity_id(z_zenoh_t *z);
z_vle_t _z_get_resource_id(z_zenoh_t *z, const char *rname);
int _z_register_res_decl(z_zenoh_t *z, z_vle_t rid, const char *rname);
_z_res_decl_t *_z_get_res_decl_by_rid(z_zenoh_t *z, z_vle_t rid);
_z_res_decl_t *_z_get_res_decl_by_rname(z_zenoh_t *z, const char *rname);

void _z_register_subscription(z_zenoh_t *z, z_vle_t rid, z_vle_t id, z_data_handler_t data_handler, void *arg);
const char * _z_get_resource_name(z_zenoh_t *z, z_vle_t rid);
z_list_t * _z_get_subscriptions_by_rid(z_zenoh_t *z, z_vle_t rid);
z_list_t * _z_get_subscriptions_by_rname(z_zenoh_t *z, const char *rname);
void _z_unregister_subscription(z_sub_t *s) ;

void _z_register_storage(z_zenoh_t *z, z_vle_t rid, z_vle_t id, z_data_handler_t data_handler, z_query_handler_t query_handler, void *arg);
z_list_t * _z_get_storages_by_rid(z_zenoh_t *z, z_vle_t rid);
z_list_t * _z_get_storages_by_rname(z_zenoh_t *z, const char *rname);
void _z_unregister_storage(z_sto_t *s) ;

void _z_register_eval(z_zenoh_t *z, z_vle_t rid, z_vle_t id, z_query_handler_t query_handler, void *arg);
z_list_t * _z_get_evals_by_rid(z_zenoh_t *z, z_vle_t rid);
z_list_t * _z_get_evals_by_rname(z_zenoh_t *z, const char *rname);
void _z_unregister_eval(z_eva_t *s) ;

int _z_matching_remote_sub(z_zenoh_t *z, z_vle_t rid);

typedef struct {  
  z_vle_t qid;
  z_reply_handler_t reply_handler;
  void *arg;
} _z_replywaiter_t;

void _z_register_query(z_zenoh_t *z, z_vle_t qid, z_reply_handler_t reply_handler, void *arg);
_z_replywaiter_t *_z_get_query(z_zenoh_t *z, z_vle_t qid);

typedef struct {  
  char *rname;
  z_vle_t rid;
  z_vle_t id;
  z_data_handler_t data_handler;
  void *arg;
}  _z_sub_t;

typedef struct {  
  char *rname;
  z_vle_t rid;
  z_vle_t id;
  z_data_handler_t data_handler;
  z_query_handler_t query_handler;
  void *arg;
}  _z_sto_t;

typedef struct {  
  char *rname;
  z_vle_t rid;
  z_vle_t id;
  z_query_handler_t query_handler;
  void *arg;
}  _z_eva_t;

#endif /* ZENOH_C_INTERNAL_H_ */
