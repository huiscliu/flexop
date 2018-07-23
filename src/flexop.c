
#include "flexop.h"

static FLEXOP flexop_iopt;

static void flexop_key_destroy(FLEXOP_KEY *o)
{
    char **p;

    flexop_free(o->name);
    flexop_free(o->help);
    o->name = o->help = NULL;

    if (o->type == VT_STRING && o->used) {
        flexop_free(*(char **)o->var);
        *(char **)o->var = NULL;
    }
    else if (o->type == VT_VEC_INT || o->type == VT_VEC_FLOAT || o->type == VT_VEC_STRING) {
        flexop_vec_destroy(o->var);
    }

    if (o->keys != NULL) {
        for (p = o->keys; *p != NULL; p++) flexop_free(*p);

        flexop_free(o->keys);
        o->keys = NULL;
    }
}

void flexop_preset(const char *str)
{
    if (flexop_iopt.initialized) {
        flexop_error(1, "flexop_preset must be called before flexop_init!\n");
    }

    flexop_parse_options(&flexop_iopt.argcp, &flexop_iopt.argvp, &flexop_iopt.allocp, str);
}

static void flexop_register(const char *name, const char *help, const char **keys, void *var, FLEXOP_VTYPE type)
{
    FLEXOP_KEY *o;
    static int initialized = 0;

    if (name != NULL && flexop_iopt.initialized) {
        flexop_printf("flexop: option \"-%s\" not registered.\n", name);
        return;
    }

    if (!initialized && type != VT_INIT) {
        initialized = 1;

        /* Register title for user options */
        flexop_register("\nUser options:", "\n", NULL, "user", VT_TITLE);
        flexop_iopt.sorted = 0;
    }
    else if (type == VT_INIT) {
        /* register some global options */
        initialized = 1;
        flexop_register("\nGeneric options:", "\n", NULL, "generic", VT_TITLE);

        flexop_register("-help", "Print options help then exit", NULL, &flexop_iopt.help_category, VT_STRING);
        flexop_register("-option_file", "Options file", NULL, &flexop_iopt.opt_file, VT_STRING);

        flexop_iopt.sorted = 0;

        return;
    }

    if (name == NULL) return;

    /* mark */
    flexop_iopt.sorted = 0;

    if (*name == '-' || *name == '+') name++;

    /* invalidate index */
    if (flexop_iopt.index != NULL) {
        flexop_free(flexop_iopt.index);
        flexop_iopt.index = NULL;
    }

    /* memory */
    if (flexop_iopt.size >= flexop_iopt.alloc) {
        flexop_iopt.alloc += 8;

        flexop_iopt.options = flexop_realloc(flexop_iopt.options, flexop_iopt.alloc * sizeof(*flexop_iopt.options));
    }

    /* save option */
    o = flexop_iopt.options + (flexop_iopt.size++);

    o->name = strdup(name);
    o->help = help == NULL ? NULL : strdup(help);
    o->keys = NULL;
    o->var = var;
    o->type = type;
    o->used = 0;

    if (type == VT_KEYWORD) {
        /* make a copy of the keywords list */
        const char **p;
        char **q;

        if (keys == NULL) {
            flexop_printf("flexop_register_keyword(): keys should not be NULL (option \"-%s\").\n", name);
            flexop_printf("Option not registered.\n");
            flexop_key_destroy(flexop_iopt.options + flexop_iopt.size);
            flexop_iopt.size--;
            return;
        }

        if (keys[0] == NULL) {
            flexop_key_destroy(flexop_iopt.options + flexop_iopt.size);
            flexop_iopt.size--;
            return;
        }

        for (p = keys; *p != NULL; p++);

        o->keys = flexop_alloc((p - keys + 1) * sizeof(*keys));

        for (p = keys, q = o->keys; *p != NULL; p++, q++) {
            if ((*p)[0] == '\0') {
                flexop_printf("WARNING: empty string in the keywords list for the option \"-%s\".\n", o->name);
            }

            *q = strdup(*p);
        }

        *q = NULL;
    }
    else if (type == VT_VEC_INT) {
        flexop_vec_init((FLEXOP_VEC *)o->var, VT_INT, -1, name);
    }
    else if (type == VT_VEC_FLOAT) {
        flexop_vec_init((FLEXOP_VEC *)o->var, VT_FLOAT, -1, name);
    }
    else if (type == VT_VEC_STRING) {
        flexop_vec_init((FLEXOP_VEC *)o->var, VT_STRING, -1, name);
    }
}

/* Wrapper functions for enforcing prototype checking */
void flexop_register_no_arg(const char *name, const char *help, int *var)
{
    flexop_register(name, help, NULL, var, VT_NONE);
}

void flexop_register_int(const char *name, const char *help, FLEXOP_INT *var)
{
    flexop_register(name, help, NULL, var, VT_INT);
}

void flexop_register_float(const char *name, const char *help, FLEXOP_FLOAT *var)
{
    flexop_register(name, help, NULL, var, VT_FLOAT);
}

void flexop_register_string(const char *name, const char *help, char **var)
{
    flexop_register(name, help, NULL, var, VT_STRING);
}

void flexop_register_keyword(const char *name, const char *help, const char **keys, int *var)
{
    flexop_register(name, help, keys, var, VT_KEYWORD);
}

void flexop_register_title(const char *str, const char *help, const char *category)
{
    /* Note: category will be stored in '->var' */
    flexop_register(str, help, NULL, (void *)category, VT_TITLE);
}

void flexop_register_handler(const char *name, const char *help, FLEXOP_HANDLER func)
{
    flexop_register(name, help, NULL, func, VT_HANDLER);
}

