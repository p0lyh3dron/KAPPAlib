/*
 *    libk_assemble.h    --    Header for KAPPA assembly
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 9, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This file declares functions for assembling KAPPA source code.
 */
#ifndef _LIBK_ASSEMBLE_H
#define _LIBK_ASSEMBLE_H

#include "types.h"

typedef enum {
    _K_CMP_E,
    _K_CMP_NE,
    _K_CMP_L,
    _K_CMP_LE,
    _K_CMP_G,
    _K_CMP_GE
} _k_cmp_e;

/*
 *    Generates the assembly for the prelude.
 *
 *    @param k_env_t *env    The environment to generate the prelude for.
 */
void _k_assemble_prelude(k_env_t *env);

/*
 *    Generates assembly for memory allocation.
 *
 *    @param k_env_t *env    The environment to generate the memory allocation for.
 *    @param unsigned long   The size of the memory to allocate.
 *    @param unsigned long   The offset into the function to allocate at.
 */
void _k_assemble_allocate(k_env_t *env, unsigned long size, unsigned long offset);

/*
 *    Generates a paremeter store for a function.
 *
 *    @param k_env_t *env    The environment to generate the parameter store for.
 *    @param unsigned long   The offset of the parameter to store.
 *    @param unsigned long   The current parameter index.
 */
void _k_assemble_parameter_store(k_env_t *env, unsigned long offset, unsigned long index);

/*
 *    Generates a parameter load for a function.
 *
 *    @param k_env_t *env    The environment to generate the parameter load for.
 *    @param unsigned long   The current parameter index.
 */
void _k_assemble_parameter_load(k_env_t *env, unsigned long index);

/*
 *    Pops the parameters off the stack.
 *
 *    @param k_env_t *env    The environment to pop the parameters for.
 */
void _k_assemble_pop_parameters(k_env_t *env);

/*
 *    Generates the assembly for a call.
 *
 *    @param k_env_t *env    The environment to generate the call for.
 *    @param unsigned long   The offset of the function to call.
 */
void _k_assemble_call(k_env_t *env, unsigned long offset);

/*
 *    Generates assembly for an assignment.
 *
 *    @param k_env_t *env           The environment to generate the assignment for.
 *    @param unsigned long offset   The offset of the variable to assign to.
 */
void _k_assemble_assignment(k_env_t *env, unsigned long offset);

/*
 *    Generates assembly for a move into a register.
 *
 *    @param k_env_t *env    The environment to generate the move for.
 *    @param unsigned long   The offset of the variable to move.
 */
void _k_assemble_move(k_env_t *env, unsigned long offset);

/*
 *    Generates assembly to put integer into rax.
 *
 *    @param k_env_t *env        The environment to generate assembly for.
 *    @param long     integer    The integer to put into rax.
 */
void _k_assemble_mov_integer(k_env_t *env, long integer);

/*
 *    Generates assembly to store rcx.
 *
 *    @param k_env_t *env    The environment to generate assembly for.
 */
void _k_assemble_store_rcx(k_env_t *env);

/*
 *    Generates assembly to load rcx.
 *
 *    @param k_env_t *env    The environment to generate assembly for.
 */
void _k_assemble_load_rcx(k_env_t *env);

/*
 *    Generates assembly to mov rax into rcx.
 *
 *    @param k_env_t *env    The environment to generate assembly for.
 */
void _k_assemble_mov_rcx_rax(k_env_t *env);

/*
 *    Generates assembly to swap rax and rcx.
 *
 *    @param k_env_t *env    The environment to generate assembly for.
 */
void _k_assemble_swap_rax_rcx(k_env_t *env);

/*
 *    Generates assembly to move rdx to rax.
 *
 *    @param k_env_t *env    The environment to generate assembly for.
 */
void _k_assemble_mov_rdx_rax(k_env_t *env);

/*
 *    Generates assembly for an addition.
 *
 *    @param k_env_t *env    The environment to generate the addition for.
 */
void _k_assemble_addition(k_env_t *env);

/*
 *    Generates assembly for a subtraction.
 *
 *    @param k_env_t *env    The environment to generate the subtraction for.
 */
void _k_assemble_subtraction(k_env_t *env);

/*
 *    Generates assembly for a multiplication.
 *
 *    @param k_env_t *env    The environment to generate the multiplication for.
 */ 
void _k_assemble_multiplication(k_env_t *env);

/*
 *    Generates assembly for a division.
 *
 *    @param k_env_t *env    The environment to generate the division for.
 */
void _k_assemble_division(k_env_t *env);

/*
 *    Generates assembly for a comparison.
 *
 *    @param k_env_t      *env     The environment to generate the comparison for.
 *    @param _k_op_type_e  cmp     The comparison to generate.
 */
void _k_assemble_comparison(k_env_t *env, _k_op_type_e cmp);

/*
 *    Generates assembly for a while.
 *
 *    @param k_env_t *env    The environment to generate the while for.
 * 
 *    @return char *         The address of the je offset.
 */
char *_k_assemble_while(k_env_t *env);

/*
 *    Generates assembly for a jump.
 *
 *    @param k_env_t *env    The environment to generate the jump for.
 *    @param char *address   The address to jump to.
 */
void _k_assemble_jump(k_env_t *env, char *address);

/*
 *    Generates assembly for a return.
 *
 *    @param k_env_t *env    The environment to generate the return for.
 */
void _k_assemble_return(k_env_t *env);

/*
 *    Prints the assembly bytestream for debugging.
 *
 *    @param k_env_t *env    The environment to print the assembly for.
 * 
 *    @return char *         The assembly bytestream.
 */
char *_k_print_assembly(k_env_t *env);

#endif /* _LIBK_ASSEMBLE_H  */