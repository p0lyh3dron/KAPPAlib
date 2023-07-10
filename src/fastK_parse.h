/*
 *    fastK_parse.h    --    Header for KAPPA parsing
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 9, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This file declares the functions and data structures used to parse
 *    KAPPA files.
 */
#ifndef FASTK_PARSE_H
#define FASTK_PARSE_H

#include "fastK.h"

/*
 *    Advances the lexer to the next char.
 *
 *    @param k_env_t    *env       The environment to advance the lexer in.
 */
void k_advance_lexer(k_env_t *env);

/*
 *    Advances to the next token.
 *
 *    @param k_env_t    *env       The environment to advance the lexer in.
 */
void k_advance_token(k_env_t *env);

/*
 *    Reverts to the previous token.
 *
 *    @param k_env_t    *env       The environment to revert the lexer in.
 */
void k_revert_token(k_env_t *env);

/*
 *    Skips whitespace in a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to skip whitespace in.
 *    @param const char *source    The source to skip whitespace in.
 */
void k_skip_whitespace(k_env_t *env, const char *source);

/*
 *    Gets a tokenable from its type.
 *
 *    @param k_token_type_e type    The type to get the tokenable from.
 * 
 *    @return const k_tokenable_t *   The tokenable.
 */
const k_tokenable_t *k_get_tokenable(k_token_type_e type);

/*
 *    Checks if a token string matches another string.
 *
 *    @param k_token_t *token    The token to check.
 *    @param const char *str     The string to check.
 *    @param const char *source  The source to check.
 * 
 *    @return unsigned long      1 if the token string does not match the string, 0 if it does.
 */
unsigned long k_token_string_matches(k_token_t *token, const char *str, const char *source);

/*
 *    Deduces the type of a token.
 *
 *    @param k_env_t    *env       The environment to deduce the token type in.
 *    @param const char *source    The source to deduce the token type in.
 *    
 *    @return const k_tokenable_t *    The type index of the token.
 */
const k_tokenable_t *k_deduce_token_type(k_env_t *env, const char *source);

/*
 *    Parses a single token.
 *
 *    @param k_env_t    *env       The environment to parse the token in.
 *    @param const char *source    The source to parse the token in.
 * 
 *    @return const char *    The parsed token.
 */
const char *k_parse_token(k_env_t *env, const char *source);

/*
 *    Tokenizes a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to tokenize the source in.
 *    @param const char *source    The source to tokenize.
 */
void k_tokenize(k_env_t *env, const char *source);

/*
 *    Creates a KAPPA runtime.
 *
 *    @param k_env_t    *env       The environment to create the runtime in.
 *    @param const char *source    The source to create the runtime with.
 */
void k_create_runtime(k_env_t *env, const char *source);

/*
 *    Performs lexical analysis on the token stream.
 *
 *    @param k_env_t    *env       The environment to perform lexical analysis in.
 *    @param const char *source    The source to perform lexical analysis on.
 */
void k_lexical_analysis(k_env_t *env, const char *source);

#endif /* FASTK_PARSE_H  */