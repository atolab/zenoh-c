#ifndef ZENOH_C_H_DEFINED_
#define ZENOH_C_H_DEFINED_

#include "zenoh/types.h"

#define Z_SN_KEY 0x01
#define Z_COMPACT_CLIENT_KEY 0x02
#define Z_USER_KEY 0x50
#define Z_PASSWD_KEY 0x51

#define Z_INFO_PID_KEY 0x00
#define Z_INFO_PEER_KEY 0x01
#define Z_INFO_PEER_PID_KEY 0x02

z_zenoh_p_result_t 
z_open(char* locator, z_on_disconnect_t on_disconnect, const z_vec_t *ps);

z_vec_t
z_info(z_zenoh_t *z);

z_sub_p_result_t 
z_declare_subscriber(z_zenoh_t *z, const char* resource, const z_sub_mode_t *sm, z_data_handler_t data_handler, void *arg);

z_pub_p_result_t 
z_declare_publisher(z_zenoh_t *z, const char *resource);

z_sto_p_result_t 
z_declare_storage(z_zenoh_t *z, const char* resource, z_data_handler_t data_handler, z_query_handler_t query_handler, void *arg);

z_eval_p_result_t 
z_declare_eval(z_zenoh_t *z, const char* resource, z_query_handler_t query_handler, void *arg);

int z_stream_compact_data(z_pub_t *pub, const unsigned char *payload, size_t len);
int z_stream_data(z_pub_t *pub, const unsigned char *payload, size_t len);
int z_write_data(z_zenoh_t *z, const char* resource, const unsigned char *payload, size_t length);

int z_stream_data_wo(z_pub_t *pub, const unsigned char *payload, size_t len, uint8_t encoding, uint8_t kind);
int z_write_data_wo(z_zenoh_t *z, const char* resource, const unsigned char *payload, size_t len, uint8_t encoding, uint8_t kind);

int z_pull(z_sub_t *sub);

int z_query(z_zenoh_t *z, const char* resource, const char* predicate, z_reply_handler_t reply_handler, void *arg);
int z_query_wo(z_zenoh_t *z, const char* resource, const char* predicate, z_reply_handler_t reply_handler, void *arg, z_query_dest_t dest_storages, z_query_dest_t dest_evals);

int z_undeclare_subscriber(z_sub_t *z);
int z_undeclare_publisher(z_pub_t *z);
int z_undeclare_storage(z_sto_t *z);
int z_undeclare_eval(z_eva_t *z);

int z_close(z_zenoh_t *z);

#endif /* ZENOH_C_H_DEFINED_ */
