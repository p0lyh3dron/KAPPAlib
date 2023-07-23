/*
 *    example.c    --    example source file
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on February 27, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This is an example source file for the usage of the KAPPA
 *    interpreter.
 */
#include <stdio.h>
#include <stdlib.h>

#include "libk.h"

int main() {
    FILE *fp = fopen("fib.k", "r");

    if (fp == (FILE*)0x0) {
        fprintf(stderr, "Failed to open example.k!\n");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *source = malloc(fsize + 1);

    fread(source, fsize, 1, fp);
    fclose(fp);

    k_env_t *env = k_new_env();

    if (env == (k_env_t*)0x0) {
        fprintf(stderr, "Failed to create KAPPA environment!\n");
        return 1;
    }

    k_set_log_handler(env, puts);

    k_build_error_t ret = k_build(env, source);

    if (ret != K_ERROR_NONE) {
        fprintf(stderr, "Failed to build example.k!\n");
        return 1;
    }

    free(source);

    unsigned long fib;
    unsigned long factorial;

    k_call_function(env, "fib", &fib, 1, 10);
    k_call_function(env, "factorial", &factorial, 1, 10);

    printf("fib(%d) = %lu\n", 10, fib);
    printf("factorial(%d) = %lu\n", 10, factorial);

    k_destroy_env(env);
    
    return 0;
}