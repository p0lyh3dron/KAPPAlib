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
#ifndef UTIL_H
#define UTIL_H

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

/*
 *    Returns the current error.
 *
 *    @param k_env_t *env       The environment to get the error from.
 *    @param const char *msg    The error text.
 *    @param ...                The arguments to the error text.
 * 
 *    @return const char *   The current error.
 */
const char *k_get_error(k_env_t *env, const char *msg, ...);

/*
 *    Returns the identifier string.
 *
 *    @param const char *source    The source to get the identifier from.
 *    @param unsigned long index   The index to start at.
 *    @param unsigned long length  The length of the identifier.
 *
 *    @return char *   The identifier string.
 */
char *k_get_token_str(const char *source, unsigned long index, unsigned long length);

/*
 *    Swaps the endianness of a 4-byte integer.
 *
 *    @param unsigned int *a    The integer.
 */
void swap_endian32(unsigned int *a);

#endif /* UTIL_H  */