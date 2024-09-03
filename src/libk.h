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
 *    Builds a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to compile the source in.
 *    @param const char *source    The source to compile.
 * 
 *    @return k_build_error_t    The error code.
 */
char *k_build(const char *source);

#endif /* _LIBK_H  */