/*
 *    util.c    --    source for utility functions for KAPPA parsing
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 2, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This file defines the utility functions for KAPPA parsing and
 *    interpretation.
 */
#include "libk.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *    Returns the current error.
 *
 *    @param k_env_t *env       The environment to get the error from.
 *    @param const char *msg    The error text.
 *    @param ...                The arguments to the error text.
 * 
 *    @return const char *   The current error.
 */
const char *_k_get_error(k_env_t *env, const char *msg, ...) {
    static char error[256];
    static char buffer[256];

    va_list args;

    memset(error, 0, 256);

    va_start(args, msg);
    vsprintf(buffer, msg, args);
    va_end(args);

    sprintf(error, "\e[1;31mError\e[0m | %lu-%lu: %s", env->cur_token->line, env->cur_token->column, buffer);

    return error;
}

/*
 *    Returns the current warning.
 *
 *    @param k_env_t *env       The environment to get the warning from.
 *    @param const char *msg    The warning text.
 *    @param ...                The arguments to the warning text.
 * 
 *    @return const char *   The current warning.
 */
const char *_k_get_warning(k_env_t *env, const char *msg, ...) {
    static char warning[256];
    static char buffer[256];

    va_list args;

    memset(warning, 0, 256);

    va_start(args, msg);
    vsprintf(buffer, msg, args);
    va_end(args);

    sprintf(warning, "\e[1;33mWarning\e[0m | %lu-%lu: %s", env->cur_token->line, env->cur_token->column, buffer);

    return warning;
}

/*
 *    Returns the identifier string.
 *
 *    @param k_env_t *env       The environment to get the identifier from.
 *
 *    @return char *   The identifier string.
 */
char *_k_get_token_str(k_env_t *env) {
    return env->cur_token->str;
}

/*
 *    Swaps the endianness of a 4-byte integer.
 *
 *    @param unsigned int *a    The integer.
 */
void _swap_endian32(unsigned int *a) {
    *a = ((*a >> 24) & 0x000000FF) | ((*a >> 8) & 0x0000FF00) |
         ((*a << 8) & 0x00FF0000) | ((*a << 24) & 0xFF000000);
}