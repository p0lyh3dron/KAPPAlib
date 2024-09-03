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

    while (read(0, &c, 1) > 0) {
        source[i++] = c;
    }

    source[i] = '\0'; // source is now a null-terminated string

    char *result = k_build(source);

    fprintf(stdout, "%s", result);
    
    return 0;
}