/*
 *    libk_assemble.c    --    Source for KAPPA assembly
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 2, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This file defines functions for assembling KAPPA source code.
 */
#include "libk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#include "libk_assemble.h"

/*
 *    Generates the assembly for the prelude.
 *
 *    @param k_env_t *env    The environment to generate the prelude for.
 */
void _k_generate_prelude(k_env_t *env) {
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
void _k_generate_assignment(k_env_t *env, unsigned long offset) {
    /*
     *     48 89 85 00 00 00 01   mov  [rbp + offset], rax
     */
    const char *assignment = "\x48\x89\x85";
    char        offset_str[4];
    signed long offset_signed = 0xFFFFFFFF - offset + 1;

    memcpy(offset_str, &offset_signed, 4);

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
void _k_generate_move(k_env_t *env, char reg, unsigned long offset) {
    /*
     *     48 8B 85 00 00 00 01   mov  reg, [rbp + offset]
     */
    const char *move = (reg == 'a') ? "\x48\x8B\x85" : "\x48\x8B\x8D";
    char        offset_str[4];
    signed long offset_signed = 0xFFFFFFFF - offset + 1;

    memcpy(offset_str, &offset_signed, 4);

    env->cur_function->source = (char *)realloc(env->cur_function->source, env->cur_function->size + 7);

    memcpy(env->cur_function->source + env->cur_function->size, move, 3);
    memcpy(env->cur_function->source + env->cur_function->size + 3, offset_str, 4);

    env->cur_function->size  += 7;
}

/*
 *    Generates assembly to put integer into rax.
 *
 *    @param k_env_t *env        The environment to generate assembly for.
 *    @param long     integer    The integer to put into rax.
 */
void _k_generate_put_integer(k_env_t *env, long integer) {
    /*
     *    48 C7 C0 00 00 00 00    mov rax, integer
     */
    const char *put = "\x48\xC7\xC0";
    char        integer_str[4];

    memcpy(integer_str, &integer, 4);

    env->cur_function->source = (char *)realloc(env->cur_function->source, env->cur_function->size + 7);

    memcpy(env->cur_function->source + env->cur_function->size, put, 3);
    memcpy(env->cur_function->source + env->cur_function->size + 3, integer_str, 4);

    env->cur_function->size  += 7;
}

/*
 *    Generates assembly to mov rax into rcx.
 *
 *    @param k_env_t *env    The environment to generate assembly for.
 */
void _k_generate_put_rax_rcx(k_env_t *env) {
    /*
     *    48 89 C1    mov rcx, rax
     */
    const char *put = "\x48\x89\xC1";

    env->cur_function->source = (char *)realloc(env->cur_function->source, env->cur_function->size + 3);

    memcpy(env->cur_function->source + env->cur_function->size, put, 3);

    env->cur_function->size  += 3;
}

/*
 *    Generates assembly for an addition.
 *
 *    @param k_env_t *env    The environment to generate the addition for.
 */
void _k_generate_addition(k_env_t *env) {
    /*
     *     48 01 C0   add  rax, rcx
     */
    const char *addition = "\x48\x01\xC8";

    env->cur_function->source = (char *)realloc(env->cur_function->source, env->cur_function->size + 3);

    memcpy(env->cur_function->source + env->cur_function->size, addition, 3);

    env->cur_function->size  += 3;
}

/*
 *    Generates assembly for a comparison.
 *
 *    @param k_env_t *env    The environment to generate the comparison for.
 *    @param _k_cmp_e cmp     The comparison to generate.
 */
void _k_generate_comparison(k_env_t *env, _k_cmp_e cmp) {
    /*
     *     48 39 C1   cmp  rcx, rasx
     */
    const char *comparison = "\x48\x39\xC1";

    env->cur_function->source = (char *)realloc(env->cur_function->source, env->cur_function->size + 3);

    memcpy(env->cur_function->source + env->cur_function->size, comparison, 3);

    env->cur_function->size  += 3;

    const char *set = (const char*)0x0;

    switch (cmp) {
        case _K_CMP_E:
            /*
             *     0F 94 C0   sete al
             */
            set = "\x0F\x94\xC0";
            break;
        case _K_CMP_NE:
            /*
             *     0F 95 C0   setne al
             */
            set = "\x0F\x95\xC0";
            break;
        case _K_CMP_L:
            /*
             *     0F 9C C0   setl al
             */
            set = "\x0F\x9C\xC0";
            break;
        case _K_CMP_LE:
            /*
             *     0F 9E C0   setle al
             */
            set = "\x0F\x9E\xC0";
            break;
        case _K_CMP_G:
            /*
             *     0F 9F C0   setg al
             */
            set = "\x0F\x9F\xC0";
            break;
        case _K_CMP_GE:
            /*
             *     0F 9D C0   setge al
             */
            set = "\x0F\x9D\xC0";
            break;
    }

    env->cur_function->source = (char *)realloc(env->cur_function->source, env->cur_function->size + 3);

    memcpy(env->cur_function->source + env->cur_function->size, set, 3);

    env->cur_function->size  += 3;

    /*
     *     48 0F B6 C0   movzx rax, al
     */
    const char *movzx = "\x48\x0F\xB6\xC0";

    env->cur_function->source = (char *)realloc(env->cur_function->source, env->cur_function->size + 4);

    memcpy(env->cur_function->source + env->cur_function->size, movzx, 4);

    env->cur_function->size  += 4;
}

/*
 *    Generates assembly for a while.
 *
 *    @param k_env_t *env    The environment to generate the while for.
 * 
 *    @return char *         The address of the je offset.
 */
char *_k_generate_while(k_env_t *env) {
    /*
     *    48 83 F8 00          cmp rax, 0
     *    0F 84 00 00 00 00    je  end
     */
    const char *while_start = "\x48\x83\xF8\x00\x0F\x84\x00\x00\x00\x00";
    char        offset_str[4];
    signed long offset_signed = 0xFFFFFFFF - env->cur_function->size + 1;

    memcpy(offset_str, &offset_signed, 4);

    env->cur_function->source = (char *)realloc(env->cur_function->source, env->cur_function->size + 10);

    memcpy(env->cur_function->source + env->cur_function->size, while_start, 10);

    env->cur_function->size  += 10;

    return env->cur_function->source + env->cur_function->size - 4;
}

/*
 *    Generates assembly for a jump.
 *
 *    @param k_env_t *env    The environment to generate the jump for.
 *    @param char *address   The address to jump to.
 */
void _k_generate_jump(k_env_t *env, char *address) {
    /*
     *    E9 00 00 00 00    jmp address
     */
    const char *jump = "\xE9";
    char        offset_str[4];
    signed long offset_signed = address - (env->cur_function->source + env->cur_function->size + 5);

    memcpy(offset_str, &offset_signed, 4);

    env->cur_function->source = (char *)realloc(env->cur_function->source, env->cur_function->size + 5);

    memcpy(env->cur_function->source + env->cur_function->size, jump, 1);

    env->cur_function->size  += 1;

    memcpy(env->cur_function->source + env->cur_function->size, offset_str, 4);

    env->cur_function->size  += 4;
}

/*
 *    Generates assembly for a return.
 *
 *    @param k_env_t *env    The environment to generate the return for.
 */
void _k_generate_return(k_env_t *env) {
    /*
     *    48 89 EC    mov rsp, rbp
     *    5D          pop rbp
     *    C3          ret
     */
    const char *ret = "\x48\x89\xEC\x5D\xC3";

    env->cur_function->source = (char *)realloc(env->cur_function->source, env->cur_function->size + 5);

    memcpy(env->cur_function->source + env->cur_function->size, ret, 5);

    env->cur_function->size  += 5;
}

/*
 *    Prints the assembly bytestream for debugging.
 *
 *    @param k_env_t *env    The environment to print the assembly for.
 * 
 *    @return char *         The assembly bytestream.
 */
char *_k_print_assembly(k_env_t *env) {
    char *assembly = malloc(env->cur_function->size * 3 + 1);

    for (unsigned long i = 0; i < env->cur_function->size; i++) {
        sprintf(assembly + i * 3, "%02X ", (unsigned char)env->cur_function->source[i]);
    }

    assembly[env->cur_function->size * 3] = '\0';

    return assembly;
}