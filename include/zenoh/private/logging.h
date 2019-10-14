#ifndef ZENOH_C_LOGGING_H_
#define ZENOH_C_LOGGING_H_

#if (ZENOH_DEBUG == 2)
#include <stdio.h>

#define _Z_DEBUG(x) printf(x)
#define _Z_DEBUG_VA(x, ...) printf(x, __VA_ARGS__) 
#define _Z_ERROR(x, ...) printf(x, __VA_ARGS__) 
#elif (ZENOH_DEBUG == 1)

#define _Z_ERROR(x, ...) printf(x, __VA_ARGS__) 
#define _Z_DEBUG_VA(x, ...) (void)(_z_dummy_arg)
#define _Z_DEBUG(x) (void)(_z_dummy_arg)
#elif (ZENOH_DEBUG == 0)
#define _Z_DEBUG(x) (void)(_z_dummy_arg)
#define _Z_DEBUG_VA(x, ...) (void)(_z_dummy_arg)
#define _Z_ERROR(x, ...) (void)(_z_dummy_arg)
#endif 

#endif /* ZENOH_C_LOGGING_H_ */
