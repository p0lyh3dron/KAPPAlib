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

typedef struct {
    int      (*func)(void *, void *, void *, void *);
    void      *a0;
    void      *a1;
    void      *a2;
    char       flags;
} _k_inst2_t;

typedef struct {
    long r;
    char rf;
} _k_reg_t;

typedef struct _k_frame_s {
    long        sp;
    long        bp;
    _k_inst2_t *cur;
    _k_reg_t    r[32];
    char        cmp;

    _k_var_t *vars;
    long      var_count;

    struct _k_frame_s *next;
    struct _k_frame_s *prev;
} _k_frame_t;

typedef struct {
    char *name;
    void *ptr;
} _k_label_t;

typedef struct {
    char *source;
    char *mem;
    long  size;

    _k_inst2_t *insts;
    _k_inst2_t *cur;
    long        inst_count;

    _k_label_t *labels;
    long        label_count;

    _k_frame_t *frame;
} _k_interp_t;

typedef struct {
    char *name;
    int (*func)(_k_interp_t *, char *, char *, char *);
} _k_inst_t;

typedef enum {
    _K_INST_PUSHR,
    _K_INST_POPRR,
    _K_INST_NEWSV,
    _K_INST_LEAVE,
    _K_INST_MOVRN,
    _K_INST_MOVRR,
    _K_INST_CALLF,
    _K_INST_LOADR,
    _K_INST_SAVER,
    _K_INST_ADDRR,
    _K_INST_SUBRR,
    _K_INST_MULRR,
    _K_INST_DIVRR,
    _K_INST_LESRR,
    _K_INST_GRERR,
    _K_INST_EQURR,
    _K_INST_CMPRD,
    _K_INST_JMPEQ,
    _K_INST_JMPAL,
    _K_INST_DEREF,
    _K_INST_REFSV,
    _K_INST_SAVEA
} _k_inst_e;

void _k_print_args(_k_interp_t *interp) {
    for (int i = 0; i < interp->frame->var_count; ++i) {
        fprintf(stderr, "%s: %s = %ld (long) %f (double)\n", interp->frame->vars[i].type, interp->frame->vars[i].name, *(long*)interp->frame->vars[i].mem, *(double*)interp->frame->vars[i].mem);
    }
}

int _k_find_label(_k_interp_t *interp, const char *label) {
    for (long i = 0; i < interp->label_count; ++i) {
        if (strcmp(interp->labels[i].name, label) == 0) {
            interp->frame->cur = (_k_inst2_t *)(interp->labels[i].ptr);

            return 0;
        }
    }

    return 0;
}

void *_k_get_register(_k_interp_t *interp, char *reg) {
    if (reg[0] == 'r') {
        return (void*)atoi(reg + 1);
    } else {
        return (void*)0x0;
    }
}

int _k_pushr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    interp->frame->sp -= sizeof(long);
    memcpy(interp->mem + interp->frame->sp, &interp->frame->r[(long)a0], sizeof(long));

    return 0;
}

int _k_poprr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    memcpy(&interp->frame->r[(long)a0], interp->mem + interp->frame->sp, sizeof(long));
    interp->frame->sp += sizeof(long);

    return 0;
}

int _k_newsv(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    interp->frame->sp -= sizeof(long);
    
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

    interp->frame->var_count++;

    return 0;
}

int _k_leave(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    _k_frame_t *frame = interp->frame;

    if (frame->prev != (_k_frame_t*)0x0) {
        interp->frame->prev->r[0].r  = interp->frame->r[0].r;
        interp->frame->prev->r[0].rf = interp->frame->r[0].rf;
    }

    interp->frame = interp->frame->prev;

    free(frame->vars);
    free(frame);

    return 0;
}

int _k_movrn(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    _k_reg_t *r0 = &interp->frame->r[(long)a0];
    
    r0->r  = *(long*)&a1;
    r0->rf = 0;

    return 0;
}

