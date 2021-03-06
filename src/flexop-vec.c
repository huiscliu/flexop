
#include "flexop-vec.h"

int flexop_vec_initialized(FLEXOP_VEC *vec)
{
    if (vec == NULL) return 0;

    if (vec->magic == FLEXOP_VEC_MAGIC_NUMBER) {
        return 1;
    }
    else {
        return 0;
    }
}

void flexop_vec_init(FLEXOP_VEC *vec, FLEXOP_VTYPE type, FLEXOP_INT tsize, const char *key)
{
    if (vec == NULL) return;
    bzero(vec, sizeof(FLEXOP_VEC));

    /* data type */
    vec->type = type;

    assert(key != NULL);
    vec->key = strdup(key);

    /* sizeof type */
    if (type == VT_INT) {
        vec->tsize = sizeof(FLEXOP_INT);
    }
    else if (type == VT_UINT) {
        vec->tsize = sizeof(FLEXOP_UINT);
    }
    else if (type == VT_FLOAT) {
        vec->tsize = sizeof(FLEXOP_FLOAT);
    }
    else if (type == VT_STRING) {
        vec->tsize = sizeof(char *);
    }
    else {
        vec->tsize = tsize;
        flexop_error(1, "flexop: data type is not supported yet.\n");
    }

    vec->magic = FLEXOP_VEC_MAGIC_NUMBER;
}

void flexop_vec_destroy(FLEXOP_VEC *vec)
{
    FLEXOP_INT i;

    if (vec == NULL) return;

    if (vec->type == VT_STRING) {
        for (i = 0; i < vec->size; i++) {
            free(((char **)vec->d)[i]);
        }
    }

    flexop_free(vec->d);
    free(vec->key);
    bzero(vec, sizeof(FLEXOP_VEC));
}

/* add entry */
void flexop_vec_add_entry(FLEXOP_VEC *v, void *e)
{
    assert(v != NULL);
    assert(e != NULL);

    if (v->size >= v->alloc) {
        v->alloc += 16;

        assert(v->tsize > 0);
        v->d = flexop_realloc(v->d, v->alloc * v->tsize);
    }

    if (v->type == VT_INT) {
        ((FLEXOP_INT *)v->d)[v->size++] = *(FLEXOP_INT *)e;
    }
    else if (v->type == VT_UINT) {
        ((FLEXOP_UINT *)v->d)[v->size++] = *(FLEXOP_UINT *)e;
    }
    else if (v->type == VT_FLOAT) {
        ((FLEXOP_FLOAT *)v->d)[v->size++] = *(FLEXOP_FLOAT *)e;
    }
    else if (v->type == VT_STRING) {
        ((char **)v->d)[v->size++] = strdup(e);
    }
    else {
        flexop_error(1, "flexop: data type is not supported yet.\n");
    }
}

/* get value */
FLEXOP_INT flexop_vec_get_size(FLEXOP_VEC *v)
{
    assert(v != NULL);

    return v->size;
}

FLEXOP_INT flexop_vec_int_get_value(FLEXOP_VEC *v, FLEXOP_INT n)
{
    assert(v != NULL);
    assert(n >= 0);
    assert(n < v->size);

    assert(v->type == VT_INT);
    return ((FLEXOP_INT *)v->d)[n];
}

FLEXOP_UINT flexop_vec_uint_get_value(FLEXOP_VEC *v, FLEXOP_INT n)
{
    assert(v != NULL);
    assert(n >= 0);
    assert(n < v->size);

    assert(v->type == VT_UINT);
    return ((FLEXOP_UINT *)v->d)[n];
}

FLEXOP_FLOAT flexop_vec_float_get_value(FLEXOP_VEC *v, FLEXOP_INT n)
{
    assert(v != NULL);
    assert(n >= 0);
    assert(n < v->size);

    assert(v->type == VT_FLOAT);
    return ((FLEXOP_FLOAT *)v->d)[n];
}

char * flexop_vec_string_get_value(FLEXOP_VEC *v, FLEXOP_INT n)
{
    assert(v != NULL);
    assert(n >= 0);
    assert(n < v->size);

    assert(v->type == VT_STRING);
    return ((char **)v->d)[n];
}

void flexop_vec_print(FLEXOP_VEC *v)
{
    FLEXOP_INT i;

    if (v->type == VT_INT) {
        FLEXOP_INT *p;

        p = v->d;
        flexop_printf("flexop: key: \"%s\", vector of int, size: %d, values:", v->key, v->size);

        for (i = 0; i < v->size; i++) {
            flexop_printf(" %"IFMT, p[i]);
        }

        flexop_printf("\n");
    }
    else if (v->type == VT_UINT) {
        FLEXOP_UINT *p;

        p = v->d;
        flexop_printf("flexop: key: \"%s\", vector of int, size: %d, values:", v->key, v->size);

        for (i = 0; i < v->size; i++) {
            flexop_printf(" %"UFMT, p[i]);
        }

        flexop_printf("\n");
    }
    else if (v->type == VT_FLOAT) {
        FLEXOP_FLOAT *p;

        p = v->d;
        flexop_printf("flexop: key: \"%s\", vector of float, size: %d, values:", v->key, v->size);

        for (i = 0; i < v->size; i++) {
            flexop_printf(" %"FFMT, p[i]);
        }

        flexop_printf("\n");
    }
    else if (v->type == VT_STRING) {
        char **p;

        p = v->d;
        flexop_printf("flexop: key: \"%s\", vector of string, size: %d, values:", v->key, v->size);

        for (i = 0; i < v->size; i++) {
            flexop_printf(" \"%s\"", p[i]);
        }

        flexop_printf("\n");
    }
    else {
        flexop_error(1, "flexop: parsing module not implemented yet.\n");
    }
}
