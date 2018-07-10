
#include "flexop-utils.h"

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
