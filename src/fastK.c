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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    {K_TOKEN_TYPE_KEYWORD,       (const char*)0x0,                                                  K_TOKEN_TERMINATABLE_UNKNOWN},
    {K_TOKEN_TYPE_ENDLINE,       ";",                                                               K_TOKEN_TERMINATABLE_SINGLE},
    {K_TOKEN_TYPE_SEPARATOR,     ",",                                                               K_TOKEN_TERMINATABLE_SINGLE},
};

const char *_keywords[] = {
    "if",
    "else",
    "while",
    "return",
};

const char *_types[] = {
    "s8",
    "s16",
    "s32",
    "s64",
    "u8",
    "u16",
    "u32",
    "u64",
    "f32",
    "f64",
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

    env->lexer     = (k_lexer_t*)0x0;
    env->runtime   = (k_runtime_t*)0x0;
    env->error     = (void (*)(const char*))0x0;
    env->cur_token = (k_token_t*)0x0;

    return env;
}

/*
 *    Sets the error handler for a KAPPA environment.
 *
 *    @param k_env_t *env                      The environment to set the error handler for.
 *    @param void (*error)(const char *msg)    The error handler.
 */
void k_set_error_handler(k_env_t *env, void (*error)(const char *msg)) {
    if (env == (k_env_t*)0x0)
        return;

    env->error = error;
}

/*
 *    Returns the current error.
 *
 *    @param k_env_t *env       The environment to get the error from.
 *    @param const char *msg    The error text.
 * 
 *    @return const char *   The current error.
 */
const char *k_get_error(k_env_t *env, const char *msg) {
    static char error[256];

    memset(error, 0, 256);

    sprintf(error, "Error | %lu-%lu: %s", env->cur_token->line, env->cur_token->column, msg);

    return error;
}

/*
 *    Advances the lexer to the next char.
 *
 *    @param k_env_t    *env       The environment to advance the lexer in.
 */
void k_advance_lexer(k_env_t *env) {
    env->lexer->index++;
    env->lexer->column++;
}

/*
 *    Advances to the next token.
 *
 *    @param k_env_t    *env       The environment to advance the lexer in.
 */
void k_advance_token(k_env_t *env) {
    env->cur_token += 1;

    env->cur_type  = env->cur_token->tokenable->type;
}

/*
 *    Skips whitespace in a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to skip whitespace in.
 *    @param const char *source    The source to skip whitespace in.
 */
void k_skip_whitespace(k_env_t *env, const char *source) {
    unsigned long *index = &env->lexer->index;
    k_lexer_t     *lexer = env->lexer;

    while (source[*index] == ' '  || source[*index] == '\t' || source[*index] == '\r' || source[*index] == '\n') {
        if (source[*index] == '\n') {
            lexer->line++;
            lexer->index++;

            lexer->column = 1;
        }
        else
            k_advance_lexer(env);
    }

    return;
}

/*
 *    Gets a tokenable from its type.
 *
 *    @param k_token_type_e type    The type to get the tokenable from.
 * 
 *    @return k_tokenable_t *   The tokenable.
 */
k_tokenable_t *k_get_tokenable(k_token_type_e type) {
    unsigned long i = 0;

    for (i = 0; i < ARRAY_SIZE(_tokenables); i++) {
        if (_tokenables[i].type == type)
            return &_tokenables[i];
    }

    return (k_tokenable_t*)0x0;
}

/*
 *    Returns the identifier string.
 *
 *    @param const char *source    The source to get the identifier from.
 *    @param unsigned long index   The index to start at.
 *
 *    @return char *   The identifier string.
 */
char *k_get_identifier(const char *source, unsigned long index) {
    static char identifier[256];
    unsigned long i = 0;

    memset(identifier, 0, 256);

    for (i = 0; i < 256; i++) {
        if (source[index] == '\0' || source[index] == ' ' || source[index] == '\t' || source[index] == '\r' || source[index] == '\n')
            break;

        identifier[i] = source[index];
        index++;
    }

    return identifier;
}

/*
 *    Checks if a token string matches another string.
 *
 *    @param k_token_t *token    The token to check.
 *    @param const char *str     The string to check.
 *    @param const char *source  The source to check.
 * 
 *    @return unsigned long      1 if the token string does not match the string, 0 if it does.
 */
unsigned long k_token_string_matches(k_token_t *token, const char *str, const char *source) {
    unsigned long i = 0;

    for (i = 0; i < strlen(str); i++) {
        if (source[token->index + i] != str[i])
            return 1;
    }

    return 0;
}

/*
 *    Deduces the type of a token.
 *
 *    @param k_env_t    *env       The environment to deduce the token type in.
 *    @param const char *source    The source to deduce the token type in.
 *    
 *    @return k_tokenable_t *    The type index of the token.
 */
