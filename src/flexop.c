
#include "flexop.h"

static FLEXOP itnl_opt;

static void flexop_key_destroy(FLEXOP_KEY *o)
{
    char **p;

    flexop_free(o->name);
    flexop_free(o->help);
    o->name = o->help = NULL;

    if (o->type == VT_STRING) {
        flexop_free(*(char **)o->var);
        *(char **)o->var = NULL;
    }

    if (o->keys != NULL) {
        for (p = o->keys; *p != NULL; p++) flexop_free(*p);

        flexop_free(o->keys);
        o->keys = NULL;
    }
}

static void flexop_register(const char *name, const char *help, const char **keys,
        void *var, FLEXOP_VTYPE type, int append)
{
    FLEXOP_KEY *o;
    static int initialized = 0;

    if (name != NULL && itnl_opt.initialized) {
        flexop_printf("flexop: option \"-%s\" not registered.\n", name);
        return;
    }

    if (!initialized && type != VT_INIT) {
        initialized = 1;

        /* Register title for user options */
        flexop_register("\nUser options:", "\n", NULL, "user", VT_TITLE, 0);
    }
    else if (type == VT_INIT) {
        /* register some global options */
        initialized = 1;
        flexop_register("\nGeneric options:", "\n", NULL, "generic", VT_TITLE, 0);

        flexop_register("-help", "Print options help then exit", NULL,
                &itnl_opt.help_category, VT_STRING, 0);
        return;
    }

    if (name == NULL) return;

    if (*name == '-' || *name == '+') name++;

    if (itnl_opt.index != NULL) {
        /* invalidate index */
        flexop_free(itnl_opt.index);
        itnl_opt.index = NULL;
    }

    /* memory */
    if (itnl_opt.size >= itnl_opt.alloc) {
        itnl_opt.alloc += 8;

        itnl_opt.options = flexop_realloc(itnl_opt.options, itnl_opt.alloc * sizeof(*itnl_opt.options));
    }

    /* save option */
    o = itnl_opt.options + (itnl_opt.size++);

    o->name = strdup(name);
    o->help = help == NULL ? NULL : strdup(help);
    o->keys = NULL;
    o->var = var;
    o->type = type;
    o->used = 0;
    o->append = append;

    if (type == VT_STRING) {
        if (*(char **)o->var != NULL) {
            /* duplicate the string (it is then safe to free it) */
            *((char **)o->var) = strdup(*(char **)o->var);
        }
    }
    else if (type == VT_KEYWORD) {
        /* make a copy of the keywords list */
        const char **p;
        char **q;

        if (keys == NULL) {
            flexop_printf("flexop_register_keyword(): keys should not be NULL "
                    "(option \"-%s\").\n", name);
            flexop_printf("Option not registered.\n");
            flexop_key_destroy(itnl_opt.options + itnl_opt.size);
            itnl_opt.size--;
            return;
        }

        if (keys[0] == NULL) {
            flexop_key_destroy(itnl_opt.options + itnl_opt.size);
            itnl_opt.size--;
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
}

/* Wrapper functions for enforcing prototype checking */
static void flexop_register_init(void)
{
    flexop_register(NULL, NULL, NULL, NULL, VT_INIT, 0);
}

void flexop_register_no_arg(const char *name, const char *help, int *var)
{
    flexop_register(name, help, NULL, var, VT_NONE, 0);
}

void flexop_register_int(const char *name, const char *help, FLEXOP_INT *var)
{
    flexop_register(name, help, NULL, var, VT_INT, 0);
}

void flexop_register_float(const char *name, const char *help, FLEXOP_FLOAT *var)
{
    flexop_register(name, help, NULL, var, VT_FLOAT, 0);
}

void flexop_register_string(const char *name, const char *help, char **var)
{
    flexop_register(name, help, NULL, var, VT_STRING, 0);
}

void flexop_register_keyword(const char *name, const char *help,
        const char **keys, int *var)
{
    flexop_register(name, help, keys, var, VT_KEYWORD, 0);
}

void flexop_register_title(const char *str, const char *help, const char *category)
{
    /* Note: category will be stored in '->var' */
    flexop_register(str, help, NULL, (void *)category, VT_TITLE, 0);
}

void flexop_register_handler(const char *name, const char *help, FLEXOP_HANDLER func, int append)
{
    flexop_register(name, help, NULL, func, VT_HANDLER, append);
}

static int flexop_comp(const void *i0, const void *i1)
{
    FLEXOP_KEY *o0 = itnl_opt.options + *(int *)i0, *o1 = itnl_opt.options + *(int *)i1;

    /* This makes all separators to appear at the end of the list */
    if (o0->type == VT_TITLE || o1->type == VT_TITLE) {
        if (o0->type != VT_TITLE) return -1;

        if (o1->type != VT_TITLE) return 1;

        return (int)(o0 - o1);
    }

    return strcmp(o0->name, o1->name);
}

static void flexop_sort(void)
{
    int i;

    if (itnl_opt.index != NULL) return;

    /* append a dummy entry at the end of the list as the key for bsearch */
    flexop_register("dummy", NULL, NULL, NULL, VT_NONE, 0);
    itnl_opt.index = flexop_alloc(itnl_opt.size * sizeof(*itnl_opt.index));

    for (i = 0; i < (int)itnl_opt.size; i++) itnl_opt.index[i] = i;

    /* restore real size */
    itnl_opt.size--;

    qsort(itnl_opt.index, itnl_opt.size, sizeof(*itnl_opt.index), flexop_comp);

    /* clean up */
    flexop_free(itnl_opt.options[itnl_opt.size].name);
    itnl_opt.options[itnl_opt.size].name = NULL;
}

void flexop_reset(void)
{
    FLEXOP_KEY *o;

    if (itnl_opt.options != NULL) {
        for (o = itnl_opt.options; o < itnl_opt.options + itnl_opt.size; o++) flexop_key_destroy(o);

        flexop_free(itnl_opt.options);
        flexop_free(itnl_opt.index);

        itnl_opt.options = NULL;
        itnl_opt.index = NULL;
        itnl_opt.size = itnl_opt.alloc = 0;
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

        assert(p + 3 < buffer + sizeof(buffer));        /* +1 for '>' */

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

    if (itnl_opt.argc <= 1) return;

    flexop_printf("Command-line:");

    for (i = 0; i < itnl_opt.argc; i++) flexop_printf(" %s", itnl_opt.argv[i]);

    flexop_printf("\n");
}

/* prints all options which have been called by the user */
void flexop_show_used(void)
{
    int flag = 0;
    FLEXOP_KEY *o;
    char **pp;

    if (!itnl_opt.initialized) flexop_error(1, "%s must be called after flexop_init!\n", __func__);

    for (o = itnl_opt.options; o < itnl_opt.options + itnl_opt.size; o++) {
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

    if (itnl_opt.help_category == NULL) return;

    flag = 1;
    matched = 0;
    all_flag = !strcmp(itnl_opt.help_category, "all");

    for (o = itnl_opt.options; o < itnl_opt.options + itnl_opt.size; o++) {
        if (!all_flag && o->type == VT_TITLE) {
            flag = (o->var == NULL || !strcmp(itnl_opt.help_category, o->var));

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
        }

        flexop_printf("\n");
        if (o->help != NULL) flexop_print_help(o, o->help);

        if (o->type == VT_HANDLER && o->var != NULL) ((FLEXOP_HANDLER)o->var)(o, NULL);
    }

    if (matched) {
        flexop_printf("\n");
    }
    else {
        if (strcmp(itnl_opt.help_category, "help")) {
            flexop_printf("Unknown help category '%s'.\n", itnl_opt.help_category);
        }

        qsort(list, list_count, sizeof(*list), comp_string);
        flexop_printf("Usage:\n    %s -help <category>\n"
                "where <category> should be one of:\n", itnl_opt.argv[0]);

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

    flexop_reset();

    exit(0);
}

/* parses cmdline parameters, processes and removes known options from
   the argument list */
void flexop_parse_cmdline(int *argc, char ***argv)
{
    static int firstcall = 1;     /* 1st time calling this function */
    FLEXOP_KEY *o, *key = NULL;
    char **pp;
    char *p, *arg;
    int i, j;
    int *k = NULL;                /* points to sorted indices of options */

    if (!firstcall) {
        flexop_error(1, "flexop: flexop_parse_cmdline can be called only once.\n");
    }

    /* register internal opt */
    if (itnl_opt.options == NULL) {
        /* this will register the options '-help' */
        flexop_register(NULL, NULL, NULL, NULL, VT_NONE, 0);
    }

    flexop_sort();
    key = itnl_opt.options + itnl_opt.size;

    /* check and warn duplicate options */
    j = 0;
    for (i = 1; i < (int)itnl_opt.size; i++) {
        o = itnl_opt.options + itnl_opt.index[i];
        if (o->type == VT_TITLE) break;

        if (flexop_comp(itnl_opt.index + i, itnl_opt.index + j) == 0)
            flexop_warning("duplicate option \"-%s\".\n",o->name);
        j = i;
    }

    itnl_opt.argc = *argc;
    itnl_opt.argv = flexop_alloc((itnl_opt.argc + 1) * sizeof(*itnl_opt.argv));

    for (i = 0; i < itnl_opt.argc; i++) itnl_opt.argv[i] = strdup((*argv)[i]);

    itnl_opt.argv[i] = NULL;

    /* parse */
    for (i = 1; i < *argc; i++) {
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
            k = bsearch(itnl_opt.index + itnl_opt.size, itnl_opt.index, itnl_opt.size, sizeof(*itnl_opt.index), flexop_comp);

            flexop_free(q);
            key->name = NULL;
        }

        if (k == NULL) flexop_error(1, "unknown option \"%s\"!\n", p);

        o = itnl_opt.options + (*k);

        /* process option */
        if (o->type != VT_NONE) {
            if (arg == NULL && (arg = (*argv)[++i]) == NULL) {
                if (!strcmp(o->name, itnl_opt.help_category = strdup("help"))) {
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
                flexop_free(*(char **)o->var);
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
                if (o->append) {
                    for (pp = o->keys; pp != NULL && *pp != NULL; pp++);

                    j = pp - o->keys;
                }
                else {
                    j = 0;
                    if (o->keys != NULL) {
                        flexop_free(o->keys[0]);
                        o->keys[0] = NULL;
                    }
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
        }
    }

    firstcall = 0;

    return;
}

void flexop_init(int *argc, char ***argv)
{
    /* option init */
    flexop_register_init();

    /* option parse */
    flexop_parse_cmdline(argc, argv);
    flexop_help();

    /* mark status, has been initialized */
    itnl_opt.initialized = 1;
}

void flexop_finalize(void)
{
    itnl_opt.initialized = 1;
}