int _k_movrf(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    _k_reg_t *r0 = &interp->frame->r[(long)a0];

    *(double*)&(r0->r)  = *(double*)&a1;
    r0->rf = 1;

    return 0;
}

int _k_movrr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    memcpy(&interp->frame->r[(long)a0], &interp->frame->r[(long)a1], sizeof(_k_reg_t));

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
    frame->cur = interp->frame->cur;

    interp->frame = frame;

    _k_find_label(interp, a0);

    frame->cur--;

    return 0;
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
                interp->frame->r[(long)a0].rf = 1;

                double f = *(double*)interp->frame->vars[i].mem;

                memcpy(&interp->frame->r[(long)a0], &f, sizeof(long));
            }

            else {
                interp->frame->r[(long)a0].rf = 0;

                long l = *(long*)interp->frame->vars[i].mem;

                memcpy(&interp->frame->r[(long)a0], &l, sizeof(long));
            }

            return 0;
        }
    }

    printf("Failed to load variable %s!\n", a1);

    return 1;
}

int _k_saver(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    _k_reg_t *r0 = &interp->frame->r[(long)a1];

    for (long i = 0; i < interp->frame->var_count; ++i) {
        if (strcmp(interp->frame->vars[i].name, a0) == 0) {
            if (interp->frame->vars[i].type[0] == 'f') {
                double f = *(double*)&(r0->r);

                memcpy(interp->frame->vars[i].mem, &f, sizeof(double));
            }

            else {
                long l = *(long*)_k_get_register(interp, a1);

                memcpy(interp->frame->vars[i].mem, &l, sizeof(long));
            }

            //_k_print_args(interp);

            return 0;
        }
    }

    printf("Failed to save variable %s!\n", a0);

    return 1;
}

