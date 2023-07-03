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
#include "fastK.h"

#include <stdarg.h>

/*
 *    Returns the current error.
 *
 *    @param k_env_t *env       The environment to get the error from.
 *    @param const char *msg    The error text.
 *    @param ...                The arguments to the error text.
 * 
 *    @return const char *   The current error.
 */
const char *k_get_error(k_env_t *env, const char *msg, ...) {
    static char error[256];
    static char buffer[256];

    va_list args;

    memset(error, 0, 256);

    va_start(args, msg);
    vsprintf(buffer, msg, args);
    va_end(args);

    sprintf(error, "Error | %lu-%lu: %s", env->cur_token->line, env->cur_token->column, buffer);

    return error;
}

/*
 *    Returns the token string.
 *
 *    @param const char *source    The source to get the identifier from.
 *    @param unsigned long index   The index to start at.
 *    @param unsigned long length  The length of the identifier.
 *
 *    @return char *   The identifier string.
 */
char *k_get_token_str(const char *source, unsigned long index, unsigned long length) {
    static char identifier[256];
    unsigned long i = 0;

    memset(identifier, 0, 256);

    for (i = 0; i < length; i++) {
        identifier[i] = source[index];
        index++;
    }

    return identifier;
}

/*
 *    Swaps the endianness of a 4-byte integer.
 *
 *    @param unsigned int *a    The integer.
 */
void swap_endian32(unsigned int *a) {
    *a = ((*a >> 24) & 0x000000FF) | ((*a >> 8) & 0x0000FF00) |
         ((*a << 8) & 0x00FF0000) | ((*a << 24) & 0xFF000000);
}