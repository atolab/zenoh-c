%module zenohc 
%{ 
#define ZENOH_C_SWIG 1
typedef unsigned char uint8_t;
typedef  unsigned long long  z_vle_t;
typedef struct { unsigned int length; uint8_t* elem; } z_array_uint8_t;

typedef struct {
  unsigned int r_pos;
  unsigned int w_pos;
  unsigned int capacity;
  unsigned char* buf;
} z_iobuf_t;

z_iobuf_t z_iobuf_make(unsigned int capacity);
z_iobuf_t z_iobuf_wrap(unsigned char *buf, unsigned int capacity);

extern void z_iobuf_free(z_iobuf_t* buf);
extern unsigned int z_iobuf_readable(const z_iobuf_t* buf);
extern unsigned int z_iobuf_writable(const z_iobuf_t* buf);
extern void z_iobuf_write(z_iobuf_t* buf, unsigned char b);
extern void z_iobuf_write_slice(z_iobuf_t* buf, const unsigned char *bs, unsigned int offset, unsigned int length);
extern void z_iobuf_write_bytes(z_iobuf_t* buf, const unsigned char *bs, unsigned int length);
extern uint8_t z_iobuf_read(z_iobuf_t* buf);
extern uint8_t* z_iobuf_read_n(z_iobuf_t* buf, unsigned int length);
extern uint8_t* z_iobuf_read_to_n(z_iobuf_t* buf, unsigned char* dest, unsigned int length);
extern void z_iobuf_put(z_iobuf_t* buf, uint8_t b, unsigned int pos);
extern void z_iobuf_clear(z_iobuf_t *buf);
extern z_array_uint8_t z_iobuf_to_array(z_iobuf_t* buf);


typedef struct {
  unsigned int flags;
  // TODO: Add support for timestamp
  // unsigned long long timestamp;
  unsigned short encoding;
  unsigned short kind;  
} z_data_info_t;

typedef struct {
  char kind;
  z_iobuf_t stoid;
  z_vle_t rsn;
  char* rname;
  z_iobuf_t data;
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

typedef void reply_callback_t(z_reply_value_t reply);

typedef void subscriber_callback_t(z_resource_id_t rid, z_iobuf_t data, z_data_info_t info);

typedef struct {
  int z;
} z_sub_t;

typedef struct {
  z_vle_t rid;
} z_pub_t;


enum result_kind {
  Z_OK_TAG,
  Z_ERROR_TAG    
};
typedef struct { enum result_kind tag; union { z_sub_t sub; int error; } value;} z_sub_result_t;
typedef struct { enum result_kind tag; union { z_pub_t pub; int error; } value;} z_pub_result_t;
typedef struct { enum result_kind tag; union { char* string; int error; } value;} z_string_result_t;
typedef struct { enum result_kind tag; union { z_zenoh_t * zenoh; int error; } value;} z_zenoh_p_result_t; 


#include "zenoh/config.h"
#include "zenoh/types.h"
#include "zenoh/msg.h"
#include "zenoh/codec.h"

extern z_zenoh_result_t
z_open(char* locator, on_disconnect_t *on_disconnect);

extern  z_zenoh_t*
z_open_ptr(char* locator);

extern void z_close(z_zenoh_t* z);
extern int z_declare_resource_ir(z_zenoh_t *z, const char* resource);
extern int z_declare_subscriber(z_zenoh_t *z, z_vle_t rid, z_sub_mode_t sm, subscriber_callback_t *callback);
extern int z_declare_publisher(z_zenoh_t *z, z_vle_t rid);

extern int z_stream_compact_data(z_zenoh_t *z, z_vle_t rid, const z_iobuf_t *payload);
extern int z_stream_data_wo(z_zenoh_t *z, z_vle_t rid, const z_iobuf_t *data, uint8_t encoding, uint8_t kind);

extern int z_write_data(z_zenoh_t *z, const char* resource, const z_iobuf_t *payload_header);
extern int z_query(z_zenoh_t *z, const char* resource, const char* predicate, reply_callback_t *callback);

%}

#define ZENOH_C_SWIG 1
typedef  unsigned long long  z_vle_t;
typedef unsigned char uint8_t;

typedef struct { unsigned int length; uint8_t* elem; } z_array_uint8_t;

typedef struct {
  unsigned int r_pos;
  unsigned int w_pos;
  unsigned int capacity;
  unsigned char* buf;
} z_iobuf_t;

extern z_iobuf_t z_iobuf_make(unsigned int capacity);
extern z_iobuf_t z_iobuf_wrap(unsigned char *buf, unsigned int capacity);

extern void z_iobuf_free(z_iobuf_t* buf);
extern unsigned int z_iobuf_readable(const z_iobuf_t* buf);
extern unsigned int z_iobuf_writable(const z_iobuf_t* buf);
extern void z_iobuf_write(z_iobuf_t* buf, unsigned char b);
extern void z_iobuf_write_slice(z_iobuf_t* buf, const unsigned char *bs, unsigned int offset, unsigned int length);
extern void z_iobuf_write_bytes(z_iobuf_t* buf, const unsigned char *bs, unsigned int length);
extern uint8_t z_iobuf_read(z_iobuf_t* buf);
extern uint8_t* z_iobuf_read_n(z_iobuf_t* buf, unsigned int length);
extern uint8_t* z_iobuf_read_to_n(z_iobuf_t* buf, unsigned char* dest, unsigned int length);
extern void z_iobuf_put(z_iobuf_t* buf, uint8_t b, unsigned int pos);
extern void z_iobuf_clear(z_iobuf_t *buf);
extern z_array_uint8_t z_iobuf_to_array(z_iobuf_t* buf);

typedef struct {
  unsigned int flags;
  // TODO: Add support for timestamp
  // unsigned long long timestamp;
  unsigned short encoding;
  unsigned short kind;  
} z_data_info_t;

typedef struct {
  char kind;
  z_iobuf_t stoid;
  z_vle_t rsn;
  char* rname;
  z_iobuf_t data;
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

typedef void reply_callback_t(z_reply_value_t reply);

typedef void subscriber_callback_t(z_resource_id_t rid, z_iobuf_t data, z_data_info_t info);

typedef struct {
  int z;
} z_sub_t;

typedef struct {
  z_vle_t rid;
} z_pub_t;

#include "zenoh/config.h"
#include "zenoh/types.h"
#include "zenoh/msg.h"
#include "zenoh/codec.h"

extern z_zenoh_result_t
z_open(char* locator, on_disconnect_t *on_disconnect);

extern  z_zenoh_t*
z_open_ptr(char* locator);

extern void z_close(z_zenoh_t* z);
extern int z_declare_resource_ir(z_zenoh_t *z, const char* resource);
extern int z_declare_subscriber(z_zenoh_t *z, z_vle_t rid, z_sub_mode_t sm, subscriber_callback_t *callback);
extern int z_declare_publisher(z_zenoh_t *z, z_vle_t rid);

extern int z_stream_compact_data(z_zenoh_t *z, z_vle_t rid, const z_iobuf_t *payload);
extern int z_stream_data_wo(z_zenoh_t *z, z_vle_t rid, const z_iobuf_t *data, uint8_t encoding, uint8_t kind);

extern int z_write_data(z_zenoh_t *z, const char* resource, const z_iobuf_t *payload_header);
extern int z_query(z_zenoh_t *z, const char* resource, const char* predicate, reply_callback_t *callback);