/*
 *    fastK.c    --    source for fast KAPPA computation
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on February 23, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    The definitions for functions related to KAPPA parsing and
 *    interpretation are contained in this file.
 */
#include "fastK.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builtin.h"
#include "util.h"

#include "fastK_compile.h"
#include "fastK_parse.h"

/*
 *    Creates a new KAPPA environment.
 *
 *    @return k_env_t *    A pointer to the new environment.
 */
k_env_t *k_new_env(void) {
    k_env_t *env = malloc(sizeof(k_env_t));

    if (env == (k_env_t*)0x0)
        return (k_env_t*)0x0;

    env->lexer     = (k_lexer_t*)0x0;
    env->runtime   = (k_runtime_t*)0x0;
    env->log       = (int (*)(const char*))0x0;
    env->cur_token = (k_token_t*)0x0;

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
 *    Parses a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to parse the source in.
 *    @param const char *source    The source to parse.
 */
void k_parse(k_env_t *env, const char *source) {
    if (env == (k_env_t*)0x0 || source == (const char*)0x0)
        return;

    if (env->lexer != (k_lexer_t*)0x0)
        free(env->lexer);

    env->lexer = malloc(sizeof(k_lexer_t));

    if (env->lexer == (k_lexer_t*)0x0)
        return;

    env->lexer->tokens = (k_token_t*)0x0;
    env->lexer->token_count = 0;
    env->lexer->index = 0;
    env->lexer->line = 1;
    env->lexer->column = 0;

    k_lexical_analysis(env, source);
    k_compile(env, source);
}

/*
 *    Destroys a KAPPA environment.
 *
 *    @param k_env_t *env    The environment to destroy.
 */
void k_destroy_env(k_env_t *env) {
    if (env != (k_env_t*)0x0) {
        if (env->lexer != (k_lexer_t*)0x0) {
            if (env->lexer->tokens != (k_token_t*)0x0) {
                free(env->lexer->tokens);
            }
            free(env->lexer);
        }
        free(env);
    }
}