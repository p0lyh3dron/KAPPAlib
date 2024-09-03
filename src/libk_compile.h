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
 *    Compiles a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to compile the source in.
 *    @param const char *source    The source to compile.
 * 
 *    @return k_build_error_t    The error code.
 */
char *_k_compile(_k_token_t *tokens);

#endif /* _LIBK_COMPILE_H  */