void flexop_register_vec_int(const char *name, const char *help, FLEXOP_VEC *var)
{
    flexop_register(name, help, NULL, var, VT_VEC_INT);
}

void flexop_register_vec_float(const char *name, const char *help, FLEXOP_VEC *var)
{
    flexop_register(name, help, NULL, var, VT_VEC_FLOAT);
}

void flexop_register_vec_string(const char *name, const char *help, FLEXOP_VEC *var)
{
    flexop_register(name, help, NULL, var, VT_VEC_STRING);
}

static int flexop_comp(const void *i0, const void *i1)
{
    FLEXOP_KEY *o0 = flexop_iopt.options + *(int *)i0, *o1 = flexop_iopt.options + *(int *)i1;

    /* This makes all separators to appear at the end of the list */
    if (o0->type == VT_TITLE || o1->type == VT_TITLE) {
        if (o0->type != VT_TITLE) return -1;

        if (o1->type != VT_TITLE) return 1;

        return (int)(o0 - o1);
    }

    return strcmp(o0->name, o1->name);
}

void flexop_sort(FLEXOP *opt)
{
    int i;

    if (opt->index != NULL) return;

    if (opt->sorted) return;

    /* append a dummy entry at the end of the list as the key for bsearch */
    flexop_register("dummy", NULL, NULL, NULL, VT_NONE);
    opt->index = flexop_alloc(opt->size * sizeof(*opt->index));

    for (i = 0; i < (int)opt->size; i++) opt->index[i] = i;

    /* restore real size */
    opt->size--;

    qsort(opt->index, opt->size, sizeof(*opt->index), flexop_comp);

    /* clean up */
    flexop_free(opt->options[opt->size].name);
    opt->options[opt->size].name = NULL;

    /* mark */
    opt->sorted = 1;
}

void flexop_reset(FLEXOP *opt)
{
    FLEXOP_KEY *o;

    if (opt->options != NULL) {
        for (o = opt->options; o < opt->options + opt->size; o++) flexop_key_destroy(o);

        flexop_free(opt->options);
        flexop_free(opt->index);

        opt->options = NULL;
        opt->index = NULL;
        opt->size = opt->alloc = 0;
    }
}

/* format and print the help text of option 'o' */
void flexop_print_help(FLEXOP_KEY *o, const char *help)
{
    static char *indent = "     ";
    char buffer[4096];
    char c, *p;
    const char *p0;
    size_t len;

    if (help == NULL) return;

    /* copy help text, expanding tabs */
    len = 0;
    p = buffer;
    p0 = help;
    while (*p0 != '\0' && p - buffer < (int)sizeof(buffer) - 1) {
        c = *(p0++);

        if (c == '\t') {
            *(p++) = ' ';
            len++;

            while (len % 8 != 0 && p - buffer < (int)sizeof(buffer) - 1) {
                *(p++) = ' ';
                len++;
            }

            continue;
        }

        *(p++) = c;
        len++;

        if (c == '\n') len = 0;
    }

    if (o->type == VT_KEYWORD) {
        char **pp;

        /* +1 for '>' */
        assert(p + 3 < buffer + sizeof(buffer));

        memcpy(p, " <\"", 3);
        p += 3;

        for (pp = o->keys; *pp != NULL; pp++) {
            if (pp > o->keys) {
                assert(p + 5 < buffer + sizeof(buffer));
                memcpy(p, "\", \"", 4);
                p += 4;
            }

            len = strlen(*pp);

            assert(p + len + 1 < buffer + sizeof(buffer));

            memcpy(p, *pp, len);

            p += len;
        }

        *(p++) = '"';
        *(p++) = '>';
    }
    else if (o->type == VT_NONE) {
        p0 = " (the opposite option is \"+%s\")";
        len = strlen(p0) - 2 + strlen(o->name);;

        assert(p + len + 1 < buffer + sizeof(buffer));

        sprintf(p, p0, o->name);
        p += len;
    }

    *p = '\0';

    len = sizeof(indent) - 1;
    flexop_printf(indent);
    p0 = p = buffer;

    while (*p != '\0') {
        while (*p != '\0' && *p != '\n' && isspace(*(char *)p)) p++;

        while (*p != '\0' && *p != '\n' && !isspace(*(char *)p)) p++;

        c = *p;
        if (*p != '\0') *p = '\0';

        if (p == p0) break;

        if (p - p0 + len + 1 >= 78 && len >= sizeof(indent)) {
            flexop_printf("\n%s", indent);
            len = sizeof(indent) - 1;

            while (isspace(*(const char *)p0)) p0++;
        }

        flexop_printf("%s", p0);

        if (c != '\0') {
            flexop_printf("%c", c);
            *(p++) = c;
        }

        len += p - p0;
        if (c == '\n') {
            flexop_printf("%s", indent);
            len = sizeof(indent) - 1;

            while (isspace(*(char *)p)) p++;
        }

        p0 = p;
    }

    flexop_printf("%s\n", p0);
}

void flexop_show_cmdline(void)
{
    int i;

    if (flexop_iopt.argc > 0) {
        flexop_printf("Command-line:");

        for (i = 0; i < flexop_iopt.argc; i++) flexop_printf(" %s", flexop_iopt.argv[i]);

        flexop_printf("\n");
    }

    if (flexop_iopt.argcp > 0) {
        flexop_printf("Preset:");

        for (i = 0; i < flexop_iopt.argcp; i++) flexop_printf(" %s", flexop_iopt.argvp[i]);

        flexop_printf("\n");
    }

    if (flexop_iopt.argcf > 0) {
        flexop_printf("Option file:");

        for (i = 0; i < flexop_iopt.argcf; i++) flexop_printf(" %s", flexop_iopt.argvf[i]);

        flexop_printf("\n");
    }
}

