/*
 *    util.h    --    header for utility functions for KAPPA parsing
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 1, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    The declarations for utility functions for KAPPA parsing and
 *    interpretation are contained in this file.
 */
#ifndef _LIBK_UTIL_H
#define _LIBK_UTIL_H

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define MAX(x, y)     ((x) > (y) ? (x) : (y))
#define MIN(x, y)     ((x) < (y) ? (x) : (y))
#define XOR(x, y)     ((x) && !(y) || !(x) && (y))

#include "libk.h"

/*
 *    Returns the current error.
 *
 *    @param k_env_t *env       The environment to get the error from.
 *    @param const char *msg    The error text.
 *    @param ...                The arguments to the error text.
 * 
 *    @return const char *   The current error.
 */
const char *_k_get_error(k_env_t *env, const char *msg, ...);

/*
 *    Returns the current warning.
 *
 *    @param k_env_t *env       The environment to get the warning from.
 *    @param const char *msg    The warning text.
 *    @param ...                The arguments to the warning text.
 * 
 *    @return const char *   The current warning.
 */
const char *_k_get_warning(k_env_t *env, const char *msg, ...);

/*
 *    Returns the identifier string.
 *
 *    @param k_env_t *env       The environment to get the identifier from.
 *
 *    @return char *   The identifier string.
 */
char *_k_get_token_str(k_env_t *env);

/*
 *    Appends a string to a string.
 *
 *    @param char **str    The string to append to.
 *    @param char *app    The string to append.
 */
void _k_append_str(char **str, char *app, ...);

/*
 *    Swaps the endianness of a 4-byte integer.
 *
 *    @param unsigned int *a    The integer.
 */
void _swap_endian32(unsigned int *a);

#endif /* _LIBK_UTIL_H  */