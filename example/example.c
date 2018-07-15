
#include "flexop.h"

int main(int argc, char **argv)
{
    FLEXOP_INT m = 20;
    FLEXOP_FLOAT f = 1.; 
    char *hi = "hello world.";
    FLEXOP_VEC vi;

    /* register key words */
    flexop_register_int("m", "int", &m);
    flexop_register_float("f", "float", &f);
    flexop_register_string("hi", "string", &hi);
    flexop_register_vec_int("vi", "vector of int", &vi);

    /* init */
    flexop_init(&argc, &argv);

    printf("m: %"IFMT"\n", m);
    printf("f: %"FFMT"\n", f);
    printf("hi: %s\n", hi);

    flexop_vec_print(&vi);

    flexop_finalize();

    return 0;
}
