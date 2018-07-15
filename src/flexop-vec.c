
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

void flexop_vec_init(FLEXOP_VEC *vec, FLEXOP_TYPE type, FLEXOP_INT tsize)
{
    if (vec == NULL) return;
    bzero(vec, sizeof(FLEXOP_VEC));

    /* data type */
    vec->type = type;

    /* sizeof type */
    if (type == FLEXOP_T_INT) {
        vec->tsize = sizeof(FLEXOP_INT);
    }
    else if (type == FLEXOP_T_FLOAT) {
        vec->tsize = sizeof(FLEXOP_FLOAT);
    }
    else if (type == FLEXOP_T_STRING) {
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

    if (vec->type == FLEXOP_T_STRING) {
        for (i = 0; i < vec->size; i++) {
            free(((char **)vec->d)[i]);
        }
    }

    flexop_free(vec->d);
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

    if (v->type == FLEXOP_T_INT) {
        ((FLEXOP_INT *)v->d)[v->size++] = *(FLEXOP_INT *)e;
    }
    else if (v->type == FLEXOP_T_FLOAT) {
        ((FLEXOP_FLOAT *)v->d)[v->size++] = *(FLEXOP_FLOAT *)e;
    }
    else if (v->type == FLEXOP_T_STRING) {
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

    assert(v->type == FLEXOP_T_INT);
    return ((FLEXOP_INT *)v->d)[n];
}

FLEXOP_FLOAT flexop_vec_float_get_value(FLEXOP_VEC *v, FLEXOP_INT n)
{
    assert(v != NULL);
    assert(n >= 0);
    assert(n < v->size);

    assert(v->type == FLEXOP_T_FLOAT);
    return ((FLEXOP_FLOAT *)v->d)[n];
}

char * flexop_vec_string_get_value(FLEXOP_VEC *v, FLEXOP_INT n)
{
    assert(v != NULL);
    assert(n >= 0);
    assert(n < v->size);

    assert(v->type == FLEXOP_T_STRING);
    return ((char **)v->d)[n];
}
