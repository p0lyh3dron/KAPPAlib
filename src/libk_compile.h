/*
 *    libk_compile.h    --    Header for KAPPA compilation
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 9, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This file declares the functions used to compile KAPPA source code.
 */
#ifndef _LIBK_COMPILE_H
#define _LIBK_COMPILE_H

#include "types.h"

/*
 *    Deduces the size of a variable.
 *
 *    @param const char *type    The type of the variable.
 * 
 *    @return unsigned long      The size of the variable.
 */
unsigned long _k_deduce_size(const char *type);

/*
 *    Adds a variable to the compiler environment.
 *
 *    @param k_env_t       *env     The environment to add the variable to.
 *    @param _k_variable_t *var     The variable to add.
 *    @param char           global  Whether or not the variable is global.
 */
void _k_add_var(k_env_t *env, _k_variable_t *var, char global);

/*
 *    Gets a variable from the compiler environment.
 *
 *    @param k_env_t    *env     The environment to get the variable from.
 *    @param const char *name    The name of the variable.
 * 
 *    @return _k_variable_t *    The variable.
 */
_k_variable_t *_k_get_var(k_env_t *env, const char *name);

/*
 *    Gets the address of a function.
 *
 *    @param k_env_t    *env    The environment to get the function from.
 *    @param const char *name    The name of the function.
 *
 *    @return char *      The address of the function.
 */
char *_k_get_function(k_env_t *env, const char *name);

/*
 *    Compiles an expression.
 *
 *    @param k_env_t    *env       The environment to parse the expression in.
 *
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_expression(k_env_t *env);

/*
 *    Compiles a statement.
 *
 *    @param k_env_t    *env       The environment to parse the statement in.
 * 
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_statement(k_env_t *env);

/*
 *    Compiles a global declaration.
 *
 *    @param k_env_t    *env       The environment to parse the function declaration in.
 * 
 *    @return k_build_error_t    The error code, if any.
 */
k_build_error_t _k_compile_loop(k_env_t *env, const char *source);

/*
 *    Compiles a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to compile the source in.
 *    @param const char *source    The source to compile.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile(k_env_t *env, const char *source);

#endif /* _LIBK_COMPILE_H  */