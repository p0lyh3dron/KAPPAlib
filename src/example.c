/*
 *    example_fractal.c    --    example source file
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on August 12, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This is an example source file for creating fractals.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "libk.h"

int main() {
    char source[0xFFFF];
    char c;
    int  i = 0;

    while (read(0, &c, 1) > 0) source[i++] = c;

    source[i] = '\0';

    char *result = k_build(source, 1);

    const char *error = k_get_error_message(k_get_error_code());

    if (error != (const char *)0x0) {
        fprintf(stderr, "\e[31m\033[1mError\e[0m\033[0m: %s\n", error);
        return 1;
    }

    fprintf(stdout, "%s", result);
    
    return 0;
}