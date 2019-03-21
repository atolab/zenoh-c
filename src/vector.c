#include "zenoh/vector.h"
#include <stdlib.h>

#include <assert.h>

inline z_vec_t* make_z_vec(unsigned int capacity) {
  z_vec_t* v = (z_vec_t*) malloc(sizeof(z_vec_t));
  v->capacity_ = capacity;
  v->length_ = 0;
  v->elem_ = (void**)malloc(sizeof(void*) * capacity);
  return v;
}

z_vec_t* clone_z_vec(z_vec_t* v) {
  z_vec_t* u = make_z_vec(v->capacity_);
  int i;
  for (i = 0; i < v->length_; ++i) 
    z_vec_append(u, v->elem_[i]);
  return u;
}

void free_z_vec(z_vec_t* v) {
  int i;
  for (i = 0; i < v->length_; ++i) 
    free(v->elem_[i]);
  v->length_ = 0;
  v-> capacity_ = 0;
  free(v->elem_);
  v->elem_ = 0;
  free(v);
}

inline unsigned int z_vec_length(z_vec_t* v) { return v->length_; }

void z_vec_append(z_vec_t* v, void* e) {
  assert(v->length_ < v->capacity_); 
  v->elem_[v->length_] = e;
  v->length_ = v->length_ + 1;
}
void* z_vec_get(z_vec_t* v, unsigned int i) {
  assert(i < v->length_);
  return v->elem_[i];
}
void z_vec_set(z_vec_t* v, unsigned int i, void* e) {
  assert(i < v->capacity_);
  v->elem_[i] = e;
}

