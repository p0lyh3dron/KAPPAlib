/*
 *    libk_interpret.c    --    source for KAPPA's interpreter.
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on September 1, 2024
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This file contains a standalone interpreter for the KAPPA language.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *name;
    char *type;
    char *mem;
} _k_var_t;

typedef struct _k_frame_s {
    long sp;
    long bp;
    long ip;
    long r[32];
    char rf[32];
    char cmp;

    _k_var_t *vars;
    long      var_count;

    struct _k_frame_s *next;
    struct _k_frame_s *prev;
} _k_frame_t;

typedef struct {
    char *source;
    char *mem;
    long  size;

    _k_frame_t *frame;
} _k_interp_t;

typedef struct {
    char *name;
    int (*func)(_k_interp_t *, char *, char *, char *);
} _k_inst_t;

void _k_print_args(_k_interp_t *interp) {
    for (int i = 0; i < interp->frame->var_count; ++i) {
        printf("%s: %s = %ld (long) %f (double)\n", interp->frame->vars[i].type, interp->frame->vars[i].name, *(long*)interp->frame->vars[i].mem, *(double*)interp->frame->vars[i].mem);
    }
}

char *_k_fetch_line(_k_interp_t *interp) {
    static char ret[256];
    char *line = interp->source + interp->frame->ip;
    while (interp->source[interp->frame->ip++] != '\n');

    strncpy(ret, line, interp->source + interp->frame->ip - line);

    ret[interp->source + interp->frame->ip - line] = '\0';

    return ret;
}

int _k_find_label(_k_interp_t *interp, const char *label) {
    int         i = 0;
    static char buf[256];
    for (; label[i] != '\n' && label[i] != '\0'; i++) {
        buf[i] = label[i]; 
    }

    buf[i]     = ':';
    buf[i + 1] = '\0';

    if (strstr(interp->source, buf) == (char*)0x0) {
        fprintf(stderr, "Failed to find label %s!\n", buf);
        
        return 1;
    }

    interp->frame->ip = strstr(interp->source, buf) - interp->source;

    return 0;
}

long *_k_get_register(_k_interp_t *interp, char *reg) {
    if (reg[0] == 'r') {
        return &interp->frame->r[atoi(reg + 1)];
    } else {
        return (long*)0x0;
    }
}

int _k_pushr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    interp->frame->sp -= sizeof(long);
    memcpy(interp->mem + interp->frame->sp, _k_get_register(interp, a0), sizeof(long));

    return 0;
}

int _k_poprr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    memcpy(_k_get_register(interp, a0), interp->mem + interp->frame->sp, sizeof(long));
    interp->frame->sp += sizeof(long);

    return 0;
}

int _k_newsv(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    interp->frame->vars = realloc(interp->frame->vars, sizeof(_k_var_t) * (interp->frame->var_count + 1));

    char buf[256];

    for (int i = 0; i < 256; i++) {
        if (a1[i] == '\n' || a1[i] == '\0') {
            buf[i] = '\0';
            break;
        }

        buf[i] = a1[i];
    }

    interp->frame->vars[interp->frame->var_count].name = strdup(buf);

    for (int i = 0; i < 256; i++) {
        if (a0[i] == '\n' || a0[i] == '\0') {
            buf[i] = '\0';
            break;
        }

        buf[i] = a0[i];
    }

    interp->frame->vars[interp->frame->var_count].type = strdup(buf);
    interp->frame->vars[interp->frame->var_count].mem  = interp->mem + interp->frame->sp;

    interp->frame->sp -= sizeof(long);

    interp->frame->var_count++;

    return 0;
}

int _k_leave(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    _k_frame_t *frame = interp->frame;

    if (frame->prev != (_k_frame_t*)0x0) {
        interp->frame->prev->r[0]  = interp->frame->r[0];
        interp->frame->prev->rf[0] = interp->frame->rf[0];
    }

    interp->frame = interp->frame->prev;

    free(frame->vars);
    free(frame);

    return 0;
}

int _k_movrn(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    if (strchr(a1, '.')) {
        interp->frame->rf[atoi(a0 + 1)] = 1;

        double f = strtof(a1, (char**)0x0);

        memcpy(_k_get_register(interp, a0), &f, sizeof(long));
    }

    else {
        interp->frame->rf[atoi(a0 + 1)] = 0;

        long l = strtol(a1, (char**)0x0, 10);

        memcpy(_k_get_register(interp, a0), &l, sizeof(long));
    }

    return 0;
}

int _k_movrr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    if (interp->frame->rf[atoi(a1 + 1)]) {
        double f = *(double*)_k_get_register(interp, a1);

        memcpy(_k_get_register(interp, a0), &f, sizeof(long));

        interp->frame->rf[atoi(a0 + 1)] = 1;
    }

    else {
        long l = *(long*)_k_get_register(interp, a1);

        memcpy(_k_get_register(interp, a0), &l, sizeof(long));

        interp->frame->rf[atoi(a0 + 1)] = 0;
    }

    return 0;
}

int _k_callf(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    _k_frame_t *frame = malloc(sizeof(_k_frame_t));

    frame->vars = (_k_var_t*)0x0;
    frame->var_count = 0;
    frame->next = (_k_frame_t*)0x0;
    frame->prev = interp->frame;
    frame->sp = interp->frame->sp;
    frame->bp = interp->frame->sp;
    frame->ip = interp->frame->ip;

    interp->frame = frame;

    return _k_find_label(interp, a0);
}

int _k_loadr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    char buf[256];

    for (int i = 0; i < 256; i++) {
        if (a1[i] == '\n' || a1[i] == '\0') {
            buf[i] = '\0';
            break;
        }

        buf[i] = a1[i];
    }

    for (long i = 0; i < interp->frame->var_count; ++i) {
        if (strcmp(interp->frame->vars[i].name, buf) == 0) {
            if (interp->frame->vars[i].type[0] == 'f') {
                interp->frame->rf[atoi(a0 + 1)] = 1;

                double f = *(double*)interp->frame->vars[i].mem;

                memcpy(_k_get_register(interp, a0), &f, sizeof(long));
            }

            else {
                interp->frame->rf[atoi(a0 + 1)] = 0;

                long l = *(long*)interp->frame->vars[i].mem;

                memcpy(_k_get_register(interp, a0), &l, sizeof(long));
            }

            return 0;
        }
    }

    printf("Failed to load variable %s!\n", a1);

    return 1;
}

int _k_saver(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    for (long i = 0; i < interp->frame->var_count; ++i) {
        if (strcmp(interp->frame->vars[i].name, a0) == 0) {
            if (interp->frame->vars[i].type[0] == 'f') {
                double f = *(double*)_k_get_register(interp, a1);

                memcpy(interp->frame->vars[i].mem, &f, sizeof(double));
            }

            else {
                long l = *(long*)_k_get_register(interp, a1);

                memcpy(interp->frame->vars[i].mem, &l, sizeof(long));
            }

            _k_print_args(interp);

            return 0;
        }
    }

    printf("Failed to save variable %s!\n", a0);

    return 1;
}

int _k_addrr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    if (interp->frame->rf[atoi(a1 + 1)] && interp->frame->rf[atoi(a2 + 1)]) {
        double f = *(double*)_k_get_register(interp, a1) + *(double*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &f, sizeof(double));
    }

    else if (interp->frame->rf[atoi(a1 + 1)] && !interp->frame->rf[atoi(a2 + 1)]) {
        double f = *(double*)_k_get_register(interp, a1) + *(long*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &f, sizeof(double));
    }

    else if (!interp->frame->rf[atoi(a1 + 1)] && interp->frame->rf[atoi(a2 + 1)]) {
        double f = *(long*)_k_get_register(interp, a1) + *(double*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &f, sizeof(double));
    }

    else {
        long l = *(long*)_k_get_register(interp, a1) + *(long*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &l, sizeof(long));
    }

    return 0;
}

int _k_subrr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    if (interp->frame->rf[atoi(a1 + 1)] && interp->frame->rf[atoi(a2 + 1)]) {
        double f = *(double*)_k_get_register(interp, a1) - *(double*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &f, sizeof(double));
    }

    else if (interp->frame->rf[atoi(a1 + 1)] && !interp->frame->rf[atoi(a2 + 1)]) {
        double f = *(double*)_k_get_register(interp, a1) - *(long*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &f, sizeof(double));
    }

    else if (!interp->frame->rf[atoi(a1 + 1)] && interp->frame->rf[atoi(a2 + 1)]) {
        double f = *(long*)_k_get_register(interp, a1) - *(double*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &f, sizeof(double));
    }

    else {
        long l = *(long*)_k_get_register(interp, a1) - *(long*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &l, sizeof(long));
    }

    return 0;
}

int _k_mulrr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    if (interp->frame->rf[atoi(a1 + 1)] && interp->frame->rf[atoi(a2 + 1)]) {
        double f = *(double*)_k_get_register(interp, a1) * *(double*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &f, sizeof(double));
    }

    else if (interp->frame->rf[atoi(a1 + 1)] && !interp->frame->rf[atoi(a2 + 1)]) {
        double f = *(double*)_k_get_register(interp, a1) * *(long*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &f, sizeof(double));
    }

    else if (!interp->frame->rf[atoi(a1 + 1)] && interp->frame->rf[atoi(a2 + 1)]) {
        double f = *(long*)_k_get_register(interp, a1) * *(double*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &f, sizeof(double));
    }

    else {
        long l = *(long*)_k_get_register(interp, a1) * *(long*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &l, sizeof(long));
    }

    return 0;
}

int _k_divrr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    if (interp->frame->rf[atoi(a1 + 1)] && interp->frame->rf[atoi(a2 + 1)]) {
        double f = *(double*)_k_get_register(interp, a1) / *(double*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &f, sizeof(double));
    }

    else if (interp->frame->rf[atoi(a1 + 1)] && !interp->frame->rf[atoi(a2 + 1)]) {
        double f = *(double*)_k_get_register(interp, a1) / *(long*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &f, sizeof(double));
    }

    else if (!interp->frame->rf[atoi(a1 + 1)] && interp->frame->rf[atoi(a2 + 1)]) {
        double f = *(long*)_k_get_register(interp, a1) / *(double*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &f, sizeof(double));
    }

    else {
        long l = *(long*)_k_get_register(interp, a1) / *(long*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &l, sizeof(long));
    }

    return 0;
}

int _k_lesrr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    if (interp->frame->rf[atoi(a1 + 1)] && interp->frame->rf[atoi(a2 + 1)]) {
        long l = (long)(*(double*)_k_get_register(interp, a1) < *(double*)_k_get_register(interp, a2));

        memcpy(_k_get_register(interp, a0), &l, sizeof(double));
    }

    else if (interp->frame->rf[atoi(a1 + 1)] && !interp->frame->rf[atoi(a2 + 1)]) {
        long l = (long)(*(double*)_k_get_register(interp, a1) < *(long*)_k_get_register(interp, a2));

        memcpy(_k_get_register(interp, a0), &l, sizeof(double));
    }

    else if (!interp->frame->rf[atoi(a1 + 1)] && interp->frame->rf[atoi(a2 + 1)]) {
        long l = (long)(*(long*)_k_get_register(interp, a1) < *(double*)_k_get_register(interp, a2));

        memcpy(_k_get_register(interp, a0), &l, sizeof(double));
    }

    else {
        long l = *(long*)_k_get_register(interp, a1) < *(long*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &l, sizeof(long));
    }

    return 0;
}

int _k_grerr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    if (interp->frame->rf[atoi(a1 + 1)] && interp->frame->rf[atoi(a2 + 1)]) {
        long l = (long)(*(double*)_k_get_register(interp, a1) > *(double*)_k_get_register(interp, a2));

        memcpy(_k_get_register(interp, a0), &l, sizeof(double));
    }

    else if (interp->frame->rf[atoi(a1 + 1)] && !interp->frame->rf[atoi(a2 + 1)]) {
        long l = (long)(*(double*)_k_get_register(interp, a1) > *(long*)_k_get_register(interp, a2));

        memcpy(_k_get_register(interp, a0), &l, sizeof(double));
    }

    else if (!interp->frame->rf[atoi(a1 + 1)] && interp->frame->rf[atoi(a2 + 1)]) {
        long l = (long)(*(long*)_k_get_register(interp, a1) > *(double*)_k_get_register(interp, a2));

        memcpy(_k_get_register(interp, a0), &l, sizeof(double));
    }

    else {
        long l = *(long*)_k_get_register(interp, a1) > *(long*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &l, sizeof(long));
    }

    return 0;
}

int _k_equrr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    if (interp->frame->rf[atoi(a1 + 1)] && interp->frame->rf[atoi(a2 + 1)]) {
        long l = (long)(*(double*)_k_get_register(interp, a1) == *(double*)_k_get_register(interp, a2));

        memcpy(_k_get_register(interp, a0), &l, sizeof(double));
    }

    else if (interp->frame->rf[atoi(a1 + 1)] && !interp->frame->rf[atoi(a2 + 1)]) {
        long l = (long)(*(double*)_k_get_register(interp, a1) == *(long*)_k_get_register(interp, a2));

        memcpy(_k_get_register(interp, a0), &l, sizeof(double));
    }

    else if (!interp->frame->rf[atoi(a1 + 1)] && interp->frame->rf[atoi(a2 + 1)]) {
        long l = (long)(*(long*)_k_get_register(interp, a1) == *(double*)_k_get_register(interp, a2));

        memcpy(_k_get_register(interp, a0), &l, sizeof(double));
    }

    else {
        long l = *(long*)_k_get_register(interp, a1) == *(long*)_k_get_register(interp, a2);

        memcpy(_k_get_register(interp, a0), &l, sizeof(long));
    }

    return 0;
}

int _k_cmprd(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    long isf = strchr(a1, '.') ? 1 : 0;

    if (interp->frame->rf[atoi(a0 + 1)] && isf) {
        long l = (long)(*(double*)_k_get_register(interp, a0) == strtof(a1, (char**)0x0));

        interp->frame->cmp = l;
    }

    else if (interp->frame->rf[atoi(a0 + 1)] && !isf) {
        long l = (long)(*(double*)_k_get_register(interp, a0) == strtol(a1, (char**)0x0, 10));

        interp->frame->cmp = l;
    }

    else if (!interp->frame->rf[atoi(a0 + 1)] && isf) {
        long l = (long)(*(long*)_k_get_register(interp, a0) == strtof(a1, (char**)0x0));

        interp->frame->cmp = l;
    }

    else {
        long l = *(long*)_k_get_register(interp, a0) == strtol(a1, (char**)0x0, 10);

        interp->frame->cmp = l;
    }

    return 0;
}

int _k_jmpeq(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    if (interp->frame->cmp) {
        return _k_find_label(interp, a0);
    }

    return 0;
}

int _k_jmpal(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    return _k_find_label(interp, a0);
}

const _k_inst_t _k_inst_list[] = {
    {"\tpushr:", _k_pushr},
    {"\tpoprr:", _k_poprr},
    {"\tnewsv:", _k_newsv},
    {"\tleave:", _k_leave},
    {"\tmovrn:", _k_movrn},
    {"\tmovrr:", _k_movrr},
    {"\tcallf:", _k_callf},
    {"\tloadr:", _k_loadr},
    {"\tsaver:", _k_saver},
    {"\taddrr:", _k_addrr},
    {"\tsubrr:", _k_subrr},
    {"\tmulrr:", _k_mulrr},
    {"\tdivrr:", _k_divrr},
    {"\tlesrr:", _k_lesrr},
    {"\tgrerr:", _k_grerr},
    {"\tequrr:", _k_equrr},
    {"\tcmprd:", _k_cmprd},
    {"\tjmpeq:", _k_jmpeq},
    {"\tjmpal:", _k_jmpal}
};

int main() {
    FILE *fp = fopen("math.kasm", "r");

    if (fp == (FILE*)0x0) {
        fprintf(stderr, "Failed to open math.kasm!\n");
        return 1;
    }

    fseek(fp, 0, SEEK_END);

    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *source = malloc(fsize + 1);

    fread(source, fsize, 1, fp);

    fclose(fp);

    _k_interp_t *interp = malloc(sizeof(_k_interp_t));

    interp->source = source;
    interp->size   = 0xFFFF;
    interp->mem    = malloc(interp->size);

    interp->frame = malloc(sizeof(_k_frame_t));
    interp->frame->vars = (_k_var_t*)0x0;
    interp->frame->var_count = 0;
    interp->frame->next = (_k_frame_t*)0x0;
    interp->frame->prev = (_k_frame_t*)0x0;
    interp->frame->sp = interp->size;
    interp->frame->bp = interp->size;
    interp->frame->ip = 0;

    long   r0d = 0;
    double r0f = 0.0;

    do {
        r0d = interp->frame->r[0];
        r0f = *(double*)&r0d;

        char *line = _k_fetch_line(interp);

        printf("Executing %s", line);

        char *inst = strtok(line, " ");
        char *a0   = strtok((char*)0x0, " ");
        char *a1   = strtok((char*)0x0, " ");
        char *a2   = strtok((char*)0x0, " ");

        if (inst == (char*)0x0)
            continue;

        for (int i = 0; i < sizeof(_k_inst_list) / sizeof(_k_inst_t); i++) {
            if (strcmp(inst, _k_inst_list[i].name) == 0) {
                if (_k_inst_list[i].func(interp, a0, a1, a2)) return 1;

                break;
            }
        }
    } while(interp->frame != (_k_frame_t*)0x0);

    printf("r0 = %d (as decimal)\n", r0d);
    printf("r0 = %f (as float)\n", r0f);

    free(interp->mem);
    free(interp->source);
    free(interp);

    return 0;
}