/*
 *    fastK_assemble.h    --    Header for KAPPA assembly
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 9, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This file declares functions for assembling KAPPA source code.
 */
#ifndef FASTK_ASSEMBLE_H
#define FASTK_ASSEMBLE_H

#include "fastK.h"

typedef enum {
    K_CMP_E,
    K_CMP_NE,
    K_CMP_L,
    K_CMP_LE,
    K_CMP_G,
    K_CMP_GE
} k_cmp_e;

/*
 *    Generates the assembly for the prelude.
 *
 *    @param k_env_t *env    The environment to generate the prelude for.
 */
void k_generate_prelude(k_env_t *env);

/*
 *    Generates assembly for an assignment.
 *
 *    @param k_env_t *env           The environment to generate the assignment for.
 *    @param unsigned long offset   The offset of the variable to assign to.
 */
void k_generate_assignment(k_env_t *env, unsigned long offset);

/*
 *    Generates assembly for a move into a register.
 *
 *    @param k_env_t *env    The environment to generate the move for.
 *    @param char reg        The register to move into.
 *    @param unsigned long   The offset of the variable to move.
 */
void k_generate_move(k_env_t *env, char reg, unsigned long offset);

/*
 *    Generates assembly to put integer into rax.
 *
 *    @param k_env_t *env        The environment to generate assembly for.
 *    @param long     integer    The integer to put into rax.
 */
void k_generate_put_integer(k_env_t *env, long integer);

/*
 *    Generates assembly to mov rax into rcx.
 *
 *    @param k_env_t *env    The environment to generate assembly for.
 */
void k_generate_put_rax_rcx(k_env_t *env);

/*
 *    Generates assembly for an addition.
 *
 *    @param k_env_t *env    The environment to generate the addition for.
 */
void k_generate_addition(k_env_t *env);

/*
 *    Generates assembly for a comparison.
 *
 *    @param k_env_t *env    The environment to generate the comparison for.
 *    @param k_cmp_e cmp     The comparison to generate.
 */
void k_generate_comparison(k_env_t *env, k_cmp_e cmp);

/*
 *    Generates assembly for a while.
 *
 *    @param k_env_t *env    The environment to generate the while for.
 * 
 *    @return char *         The address of the je offset.
 */
char *k_generate_while(k_env_t *env);

/*
 *    Generates assembly for a jump.
 *
 *    @param k_env_t *env    The environment to generate the jump for.
 *    @param char *address   The address to jump to.
 */
void k_generate_jump(k_env_t *env, char *address);

/*
 *    Generates assembly for a return.
 *
 *    @param k_env_t *env    The environment to generate the return for.
 */
void k_generate_return(k_env_t *env);

/*
 *    Prints the assembly bytestream for debugging.
 *
 *    @param k_env_t *env    The environment to print the assembly for.
 * 
 *    @return char *         The assembly bytestream.
 */
char *k_print_assembly(k_env_t *env);

#endif /* FASTK_ASSEMBLE_H  */