/*
 *    libk_operator.c    --    Source for KAPPA compilation
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on August 5, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    The definitions for how KAPPA operators are compiled are contained in this file.
 */
#include "libk_operator.h"

#include "libk_assemble.h"
#include "libk_compile.h"

#include "util.h"

/*
 *    Compiles the multiplication operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_mul(k_env_t* env, node_t *node) {
    _k_assemble_multiplication(env);

    return K_ERROR_NONE;
}

/*
 *    Compiles the division operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 *
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_div(k_env_t* env, node_t *node) {
    _k_assemble_swap_rax_rcx(env);
    _k_assemble_division(env);

    return K_ERROR_NONE;
}

/*
 *    Compiles the modulo operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_mod(k_env_t* env, node_t *node) {
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
 *    @param  node_t     *node   The tree node with the operator.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_add(k_env_t* env, node_t *node) {
    _k_assemble_addition(env);

    return K_ERROR_NONE;
}

/*
 *    Compiles the subtraction operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_sub(k_env_t* env, node_t *node) {
    _k_assemble_swap_rax_rcx(env);
    _k_assemble_subtraction(env);

    return K_ERROR_NONE;
}

/*
 *    Compiles a comparison operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_cmp(k_env_t* env, node_t *node) {
    _k_assemble_comparison(env, *(_k_op_type_e*)node->data);

    return K_ERROR_NONE;
}

/*
 *    Compiles a reference operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 *
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_ref(k_env_t* env, node_t *node) {

}

/*
 *    Compiles a dereference operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 *
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_deref(k_env_t* env, node_t *node) {
    
}

/*
 *    Compiles a pointer assignment operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 */
k_build_error_t _k_compile_ptr_assign(k_env_t* env, node_t *node) {
    _k_assemble_swap_rax_rcx(env);
    _k_assemble_move_ptr(env);
}

/*
 *    Compiles the assignment operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 *
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_assignment(k_env_t *env, node_t *node) {
    _k_variable_t *var = _k_get_var(env, (*(_k_token_t**)node->left->data)->str);

    /* Assembly generated should put arithmetic register into local address. */
    if (var->flags & _K_VARIABLE_FLAG_GLOBAL)
        _k_assemble_assignment_global(env, env->runtime->size - var->offset);

    else _k_assemble_assignment(env, var->offset, var->size, var->flags & _K_VARIABLE_FLAG_FLOAT);

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
    _K_OP_ASSIGN,
    _K_OP_REF,
    _K_OP_DEREF,
    _K_OP_PTR_ASSIGN,
};

unsigned long                 _op_list_size = ARRAY_SIZE(_op_list);

const k_build_error_t       (*_op_compile_list[])(k_env_t*, node_t*) = {
    _k_compile_mul,
    _k_compile_div,
    _k_compile_mod,
    _k_compile_add,
    _k_compile_sub,
    _k_compile_cmp,
    _k_compile_cmp,
    _k_compile_cmp,
    _k_compile_cmp,
    _k_compile_cmp,
    _k_compile_cmp,
    _k_compile_assignment,
    _k_compile_ref,
    _k_compile_deref,
    _k_compile_ptr_assign
};

unsigned long      _op_compile_list_size = ARRAY_SIZE(_op_compile_list);

unsigned long      _op_hierarchy_list[] = {
    3,
    3,
    3,
    2,
    2,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    4,
    4,
    0
};

unsigned long      _op_hierarchy_list_size = ARRAY_SIZE(_op_hierarchy_list);