#ifndef ZENOH_C_TYPES_H_
#define ZENOH_C_TYPES_H_

#include "zenoh/result.h"

extern const int _z_dummy_arg;

# define Z_UNUSED_ARG(z) (void)(z)
# define Z_UNUSED_ARG_2(z1, z2) (void)(z1); (void)(z2)
# define Z_UNUSED_ARG_3(z1, z2, z3) (void)(z1); (void)(z2); (void)(z3)
# define Z_UNUSED_ARG_4(z1, z2, z3, z4) (void)(z1); (void)(z2); (void)(z3); (void)(z4)
# define Z_UNUSED_ARG_5(z1, z2, z3, z4, z5) (void)(z1); (void)(z2); (void)(z3); (void)(z4); (void)(z5)

Z_RESULT_DECLARE (char*, string)

#endif /* ZENOH_C_TYPES_H_ */
