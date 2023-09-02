/*
 *    libk_operator.h    --    Header for KAPPA operators
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on August 5, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This file declares the functions for compilation of KAPPA operators.
 */
#ifndef _LIBK_OPERATOR_H
#define _LIBK_OPERATOR_H

#include "tree.h"
#include "types.h"

extern const _k_op_type_e _op_list[];
extern unsigned long      _op_list_size;

extern const k_build_error_t       (*_op_compile_list[])(k_env_t*, node_t*);
extern unsigned long                 _op_compile_list_size;

extern unsigned long      _op_hierarchy_list[];
extern unsigned long      _op_hierarchy_list_size;

/*
 *    Compiles the multiplication operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_mul(k_env_t* env, node_t *node);

/*
 *    Compiles the division operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 *
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_div(k_env_t* env, node_t *node);

/*
 *    Compiles the modulo operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_mod(k_env_t* env, node_t *node);

/*
 *    Compiles the addition operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_add(k_env_t* env, node_t *node);

/*
 *    Compiles the subtraction operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_sub(k_env_t* env, node_t *node);

/*
 *    Compiles a comparison operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_cmp(k_env_t* env, node_t *node);

/*
 *    Compiles a reference operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 *
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_ref(k_env_t* env, node_t *node);

/*
 *    Compiles a dereference operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 *
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile_deref(k_env_t* env, node_t *node);

/*
 *    Compiles a pointer assignment operator.
 *
 *    @param  k_env_t*    env    The environment to compile the operator in.
 *    @param  node_t     *node   The tree node with the operator.
 */
k_build_error_t _k_compile_ptr_assign(k_env_t* env, node_t *node);

#endif /* _LIBK_OPERATOR_H  */