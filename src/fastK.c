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

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#include <malloc.h>

const k_tokenable_t _tokenables[] = {
    {K_TOKEN_TYPE_UNKNOWN,       (const char*)0x0,                                                  K_TOKEN_TERMINATABLE_UNKNOWN},
    {K_TOKEN_TYPE_EOF,           "\0",                                                              K_TOKEN_TERMINATABLE_SINGLE},
    {K_TOKEN_TYPE_IDENTIFIER,    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", K_TOKEN_TERMINATABLE_MULTIPLE},
    {K_TOKEN_TYPE_NUMBER,        "0123456789",                                                      K_TOKEN_TERMINATABLE_MULTIPLE},
    {K_TOKEN_TYPE_STRING,        "\"",                                                              K_TOKEN_TERMINATABLE_REOCCUR},
    {K_TOKEN_TYPE_OPERATOR,      "+-*/%&|!^~<>=",                                                   K_TOKEN_TERMINATABLE_MULTIPLE},
    {K_TOKEN_TYPE_COMMENT,       "$",                                                               K_TOKEN_TERMINATABLE_REOCCUR},
    {K_TOKEN_TYPE_NEWSTATEMENT,  "{",                                                               K_TOKEN_TERMINATABLE_SINGLE},
    {K_TOKEN_TYPE_ENDSTATEMENT,  "}",                                                               K_TOKEN_TERMINATABLE_SINGLE},
    {K_TOKEN_TYPE_NEWEXPRESSION, "(",                                                               K_TOKEN_TERMINATABLE_SINGLE},
    {K_TOKEN_TYPE_ENDEXPRESSION, ")",                                                               K_TOKEN_TERMINATABLE_SINGLE},
    {K_TOKEN_TYPE_NEWINDEX,      "[",                                                               K_TOKEN_TERMINATABLE_SINGLE},
    {K_TOKEN_TYPE_ENDINDEX,      "]",                                                               K_TOKEN_TERMINATABLE_SINGLE},
    {K_TOKEN_TYPE_DECLARATOR,    ":",                                                               K_TOKEN_TERMINATABLE_SINGLE},
};

const char *_keywords[] = {
    "if",
    "else",
    "while",
    "return",
};

/*
 *    Creates a new KAPPA environment.
 *
 *    @return k_env_t *    A pointer to the new environment.
 */
k_env_t *k_new_env(void) {
    k_env_t *env = malloc(sizeof(k_env_t));

    if (env == (k_env_t*)0x0)
        return (k_env_t*)0x0;

    env->lexer   = (k_lexer_t*)0x0;

    return env;
}

/*
 *    Advances the lexer to the next char.
 *
 *    @param k_env_t    *env       The environment to advance the lexer in.
 *    @param const char *source    The source to advance the lexer in.
 */
void k_advance_lexer(k_env_t *env, const char *source) {
    if (env == (k_env_t*)0x0 || source == (const char*)0x0)
        return;

    if (env->lexer == (k_lexer_t*)0x0)
        return;

    env->lexer->index++;
    env->lexer->column++;
}

/*
 *    Skips whitespace in a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to skip whitespace in.
 *    @param const char *source    The source to skip whitespace in.
 */
void k_skip_whitespace(k_env_t *env, const char *source) {
    if (env == (k_env_t*)0x0 || source == (const char*)0x0)
        return;

    if (env->lexer == (k_lexer_t*)0x0)
        return;

    unsigned long *index = &env->lexer->index;
    k_lexer_t     *lexer = env->lexer;

loop_ws:

    while (source[*index] == ' '  || source[*index] == '\t' || source[*index] == '\r')
        k_advance_lexer(env, source);
    
    if (source[lexer->index] == '\n') {
        lexer->line++;
        lexer->index++;

        lexer->column = 0;
    }

    if (source[*index] == ' '  || source[*index] == '\t' || source[*index] == '\r')
        goto loop_ws;

    return;
}

/*
 *    Deduces the type of a token.
 *
 *    @param k_env_t    *env       The environment to deduce the token type in.
 *    @param const char *source    The source to deduce the token type in.
 *    
 *    @return unsigned long    The type index of the token.
 */
unsigned long k_deduce_token_type(k_env_t *env, const char *source) {
    unsigned long i = 0;

    if (env == (k_env_t*)0x0 || source == (const char*)0x0)
        return 0;

    if (env->lexer == (k_lexer_t*)0x0)
        return 0;

    for (i = 0; i < ARRAY_SIZE(_tokenables); i++) {
        if (_tokenables[i].chars == (const char*)0x0)
            continue;

        if (strchr(_tokenables[i].chars, source[env->lexer->index]) != (char*)0x0)
            return i;
    }

    return 0;
}

/*
 *    Parses a single token.
 *
 *    @param k_env_t    *env       The environment to parse the token in.
 *    @param const char *source    The source to parse the token in.
 * 
 *    @return const char *    The parsed token.
 */
const char *k_parse_token(k_env_t *env, const char *source) {
    unsigned long i = 0;
    static char   token[1024];

    if (env == (k_env_t*)0x0 || source == (const char*)0x0)
        return (const char*)0x0;

    if (env->lexer == (k_lexer_t*)0x0)
        return (const char*)0x0;

    memset(token, 0, sizeof(token));

    unsigned long type_index = env->lexer->tokens[env->lexer->token_count].type_index;
    k_lexer_t     *lexer     = env->lexer;

    switch (_tokenables[type_index].terminatable) {
        case K_TOKEN_TERMINATABLE_UNKNOWN:
            k_advance_lexer(env, source);

            return (const char*)0x0;
        case K_TOKEN_TERMINATABLE_SINGLE:
            token[0] = source[lexer->index];

            k_advance_lexer(env, source);
            break;
        case K_TOKEN_TERMINATABLE_MULTIPLE:
            do {
                token[i++] = source[lexer->index];

                k_advance_lexer(env, source);
            } while (strchr(_tokenables[type_index].chars, source[lexer->index]) != (char*)0x0);
            break;
        case K_TOKEN_TERMINATABLE_REOCCUR:
            do {
                token[i++] = source[lexer->index];

                k_advance_lexer(env, source);
            } while (source[lexer->index] != _tokenables[type_index].chars[0]);
            break;
    }

    return token;
}

/*
 *    Tokenizes a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to tokenize the source in.
 *    @param const char *source    The source to tokenize.
 */
void k_tokenize(k_env_t *env, const char *source) {
    unsigned int i   = 0;

    if (env == (k_env_t*)0x0 || source == (const char*)0x0)
        return;

    if (env->lexer == (k_lexer_t*)0x0)
        return;

    k_lexer_t *lexer = env->lexer;

    do {
        k_skip_whitespace(env, source);

        lexer->tokens = realloc(lexer->tokens, (i + 1) * sizeof(k_token_t));

        if (lexer->tokens == (k_token_t*)0x0)
            return;
        
        lexer->tokens[i].type_index   = k_deduce_token_type(env, source);
        lexer->tokens[i].line         = lexer->line;
        lexer->tokens[i].column       = lexer->column;
        lexer->tokens[i].index        = lexer->index;

        lexer->token_count = i++;

        k_parse_token(env, source);
    } while (source[lexer->index] != '\0');
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

    k_tokenize(env, source);
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