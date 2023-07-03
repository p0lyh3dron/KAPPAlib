/*
 *    fastK_assemble.c    --    Source for KAPPA assembly
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 2, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This file defines functions for assembling KAPPA source code.
 */
#include "fastK.h"

#include "util.h"

/*
 *    Generates the assembly for the prelude.
 *
 *    @param k_env_t *env    The environment to generate the prelude for.
 */
void k_generate_prelude(k_env_t *env) {
    /*
     *     55            push rbp
     *     48 89 E5      mov  rbp, rsp
     */
    const char *prelude = "\x55\x48\x89\xE5";

    env->cur_function->source = (char *)realloc(env->cur_function->source, env->cur_function->size + 4);

    memcpy(env->cur_function->source + env->cur_function->size, prelude, 4);

    env->cur_function->size  += 4;
}

/*
 *    Generates assembly for an assignment.
 *
 *    @param k_env_t *env           The environment to generate the assignment for.
 *    @param unsigned long offset   The offset of the variable to assign to.
 */
void k_generate_assignment(k_env_t *env, unsigned long offset) {
    /*
     *     48 89 85 00 00 00 01   mov  [rbp + 1], rax
     */
    const char *assignment = "\x48\x89\x85";
    char        offset_str[4];
    signed long offset_signed = -offset;

    swap_endian32(&offset_signed);

    memcpy(offset_str, &offset, 4);

    env->cur_function->source = (char *)realloc(env->cur_function->source, env->cur_function->size + 7);

    memcpy(env->cur_function->source + env->cur_function->size, assignment, 3);
    memcpy(env->cur_function->source + env->cur_function->size + 3, offset_str, 4);

    env->cur_function->size  += 7;
}

/*
 *    Generates assembly for a move into a register.
 *
 *    @param k_env_t *env    The environment to generate the move for.
 *    @param char reg        The register to move into.
 *    @param unsigned long   The offset of the variable to move.
 */
void k_generate_move(k_env_t *env, char reg, unsigned long offset) {
    /*
     *     48 8B 85 00 00 00 01   mov  rax, [rbp + 1]
     */
    const char *move = (reg == 'a') ? "\x48\x8B\x85" : "\x48\x8B\x8D";
    char        offset_str[4];
    signed long offset_signed = -offset;

    swap_endian32(&offset_signed);

    memcpy(offset_str, &offset, 4);

    env->cur_function->source = (char *)realloc(env->cur_function->source, env->cur_function->size + 7);

    memcpy(env->cur_function->source + env->cur_function->size, move, 3);
    memcpy(env->cur_function->source + env->cur_function->size + 3, offset_str, 4);

    env->cur_function->size  += 7;
}

/*
 *    Generates assembly for an addition.
 *
 *    @param k_env_t *env    The environment to generate the addition for.
 */
void k_generate_addition(k_env_t *env) {
    /*
     *     48 01 C0   add  rax, rcx
     */
    const char *addition = "\x48\x01\xC8";

    env->cur_function->source = (char *)realloc(env->cur_function->source, env->cur_function->size + 3);

    memcpy(env->cur_function->source + env->cur_function->size, addition, 3);

    env->cur_function->size  += 3;
}