/* prints all options which have been called by the user */
void flexop_show_used(void)
{
    int flag = 0, i;
    FLEXOP_KEY *o;
    char **pp;

    if (!flexop_iopt.initialized) flexop_error(1, "%s must be called after flexop_init!\n", __func__);

    for (o = flexop_iopt.options; o < flexop_iopt.options + flexop_iopt.size; o++) {
        if (o->used == 0 || o->type == VT_TITLE) continue;

        if (!flag) {
            flexop_printf("*-------------------- "
                    "Parameter(s) set through options "
                    "--------------------\n");
            flag = 1;
        }

        switch (o->type) {
            case VT_INIT:
                flexop_error(1, "unexpected.\n");
                break;

            case VT_NONE:
                flexop_printf("* %s: %s\n", o->help == NULL ? o->name : o->help,
                        *(int *)o->var == 1 ? "True" : "False");
                break;

            case VT_INT:
                flexop_printf("* %s: %"IFMT"\n", o->help == NULL ? o->name : o->help,
                        *(FLEXOP_INT *)o->var);
                break;

            case VT_FLOAT:
                flexop_printf("* %s: %"FFMT"\n", o->help == NULL ? o->name : o->help,
                        (FLEXOP_FLOAT)*(FLEXOP_FLOAT *)o->var);
                break;

            case VT_STRING:
                flexop_printf("* %s: %s\n", o->help == NULL ? o->name : o->help,
                        *(char **)o->var == NULL ?  "none" : *(char **)o->var);
                break;

            case VT_KEYWORD:
                flexop_printf("* %s: %s\n", o->help == NULL ? o->name : o->help,
                        *(int *)o->var < 0 ?  "none" : (o->keys)[*(int *)o->var]);
                break;

            case VT_HANDLER:
                flexop_printf("* %s", o->help == NULL ? o->name : o->help);

                if ((pp = o->keys) != NULL && pp[0] != NULL && pp[1] != NULL) {
                    flexop_printf(":\n");

                    for (; pp != NULL && *pp != NULL; pp++) flexop_printf("*   %s\n", *pp);
                }
                else if ((pp = o->keys) != NULL && pp[0] != NULL) {
                    flexop_printf(": %s\n", *pp);
                }
                else {
                    flexop_printf("\n");
                }

                break;

            case VT_TITLE:
                break;

            case VT_VEC_INT:
                {
                    FLEXOP_VEC *v;

                    flexop_printf("* %s:", o->help == NULL ? o->name : o->help);

                    v = o->var;
                    for (i = 0; i < v->size; i++) {
                        flexop_printf(" %"IFMT, flexop_vec_int_get_value(v, i));
                    }

                    flexop_printf("\n");
                }

                break;

            case VT_VEC_FLOAT:
                {
                    FLEXOP_VEC *v;

                    flexop_printf("* %s:", o->help == NULL ? o->name : o->help);

                    v = o->var;
                    for (i = 0; i < v->size; i++) {
                        flexop_printf(" %"FFMT, flexop_vec_float_get_value(v, i));
                    }

                    flexop_printf("\n");
                }

                break;

            case VT_VEC_STRING:
                {
                    FLEXOP_VEC *v;

                    flexop_printf("* %s:", o->help == NULL ? o->name : o->help);

                    v = o->var;
                    for (i = 0; i < v->size; i++) {
                        flexop_printf(" %s", flexop_vec_string_get_value(v, i));
                    }

                    flexop_printf("\n");
                }

                break;
        }
    }

    if (flag) {
        flexop_printf("*-----------------------------------------------------"
                "-------------------------\n");
    }
}

static int comp_string(const void *p1, const void *p2)
{
    return strcmp(*(char **)p1, *(char **)p2);
}