k_tokenable_t *k_deduce_token_type(k_env_t *env, const char *source) {
    unsigned long i = 0;

    for (i = 0; i < ARRAY_SIZE(_tokenables); i++) {
        if (_tokenables[i].chars == (const char*)0x0)
            continue;

        if (strchr(_tokenables[i].chars, source[env->lexer->index]) != (char*)0x0)
            return &_tokenables[i];
    }

    env->error(k_get_error(env, "Unknown token"));

    return k_get_tokenable(K_TOKEN_TYPE_UNKNOWN);
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
    static char    token[1024];
    unsigned long  i   = 0;
    unsigned long *idx = &env->lexer->index;
    k_tokenable_t *tok = env->cur_token->tokenable;

    memset(token, 0, sizeof(token));

    switch (tok->terminatable) {
        case K_TOKEN_TERMINATABLE_UNKNOWN:
            k_advance_lexer(env);

            return (const char*)0x0;
        case K_TOKEN_TERMINATABLE_SINGLE:
            token[0] = source[*idx];

            k_advance_lexer(env);
            break;
        case K_TOKEN_TERMINATABLE_MULTIPLE:
            do {
                token[i++] = source[*idx];

                k_advance_lexer(env);
            } while (strchr(tok->chars, source[*idx]) != (char*)0x0);
            break;
        case K_TOKEN_TERMINATABLE_REOCCUR:
            do {
                token[i++] = source[*idx];

                k_advance_lexer(env);
            } while (source[*idx] != tok->chars[0]);
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
    unsigned int   i         = 0;
    k_lexer_t     *lexer     = env->lexer;

    do {
        k_skip_whitespace(env, source);

        lexer->tokens = realloc(lexer->tokens, (i + 1) * sizeof(k_token_t));

        if (lexer->tokens == (k_token_t*)0x0)
            return;

        env->cur_token                = &lexer->tokens[i];

        lexer->tokens[i].line         = lexer->line;
        lexer->tokens[i].column       = lexer->column;
        lexer->tokens[i].index        = lexer->index;
        lexer->tokens[i].tokenable    = k_deduce_token_type(env, source);
        lexer->token_count            = ++i;

        k_parse_token(env, source);
    } while (env->cur_token->tokenable->type != K_TOKEN_TYPE_EOF);
}

/*
 *    Creates a KAPPA runtime.
 *
 *    @param k_env_t    *env       The environment to create the runtime in.
 *    @param const char *source    The source to create the runtime with.
 */
void k_create_runtime(k_env_t *env, const char *source) {
    if (env->runtime != (k_runtime_t*)0x0)
        free(env->runtime);

    env->runtime = malloc(sizeof(k_runtime_t));

    if (env->runtime == (k_runtime_t*)0x0)
        return;

    env->runtime->data           = (char*)0x0;
    env->runtime->ops            = (char*)0x0;
    env->runtime->mem            = (char*)0x0;
    env->runtime->size           = 0;
    env->runtime->function_table = (k_function_t*)0x0;

    env->functions               = (k_interp_func_t*)0x0;
    env->function_count          = 0;
    env->globals                 = (k_interp_var_t*)0x0;
    env->global_count            = 0;
    env->scope                   = (k_interp_scope_t*)0x0;
}

/*
 *    Performs lexical analysis on the token stream.
 *
 *    @param k_env_t    *env       The environment to perform lexical analysis in.
 *    @param const char *source    The source to perform lexical analysis on.
 */
void k_lexical_analysis(k_env_t *env, const char *source) {
    unsigned long  new_token_count = 0;
    k_token_t     *new_tokens = (k_token_t *)0x0;
    k_tokenable_t *tok        = (k_tokenable_t*)0x0;

    k_tokenize(env, source);

    for (unsigned long i = 0; i < env->lexer->token_count; i++) {
        tok = env->lexer->tokens[i].tokenable;

        if (tok->type == K_TOKEN_TYPE_IDENTIFIER) {
            /* Check if an identifier is a constant or a keyword.  */
            const char *id = k_get_identifier(source, env->lexer->tokens[i].index);

            if (strchr(k_get_tokenable(K_TOKEN_TYPE_NUMBER)->chars, id[0]) != (char*)0x0) {
                env->lexer->tokens[i].tokenable = k_get_tokenable(K_TOKEN_TYPE_NUMBER);
            } else {
                for (unsigned long j = 0; j < ARRAY_SIZE(_keywords); j++) {
                    if (strcmp(id, _keywords[j]) == 0) {
                        env->lexer->tokens[i].tokenable = k_get_tokenable(K_TOKEN_TYPE_KEYWORD);
                        break;
                    }
                }
            }
        }

        /* Remove comments from the token stream.  */
        if (tok->type != K_TOKEN_TYPE_COMMENT) {
            new_tokens = realloc(new_tokens, (new_token_count + 1) * sizeof(k_token_t));
            new_tokens[new_token_count] = env->lexer->tokens[i];
            new_token_count++;
        }
    }

    free(env->lexer->tokens);

    env->lexer->tokens      = new_tokens;
    env->lexer->token_count = new_token_count;
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