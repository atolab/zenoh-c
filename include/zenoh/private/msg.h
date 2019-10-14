#ifndef ZENOH_C_MSG_H_
#define ZENOH_C_MSG_H_

#include "zenoh/private/internal.h"

/* Message ID */

#define _Z_SCOUT           0x01
#define _Z_HELLO           0x02

#define _Z_OPEN            0x03
#define _Z_ACCEPT          0x04
#define _Z_CLOSE           0x05

#define _Z_DECLARE         0x06

#define _Z_COMPACT_DATA    0x07
#define _Z_STREAM_DATA     0x1a
#define _Z_BATCH_DATA      0x08
#define _Z_WRITE_DATA      0x09

#define _Z_QUERY           0x0a
#define _Z_PULL            0x0b

#define _Z_PING_PONG       0x0c

#define _Z_SYNCH           0x0e
#define _Z_ACKNACK         0x0f

#define _Z_KEEP_ALIVE      0x10
#define _Z_CONDUIT_CLOSE   0x11
#define _Z_FRAGMENT        0x12
#define _Z_CONDUIT         0x13
#define _Z_MIGRATE         0x14

#define _Z_REPLY           0x19

#define _Z_RSPACE          0x18

/* Message Header _FLAGs */
#define _Z_S_FLAG  0x20
#define _Z_M_FLAG  0x20
#define _Z_P_FLAG  0x20

#define _Z_R_FLAG  0x40
#define _Z_N_FLAG  0x40
#define _Z_C_FLAG  0x40
#define _Z_E_FLAG  0x40

#define _Z_A_FLAG  0x80
#define _Z_U_FLAG  0x80

#define _Z_Z_FLAG  0x80
#define _Z_L_FLAG  0x20
#define _Z_H_FLAG  0x40

#define _Z_G_FLAG  0x80
#define _Z_I_FLAG  0x20
#define _Z_F_FLAG  0x80
#define _Z_O_FLAG  0x20

#define _Z_MID_MASK 0x1f
#define _Z_FLAGS_MASK = 0xe0

#define _Z_HAS_FLAG(h, f) ((h & f) != 0)
#define _Z_MID(h) (_Z_MID_MASK & h)
#define _Z_FLAGS(h) (_Z_FLAGS_MASK & h)

/* Scout Flags */
#define _Z_SCOUT_BROKER 0x01


/* Declaration Id */ 

#define _Z_RESOURCE_DECL  0x01
#define _Z_PUBLISHER_DECL  0x02
#define _Z_SUBSCRIBER_DECL  0x03
#define _Z_SELECTION_DECL  0x04
#define _Z_BINDING_DECL  0x05
#define _Z_COMMIT_DECL  0x06
#define _Z_RESULT_DECL  0x07
#define _Z_FORGET_RESOURCE_DECL  0x08
#define _Z_FORGET_PUBLISHER_DECL  0x09
#define _Z_FORGET_SUBSCRIBER_DECL  0x0a
#define _Z_FORGET_SELECTION_DECL  0x0b
#define _Z_STORAGE_DECL  0x0c
#define _Z_FORGET_STORAGE_DECL  0x0d
#define _Z_EVAL_DECL  0x0e
#define _Z_FORGET_EVAL_DECL  0x0f

/* Close Reasons */
#define _Z_PEER_CLOSE 0
#define _Z_ERROR_CLOSE 1

/* Payload Header */
#define _Z_SRC_ID 0x01
#define _Z_SRC_SN 0x02
#define _Z_BRK_ID 0x04
#define _Z_BRK_SN 0x08
#define _Z_T_STAMP 0x10
#define _Z_KIND 0x20
#define _Z_ENCODING 0x40

#define _HAS_PROPERTIES (m) (m.properties != 0) 

/*------------------ Scout Message ------------------*/
typedef struct {  
  z_vle_t mask;  
} _z_scout_t;

/*------------------ Hello Message ------------------*/
typedef struct {  
  z_vle_t mask;
  z_vec_t *locators;  
} _z_hello_t;

/*------------------ Open Message ------------------*/
typedef struct {  
  uint8_t version;  
  z_array_uint8_t pid; 
  z_vle_t lease;    
  // z_vec_t *locators; 
} _z_open_t;

/*------------------ Accept Message ------------------*/
typedef struct {    
  z_array_uint8_t client_pid;
  z_array_uint8_t broker_pid; 
  z_vle_t lease;   
} _z_accept_t;

/*------------------ Close Message ------------------*/
typedef struct {  
  z_array_uint8_t pid;
  uint8_t reason;
} _z_close_t; 

/*------------------  Resource Declaration Message ------------------*/
// in types.h

/*------------------ Delcare Publisher ------------------*/
typedef struct {   
  z_vle_t rid;    
} _z_pub_decl_t;

/*------------------ Forget Publisher Message ------------------*/
typedef struct {   
  z_vle_t rid; 
} _z_forget_pub_decl_t;

/*------------------ Declare Storage ------------------*/
typedef struct {   
  z_vle_t rid;    
} _z_storage_decl_t;

/*------------------ Forget Storage Message ------------------*/
typedef struct {   
  z_vle_t rid; 
} _z_forget_sto_decl_t;

/*------------------ Declare Eval ------------------*/
typedef struct {   
  z_vle_t rid;    
} _z_eval_decl_t;