void flexop_help(void)
{
    FLEXOP_KEY *o;
    char **pp;
    int all_flag, flag, matched;
    char **list = NULL;
    int i, list_count = 0, list_allocated = 0;

    if (flexop_iopt.help_category == NULL) return;

    flag = 1;
    matched = 0;
    all_flag = !strcmp(flexop_iopt.help_category, "all");

    for (o = flexop_iopt.options; o < flexop_iopt.options + flexop_iopt.size; o++) {
        if (!all_flag && o->type == VT_TITLE) {
            flag = (o->var == NULL || !strcmp(flexop_iopt.help_category, o->var));

            if (o->var != NULL && (list_count == 0 || strcmp(list[list_count - 1], o->var))) {
                if (list_count >= list_allocated) {
                    list = flexop_realloc(list, (list_allocated + 128) * sizeof(*list));
                    list_allocated += 128;
                }

                list[list_count++] = o->var;
            }
        }

        if (!flag) continue;

        /* skip option without help text */
        if (o->help == NULL) continue;

        matched = 1;
        switch (o->type) {
            case VT_INIT:
                flexop_error(1, "unexpected.\n");
                break;

            case VT_NONE:
                flexop_printf("  -%s (%s)", o->name, *(int *)o->var ? "True" : "False");
                break;

            case VT_INT:
                flexop_printf("  -%s <integer> (%"IFMT")", o->name, *(FLEXOP_INT *)o->var);
                break;

            case VT_FLOAT:
                flexop_printf("  -%s <real> (%"FFMT")", o->name, (FLEXOP_FLOAT)*(FLEXOP_FLOAT *)o->var);
                break;

            case VT_STRING:
                flexop_printf("  -%s <string> (\"%s\")", o->name, 
                        *(char **)o->var == NULL ?  "none" : *(char **)o->var);
                break;

            case VT_KEYWORD:
                flexop_printf("  -%s <keyword> (\"%s\")", o->name, 
                        *(int *)o->var<0 ? "none" : (o->keys)[*(int *)o->var]);
                break;

            case VT_HANDLER:
                flexop_printf("  -%s <string>", o->name);
                if ((pp = o->keys) != NULL && *pp != NULL && pp[1] == NULL)
                    flexop_printf(" (%s)", *pp);
                break;

            case VT_TITLE:
                flexop_printf("%s", o->name);

                if (o->var != NULL) flexop_printf(" (category \"%s\")", (char *)o->var);

                break;

            case VT_VEC_INT:
                {
                    FLEXOP_VEC *v;

                    flexop_printf("  -%s <integer> (", o->name);

                    v = o->var;
                    for (i = 0; i < v->size; i++) {
                        flexop_printf("%"IFMT, flexop_vec_int_get_value(v, i));

                        if (i < v->size - 1) flexop_printf(" ");
                    }

                    flexop_printf(")");
                }

                break;

            case VT_VEC_FLOAT:
                {
                    FLEXOP_VEC *v;

                    flexop_printf("  -%s <float> (", o->name);

                    v = o->var;
                    for (i = 0; i < v->size; i++) {
                        flexop_printf("%"FFMT, flexop_vec_float_get_value(v, i));

                        if (i < v->size - 1) flexop_printf(" ");
                    }

                    flexop_printf(")");
                }

                break;

            case VT_VEC_STRING:
                {
                    FLEXOP_VEC *v;

                    flexop_printf("  -%s <string> (", o->name);

                    v = o->var;
                    for (i = 0; i < v->size; i++) {
                        flexop_printf("%s", flexop_vec_string_get_value(v, i));

                        if (i < v->size - 1) flexop_printf(" ");
                    }

                    flexop_printf(")");
                }

                break;
        }

        flexop_printf("\n");
        if (o->help != NULL) flexop_print_help(o, o->help);

        if (o->type == VT_HANDLER && o->var != NULL) ((FLEXOP_HANDLER)o->var)(o, NULL);
    }

    if (matched) {
        flexop_printf("\n");
    }
    else {
        if (strcmp(flexop_iopt.help_category, "help")) {
            flexop_printf("Unknown help category '%s'.\n", flexop_iopt.help_category);
        }

        qsort(list, list_count, sizeof(*list), comp_string);
        flexop_printf("Usage:\n    %s -help <category>\n"
                "where <category> should be one of:\n", flexop_iopt.argv[0]);

        flexop_printf("    all");
        list_allocated = 7;

        for (i = 0; i < list_count; i++) {
            if ((list_allocated += strlen(list[i]) + 2) > 78) {
                list_allocated = strlen(list[i]) + 4;
                flexop_printf(",\n    %s", list[i]);
            }
            else {
                flexop_printf(", %s", list[i]);
            }
        }

        flexop_printf("\n");
    }

    flexop_free(list);

    flexop_reset(&flexop_iopt);

    exit(0);
}

/*---------------------------------------------------------------------------*/
void flexop_parse_options(int *argc, char ***argv, int *alloc, const char *optstr)
{
    char quote = '\0', c, *p, *q;
    const char *optstr0 = optstr, *r;
    int ac = *argc;

    p = flexop_alloc(strlen(optstr) + 1);

    while (1) {
        while (isspace(*(const char *)optstr)) optstr++;

        if (*optstr == '#') {
            /* skip to eol */
            while (*(++optstr) != '\n' && *optstr != '\0');

            if (*optstr == '\n') continue;
        }

        if (*optstr == '\0') break;

        q = p;
        while (1) {
            if ((c = *optstr) == '\0' || (quote == '\0' && isspace((char)c))) break;

            if (c == quote) {
                quote = '\0';
                optstr++;
                continue;
            }

            if (c == '\\') {
                if ((c = *(++optstr)) == '\0') break;

                *(q++) = c;
                optstr++;
                continue;
            }

            if (quote == '\0' && (c == '\'' || c == '"')) {
                r = optstr;

                while (--r >= optstr0 && *r == '\\');

                if (((int)(optstr - r - 1)) % 2 == 0) {
                    quote = c;
                    optstr++;
                    continue;
                }
            }

            *(q++) = c;
            optstr++;
        }

        if (quote != '\0') {
            if (c == '\0') flexop_error(1, "invalid string: %s\n", optstr0);

            optstr++;
        }

        *q = '\0';
        if (ac >= *alloc - 1) {
            *argv = flexop_realloc(*argv, (*alloc + 16) * sizeof(**argv));
            *alloc += 16;
        }

        (*argv)[ac++] = strdup(p);
    }

    flexop_free(p);

    if (ac >= *alloc) {
        *argv = flexop_realloc(*argv, ((*alloc) + 1) * sizeof(**argv));
        ++(*alloc);
    }

    (*argv)[ac] = NULL;

    if (*argc < ac) *argc = ac;
}

