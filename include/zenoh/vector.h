#ifndef ZENOH_C_VECTOR_H
#define ZENOH_C_VECTOR_H


typedef struct {
  unsigned int capacity_;
  unsigned int length_;
  void** elem_; 
} z_vec_t;

z_vec_t* make_z_vec(unsigned int capacity);
z_vec_t* clone_z_vec(z_vec_t* v);
void free_z_vec(z_vec_t* v);

unsigned int z_vec_length(z_vec_t* v);
void z_vec_append(z_vec_t* v, void* e);
void* z_vec_get(z_vec_t* v, unsigned int i);

void z_vec_set(z_vec_t* sv, unsigned int i, void* e);


#endif /* ZENOH_C_VECTOR_H */