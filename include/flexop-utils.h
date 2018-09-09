
#ifndef FLEX_OPTION_UT_H
#define FLEX_OPTION_UT_H

#include "flexop-types.h" 

#ifdef __cplusplus
extern "C" {
#endif

void flexop_warning(const char *fmt, ...);
void flexop_error(int code, const char *fmt, ...);
int  flexop_printf(const char *fmt, ...);

void * flexop_alloc(size_t size);
void * flexop_realloc(void *ptr, size_t size);
void * flexop_calloc(size_t nmemb, size_t size);
void   flexop_free(void *ptr);

FLEXOP_FLOAT flexop_atof(const char *ptr);
FLEXOP_INT flexop_atoi(const char *ptr);

void flexop_set_print_mark(int m);

#ifdef __cplusplus
}
#endif

#endif
