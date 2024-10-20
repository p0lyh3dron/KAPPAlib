/*
 *    libk.c    --    source for fast KAPPA computation
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on February 23, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    The definitions for functions related to KAPPA parsing and
 *    interpretation are contained in this file.
 */
#include "libk.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "builtin.h"

#include "libk_compile.h"
#include "libk_parse.h"

/*
 *    Builds a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to compile the source in.
 *    @param const char *source    The source to compile.
 * 
 *    @return k_build_error_t    The error code.
 */
char *k_build(const char *source, int flags) {
    return _k_compile(_k_lexical_analysis(source), flags);
}

/*
 *    Gets the error code.
 *
 *    @return int    The error code.
 */
int k_get_error_code() {
    return _k_get_error_code();
}

/*
 *    Gets the error message.
 *
 *    @param int error_code    The error code.
 * 
 *    @return const char    The error message.
 */
const char *k_get_error_message(int error_code) {
    switch (error_code) {
        case 0: return (const char *)0x0;
        case 1: return "Unexpected literal following literal";
        case 2: return "Keyword statement cannot exist in expression";
    }

    return "Unknown error";
}