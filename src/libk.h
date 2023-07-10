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
 */
void k_build(k_env_t *env, const char *source);

/*
 *    Destroys a KAPPA environment.
 *
 *    @param k_env_t *env    The environment to destroy.
 */
void k_destroy_env(k_env_t *env);

#endif /* _LIBK_H  */