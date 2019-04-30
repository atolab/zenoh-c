#ifndef ZENOH_C_VECTOR_H
#define ZENOH_C_VECTOR_H

#define Z_ARRAY_DECLARE(T) \
typedef struct { \
  unsigned int length; \
  T* elem; \
} z_array_##T;

#define Z_ARRAY_S_MAKE(T, arr, len) \
z_array_##T arr = {len, (T*)malloc(len*sizeof(T))};

#define Z_ARRAY_H_MAKE(T, arr, len) \
z_array_##T * arr = (z_array_##T*)malloc(sizeof(z_array_##T)); \
arr->length = len; \
arr->elem = (T*)malloc(len*sizeof(T));

typedef struct {
  unsigned int capacity_;
  unsigned int length_;
  void** elem_; 
} z_vec_t;

z_vec_t z_vec_make(unsigned int capacity);
z_vec_t z_vec_clone(const z_vec_t* v);
void z_vec_free(z_vec_t* v);

unsigned int z_vec_length(const z_vec_t* v);
/* 
 * Append an element ot the vector and takes ownership of the appended element.
 */
void z_vec_append(z_vec_t* v, void* e);

const void* z_vec_get(const z_vec_t* v, unsigned int i);

/* 
 * Set the element at the i-th position of the vector and takes ownership.
 */
void z_vec_set(z_vec_t* sv, unsigned int i, void* e);


/*-------- Linked List --------*/

typedef struct z_list  {
  void *elem;
  struct z_list *tail;
} z_list_t;

extern z_list_t * z_list_empty;
z_list_t * z_list_of(void *x);
z_list_t * z_list_cons(z_list_t *xs, void *x);
void * z_list_head(z_list_t *xs);
z_list_t * z_list_tail(z_list_t *xs);
unsigned int z_list_len(z_list_t *xs);
/**
 * Drops the element at the specified position. 
 */
z_list_t * z_list_drop_elem(z_list_t *xs, unsigned int position);
void z_list_free(z_list_t *xs);


#endif /* ZENOH_C_VECTOR_H */