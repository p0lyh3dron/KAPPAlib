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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builtin.h"

/*
 *    Skips whitespace in a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to skip whitespace in.
 *    @param const char *source    The source to skip whitespace in.
 */
void _k_skip_whitespace(const char *source, int *idx, int *line, int *col) {
    while (source[*idx] == ' '  || source[*idx] == '\t' || 
           source[*idx] == '\r' || source[*idx] == '\n') {
        if (source[*idx] == '\n') {
            ++*line;
            *col = 0;
        }
        
        ++*idx;
        ++*col;
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
const _k_tokenable_t *_k_deduce_token_type(const char *source, int idx) {
    unsigned long i = 0;

    for (i = 0; i < _tokenables_length; i++) {
        if (_tokenables[i].chars == (const char*)0x0)
            continue;

        if (strchr(_tokenables[i].chars, source[idx]) != (char*)0x0)
            return &_tokenables[i];
    }

    fprintf(stderr, "Unknown token: %c\n", source[idx]);

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
char *_k_parse_token(const char *source, const _k_tokenable_t *tok, int *idx, int *line, int *col) {
    unsigned long  i          = 0;

    switch (tok->terminatable) {
        case _K_TOKEN_TERMINATABLE_UNKNOWN:
            *idx += 1;
            *col += 1;

            return (char*)0x0;
        case _K_TOKEN_TERMINATABLE_SINGLE:
            i++;

            *idx += 1;
            *col += 1;
            break;
        case _K_TOKEN_TERMINATABLE_MULTIPLE:
            do {
                i++;

                *idx += 1;
                *col += 1;
            } while (strchr(tok->chars, source[*idx]) != (char*)0x0);
            break;
        case _K_TOKEN_TERMINATABLE_REOCCUR:
            do {
                i++;

                if (source[*idx] == '\n') {
                    *line += 1;
                    *col = 0;
                }

                *idx += 1;
                *col += 1;
            } while (source[*idx] != tok->chars[0]);
            *idx += 1;
            *col += 1;
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
_k_token_t *_k_tokenize(const char *source) {
    int            idx       = 0;
    int            line      = 1;
    int            col       = 1;
    int            ti        = 0;

    _k_token_t   *tokens    = (_k_token_t*)0x0;

    do {
        _k_skip_whitespace(source, &idx, &line, &col);

        tokens = realloc(tokens, (ti + 1) * sizeof(_k_token_t));

        if (tokens == (_k_token_t*)0x0) return (_k_token_t*)0x0;

        tokens[ti].line         = line;
        tokens[ti].column       = col;
        tokens[ti].index        = idx;
        tokens[ti].tokenable    = _k_deduce_token_type(source, idx);
        tokens[ti].str          = _k_parse_token(source, tokens[ti].tokenable, &idx, &line, &col);
    } while (tokens[ti++].tokenable->type != _K_TOKEN_TYPE_EOF);

    return tokens;
}

/*
 *    Performs lexical analysis on the token stream.
 *
 *    @param k_env_t    *env       The environment to perform lexical analysis in.
 *    @param const char *source    The source to perform lexical analysis on.
 */
_k_token_t *_k_lexical_analysis(const char *source) {
    unsigned long        new_token_count  = 0;
    _k_token_t           *new_tokens      = (_k_token_t *)0x0;
    const _k_tokenable_t **tok            = (const _k_tokenable_t**)0x0;

    _k_token_t *tokens = _k_tokenize(source);
    _k_token_t *cur = tokens;

    while (cur->tokenable->type != _K_TOKEN_TYPE_EOF) {
        tok = &cur->tokenable;

        if ((*tok)->type == _K_TOKEN_TYPE_IDENTIFIER) {
            /* Check if an identifier is a constant or a keyword.  */
            const char *id = cur->str;

            /* Numerical constant.  */
            if (strspn(id, _k_get_tokenable(_K_TOKEN_TYPE_NUMBER)->chars) == strlen(id)) {
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

        if ((*tok)->type == _K_TOKEN_TYPE_OPERATOR) {
            if (strcmp(cur->str, "=") == 0) {
                *tok = _k_get_tokenable(_K_TOKEN_TYPE_ASSIGNMENT);
            }
        }

        /* Remove comments from the token stream.  */
        if ((*tok)->type != _K_TOKEN_TYPE_COMMENT) {
            new_tokens = realloc(new_tokens, (new_token_count + 1) * sizeof(_k_token_t));
            new_tokens[new_token_count] = *cur;
            new_token_count++;
        }

        cur++;
    }

    new_tokens = realloc(new_tokens, (new_token_count + 1) * sizeof(_k_token_t));
    new_tokens[new_token_count] = *cur;
    new_token_count++;

    free(tokens);

    tokens = new_tokens;

    return tokens;
}