/* processes options from file 'fn' */
void flexop_parse_options_file(const char *fn)
{
    FILE *f;
    char *p, buffer[4096];

    if ((f = fopen(fn, "r")) == NULL) {
        flexop_printf("flexop: cannot open options file \"%s\".\n", fn);
        exit(1);
    }

    while (1) {
        if (fgets(buffer, sizeof(buffer), f) == NULL) break;

        p = buffer;
        while (isspace(*(char *)p)) p++;

        if (*p == '#' || *p == '\0') continue;

        flexop_parse_options(&flexop_iopt.argcf, &flexop_iopt.argvf, &flexop_iopt.allocf, p);
    }

    fclose(f);

    if (flexop_iopt.argcf == 0) return;

    flexop_iopt.argvf[flexop_iopt.argcf] = NULL;

    /* parse */
    flexop_parse_cmdline(flexop_iopt.argcf, &flexop_iopt.argvf);
}

/* parses cmdline parameters, processes and removes known options from
   the argument list */
void flexop_parse_cmdline(int argc, char ***argv)
{
    FLEXOP_KEY *o, *key = NULL;
    char **pp;
    char *p, *arg;
    int i, j;
    int *k = NULL;                /* points to sorted indices of options */

    if (argc <= 0) return;

    flexop_sort(&flexop_iopt);
    key = flexop_iopt.options + flexop_iopt.size;

    /* parse */
    for (i = 0; i < argc; i++) {
        char *q;
        o = NULL;
        arg = NULL;
        k = NULL;

        if ((p = (*argv)[i])[0] == '-' || p[0] == '+') {
            q = strdup(p[0] == '-' && p[1] == '-' ? p + 2 : p + 1);
            if ((arg = strchr(p + 1, '=')) != NULL) {
                q[arg - p - 1] = '\0';
                arg++;
            }

            key->name = q;
            k = bsearch(flexop_iopt.index + flexop_iopt.size, flexop_iopt.index, flexop_iopt.size,
                    sizeof(*flexop_iopt.index), flexop_comp);
            flexop_free(q);
            key->name = NULL;
        }

        if (k == NULL) flexop_error(1, "unknown option \"%s\"!\n", p);

        o = flexop_iopt.options + (*k);

        /* process option */
        if (o->type != VT_NONE) {
            if (arg == NULL && (arg = (*argv)[++i]) == NULL) {
                if (!strcmp(o->name, flexop_iopt.help_category = strdup("help"))) {
                    flexop_printf("Missing argument for option \"%s\".\n", p);
                    flexop_help();
                }
                flexop_error(1, "missing argument for option \"%s\".\n", p);
            }
        }

        switch (o->type) {
            case VT_INIT:
                flexop_error(1, "unexpected.\n");
                break;

            case VT_NONE:
                *(int *)o->var = (p[0] == '-' ? 1 : 0);
                o->used = 1;
                break;

            case VT_INT:
                *(FLEXOP_INT *)o->var = flexop_atoi(arg);
                o->used = 1;
                break;

            case VT_FLOAT:
                *(FLEXOP_FLOAT *)o->var = flexop_atof(arg);
                o->used = 1;
                break;

            case VT_STRING:
                p = *(char **)o->var;
                if (o->used) flexop_free(*(char **)o->var);

                *(char **)o->var = strdup(arg);
                o->used = 1;
                break;

            case VT_KEYWORD:
                p = (*(int *)o->var < 0 ? NULL : o->keys[*(int *)o->var]);

                if (arg == NULL) {
                    *(int *)o->var = -1;
                    o->used = 1;
                    break;
                }

                for (pp = o->keys; *pp != NULL; pp++)
                    if (!strcmp(*pp, arg)) break;

                if (*pp == NULL) {
                    flexop_printf("Invalid argument \"%s\" "
                            "for the option \"-%s\".\n", arg, o->name);
                    flexop_printf("Valid keywords are: ");
                    for (pp = o->keys; *pp != NULL; pp++)
                        flexop_printf("%s\"%s\"", pp == o->keys ? "":", ", *pp);
                    flexop_printf("\n");
                    flexop_error(1, "abort.\n");
                    break;
                }

                *(int *)o->var = pp - o->keys;
                o->used = 1;

                break;

            case VT_HANDLER:
                /* save option value in o->keys */
                j = 0;
                if (o->keys != NULL) {
                    flexop_free(o->keys[0]);
                    o->keys[0] = NULL;
                }

                o->keys = flexop_realloc(o->keys, (j + 2) * sizeof(*o->keys));
                o->keys[j] = strdup(arg);
                o->keys[j + 1] = NULL;

                /* call user supplied option handler */
                if (o->var != NULL) {
                    if (!((FLEXOP_HANDLER)o->var)(o, arg)) {
                        flexop_printf("invalid argument for \"-%s\" option.\n", o->name);

                        ((FLEXOP_HANDLER)o->var)(o, NULL);
                        flexop_error(1, "abort.\n");
                    }
                }

                o->used = 1;
                break;

            case VT_TITLE:
                /* cannot happen */
                break;

            case VT_VEC_INT:
                {
                    FLEXOP_VEC *v;
                    FLEXOP_INT tp;
                    char *ta = NULL;
                    char *ip;

                    /* init vec */
                    v = o->var;
                    if (o->used && flexop_vec_initialized(v)) {
                        flexop_vec_destroy(v);
                        flexop_vec_init(v, VT_INT, -1, o->name);
                    }
                    
                    /* parse */
                    ta = strdup(arg);

                    ip = strtok(ta, " \t");

                    while (ip != NULL) {
                        tp = flexop_atoi(ip);

                        flexop_vec_add_entry(v, &tp);
                        ip = strtok(NULL, " \t");
                    }

                    o->used = 1;
                    free(ta);
                }

                break;

            case VT_VEC_FLOAT:
                {
                    FLEXOP_VEC *v;
                    FLEXOP_FLOAT tp;
                    char *ta = NULL;
                    char *ip;

                    /* init vec */
                    v = o->var;
                    if (o->used && flexop_vec_initialized(v)) {
                        flexop_vec_destroy(v);
                        flexop_vec_init(v, VT_FLOAT, -1, o->name);
                    }
                    
                    /* parse */
                    ta = strdup(arg);

                    ip = strtok(ta, " \t");

                    while (ip != NULL) {
                        tp = flexop_atof(ip);

                        flexop_vec_add_entry(v, &tp);
                        ip = strtok(NULL, " \t");
                    }

                    o->used = 1;
                    free(ta);
                }

                break;

            case VT_VEC_STRING:
                {
                    FLEXOP_VEC *v;
                    char *ta = NULL;
                    char *ip;

                    /* init vec */
                    v = o->var;
                    if (o->used && flexop_vec_initialized(v)) {
                        flexop_vec_destroy(v);
                        flexop_vec_init(v, VT_STRING, -1, o->name);
                    }
                    
                    /* parse */
                    ta = strdup(arg);

                    ip = strtok(ta, " \t");

                    while (ip != NULL) {
                        flexop_vec_add_entry(v, ip);
                        ip = strtok(NULL, " \t");
                    }

                    o->used = 1;
                    free(ta);
                }

                break;
        }
    }

    return;
}

