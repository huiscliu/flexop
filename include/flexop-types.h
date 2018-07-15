
#ifndef FLEX_OPTION_TYPES_H
#define FLEX_OPTION_TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "flexop-config.h"

/* basic type */
#if FLEXOP_USE_LONG_DOUBLE
typedef long double       FLEXOP_FLOAT;
#else
typedef double            FLEXOP_FLOAT;
#endif

#if FLEXOP_USE_LONG_LONG
typedef long long int     FLEXOP_INT;
#elif FLEXOP_USE_LONG
typedef long int          FLEXOP_INT;
#else
typedef int               FLEXOP_INT;
#endif

#define FLEXOP_VEC_MAGIC_NUMBER        (0x2619EFE)

typedef enum FLEXOP_TYPE_
{
    /* basic */
    FLEXOP_T_STRING,          /* string */
    FLEXOP_T_INT,             /* int */
    FLEXOP_T_FLOAT,           /* float */

} FLEXOP_TYPE;

typedef struct FLEXOP_VEC_
{
    void *d;
    FLEXOP_TYPE type;

    FLEXOP_INT size;
    FLEXOP_INT alloc;

    FLEXOP_INT tsize;
    FLEXOP_INT magic;

} FLEXOP_VEC;


/* All options are stored in a list 'options'. When parsing a cmdline option,
 * the corresponding variable, *o->var, is updated with the given value.
 * o->var points to the location of the variable with one of the following
 * types:
 *
 * - For VT_INT, o->var is intepreted as (INT *)o->var
 *
 * - For VT_FLOAT, o->var is intepreted as (double *)o->var
 *
 * - For options without argument (VT_NONE), o->var is intepreted as
 *   (int *)o->var, the cmdline option '-name' sets it to TRUE,
 *   and '+name' sets it to FALSE.
 *
 * - For VT_STRING, o->var is intepreted as (const char **)o->var, i.e.,
 *   it points to the address of the string pointer, calling the option
 *   will modify the pointer to point to the new string which is allocated
 *   and freed by flexop_XXXXX functions (thus user code should never
 *   free the string after calling flexop_register.
 *
 * - For VT_KEYWORD, o->keys is a NULL terminated list of valid keywords
 *   for this option, and o->var o->var is intepreted as (int *)o->var
 *   which is an index to the list of keywords */

typedef enum {
    VT_INIT,
    VT_NONE,

    VT_TITLE,

    VT_INT,
    VT_FLOAT,
    VT_STRING,

    VT_KEYWORD,
    VT_HANDLER,

//    VT_VEC_INT,
//    VT_VEC_FLOAT,
//    VT_VEC_STRING,

} FLEXOP_VTYPE;

typedef struct FLEXOP_KEY
{
    char  *name;    /* option name without leading dash */
    char  *help;    /* help text for this option */
    char  **keys;   /* list of key words if type is VT_KEYWORD */
    void  *var;     /* address of the variable to assign value to
                       It's assumed to have the following type:
                       - VT_HANDLER  (FLEXOP_HANDLER)var
                       - VT_FILENAME  (const char **)var
                       - VT_STRING  (const char **)var
                       - VT_KEYWORD  (int *)var
                       - VT_INT  (INT *)var
                       - VT_FLOAT  (double *)var
                       - VT_NONE  (int *)var */

    FLEXOP_VTYPE type;  /* type of the variable */
    int  used;          /* whether the option is specified in cmdline */

} FLEXOP_KEY;

/* Option handling function protocol.
 * 'optname' is the name of the option,
 * 'optstr' is the option string (NULL ==> print help).
 * Returns FALSE if error. */
typedef int (*FLEXOP_HANDLER)(FLEXOP_KEY *o, const char *arg);

typedef struct FLEXOP_
{
    FLEXOP_KEY *options;
    int *index;
    char *help_category;

    int argc;
    char **argv;

    size_t size;
    size_t alloc;
    int initialized;

} FLEXOP;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif
