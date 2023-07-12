/*
 *    libk_parse.c    --    Source for KAPPA parsing
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 9, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This file defines the functions for the KAPPA parser.
 */
#include "libk_parse.h"

#include <stdlib.h>
#include <string.h>

#include "builtin.h"
#include "util.h"

/*
 *    Advances the lexer to the next char.
 *
 *    @param k_env_t    *env       The environment to advance the lexer in.
 */
void _k_advance_lexer(k_env_t *env) {
    env->lexer->index++;
    env->lexer->column++;
}

/*
 *    Advances to the next token.
 *
 *    @param k_env_t    *env       The environment to advance the lexer in.
 */
void _k_advance_token(k_env_t *env) {
    if (env->cur_token->tokenable->type == _K_TOKEN_TYPE_EOF)
        return;

    env->cur_token += 1;

    env->cur_type  = env->cur_token->tokenable->type;
}

/*
 *    Reverts to the previous token.
 *
 *    @param k_env_t    *env       The environment to revert the lexer in.
 */
void _k_revert_token(k_env_t *env) {
    env->cur_token -= 1;

    env->cur_type  = env->cur_token->tokenable->type;
}

/*
 *    Skips whitespace in a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to skip whitespace in.
 *    @param const char *source    The source to skip whitespace in.
 */
void _k_skip_whitespace(k_env_t *env, const char *source) {
    unsigned long *index = &env->lexer->index;
    _k_lexer_t     *lexer = env->lexer;

    while (source[*index] == ' '  || source[*index] == '\t' || 
           source[*index] == '\r' || source[*index] == '\n') {
        if (source[*index] == '\n') {
            lexer->line++;
            lexer->index++;

            lexer->column = 1;
        }
        else
            _k_advance_lexer(env);
    }

    return;
}

/*
 *    Gets a tokenable from its type.
 *
 *    @param _k_token_type_e type    The type to get the tokenable from.
 * 
 *    @return const _k_tokenable_t *   The tokenable.
 */
const _k_tokenable_t *_k_get_tokenable(_k_token_type_e type) {
    unsigned long i = 0;

    for (i = 0; i < _tokenables_length; i++) {
        if (_tokenables[i].type == type)
            return &_tokenables[i];
    }

    return (_k_tokenable_t*)0x0;
}

/*
 *    Checks if a token string matches another string.
 *
 *    @param _k_token_t *token    The token to check.
 *    @param const char *str     The string to check.
 *    @param const char *source  The source to check.
 * 
 *    @return unsigned long      1 if the token string does not match the string, 0 if it does.
 */