void flexop_parse(int *argc, char ***argv)
{
    static int firstcall = 1;     /* 1st time calling this function */
    FLEXOP_KEY *o;
    int i, j;

    if (!firstcall) {
        flexop_error(1, "flexop: flexop_parse can be called only once.\n");
    }

    /* mark */
    firstcall = 0;

    /* register internal opt */
    if (flexop_iopt.options == NULL) {
        /* this will register the options '-help' */
        flexop_register(NULL, NULL, NULL, NULL, VT_NONE);
    }

    flexop_sort(&flexop_iopt);

    /* check and warn duplicate options */
    j = 0;
    for (i = 1; i < (int)flexop_iopt.size; i++) {
        o = flexop_iopt.options + flexop_iopt.index[i];
        if (o->type == VT_TITLE) break;

        if (flexop_comp(flexop_iopt.index + i, flexop_iopt.index + j) == 0)
            flexop_warning("duplicate option \"-%s\".\n",o->name);
        j = i;
    }

    /* handle preset options */
    flexop_parse_cmdline(flexop_iopt.argcp, &flexop_iopt.argvp);

    /* handle command line */
    assert(*argc > 0);
    flexop_iopt.argc = *argc - 1;
    flexop_iopt.argv = flexop_alloc((flexop_iopt.argc + 1) * sizeof(*flexop_iopt.argv));

    for (i = 0; i < flexop_iopt.argc; i++) flexop_iopt.argv[i] = strdup((*argv)[i + 1]);
    flexop_iopt.argv[i] = NULL;

    /* parse */
    flexop_parse_cmdline(flexop_iopt.argc, &flexop_iopt.argv);

    /* parse option file */
    if (flexop_iopt.opt_file != NULL) flexop_parse_options_file(flexop_iopt.opt_file);

    return;
}

void flexop_init(int *argc, char ***argv)
{
    /* option init */
    flexop_register(NULL, NULL, NULL, NULL, VT_INIT);

    /* option parse */
    flexop_parse(argc, argv);
    flexop_help();

    /* mark status, has been initialized */
    flexop_iopt.initialized = 1;
}

void flexop_finalize(void)
{
    int i;

    flexop_reset(&flexop_iopt);

    /* clean up, argv */
    for (i = 0; i < flexop_iopt.argc; i++) {
        free(flexop_iopt.argv[i]);
    }

    flexop_free(flexop_iopt.argv);

    /* clean up, argvp */
    for (i = 0; i < flexop_iopt.argcp; i++) {
        free(flexop_iopt.argvp[i]);
    }

    if (flexop_iopt.argcp > 0) flexop_free(flexop_iopt.argvp);

    /* clean up, argvf */
    for (i = 0; i < flexop_iopt.argcf; i++) {
        free(flexop_iopt.argvf[i]);
    }

    if (flexop_iopt.argcf > 0) flexop_free(flexop_iopt.argvf);

    flexop_iopt.initialized = 0;
}

