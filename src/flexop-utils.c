
#include "flexop-utils.h"

static int flexop_print = 1;

void flexop_warning(const char *fmt, ...)
{
    va_list ap;
    char s[1024];

    sprintf(s, "*** WARNING: %s", fmt);
    va_start(ap, fmt);
    vprintf(s, ap);
    va_end(ap);
}

void flexop_error(int code, const char *fmt, ...)
{
    va_list ap;
    char s[4096];

    sprintf(s, "*** Error: %s", fmt);
    va_start(ap, fmt);
    vprintf(s, ap);
    va_end(ap);

    if (code == 0) return;
    exit(code);
}

int flexop_printf(const char *fmt, ...)
{
    va_list ap;
    int ret = 0;

    if (!flexop_print) return 0;

    va_start(ap, fmt);
    ret = vprintf(fmt, ap);
    va_end(ap);

    return ret;
}

void * flexop_alloc(size_t size)
{
    void *ptr = (size != 0) ? malloc(size) : NULL;

    if (ptr == NULL && size != 0) {
        flexop_error(1, "failed to malloc %u bytes memory.\n", size);
    }

    return ptr;
}

void * flexop_realloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);

    if (p == NULL && size != 0) {
        flexop_error(1, "failed to reallocate %u bytes memory at %p.\n", size, ptr);
    }

    return p;
}

void * flexop_calloc(size_t nmemb, size_t size)
{
    void *ptr = (nmemb != 0 && size != 0) ? calloc(nmemb, size) : NULL;

    if (ptr == NULL && nmemb != 0 && size != 0) {
        flexop_error(1, "%s:%d, failed to calloc %d bytes memory.\n",
                __FILE__, __LINE__, size * nmemb);
    }

    return ptr;
}

void flexop_free(void *ptr)
{
    if (ptr != NULL) free(ptr);
}

FLEXOP_FLOAT flexop_atof(const char *ptr)
{
    double t;
    char *end;

#if FLEXOP_USE_LONG_DOUBLE
    t = strtold(ptr, &end);
#else
    t = strtod(ptr, &end);
#endif

    if (end == ptr || (*end != '\0' && isspace(*end) == 0)) {
        flexop_error(1, "flexop: ptr: \"%s\" is not a float number.\n", ptr);
    }

    return t;
}

FLEXOP_INT flexop_atoi(const char *ptr)
{
#if FLEXOP_USE_LONG_LONG
    long long t;
#elif FLEXOP_USE_LONG
    long t;
#else
    int t;
#endif
    char *end;

#if FLEXOP_USE_LONG_LONG
    t = strtoll(ptr, &end, 10);
#elif FLEXOP_USE_LONG
    t = strtol(ptr, &end, 10);
#else
    t = strtol(ptr, &end, 10);
#endif

    if (end == ptr || (*end != '\0' && isspace(*end) == 0)) {
        flexop_error(1, "flexop: ptr: \"%s\" is not an integer.\n", ptr);
    }

    return t;
}

FLEXOP_UINT flexop_atou(const char *ptr)
{
#if FLEXOP_USE_LONG_LONG
    unsigned long long t;
#elif FLEXOP_USE_LONG
    unsigned long t;
#else
    unsigned int t;
#endif
    char *end;

#if FLEXOP_USE_LONG_LONG
    t = strtoull(ptr, &end, 10);
#elif FLEXOP_USE_LONG
    t = strtoul(ptr, &end, 10);
#else
    t = strtoul(ptr, &end, 10);
#endif

    if (end == ptr || (*end != '\0' && isspace(*end) == 0)) {
        flexop_error(1, "flexop: ptr: \"%s\" is not an unsigned integer.\n", ptr);
    }

    return t;
}

void flexop_set_print_mark(int m)
{
    if (m) {
        flexop_print = 1;
    }
    else {
        flexop_print = 0;
    }
}
