#ifndef ZENOH_C_MSG_H_
#define ZENOH_C_MSG_H_

#include "zenoh/types.h"
#include "zenoh/iobuf.h"

/* Message ID */

#define Z_SCOUT           0x01
#define Z_HELLO           0x02

#define Z_OPEN            0x03
#define Z_ACCEPT          0x04
#define Z_CLOSE           0x05

#define Z_DECLARE         0x06

#define Z_STREAM_DATA     0x07
#define Z_BATCH_DATA      0x08
#define Z_WRITE_DATA      0x09

#define Z_QUERY           0x0a
#define Z_PULL            0x0b

#define Z_PING_PONG       0x0c

#define Z_SYNCH           0x0e
#define Z_ACKNACK         0x0f

#define Z_KEEP_ALIVE      0x10
#define Z_CONDUIT_CLOSE   0x11
#define Z_FRAGMENT        0x12
#define Z_CONDUIT         0x13
#define Z_MIGRATE         0x14

#define Z_REPLY           0x19

#define Z_RSPACE          0x18

/* Message Header _FLAGs */
#define Z_S_FLAG  0x20
#define Z_M_FLAG  0x20
#define Z_P_FLAG  0x20

#define Z_R_FLAG  0x40
#define Z_N_FLAG  0x40
#define Z_C_FLAG  0x40

#define Z_A_FLAG  0x80
#define Z_U_FLAG  0x80

#define Z_Z_FLAG  0x80
#define Z_L_FLAG  0x20
#define Z_H_FLAG  0x40

#define Z_G_FLAG  0x80
#define Z_I_FLAG  0x20
#define Z_F_FLAG  0x80
#define Z_O_FLAG  0x20

#define Z_MID_MASK 0x1f
#define Z_FLAGS_MASK = 0xe0

#define Z_HAS_FLAG (h, f) ((h & f) != 0)
#define Z_MID (h) (MID_MASK & h)
#define Z_FLAGS (h) (FLAGS_MASK & h)

/* Scout Flags */
#define Z_SCOUT_BROKER 0x01


/* Declaration Id */ 

#define Z_RESOURCE_DECL  0x01
#define Z_PUBLISHER_DECL  0x02
#define Z_SUBSCRIBER_DECL  0x03
#define Z_SELECTION_DECL  0x04
#define Z_BINDING_DECL  0x05
#define Z_COMMIT_DECL  0x06
#define Z_RESULT_DECL  0x07
#define Z_FORGET_RESOURCE_DECL  0x08
#define Z_FORGET_PUBLISHER_DECL  0x09
#define Z_FORGET_SUBSCRIBER_DECL  0x0a
#define Z_FORGET_SELECTION_DECL  0x0b
#define Z_STORAGE_DECL  0x0c
#define Z_FORGET_STORAGE_DECL  0x0d

#define Z_PUSH_MODE 0x01
#define Z_PULL_MODE 0x02
#define Z_PERIODIC_PUSH_MODE 0x03
#define Z_PERIODIC_PULL_MODE 0x04

typedef struct {
  z_vle_t id;
  char* name;
} z_property_t;

/*
 * Creates a new property with the given id and name. Notice that the ownership
 * for the name remains with the caller.
 */ 
z_property_t* z_property_make(z_vle_t id, const char* name);
void z_property_free(z_property_t** p);

#define HAS_PROPERTIES (m) (m.properties != 0) 

/*------------------ Scout Message ------------------*/
typedef struct {
  uint8_t header;
  z_vle_t mask;
  z_vec_t *properties;
} z_scout_t;

/*------------------ Hello Message ------------------*/
typedef struct {
  uint8_t header;
  z_vle_t mask;
  z_vec_t *locators;
  z_vec_t *properties;
} z_hello_t;

/*------------------ Open Message ------------------*/
typedef struct {
  uint8_t header;
  uint8_t version;  
  z_array_uint8_t pid; 
  z_vle_t lease;  
  z_vec_t *properties;
} z_open_t;


/*------------------ Accept Message ------------------*/
typedef struct {
  uint8_t header;  
  z_array_uint8_t client_pid;
  z_array_uint8_t broker_pid; 
  z_vle_t lease; 
  z_vec_t *properties;
} z_accept_t;
#endif /* ZENOH_C_MSG_H_ */