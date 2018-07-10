
#ifndef FLEX_FLEXOP_H
#define FLEX_FLEXOP_H

#include "flexop-utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/* init option */
void flexop_init(int *argc, char ***argv);

/* finalize option */
void flexop_finalize(void);

void flexop_register_init(void);
void flexop_register_no_arg(const char *name, const char *help, int *var);
void flexop_register_int(const char *name, const char *help, int *var);
void flexop_register_float(const char *name, const char *help, FLEXOP_FLOAT *var);
void flexop_register_string(const char *name, const char *help, char **var);
void flexop_register_keyword(const char *name, const char *help, const char **keys, int *var);
void flexop_register_handler(const char *name, const char *help, FLEXOP_HANDLER func, int append);
void flexop_register_title(const char *str, const char *help, const char *category);

void flexop_reset(void);
void flexop_show_cmdline(void);
void flexop_show_used(void);
void flexop_print_help(FLEXOP_KEY *o, const char *help);
void flexop_help(void);
void flexop_parse_cmdline(int *argc, char ***argv);

#ifdef __cplusplus
}
#endif

#endif
