#include "zenoh/result.h"

inline void z_message_p_result_init(z_message_p_result_t *r) {
    r->value.message = (z_message_t *)malloc(sizeof(z_message_t));    
}