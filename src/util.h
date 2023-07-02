/*
 *    fastK.h    --    header for utility functions for KAPPA parsing
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
 *
 *    @return char *   The identifier string.
 */
char *k_get_identifier(const char *source, unsigned long index);

#endif /* UTIL_H  */