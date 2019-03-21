#ifndef ZENOH_C_H_DEFINED_
#define ZENOH_C_H_DEFINED_

#include "zenoh/msg.h"

typedef struct {
  int z;
} zenoh_t;

typedef struct {
  int z;
} z_sub_t;

typedef struct {
  int z;
} z_pub_t;
typedef void *subscriber_callback(const char* , const char* , unsigned int);

zenoh_t* open(const char* locator);
void close(zenoh_t* z);


z_sub_t* subscribe(const char* resource, subscriber_callback callback);
z_pub_t* publish(const char* resource);

void stream(z_pub_t* pub, const char* data, unsigned int offset, unsigned int len);
void write(const char* resource, const char* data, unsigned int offset, unsigned int len);




#endif /* ZENOH_C_H_DEFINED_ */