int _k_addrr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    _k_reg_t *r0 = &interp->frame->r[(long)a0];
    _k_reg_t *r1 = &interp->frame->r[(long)a1];
    _k_reg_t *r2 = &interp->frame->r[(long)a2];

    if (r1->rf && r2->rf) {
        double f = *(double*)&r1->r + *(double*)&r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else if (r1->rf && !r2->rf) {
        double f = *(double*)&r1->r + r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else if (!r1->rf && r2->rf) {
        double f = r1->r + *(double*)&r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else {
        long l = r1->r + r2->r;
        memcpy(&r0->r, &l, sizeof(long));
        r0->rf = 0;
    }

    return 0;
}

int _k_subrr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    _k_reg_t *r0 = &interp->frame->r[(long)a0];
    _k_reg_t *r1 = &interp->frame->r[(long)a1];
    _k_reg_t *r2 = &interp->frame->r[(long)a2];

    if (r1->rf && r2->rf) {
        double f = *(double*)&r1->r - *(double*)&r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else if (r1->rf && !r2->rf) {
        double f = *(double*)&r1->r - r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else if (!r1->rf && r2->rf) {
        double f = r1->r - *(double*)&r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else {
        long l = r1->r - r2->r;
        memcpy(&r0->r, &l, sizeof(long));
        r0->rf = 0;
    }

    return 0;
}

int _k_mulrr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    _k_reg_t *r0 = &interp->frame->r[(long)a0];
    _k_reg_t *r1 = &interp->frame->r[(long)a1];
    _k_reg_t *r2 = &interp->frame->r[(long)a2];

    if (r1->rf && r2->rf) {
        double f = *(double*)&r1->r * *(double*)&r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else if (r1->rf && !r2->rf) {
        double f = *(double*)&r1->r * r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else if (!r1->rf && r2->rf) {
        double f = r1->r * *(double*)&r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else {
        long l = r1->r * r2->r;
        memcpy(&r0->r, &l, sizeof(long));
        r0->rf = 0;
    }

    return 0;
}

int _k_divrr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    _k_reg_t *r0 = &interp->frame->r[(long)a0];
    _k_reg_t *r1 = &interp->frame->r[(long)a1];
    _k_reg_t *r2 = &interp->frame->r[(long)a2];

    if (r1->rf && r2->rf) {
        double f = *(double*)&r1->r / *(double*)&r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else if (r1->rf && !r2->rf) {
        double f = *(double*)&r1->r / r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else if (!r1->rf && r2->rf) {
        double f = r1->r / *(double*)&r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else {
        long l = r1->r / r2->r;
        memcpy(&r0->r, &l, sizeof(long));
        r0->rf = 0;
    }

    return 0;
}

int _k_lesrr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    _k_reg_t *r0 = &interp->frame->r[(long)a0];
    _k_reg_t *r1 = &interp->frame->r[(long)a1];
    _k_reg_t *r2 = &interp->frame->r[(long)a2];

    if (r1->rf && r2->rf) {
        long f = *(double*)&r1->r < *(double*)&r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else if (r1->rf && !r2->rf) {
        long f = *(double*)&r1->r < r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else if (!r1->rf && r2->rf) {
        long f = r1->r < *(double*)&r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else {
        long l = r1->r < r2->r;
        memcpy(&r0->r, &l, sizeof(long));
        r0->rf = 0;
    }

    return 0;
}

int _k_grerr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    _k_reg_t *r0 = &interp->frame->r[(long)a0];
    _k_reg_t *r1 = &interp->frame->r[(long)a1];
    _k_reg_t *r2 = &interp->frame->r[(long)a2];

    if (r1->rf && r2->rf) {
        long f = *(double*)&r1->r > *(double*)&r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else if (r1->rf && !r2->rf) {
        long f = *(double*)&r1->r > r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else if (!r1->rf && r2->rf) {
        long f = r1->r > *(double*)&r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else {
        long l = r1->r > r2->r;
        memcpy(&r0->r, &l, sizeof(long));
        r0->rf = 0;
    }

    return 0;
}

int _k_equrr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    _k_reg_t *r0 = &interp->frame->r[(long)a0];
    _k_reg_t *r1 = &interp->frame->r[(long)a1];
    _k_reg_t *r2 = &interp->frame->r[(long)a2];

    if (r1->rf && r2->rf) {
        long f = *(double*)&r1->r == *(double*)&r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else if (r1->rf && !r2->rf) {
        long f = *(double*)&r1->r == r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else if (!r1->rf && r2->rf) {
        long f = r1->r == *(double*)&r2->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else {
        long l = r1->r == r2->r;
        memcpy(&r0->r, &l, sizeof(long));
        r0->rf = 0;
    }

    return 0;
}

int _k_cmprd(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    _k_reg_t *r0 = &interp->frame->r[(long)a0];

    long l = r0->r == (long)a1;
        
    interp->frame->cmp = l;

    return 0;
}

int _k_jmpeq(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    if (interp->frame->cmp) {
        int ret = _k_find_label(interp, a0);
        interp->frame->cur--;

        return ret;
    }

    return 0;
}

int _k_jmpal(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    return _k_find_label(interp, a0);
}

int _k_deref(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    long addr = *(long*)&interp->frame->r[(long)a1];

    memcpy(&interp->frame->r[(long)a0], (char*)addr, sizeof(long));

    /*if (interp->frame->rf[atoi(a0 + 1)])
        printf("Dereferenced %ld to %f\n", addr, *(double*)_k_get_register(interp, a0));

    else printf("Dereferenced %ld to %ld\n", addr, *(long*)_k_get_register(interp, a0));*/

    return 0;
}

int _k_refsv(_k_interp_t *interp, char *a0, char *a1, char *a2) {
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
            memcpy(&interp->frame->r[(long)a0], &interp->frame->vars[i].mem, sizeof(long));

            interp->frame->r[atoi(a0 + 1)].rf = interp->frame->vars[i].type[0] == 'f' ? 1 : 0;

            return 0;
        }
    }

    printf("Failed to reference variable %s!\n", a1);

    return 1;
}

int _k_savea(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    long addr = *(long*)&interp->frame->r[(long)a0];

    memcpy((char*)addr, &interp->frame->r[(long)a1], sizeof(long));

    /*if (interp->frame->rf[atoi(a1 + 1)])
        printf("Saved %f to %ld\n", *(double*)_k_get_register(interp, a1), addr);

    else printf("Saved %ld to %ld\n", *(long*)_k_get_register(interp, a1), addr);*/

    return 0;
}

int _k_negrr(_k_interp_t *interp, char *a0, char *a1, char *a2) {
    _k_reg_t *r0 = &interp->frame->r[(long)a0];
    _k_reg_t *r1 = &interp->frame->r[(long)a1];

    if (r1->rf) {
        double f = -*(double*)&r1->r;
        memcpy(&r0->r, &f, sizeof(double));
        r0->rf = 1;
    }

    else {
        long l = -r1->r;
        memcpy(&r0->r, &l, sizeof(long));
        r0->rf = 0;
    }

    return 0;
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
    {"\tjmpal:", _k_jmpal},
    {"\tderef:", _k_deref},
    {"\trefsv:", _k_refsv},
    {"\tsavea:", _k_savea},
    {"\tnegrr:", _k_negrr}
};

int push(_k_interp_t *interp, void *data, long size) {
    interp->frame->sp -= size;
    memcpy(interp->mem + interp->frame->sp, data, size);

    return 0;
}

int call(_k_interp_t *interp, char *func) {
    _k_frame_t *frame = malloc(sizeof(_k_frame_t));

    frame->vars = (_k_var_t*)0x0;
    frame->var_count = 0;
    frame->next = (_k_frame_t*)0x0;
    frame->prev = interp->frame;
    frame->sp = interp->frame->sp;
    frame->bp = interp->frame->sp;
    frame->cur = interp->frame->cur;

    interp->frame = frame;

    return _k_find_label(interp, func);
}

double loop(_k_interp_t *interp, _k_frame_t *start) {
    double r0 = 0;
    do {
        r0 = *(double*)&interp->frame->r[0];

        if (interp->frame->cur->func(interp, interp->frame->cur->a0, interp->frame->cur->a1, interp->frame->cur->a2)) return 1;

        interp->frame->cur++;
    } while(interp->frame != start);

    return r0;
}

void _k_translate(_k_interp_t *interp) {
    char buf[256];
    for (int i = 0; i < strlen(interp->source); i++) {
        int j = 0;

        if (interp->source[i] != '\t') {

            if (interp->source[i] == '\n') {
                continue;
            }

            interp->label_count++;

            interp->labels = realloc(interp->labels, sizeof(_k_label_t) * interp->label_count);

            while (interp->source[i + j] != ':') {
                buf[j++] = interp->source[i + j];
            }

            buf[j] = '\0';

            interp->labels[interp->label_count - 1].name = strdup(buf);
            interp->labels[interp->label_count - 1].ptr  = (void*)interp->inst_count;

            while (interp->source[i + j] != '\n') {
                j++;
            }

            i += j;

            continue;
        }

        i++;

        while (interp->source[i + j] != '\n') {
            j++;
        }

        strncpy(buf, interp->source + i, j + 1);

        buf[j] = '\0';

        char *inst = strtok(buf, " ");
        char *a0   = strtok((char*)0x0, " ");
        char *a1   = strtok((char*)0x0, " ");
        char *a2   = strtok((char*)0x0, " ");

        interp->inst_count++;

        interp->insts = realloc(interp->insts, sizeof(_k_inst2_t) * interp->inst_count);

        interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))0x0;
        interp->insts[interp->inst_count - 1].a0   = (void*)0x0;
        interp->insts[interp->inst_count - 1].a1   = (void*)0x0;
        interp->insts[interp->inst_count - 1].a2   = (void*)0x0;

        if (strcmp(inst, "pushr:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_pushr;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
        } else if (strcmp(inst, "poprr:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_poprr;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
        } else if (strcmp(inst, "newsv:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_newsv;
            interp->insts[interp->inst_count - 1].a0   = strdup(a0);
            interp->insts[interp->inst_count - 1].a1   = strdup(a1);
        } else if (strcmp(inst, "leave:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_leave;
        } else if (strcmp(inst, "movrn:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_movrn;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
            interp->insts[interp->inst_count - 1].a1    = (void*)atoi(a1);
        } else if (strcmp(inst, "movrf:") == 0) {
            double f = atof(a1);

            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_movrf;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
            interp->insts[interp->inst_count - 1].a1    = (void*)*(long*)&f;
        } else if (strcmp(inst, "movrr:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_movrr;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
            interp->insts[interp->inst_count - 1].a1   = _k_get_register(interp, a1);
        } else if (strcmp(inst, "callf:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_callf;
            interp->insts[interp->inst_count - 1].a0   = strdup(a0);
        } else if (strcmp(inst, "loadr:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_loadr;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
            interp->insts[interp->inst_count - 1].a1   = strdup(a1);
        } else if (strcmp(inst, "saver:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_saver;
            interp->insts[interp->inst_count - 1].a0   = strdup(a0);
            interp->insts[interp->inst_count - 1].a1   = _k_get_register(interp, a1);
        } else if (strcmp(inst, "addrr:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_addrr;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
            interp->insts[interp->inst_count - 1].a1   = _k_get_register(interp, a1);
            interp->insts[interp->inst_count - 1].a2   = _k_get_register(interp, a2);
        } else if (strcmp(inst, "subrr:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_subrr;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
            interp->insts[interp->inst_count - 1].a1   = _k_get_register(interp, a1);
            interp->insts[interp->inst_count - 1].a2   = _k_get_register(interp, a2);
        } else if (strcmp(inst, "mulrr:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_mulrr;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
            interp->insts[interp->inst_count - 1].a1   = _k_get_register(interp, a1);
            interp->insts[interp->inst_count - 1].a2   = _k_get_register(interp, a2);
        } else if (strcmp(inst, "divrr:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_divrr;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
            interp->insts[interp->inst_count - 1].a1   = _k_get_register(interp, a1);
            interp->insts[interp->inst_count - 1].a2   = _k_get_register(interp, a2);
        } else if (strcmp(inst, "lesrr:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_lesrr;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
            interp->insts[interp->inst_count - 1].a1   = _k_get_register(interp, a1);
            interp->insts[interp->inst_count - 1].a2   = _k_get_register(interp, a2);
        } else if (strcmp(inst, "grerr:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_grerr;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
            interp->insts[interp->inst_count - 1].a1   = _k_get_register(interp, a1);
            interp->insts[interp->inst_count - 1].a2   = _k_get_register(interp, a2);
        } else if (strcmp(inst, "equrr:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_equrr;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
            interp->insts[interp->inst_count - 1].a1   = _k_get_register(interp, a1);
            interp->insts[interp->inst_count - 1].a2   = _k_get_register(interp, a2);
        } else if (strcmp(inst, "cmprd:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_cmprd;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
            interp->insts[interp->inst_count - 1].a1   = (void*)atoi(a1);
        } else if (strcmp(inst, "jmpeq:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_jmpeq;
            interp->insts[interp->inst_count - 1].a0   = strdup(a0);
        } else if (strcmp(inst, "jmpal:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_jmpal;
            interp->insts[interp->inst_count - 1].a0   = strdup(a0);
        } else if (strcmp(inst, "deref:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_deref;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
            interp->insts[interp->inst_count - 1].a1   = _k_get_register(interp, a1);
        } else if (strcmp(inst, "refsv:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_refsv;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
            interp->insts[interp->inst_count - 1].a1   = strdup(a1);
        } else if (strcmp(inst, "savea:") == 0) {
            interp->insts[interp->inst_count - 1].func = (int(*)(void*,void*,void*,void*))_k_savea;
            interp->insts[interp->inst_count - 1].a0   = _k_get_register(interp, a0);
            interp->insts[interp->inst_count - 1].a1   = _k_get_register(interp, a1);
        }

        i += j;
    }

    for (int i = 0; i < interp->label_count; i++) {
        interp->labels[i].ptr = interp->insts + (long)interp->labels[i].ptr;
    }
}

int main() {
    FILE *fp = fopen("fractal.kasm", "r");

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

    interp->inst_count = 0;
    interp->insts = (_k_inst2_t*)0x0;

    interp->label_count = 0;
    interp->labels = (_k_label_t*)0x0;

    interp->frame = malloc(sizeof(_k_frame_t));
    interp->frame->vars = (_k_var_t*)0x0;
    interp->frame->var_count = 0;
    interp->frame->next = (_k_frame_t*)0x0;
    interp->frame->prev = (_k_frame_t*)0x0;
    interp->frame->sp = interp->size;
    interp->frame->bp = interp->size;
    interp->frame->cur = &interp->insts[0];

    _k_frame_t *frame = interp->frame;

    _k_translate(interp);

    long   r0d = 0;
    double r0f = 0.0;

    double rmin;
    double rmax;

    double imin;
    double imax;

    double real = 1.0;
    double imag = 1.0;

    double *real_ptr = &real;
    double *imag_ptr = &imag;

    //push(interp, &real, sizeof(double));
    //push(interp, &imag, sizeof(double));
    call(interp, "rmin");
    rmin = loop(interp, frame);

    call(interp, "rmax");
    rmax = loop(interp, frame);

    call(interp, "imin");
    imin = loop(interp, frame);

    call(interp, "imax");
    imax = loop(interp, frame);

    fprintf(stderr, "rmin = %f\n", rmin);
    fprintf(stderr, "rmax = %f\n", rmax);
    fprintf(stderr, "imin = %f\n", imin);
    fprintf(stderr, "imax = %f\n", imax);

    double a = 1.0;
    double b = -1.0;

    call(interp, "abs");
    push(interp, &a, sizeof(double));
    double c = loop(interp, frame);

    call(interp, "abs");
    push(interp, &b, sizeof(double));
    double d = loop(interp, frame);

    fprintf(stderr, "abs(%f) = %f\n", a, c);
    fprintf(stderr, "abs(%f) = %f\n", b, d);

    const int W = 640; const int H = 640;
    char img[W][H];

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int i = 0;

            real = rmin + (rmax - rmin) * x / W;
            imag = imin + (imax - imin) * y / H;

            while (real * real + imag * imag < 16 && i < 64) {
                call(interp, "z");
                push(interp, &real_ptr, sizeof(double));
                push(interp, &imag_ptr, sizeof(double));
                loop(interp, frame);

                //printf("z = %f + %fi\n", real, imag);

                real += rmin + (rmax - rmin) * x / W;
                imag += imin + (imax - imin) * y / H;

                i++;
            }

            img[x][y] = i == 64 ? 0 : 4 * i;
        }

        fprintf(stderr, "%d\%\n", y * 100 / H);
    }

    printf("P6\n%d %d\n100\n", W, H);

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            printf("%c%c%c", img[x][y], 0, 0);
        }
    }
    

    /*printf("r0 = %d (as decimal)\n", r0d);
    printf("r0 = %f (as float)\n", r0f);*/

    /*printf("real = %f\n", rmin);
    printf("imag = %f\n", imag);*/
    

    free(interp->mem);
    free(interp->source);
    free(interp);

    return 0;
}