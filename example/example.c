
#include "flexop.h"

int main(int argc, char **argv)
{
    FLEXOP_INT m = 20;
    FLEXOP_FLOAT f = 1.; 
    char *hi = "hello world.";

    /* register key words */
    flexop_register_int("m", "int", &m);
    flexop_register_float("f", "float", &f);
    flexop_register_string("hi", "string", &hi);

    /* init */
    flexop_init(&argc, &argv);

    printf("m: %"IFMT"\n", m);
    printf("f: %"FFMT"\n", f);
    printf("hi: %s\n", hi);

    flexop_finalize();

    return 0;
}