/*------------------ Forget Eval Message ------------------*/
typedef struct {   
  z_vle_t rid; 
} _z_forget_eval_decl_t;

/*------------------ Declare Subscriber Message ------------------*/
typedef struct {   
  z_vle_t rid;  
  z_sub_mode_t sub_mode;  
} _z_sub_decl_t;

/*------------------ Forget Subscriber Message ------------------*/
typedef struct {   
  z_vle_t rid; 
} _z_forget_sub_decl_t;

/*------------------ Declaration Commit Message ------------------*/
typedef struct {  
  uint8_t cid;
} _z_commit_decl_t;

/*------------------ Declaration Result  Message ------------------*/
typedef struct {  
  uint8_t cid;
  uint8_t status;
} _z_result_decl_t;

/**
 *  On the wire this is represented as:
 *     |header|payload|properties|
 *  The declaration of the structure does not follow this
 *  convention for alignement purposes.
 */
typedef struct {  
  z_vec_t* properties;
  union {
    _z_res_decl_t resource;
    _z_pub_decl_t pub;
    _z_sub_decl_t sub;
    _z_storage_decl_t storage;
    _z_eval_decl_t eval;
    _z_forget_pub_decl_t forget_pub;
    _z_forget_sub_decl_t forget_sub;
    _z_forget_sto_decl_t forget_sto;
    _z_forget_eval_decl_t forget_eval;
    _z_commit_decl_t commit;
    _z_result_decl_t result;
  } payload;  
  uint8_t header; 
} _z_declaration_t;

Z_ARRAY_DECLARE(_z_declaration_t)

/*------------------ Declare Messages ------------------*/
typedef struct  {  
  z_vle_t sn;
  z_array__z_declaration_t declarations;
} _z_declare_t;


/*------------------ Compact Data Message ------------------*/
typedef struct {  
  z_vle_t sn;
  z_vle_t rid;
  z_iobuf_t payload;
} _z_compact_data_t;


/*------------------ Payload Header ------------------*/
typedef struct {  
  z_vle_t src_sn;
  z_vle_t brk_sn;
  z_vle_t kind;
  z_vle_t encoding;
  uint8_t src_id[16];
  uint8_t brk_id[16];
  uint8_t flags;  
  z_timestamp_t tstamp;
  z_iobuf_t payload;
} _z_payload_header_t;

/*------------------ StreamData Message ------------------*/
typedef struct {  
  z_vle_t sn;
  z_vle_t rid;
  z_iobuf_t payload_header;
} _z_stream_data_t;

/*------------------ Write Data Message ------------------*/
typedef struct {  
  z_vle_t sn;
  char* rname;
  z_iobuf_t payload_header;
} _z_write_data_t;

/*------------------ Pull Message ------------------*/
typedef struct {
  z_vle_t sn;
  z_vle_t id;
  z_vle_t max_samples;
} _z_pull_t;

/*------------------ Query Message ------------------*/
typedef struct {
  z_array_uint8_t pid; 
  z_vle_t qid;
  char* rname;
  char* predicate;
} _z_query_t;

/*------------------ Reply Message ------------------*/
typedef struct {
  z_array_uint8_t qpid; 
  z_vle_t qid;
  z_array_uint8_t stoid;
  z_vle_t rsn;
  char* rname;
  z_iobuf_t payload_header;
} _z_reply_t;


/**
 *  On the wire this is represented as:
 *     |header|payload|properties|
 *  The declaration of the structure does not follow this
 *  convention for alignement purposes.
 */
typedef struct {
  const z_vec_t* properties;  
  union {
    _z_open_t open;
    _z_accept_t accept;
    _z_close_t close;
    _z_declare_t declare;
    _z_compact_data_t compact_data;
    _z_stream_data_t stream_data;
    _z_write_data_t write_data;
    _z_pull_t pull;
    _z_query_t query;
    _z_reply_t reply;
    _z_scout_t scout;
    _z_hello_t hello;
  } payload;
  uint8_t header; 
} _z_message_t;

_Z_RESULT_DECLARE (_z_accept_t, accept)
_Z_RESULT_DECLARE (_z_close_t, close)
_Z_RESULT_DECLARE (_z_declare_t, declare)
_Z_RESULT_DECLARE (_z_declaration_t, declaration)
_Z_RESULT_DECLARE (_z_res_decl_t, res_decl)
_Z_RESULT_DECLARE (_z_pub_decl_t, pub_decl)
_Z_RESULT_DECLARE (_z_sub_decl_t, sub_decl)
_Z_RESULT_DECLARE (_z_commit_decl_t, commit_decl)
_Z_RESULT_DECLARE (_z_result_decl_t, result_decl)
_Z_RESULT_DECLARE (_z_compact_data_t, compact_data)
_Z_RESULT_DECLARE (_z_payload_header_t, payload_header)
_Z_RESULT_DECLARE (_z_stream_data_t, stream_data)
_Z_RESULT_DECLARE (_z_write_data_t, write_data)
_Z_RESULT_DECLARE (_z_pull_t, pull)
_Z_RESULT_DECLARE (_z_query_t, query)
_Z_RESULT_DECLARE (_z_reply_t, reply)
_Z_P_RESULT_DECLARE (_z_message_t, message)

#endif /* ZENOH_C_MSG_H_ */
