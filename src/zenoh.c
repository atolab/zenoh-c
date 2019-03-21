#include "zenoh.h"
#include <stdlib.h>

zenoh_t* open(const char* locator) {
  zenoh_t *z = (zenoh_t*) malloc(sizeof(zenoh_t));
  return z;
}

void close(zenoh_t *z) { 
  free(z);
  z = 0;
} 

z_sub_t* subscribe(const char* resource, subscriber_callback callback) {
  z_sub_t *sub = (z_sub_t*)malloc(sizeof(z_sub_t));
  return sub;
}
z_pub_t* publish(const char* resource) {
  z_pub_t *pub = (z_pub_t*)malloc(sizeof(z_pub_t));
  return pub;
}

void stream(z_pub_t* pub, const char* data, unsigned int offset, unsigned int len) { }
void write(const char* resource, const char* data, unsigned int offset, unsigned int len) { }