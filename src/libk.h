/*
 *    libk.h    --    header for fast KAPPA computation
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on February 23, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    KAPPA is a strongly-typed functional programming language
 *    designed to be similar to C, but with less boilerplate,
 *    and more powerful features, alongside being interpreted
 *    in addition to being compiled.
 * 
 *    The declarations for functions related to KAPPA parsing and
 *    interpretation are contained in this file.
 */
#ifndef _LIBK_H
#define _LIBK_H

#include "types.h"

/*
 *    Creates a new KAPPA environment.
 *
 *    @return k_env_t *    A pointer to the new environment.
 */
k_env_t *k_new_env(void);

/*
 *    Sets the error handler for a KAPPA environment.
 *
 *    @param k_env_t *env                      The environment to set the error handler for.
 *    @param int (*error)(const char *msg)     The error handler.
 */
void k_set_log_handler(k_env_t *env, int (*error)(const char *msg));

/*
 *    Builds a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to compile the source in.
 *    @param const char *source    The source to compile.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t k_build(k_env_t *env, const char *source);

/*
 *    Returns a function pointer to a built function.
 *
 *    @param k_env_t    *env       The environment to get the function from.
 *    @param const char *name      The name of the function.
 * 
 *    @return void *    The function pointer.
 */
void *k_get_function(k_env_t *env, const char *name);

/*
 *    Calls a function via a wrapper.
 *
 *    @param k_env_t       *env       The environment to call the function in.
 *    @param const char    *name      The name of the function.
 *    @param void         **ret       The return value of the function.
 *    @param unsigned long  argc      The number of arguments to pass to the function.
 *    @param void          *arg       The argument to pass to the function.
 *    @param ...                       The rest of the arguments to pass to the function.
 */
void k_call_function(k_env_t *env, const char *name, void **ret, unsigned long argc, void *arg, ...);

/*
 *    Destroys a KAPPA environment.
 *
 *    @param k_env_t *env    The environment to destroy.
 */
void k_destroy_env(k_env_t *env);

#endif /* _LIBK_H  */