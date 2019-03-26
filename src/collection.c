#include "zenoh/collection.h"
#include <stdlib.h>

#include <assert.h>

inline z_vec_t z_vec_make(unsigned int capacity) {
  z_vec_t v;
  v.capacity_ = capacity;
  v.length_ = 0;
  v.elem_ = (void**)malloc(sizeof(void*) * capacity);
  return v;
}

z_vec_t z_vec_clone(const z_vec_t* v) {
  z_vec_t u = z_vec_make(v->capacity_);
  int i;
  for (i = 0; i < v->length_; ++i) 
    z_vec_append(&u, v->elem_[i]);
  return u;
}

void z_vec_free(z_vec_t* v) {  
  int i;
  for (i = 0; i < v->length_; ++i) 
    free(v->elem_[i]);
  v->length_ = 0;
  v-> capacity_ = 0;
  free(v->elem_);
  v->elem_ = 0;
  
}

inline unsigned int z_vec_length(const z_vec_t* v) { return v->length_; }

void z_vec_append(z_vec_t* v, void* e) {
  assert(v->length_ < v->capacity_); 
  v->elem_[v->length_] = e;
  v->length_ = v->length_ + 1;
}
const void* z_vec_get(const z_vec_t* v, unsigned int i) {
  assert(i < v->length_);
  return v->elem_[i];
}
void z_vec_set(z_vec_t* v, unsigned int i, void* e) {
  assert(i < v->capacity_);
  v->elem_[i] = e;
}

