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

#include "builtin.h"
#include "util.h"

#include "libk_compile.h"
#include "libk_parse.h"

/*
 *    Creates a new KAPPA environment.
 *
 *    @return k_env_t *    A pointer to the new environment.
 */
k_env_t *k_new_env(void) {
    k_env_t *env = malloc(sizeof(k_env_t));

    if (env == (k_env_t*)0x0)
        return (k_env_t*)0x0;

    env->lexer     = (_k_lexer_t*)0x0;
    env->runtime   = (_k_runtime_t*)0x0;
    env->log       = (int (*)(const char*))0x0;
    env->cur_token = (_k_token_t*)0x0;

    return env;
}

/*
 *    Sets the log handler for a KAPPA environment.
 *
 *    @param k_env_t *env                      The environment to set the error handler for.
 *    @param int (*log)(const char *msg)       The error handler.
 */
void k_set_log_handler(k_env_t *env, int (*log)(const char *msg)) {
    if (env == (k_env_t*)0x0)
        return;

    env->log = log;
}

/*
 *    Builds a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to compile the source in.
 *    @param const char *source    The source to compile.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t k_build(k_env_t *env, const char *source) {
    if (env == (k_env_t*)0x0 || source == (const char*)0x0)
        return;

    if (env->lexer != (_k_lexer_t*)0x0)
        free(env->lexer);

    env->lexer = malloc(sizeof(_k_lexer_t));

    if (env->lexer == (_k_lexer_t*)0x0)
        return;

    env->lexer->tokens = (_k_token_t*)0x0;
    env->lexer->token_count = 0;
    env->lexer->index = 0;
    env->lexer->line = 1;
    env->lexer->column = 0;

    _k_lexical_analysis(env, source);
    return _k_compile(env, source);
}

/*
 *    Returns a function pointer to a built function.
 *
 *    @param k_env_t    *env       The environment to get the function from.
 *    @param const char *name      The name of the function.
 * 
 *    @return void *    The function pointer.
 */
void *k_get_function(k_env_t *env, const char *name) {
    if (env == (k_env_t*)0x0 || name == (const char*)0x0)
        return (void*)0x0;

    for (unsigned long i = 0; i < env->runtime->function_count; i++) {
        if (strcmp(env->runtime->function_table[i].name, name) == 0) {
            return env->runtime->function_table[i].source;
        }
    }

    return (void*)0x0;
}

/*
 *    Calls a function via a wrapper.
 *
 *    @param k_env_t       *env       The environment to call the function in.
 *    @param const char    *name      The name of the function.
 *    @param void         **ret       The return value of the function.
 *    @param unsigned long  argc      The number of arguments to pass to the function.
 *    @param void          *arg       The argument to pass to the function.
 *    @param ...                       The rest of the arguments to pass to the function.
 */
void k_call_function(k_env_t *env, const char *name, void **ret, unsigned long argc, void *arg, ...) {
    if (env == (k_env_t*)0x0 || name == (const char*)0x0 || ret == (void*)0x0)
        return;

    for (unsigned long i = 0; i < env->runtime->function_count; i++) {
        if (strcmp(env->runtime->function_table[i].name, name) == 0) {
            /*
             *    Use inline assembly to put arguments in the correct registers.
             *
             *    RDI = 1, RSI = 2, R8 = 3, R9 = 4
             */
            _k_function_t *func = &env->runtime->function_table[i];

            if (func->parameter_count != argc) {
                env->log("k_call_function: argument count mismatch\n");
                return;
            }

            unsigned long j = 0;
            va_list args;

            va_start(args, arg);

            while (j < argc) {
                switch (j) {
                    case 0:
                        if (func->parameters[j++].flags & _K_VARIABLE_FLAG_FLOAT)
                            asm volatile("movq %0, %%rax\n\t"
                                         "movsd (%%rax), %%xmm7"
                                         : : "r" (&arg));
                        else
                            asm volatile("mov %0, %%rdi" : : "r" (arg));
                        break;
                    case 1:
                        if (func->parameters[j++].flags & _K_VARIABLE_FLAG_FLOAT)
                            asm volatile("movq %0, %%rax\n\t"
                                         "movsd (%%rax), %%xmm6"
                                         : : "r" (&arg));
                        else
                            asm volatile("mov %0, %%rsi" : : "r" (arg));
                        break;
                    case 2:
                        if (func->parameters[j++].flags & _K_VARIABLE_FLAG_FLOAT)
                            asm volatile("movq %0, %%rax\n\t"
                                         "movsd (%%rax), %%xmm5"
                                         : : "r" (&arg));
                        else
                            asm volatile("mov %0, %%r8" : : "r" (arg));
                        break;
                    case 3:
                        if (func->parameters[j++].flags & _K_VARIABLE_FLAG_FLOAT)
                            asm volatile("movq %0, %%rax\n\t"
                                         "movsd (%%rax), %%xmm4"
                                         : : "r" (&arg));
                        else
                            asm volatile("mov %0, %%r9" : : "r" (arg));
                        break;
                }

                arg = va_arg(args, void*);
            }

            va_end(args);

            asm volatile("mov %0, %%rax" : : "r" (env->runtime->mem + (long)func->source));
            asm volatile("call *%rax");

            /* Put return in an unlikely used register.  */
            asm volatile("mov %rax, %r13");

            if (ret != (void*)0x0) {
                if (func->flags & _K_VARIABLE_FLAG_FLOAT) {
                    asm volatile("movq %%xmm0, %0" : "=r" (*ret));
                }

                else
                    asm volatile("mov %%r13, %0" : "=r" (*ret));
            }

            return;
        }
    }
}

/*
 *    Destroys a KAPPA environment.
 *
 *    @param k_env_t *env    The environment to destroy.
 */
void k_destroy_env(k_env_t *env) {
    if (env != (k_env_t*)0x0) {
        if (env->lexer != (_k_lexer_t*)0x0) {
            if (env->lexer->tokens != (_k_token_t*)0x0) {
                for (unsigned long i = 0; i < env->lexer->token_count; i++) {
                    free(env->lexer->tokens[i].str);
                }
                free(env->lexer->tokens);
            }
            free(env->lexer);
        }

        if (env->runtime != (_k_runtime_t*)0x0) {
            if (env->runtime->mem != (char*)0x0)
                munmap(env->runtime->mem, env->runtime->size);
            if (env->runtime->function_table != (_k_function_t*)0x0)
                free(env->runtime->function_table);
            free(env->runtime);
        }

        free(env);
    }
}