static int get_option(const char *op_name, void **pvar, int type, const char *func)
{
    int *k;
    FLEXOP_KEY *o;
    FLEXOP_KEY *key;

    if (!flexop_iopt.initialized) flexop_error(1, "%s must be called after flexop_init!\n", func);

    if (op_name[0] == '-' || op_name[1] == '+') op_name++;

    /* get key */
    key = flexop_iopt.options + flexop_iopt.size;

    key->name = (void *)op_name;
    k = bsearch(flexop_iopt.index + flexop_iopt.size, flexop_iopt.index, flexop_iopt.size,
            sizeof(*flexop_iopt.index), flexop_comp);
    key->name = NULL;        /* reset key->name */

    if (k == NULL) flexop_error(1, "%s: unknown option \"-%s\"!\n", func, op_name);

    o = flexop_iopt.options + (*k);
    if (type >= 0 && (int)o->type != type) {
        flexop_printf("%s: wrong function type for \"-%s\".", func, op_name);
        switch (o->type) {
            case VT_NONE:
                flexop_error(1, "Please use flexop_get_no_arg instead.\n");
                break;

            case VT_INT:
                flexop_error(1, "Please use flexop_get_int instead.\n");
                break;

            case VT_FLOAT:
                flexop_error(1, "Please use flexop_get_double instead.\n");
                break;

            case VT_STRING:
                flexop_error(1, "Please use flexop_get_string instead.\n");
                break;

            case VT_KEYWORD:
                flexop_error(1, "Please use flexop_get_keyword instead.\n");
                break;

            case VT_HANDLER:
                flexop_error(1, "Please use options_get_handler instead.\n");
                break;

            case VT_VEC_INT:
                flexop_error(1, "Please use flexop_get_vec_int instead.\n");
                break;

            case VT_VEC_FLOAT:
                flexop_error(1, "Please use flexop_get_vec_float instead.\n");
                break;

            case VT_VEC_STRING:
                flexop_error(1, "Please use flexop_get_vec_string instead.\n");
                break;

            default:
                flexop_error(1, "No flexop_get_XXXX function for \"-%s\".\n", op_name);
                break;
        }
    }

    *pvar = NULL;
    switch (o->type) {
        case VT_NONE:
        case VT_INT:
        case VT_FLOAT:
            *pvar = o->var;
            break;

        case VT_STRING:
            *pvar = *(char **)o->var;
            break;

        case VT_KEYWORD:
            *pvar = (*(int *)o->var < 0 ? "none" : o->keys[*(int *)o->var]);
            break;

        case VT_VEC_INT:
        case VT_VEC_FLOAT:
        case VT_VEC_STRING:
            *pvar = o->var;
            break;

        default:
            /* not allowed */
            flexop_error(1, "%s:%d: unsupported or unimplemented option type.\n", __FILE__, __LINE__);
    }

    return *pvar == NULL ? 0 : 1;
}

int flexop_get_no_arg(const char *op_name)
{
    void *value;

    get_option(op_name, &value, VT_NONE, __func__);

    return *(int *)value;
}

FLEXOP_INT flexop_get_int(const char *op_name)
{
    void *value;

    get_option(op_name, &value, VT_INT, __func__);

    return *(FLEXOP_INT *)value;
}

FLEXOP_FLOAT flexop_get_float(const char *op_name)
{
    void *value;

    get_option(op_name, &value, VT_FLOAT, __func__);

    return *(FLEXOP_FLOAT *)value;
}

const char * flexop_get_keyword(const char *op_name)
{
    void *value;

    get_option(op_name, &value, VT_KEYWORD, __func__);

    return value;
}

const char * flexop_get_string(const char *op_name)
{
    void *value;

    get_option(op_name, &value, VT_STRING, __func__);

    return value;
}

FLEXOP_VEC * flexop_get_vec_int(const char *op_name)
{
    void *value;

    get_option(op_name, &value, VT_VEC_INT, __func__);

    return value;
}

FLEXOP_VEC * flexop_get_vec_float(const char *op_name)
{
    void *value;

    get_option(op_name, &value, VT_VEC_FLOAT, __func__);

    return value;
}

FLEXOP_VEC * flexop_get_vec_string(const char *op_name)
{
    void *value;

    get_option(op_name, &value, VT_VEC_STRING, __func__);

    return value;
}

