#ifndef ZENOH_C_H_DEFINED_
#define ZENOH_C_H_DEFINED_

#include "zenoh/config.h"
#include "zenoh/msg.h"
#include "zenoh/codec.h"

#define Z_SN_KEY 0x01
#define Z_COMPACT_CLIENT_KEY 0x02
#define Z_USER_KEY 0x50
#define Z_PASSWD_KEY 0x51

z_zenoh_p_result_t 
z_open(char* locator, on_disconnect_t *on_disconnect, const z_vec_t *ps);

z_zenoh_t * 
z_open_wup(char* locator, const char * uname, const char *passwd);

z_sub_p_result_t 
z_declare_subscriber(z_zenoh_t *z, const char* resource, z_sub_mode_t sm, subscriber_callback_t *callback);

z_pub_p_result_t 
z_declare_publisher(z_zenoh_t *z, const char *resource);

z_sto_p_result_t 
z_declare_storage(z_zenoh_t *z, const char* resource, subscriber_callback_t *callback, query_handler_t *handler, replies_cleaner_t *cleaner);

int z_stream_compact_data(z_pub_t *pub, const unsigned char *payload, size_t len);
int z_stream_data(z_pub_t *pub, const unsigned char *payload, size_t len);
int z_write_data(z_zenoh_t *z, const char* resource, const unsigned char *payload, size_t length);

int z_stream_data_wo(z_pub_t *pub, const unsigned char *payload, size_t len, uint8_t encoding, uint8_t kind);
int z_write_data_wo(z_zenoh_t *z, const char* resource, const unsigned char *payload, size_t len, uint8_t encoding, uint8_t kind);

int z_query(z_zenoh_t *z, const char* resource, const char* predicate, z_reply_callback_t *callback);

#endif /* ZENOH_C_H_DEFINED_ */
