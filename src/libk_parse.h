/*
 *    libk_parse.h    --    Header for KAPPA parsing
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 9, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This file declares the functions and data structures used to parse
 *    KAPPA files.
 */
#ifndef _LIBK_PARSE_H
#define _LIBK_PARSE_H

#include "types.h"

/*
 *    Performs lexical analysis on the token stream.
 *
 *    @param k_env_t    *env       The environment to perform lexical analysis in.
 *    @param const char *source    The source to perform lexical analysis on.
 */
_k_token_t *_k_lexical_analysis(const char *source);

#endif /* _LIBK_PARSE_H  */