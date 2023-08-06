/*
 *    libk_operator.c    --    Source for KAPPA compilation
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on August 5, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    The definitions for how KAPPA operators are compiled are contained in this file.
 */
#include "libk_compile.h"

#include "util.h"

/*
 *    Compiles the multiplication operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param _k_op_type_e type   The type of the operator.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t k_compile_mul(k_env_t* env, _k_op_type_e type) {
    _k_assemble_multiplication(env);

    return K_ERROR_NONE;
}

/*
 *    Compiles the division operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param _k_op_type_e type   The type of the operator.
 *
 *    @return k_build_error_t    The error code.
 */
k_build_error_t k_compile_div(k_env_t* env, _k_op_type_e type) {
    _k_assemble_swap_rax_rcx(env);
    _k_assemble_division(env);

    return K_ERROR_NONE;
}

/*
 *    Compiles the modulo operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param _k_op_type_e type   The type of the operator.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t k_compile_mod(k_env_t* env, _k_op_type_e type) {
    _k_type_t lh_type = env->runtime->current_operation->lh_type;
    _k_type_t rh_type = env->runtime->current_operation->rh_type;

    if (lh_type.is_float == 1 || rh_type.is_float == 1)
        return K_ERROR_UNALLOWED_FLOAT;

    _k_assemble_swap_rax_rcx(env);
    _k_assemble_division(env);
    _k_assemble_mov_rdx_rax(env);

    return K_ERROR_NONE;
}

/*
 *    Compiles the addition operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param _k_op_type_e type   The type of the operator.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t k_compile_add(k_env_t* env, _k_op_type_e type) {
    _k_assemble_addition(env);

    return K_ERROR_NONE;
}

/*
 *    Compiles the subtraction operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param _k_op_type_e type   The type of the operator.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t k_compile_sub(k_env_t* env, _k_op_type_e type) {
    _k_assemble_swap_rax_rcx(env);
    _k_assemble_subtraction(env);

    return K_ERROR_NONE;
}

/*
 *    Compiles a comparison operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param _k_op_type_e type   The type of the operator.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t k_compile_cmp(k_env_t* env, _k_op_type_e type) {
    _k_assemble_comparison(env, type);

    return K_ERROR_NONE;
}

const _k_op_type_e _op_list[] = {
    _K_OP_MUL,
    _K_OP_DIV,
    _K_OP_MOD,
    _K_OP_ADD,
    _K_OP_SUB,
    _K_OP_L,
    _K_OP_LE,
    _K_OP_G,
    _K_OP_GE,
    _K_OP_E,
    _K_OP_NE,
};

unsigned long                 _op_list_size = ARRAY_SIZE(_op_list);

const k_build_error_t       (*_op_compile_list[])(k_env_t*, _k_op_type_e) = {
    k_compile_mul,
    k_compile_div,
    k_compile_mod,
    k_compile_add,
    k_compile_sub,
    k_compile_cmp,
    k_compile_cmp,
    k_compile_cmp,
    k_compile_cmp,
    k_compile_cmp,
    k_compile_cmp,
};

unsigned long      _op_compile_list_size = ARRAY_SIZE(_op_compile_list);

unsigned long      _op_hierarchy_list[] = {
    0,
    0,
    0,
    1,
    1,
    2,
    2,
    2,
    2,
    2,
    2,
    3
};

unsigned long      _op_hierarchy_list_size = ARRAY_SIZE(_op_hierarchy_list);