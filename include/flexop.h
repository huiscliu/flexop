
#ifndef FLEX_FLEXOP_H
#define FLEX_FLEXOP_H

#include "flexop-vec.h"

#ifdef __cplusplus
extern "C" {
#endif

/* preset values */
void flexop_preset_cmdline(const char *str);

/* init option */
void flexop_init(int *argc, char ***argv);

/* finalize option */
void flexop_finalize(void);

/* aux func */
void flexop_show_cmdline(void);
void flexop_show_used(void);
void flexop_help(void);

/* registration */
void flexop_register_bool(const char *name, const char *help, int *var);
void flexop_register_int(const char *name, const char *help, FLEXOP_INT *var);
void flexop_register_uint(const char *name, const char *help, FLEXOP_UINT *var);
void flexop_register_float(const char *name, const char *help, FLEXOP_FLOAT *var);
void flexop_register_string(const char *name, const char *help, char **var);
void flexop_register_keyword(const char *name, const char *help, const char **keys, int *var);
void flexop_register_handler(const char *name, const char *help, FLEXOP_HANDLER func, void *hvar);
void flexop_register_title(const char *str, const char *help, const char *category);

void flexop_register_vec_int(const char *name, const char *help, FLEXOP_VEC *var);
void flexop_register_vec_uint(const char *name, const char *help, FLEXOP_VEC *var);
void flexop_register_vec_float(const char *name, const char *help, FLEXOP_VEC *var);
void flexop_register_vec_string(const char *name, const char *help, FLEXOP_VEC *var);

/* getter */
int flexop_get_bool(const char *op_name);
FLEXOP_INT flexop_get_int(const char *op_name);
FLEXOP_UINT flexop_get_uint(const char *op_name);
FLEXOP_FLOAT flexop_get_float(const char *op_name);
const char * flexop_get_keyword(const char *op_name);
const char * flexop_get_string(const char *op_name);

FLEXOP_VEC * flexop_get_vec_int(const char *op_name);
FLEXOP_VEC * flexop_get_vec_uint(const char *op_name);
FLEXOP_VEC * flexop_get_vec_float(const char *op_name);
FLEXOP_VEC * flexop_get_vec_string(const char *op_name);

/* setter */
void flexop_set_options(const char *str);
int flexop_set_bool(const char *op_name, int value);
int flexop_set_int(const char *op_name, FLEXOP_INT value);
int flexop_set_uint(const char *op_name, FLEXOP_UINT value);

int flexop_set_float(const char *op_name, FLEXOP_FLOAT value);
int flexop_set_keyword(const char *op_name, const char *value);
int flexop_set_string(const char *op_name, const char *value);
int flexop_set_handler(const char *op_name, const char *value);

int flexop_set_vec_int(const char *op_name, const char *value);
int flexop_set_vec_uint(const char *op_name, const char *value);
int flexop_set_vec_float(const char *op_name, const char *value);
int flexop_set_vec_string(const char *op_name, const char *value);

#ifdef __cplusplus
}
#endif

#endif
