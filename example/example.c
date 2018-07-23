
#include "flexop.h"

int main(int argc, char **argv)
{
    FLEXOP_INT i;
    FLEXOP_FLOAT f; 
    char *s;

    FLEXOP_VEC vi;
    FLEXOP_VEC vf;
    FLEXOP_VEC vs;

    int no_arg = 0;

    char *p_i = "-i 22";
    char *p_f = "-f 1.2";
    char *p_s = "-s hello";

    char *p_vi = "-vi \" 1 2 1 3 1 4\"";
    char *p_vf = "-vf \" 0.1 2e-3 1.2 3.33333 2.1 3.4\"";
    char *p_vs = "-vs \" hello this world\"";

    const char *keys[] = {"one", "two", "three", "four", NULL};
    int order = 0;

    /* preset values */
    flexop_preset(p_i);
    flexop_preset(p_f);
    flexop_preset(p_s);

    flexop_preset(p_vi);
    flexop_preset(p_vf);
    flexop_preset(p_vs);

    /* print */
    flexop_printf("Preset command line:\n");
    flexop_printf("----------------------------------\n");
    flexop_printf("%s\n", p_i);
    flexop_printf("%s\n", p_f);
    flexop_printf("%s\n", p_s);
    flexop_printf("%s\n", p_vi);
    flexop_printf("%s\n", p_vf);
    flexop_printf("%s\n", p_vs);
    flexop_printf("----------------------------------\n\n\n");

    /* register key words */
    flexop_register_int("i", "int", &i);
    flexop_register_float("f", "float", &f);
    flexop_register_string("s", "string", &s);

    flexop_register_vec_int("vi", "vector of int", &vi);
    flexop_register_vec_float("vf", "vector of float", &vf);
    flexop_register_vec_string("vs", "vector of string", &vs);

    flexop_register_no_arg("noarg", "bool value", &no_arg);
    flexop_register_keyword("order", "order of digital number", keys, &order);

    /* init, parse */
    flexop_init(&argc, &argv);

    flexop_printf("Parsed parameters:\n");
    flexop_printf("----------------------------------\n");
    flexop_printf("flexop: key: \"i\": %"IFMT"\n", i);
    flexop_printf("flexop: key: \"f\": %"FFMT"\n", f);
    flexop_printf("flexop: key: \"s\": %s\n", s);

    flexop_vec_print(&vi);
    flexop_vec_print(&vf);
    flexop_vec_print(&vs);

    if (no_arg) {
        flexop_printf("flexop: key: \"noarg\": value: true\n");
    }
    else {
        flexop_printf("flexop: key: \"noarg\": value: false\n");
    }

    flexop_printf("----------------------------------\n\n\n");

    /* set option, change parsed options */
    flexop_set_int("i", 8);
    flexop_set_float("f", 1.11111);
    flexop_set_string("s", "usa");
    flexop_set_vec_int("vi", "8 8 8 8 4 4 4 4");
    flexop_set_vec_float("vf", "11.11 2.2 3.1 4.4 4e-8");
    flexop_set_vec_string("vs", "a b c d z f g g g gg hi jill hill");

    flexop_printf("Changed parsed parameters through set option:\n");
    flexop_printf("----------------------------------\n");
    flexop_printf("flexop: key: \"i\": %"IFMT"\n", i);
    flexop_printf("flexop: key: \"f\": %"FFMT"\n", f);
    flexop_printf("flexop: key: \"s\": %s\n", s);

    flexop_vec_print(&vi);
    flexop_vec_print(&vf);
    flexop_vec_print(&vs);
    flexop_printf("----------------------------------\n\n\n");
    flexop_finalize();

    return 0;
}