unsigned long _k_token_string_matches(_k_token_t *token, const char *str, const char *source) {
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
 *    @return const _k_tokenable_t *    The type index of the token.
 */
const _k_tokenable_t *_k_deduce_token_type(k_env_t *env, const char *source) {
    unsigned long i = 0;

    for (i = 0; i < _tokenables_length; i++) {
        if (_tokenables[i].chars == (const char*)0x0)
            continue;

        if (strchr(_tokenables[i].chars, source[env->lexer->index]) != (char*)0x0)
            return &_tokenables[i];
    }

    env->log(_k_get_error(env, "Unknown token"));

    return _k_get_tokenable(_K_TOKEN_TYPE_UNKNOWN);
}

/*
 *    Parses a single token.
 *
 *    @param k_env_t    *env       The environment to parse the token in.
 *    @param const char *source    The source to parse the token in.
 * 
 *    @return char *    The parsed token.
 */
char *_k_parse_token(k_env_t *env, const char *source) {
    unsigned long  i          = 0;
    unsigned long *idx        = &env->lexer->index;
    const _k_tokenable_t *tok = env->cur_token->tokenable;

    switch (tok->terminatable) {
        case _K_TOKEN_TERMINATABLE_UNKNOWN:
            _k_advance_lexer(env);

            return (char*)0x0;
        case _K_TOKEN_TERMINATABLE_SINGLE:
            i++;
            _k_advance_lexer(env);
            break;
        case _K_TOKEN_TERMINATABLE_MULTIPLE:
            do {
                i++;

                _k_advance_lexer(env);
            } while (strchr(tok->chars, source[*idx]) != (char*)0x0);
            break;
        case _K_TOKEN_TERMINATABLE_REOCCUR:
            do {
                i++;

                _k_advance_lexer(env);
            } while (source[*idx] != tok->chars[0]);
            _k_advance_lexer(env);
            break;
    }

    return strndup(&source[*idx - i], i);
}

/*
 *    Tokenizes a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to tokenize the source in.
 *    @param const char *source    The source to tokenize.
 */
void _k_tokenize(k_env_t *env, const char *source) {
    unsigned int   i         = 0;
    _k_lexer_t     *lexer     = env->lexer;

    do {
        _k_skip_whitespace(env, source);

        lexer->tokens = realloc(lexer->tokens, (i + 1) * sizeof(_k_token_t));

        if (lexer->tokens == (_k_token_t*)0x0)
            return;

        env->cur_token                = &lexer->tokens[i];

        lexer->tokens[i].line         = lexer->line;
        lexer->tokens[i].column       = lexer->column;
        lexer->tokens[i].index        = lexer->index;
        lexer->tokens[i].tokenable    = _k_deduce_token_type(env, source);
        lexer->tokens[i].str          = _k_parse_token(env, source);
        lexer->token_count            = ++i;
    } while (env->cur_token->tokenable->type != _K_TOKEN_TYPE_EOF);
}

/*
 *    Creates a KAPPA runtime.
 *
 *    @param k_env_t    *env       The environment to create the runtime in.
 *    @param const char *source    The source to create the runtime with.
 */
void _k_create_runtime(k_env_t *env, const char *source) {
    if (env->runtime != (_k_runtime_t*)0x0)
        free(env->runtime);

    env->runtime = malloc(sizeof(_k_runtime_t));

    if (env->runtime == (_k_runtime_t*)0x0)
        return;

    env->runtime->mem            = (char*)0x0;
    env->runtime->size           = 0;
    env->runtime->function_table = (_k_function_t*)0x0;
    env->runtime->function_count = 0;
    env->runtime->locals        = (_k_variable_t*)0x0;
    env->runtime->local_count   = 0;
    env->runtime->globals       = (_k_variable_t*)0x0;
    env->runtime->global_count  = 0;
}

/*
 *    Performs lexical analysis on the token stream.
 *
 *    @param k_env_t    *env       The environment to perform lexical analysis in.
 *    @param const char *source    The source to perform lexical analysis on.
 */
void _k_lexical_analysis(k_env_t *env, const char *source) {
    unsigned long        new_token_count  = 0;
    _k_token_t           *new_tokens      = (_k_token_t *)0x0;
    const _k_tokenable_t **tok            = (const _k_tokenable_t**)0x0;

    _k_tokenize(env, source);

    env->cur_token = &env->lexer->tokens[0];

    for (unsigned long i = 0; i < env->lexer->token_count; i++) {
        tok = &env->cur_token->tokenable;

        if ((*tok)->type == _K_TOKEN_TYPE_IDENTIFIER) {
            /* Check if an identifier is a constant or a keyword.  */
            const char *id = _k_get_token_str(env);

            /* Numerical constant.  */
            if (strchr(_k_get_tokenable(_K_TOKEN_TYPE_NUMBER)->chars, id[0]) != (char*)0x0) {
                *tok = _k_get_tokenable(_K_TOKEN_TYPE_NUMBER);
            } 
            
            /* Reserved keyword. */
            else {
                for (unsigned long j = 0; j < _keywords_length; j++) {
                    if (strcmp(id, _keywords[j]) == 0) {
                        *tok = _k_get_tokenable(_K_TOKEN_TYPE_KEYWORD);
                        break;
                    }
                }
            }
        }

        /* Remove comments from the token stream.  */
        if ((*tok)->type != _K_TOKEN_TYPE_COMMENT) {
            new_tokens = realloc(new_tokens, (new_token_count + 1) * sizeof(_k_token_t));
            new_tokens[new_token_count] = *env->cur_token;
            new_token_count++;
        }

        _k_advance_token(env);
    }

    free(env->lexer->tokens);

    env->lexer->tokens      = new_tokens;
    env->lexer->token_count = new_token_count;
}