static int set_option(const char *op_name, void *value, int type, const char *func)
{
    int j, *k;
    FLEXOP_KEY *o, *key;
    char **pp;

    if (!flexop_iopt.initialized)
        flexop_error(1, "%s must be called after flexop_init!\n", func);

    if (value == NULL) return 1;

    if (op_name[0] == '-' || op_name[1] == '+') op_name++;

    key = flexop_iopt.options + flexop_iopt.size;

    key->name = (void *)op_name;
    k = bsearch(flexop_iopt.index + flexop_iopt.size, flexop_iopt.index, flexop_iopt.size,
            sizeof(*flexop_iopt.index), flexop_comp);

    key->name = NULL;        /* reset key->name */

    if (k == NULL) flexop_error(1, "%s: unknown option \"-%s\"!\n", func, op_name);

    o = flexop_iopt.options + (*k);

    if (type >= 0 && (int)o->type != type) {
        flexop_printf("%s: wrong function type for \"-%s\".", func, op_name);
        switch (o->type) {
            case VT_NONE:
                flexop_error(1, "Please use flexop_set_no_arg instead.\n");
                break;

            case VT_INT:
                flexop_error(1, "Please use flexop_set_int instead.\n");
                break;

            case VT_FLOAT:
                flexop_error(1, "Please use flexop_set_double instead.\n");
                break;

            case VT_STRING:
                flexop_error(1, "Please use flexop_set_string instead.\n");
                break;

            case VT_KEYWORD:
                flexop_error(1, "Please use flexop_set_keyword instead.\n");
                break;

            case VT_HANDLER:
                flexop_error(1, "Please use flexop_set_handler instead.\n");
                break;

            case VT_VEC_INT:
            case VT_VEC_FLOAT:
            case VT_VEC_STRING:
                flexop_error(1, "Please use flexop_set_handler instead.\n");
                break;

            default:
                flexop_error(1, "No flexop_set_xyz function for \"-%s\".\n", op_name);
                break;
        }
    }

    switch (o->type) {
        case VT_NONE:
            *(int *)o->var = *(int *)value;
            o->used = 1;
            break;

        case VT_INT:
            *(FLEXOP_INT *)o->var = *(FLEXOP_INT *)value;
            o->used = 1;
            break;

        case VT_FLOAT:
            *(FLEXOP_FLOAT *)o->var = *(FLEXOP_FLOAT *)value;
            o->used = 1;
            break;

        case VT_STRING:
            flexop_free(*(char **)o->var);
            *(char **)o->var = strdup((const char *)value);
            o->used = 1;
            break;

        case VT_KEYWORD:
            if (value == NULL) {
                *(int *)o->var = -1;
                o->used = 1;
                break;
            }

            for (pp = o->keys; *pp != NULL; pp++)
                if (!strcmp(*pp, value)) break;

            if (*pp == NULL) {
                flexop_printf("%s:%d, invalid argument \"%s\" "
                        "for the option \"-%s\".\n", __FILE__, __LINE__, value, op_name);

                flexop_printf("Valid keywords are: ");
                for (pp = o->keys; *pp != NULL; pp++) flexop_printf("%s\"%s\"", pp == o->keys ? "":", ", *pp);

                flexop_printf("\n");
                flexop_error(1, "abort.\n");
                break;
            }

            *(int *)o->var = pp - o->keys;
            o->used = 1;
            break;

        case VT_HANDLER:
            j = 0;
            if (o->keys != NULL) {
                flexop_free(o->keys[0]);
                o->keys[0] = NULL;
            }

            o->keys = flexop_realloc(o->keys, (j + 2) * sizeof(*o->keys));
            o->keys[j] = strdup(value);
            o->keys[j + 1] = NULL;

            /* call user supplied option handler */
            if (o->var != NULL) {
                if (!((FLEXOP_HANDLER)o->var)(o, value)) {
                    flexop_printf("invalid argument for \"-%s\" option.\n", o->name);

                    ((FLEXOP_HANDLER)o->var)(o, NULL);
                    flexop_error(1, "flexop: abort.\n");
                }
            }

            o->used = 1;
            break;

            case VT_VEC_INT:
            {
                FLEXOP_VEC *v;
                FLEXOP_INT tp;
                char *ta = NULL;
                char *ip;

                /* init vec */
                v = o->var;
                if (o->used && flexop_vec_initialized(v)) {
                    flexop_vec_destroy(v);
                    flexop_vec_init(v, VT_INT, -1, o->name);
                }

                /* parse */
                ta = strdup(value);
                ip = strtok(ta, " \t");

                while (ip != NULL) {
                    tp = flexop_atoi(ip);

                    flexop_vec_add_entry(v, &tp);
                    ip = strtok(NULL, " \t");
                }

                o->used = 1;
                free(ta);
            }

            break;

            case VT_VEC_FLOAT:
                {
                    FLEXOP_VEC *v;
                    FLEXOP_FLOAT tp;
                    char *ta = NULL;
                    char *ip;

                    /* init vec */
                    v = o->var;
                    if (o->used && flexop_vec_initialized(v)) {
                        flexop_vec_destroy(v);
                        flexop_vec_init(v, VT_FLOAT, -1, o->name);
                    }
                    
                    /* parse */
                    ta = strdup(value);
                    ip = strtok(ta, " \t");

                    while (ip != NULL) {
                        tp = flexop_atof(ip);

                        flexop_vec_add_entry(v, &tp);
                        ip = strtok(NULL, " \t");
                    }

                    o->used = 1;
                    free(ta);
                }

                break;

            case VT_VEC_STRING:
                {
                    FLEXOP_VEC *v;
                    char *ta = NULL;
                    char *ip;

                    /* init vec */
                    v = o->var;
                    if (o->used && flexop_vec_initialized(v)) {
                        flexop_vec_destroy(v);
                        flexop_vec_init(v, VT_STRING, -1, o->name);
                    }
                    
                    /* parse */
                    ta = strdup(value);
                    ip = strtok(ta, " \t");

                    while (ip != NULL) {
                        flexop_vec_add_entry(v, ip);
                        ip = strtok(NULL, " \t");
                    }

                    o->used = 1;
                    free(ta);
                }

                break;

        default:
            /* not allowed */
            flexop_error(1, "%s:%d: unsupported or unimplemented option type.\n", __FILE__, __LINE__);
    }

    return 1;
}

void flexop_set_options(const char *str)
{
    int i, argc = 0, argc_allocated = 0;
    char **argv = NULL;

    if (!flexop_iopt.initialized)
        flexop_error(1, "%s must be called after flexop_init!\n", __func__);

    if (str == NULL) return;

    flexop_parse_options(&argc, &argv, &argc_allocated, str);
    flexop_parse_cmdline(argc, &argv);

    for (i = 0; i < argc; i++) flexop_free(argv[i]);

    flexop_free(argv);
}

int flexop_set_no_arg(const char *op_name, int value)
{
    return set_option(op_name, &value, VT_NONE, __func__);
}

int flexop_set_int(const char *op_name, FLEXOP_INT value)
{
    return set_option(op_name, &value, VT_INT, __func__);
}

int flexop_set_float(const char *op_name, FLEXOP_FLOAT value)
{
    return set_option(op_name, &value, VT_FLOAT, __func__);
}

int flexop_set_keyword(const char *op_name, const char *value)
{
    return set_option(op_name, (void *)value, VT_KEYWORD, __func__);
}

int flexop_set_string(const char *op_name, const char *value)
{
    return set_option(op_name, (void *)value, VT_STRING, __func__);
}

int flexop_set_handler(const char *op_name, const char *value)
{
    return set_option(op_name, (void *)value, VT_HANDLER, __func__);
}

int flexop_set_vec_int(const char *op_name, const char *value)
{
    return set_option(op_name, (void *)value, VT_VEC_INT, __func__);
}

int flexop_set_vec_float(const char *op_name, const char *value)
{
    return set_option(op_name, (void *)value, VT_VEC_FLOAT, __func__);
}

int flexop_set_vec_string(const char *op_name, const char *value)
{
    return set_option(op_name, (void *)value, VT_VEC_STRING, __func__);
}
