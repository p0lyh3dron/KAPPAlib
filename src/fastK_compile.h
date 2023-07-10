/*
 *    fastK_compile.h    --    Header for KAPPA compilation
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 9, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This file declares the functions used to compile KAPPA source code.
 */
#ifndef FASTK_COMPILE_H
#define FASTK_COMPILE_H

#include "fastK.h"

/*
 *    Deduces the size of a variable.
 *
 *    @param const char *type    The type of the variable.
 * 
 *    @return unsigned long      The size of the variable.
 */
unsigned long k_deduce_size(const char *type);

/*
 *    Adds a local variable to the compiler environment.
 *
 *    @param char *name    The name of the variable.
 *
 *    @return unsigned long    The offset of the variable.
 */
unsigned long k_add_local(const char *name);

/*
 *    Gets the offset of a local variable.
 *
 *    @param const char *name    The name of the variable.
 * 
 *    @return unsigned long      The offset of the variable.
 */
unsigned long k_get_local(const char *name);

/*
 *    Compiles an expression.
 *
 *    @param k_env_t    *env       The environment to parse the expression in.
 *    @param const char *source    The source to parse the expression from.
 */
void k_compile_expression(k_env_t *env, const char *source);

/*
 *    Compiles a statement.
 *
 *    @param k_env_t    *env       The environment to parse the statement in.
 *    @param const char *source    The source to parse the statement from.
 */
void k_compile_statement(k_env_t *env, const char *source);

/*
 *    Compiles a global declaration.
 *
 *    @param k_env_t    *env       The environment to parse the function declaration in.
 * 
 *    @return k_compile_error_t    The error code, if any.
 */
k_compile_error_t k_compile_global_declaration(k_env_t *env, const char *source);
/*
 *    Compiles a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to compile the source in.
 *    @param const char *source    The source to compile.
 */
void k_compile(k_env_t *env, const char *source);

#endif /* FASTK_COMPILE_H  */