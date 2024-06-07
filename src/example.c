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

#include "libk.h"

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} color_t;

int main() {
    FILE *fp = fopen("math.k", "r");

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

    float cos = 3.0;
    k_call_function(env, "cos", (void **)&cos, 1, (void*)*(unsigned long*)&cos);
    
    float sin = 4.0;
    k_call_function(env, "sin", (void **)&sin, 1, (void*)*(unsigned long*)&sin);

    float log = 5.0;
    k_call_function(env, "log", (void **)&log, 1, (void*)*(unsigned long*)&log);

    float exp = 6.0;
    k_call_function(env, "exp", (void **)&exp, 1, (void*)*(unsigned long*)&exp);

    float pow_base = 2.0;
    float pow_exp  = 0.5;
    k_call_function(env, "pow", (void **)&pow_base, 2, (void*)*(unsigned long*)&pow_base, (void*)*(unsigned long*)&pow_exp);

    printf("cos(3.0) = %f\n", cos);
    printf("sin(4.0) = %f\n", sin);
    printf("ln(5.0)  = %f\n", log);
    printf("exp(6.0) = %f\n", exp);
    printf("pow(2.0, 0.5) = %f\n", pow_base);

    k_destroy_env(env);
    
    return 0;
}