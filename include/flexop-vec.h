
#ifndef FLEX_OPTION_VEC_H
#define FLEX_OPTION_VEC_H

#include "flexop-utils.h" 

#ifdef __cplusplus
extern "C" {
#endif

/* vector */
int flexop_vec_initialized(FLEXOP_VEC *vec);
void flexop_vec_init(FLEXOP_VEC *vec, FLEXOP_VTYPE type, FLEXOP_INT tsize, const char *key);
void flexop_vec_destroy(FLEXOP_VEC *vec);

/* add entry */
void flexop_vec_add_entry(FLEXOP_VEC *v, void *e);

/* get value */
FLEXOP_INT flexop_vec_int_get_value(FLEXOP_VEC *v, FLEXOP_INT n);
FLEXOP_UINT flexop_vec_uint_get_value(FLEXOP_VEC *v, FLEXOP_INT n);
FLEXOP_FLOAT flexop_vec_float_get_value(FLEXOP_VEC *v, FLEXOP_INT n);
char * flexop_vec_string_get_value(FLEXOP_VEC *v, FLEXOP_INT n);

FLEXOP_INT flexop_vec_get_size(FLEXOP_VEC *v);

void flexop_vec_print(FLEXOP_VEC *v);

#ifdef __cplusplus
}
#endif

#endif
