#ifndef ZENOH_C_H_YAKS_DEFINED_
#define ZENOH_C_H_YAKS_DEFINED_

#define RAW             0x00
#define Custom_Encoding 0x01
#define STRING          0x02
#define PROPERTIES      0x03
#define JSON            0x04
#define SQL             0x05

#define Y_PUT 0x00
#define Y_UPDATE 0x01
#define Y_REMOVE 0x02

#include "zenoh/config.h"
#include "zenoh/msg.h"

typedef void subscriber_callback_t(uint8_t mid, z_vle_t rid, z_iobuf_t data);


int y_put(zenoh_t *z, const char *path, const z_iobuf_t *data, int encoding);
int y_remove(zenoh_t *z, const char *path, int encoding);

int y_subscribe(zenoh_t *z, const char *selector, subscriber_callback_t *callback);


#endif /* ZENOH_C_H_YAKS_DEFINED_ */