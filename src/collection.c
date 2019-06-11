#include "zenoh/collection.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

z_list_t * z_list_empty = 0;

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

z_list_t * z_list_of(void *x) {
  z_list_t *xs = (z_list_t *)malloc(sizeof(z_list_t));
  bzero(xs, sizeof(z_list_t));
  xs->elem = x;
  return xs;  
}

z_list_t * z_list_cons(z_list_t *xs, void *x) {
  z_list_t *lst = z_list_of(x);
  lst->tail = xs;
  return lst;
}

void * z_list_head(z_list_t *xs) {
  return xs->elem;
}

z_list_t * z_list_tail(z_list_t *xs) {
  return xs->tail;
}

unsigned int z_list_len(z_list_t *xs) {
  unsigned int len = 0;
  while (xs != z_list_empty) {
    len += 1;
    xs = z_list_tail(xs);
  }
  return len;
}

z_list_t * z_list_drop_elem(z_list_t *xs, unsigned int position) {
  assert (position < z_list_len(xs));
  z_list_t *head = xs;
  z_list_t *previous;
  if (position == 0) {
    xs = head->tail;
    free(head);
    return xs;
  }
  
  unsigned int idx = 0;
  while (idx < position) {
    previous = xs;
    xs = xs->tail;
    idx++;
  }

  previous->tail = xs->tail;
  free(xs);
  return head;
}

void z_list_free(z_list_t *xs) {
  z_list_t *current = xs;
  z_list_t *tail;
  if (current != z_list_empty) {
    tail = z_list_tail(current);
    free(current);
    current = tail;
  }
}

z_i_map_t *z_i_map_empty = 0;

z_i_map_t *z_i_map_make(unsigned int capacity) {
  z_i_map_t *map;
  map->elems = (z_list_t *)malloc(capacity * sizeof(z_list_t));
  memset(map->elems, 0, capacity * sizeof(z_list_t));
  map->capacity = capacity;
  map->n = 0;
  
  return map;
}

void z_i_map_set(z_i_map_t *map, int k, void *v) {
  unsigned int idx = k % map->capacity;
  z_list_t *elems = map->elems[idx];

}
void *z_i_map_get(z_i_map_t *map, int k) {
  unsigned int idx = k % map->capacity;

}
void *z_i_map_remove(z_i_map_t *map, int k) {

}
unsigned int z_i_map_capacity(z_i_map_t *map) {
  return map->capacity;
}
unsigned int z_i_map_size(z_i_map_t *map) {
  return map->n;
}
