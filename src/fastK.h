/*
 *    fastK.h    --    header for fast KAPPA computation
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on February 23, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    KAPPA is a strongly-typed functional programming language
 *    designed to be similar to C, but with less boilerplate,
 *    and more powerful features, alongside being interpreted
 *    in addition to being compiled.
 * 
 *    The declarations for functions related to KAPPA parsing and
 *    interpretation are contained in this file.
 */
#ifndef FASTK_H
#define FASTK_H

typedef enum {
    K_TOKEN_TYPE_UNKNOWN = 0,
    K_TOKEN_TYPE_EOF,
    K_TOKEN_TYPE_IDENTIFIER,
    K_TOKEN_TYPE_NUMBER,
    K_TOKEN_TYPE_STRING,
    K_TOKEN_TYPE_OPERATOR,
    K_TOKEN_TYPE_COMMENT,
    K_TOKEN_TYPE_NEWSTATEMENT,
    K_TOKEN_TYPE_ENDSTATEMENT,
    K_TOKEN_TYPE_NEWEXPRESSION,
    K_TOKEN_TYPE_ENDEXPRESSION,
    K_TOKEN_TYPE_NEWINDEX,
    K_TOKEN_TYPE_ENDINDEX,
    K_TOKEN_TYPE_DECLARATOR,
    K_TOKEN_TYPE_KEYWORD,
    K_TOKEN_TYPE_ENDLINE,
    K_TOKEN_TYPE_SEPARATOR,
} k_token_type_e;

typedef enum {
    K_TOKEN_TERMINATABLE_UNKNOWN = 0,
    K_TOKEN_TERMINATABLE_SINGLE,
    K_TOKEN_TERMINATABLE_MULTIPLE,
    K_TOKEN_TERMINATABLE_REOCCUR,
} k_token_terminatable_e;

typedef struct {
    k_token_type_e         type;
    const char            *chars;
    k_token_terminatable_e terminatable;
} k_tokenable_t;

typedef struct {
    k_tokenable_t         *tokenable;

    unsigned long          line;
    unsigned long          column;
    unsigned long          index;
    unsigned long          length;
    char                  *str;
} k_token_t;

typedef struct {
    k_token_t *tokens;
    unsigned long token_count;
    unsigned long line;
    unsigned long column;
    unsigned long index;
} k_lexer_t;

typedef struct {
    char          *name;
    char          *source;
    unsigned long  size;
} k_function_t;

typedef struct {
    char          *data;
    char          *ops;
    char          *mem;
    unsigned long  size;
    k_function_t  *function_table;
    unsigned long  function_count;
} k_runtime_t;

typedef struct {
    k_lexer_t      *lexer;
    k_runtime_t    *runtime;
    k_token_t      *cur_token;
    k_token_type_e  cur_type;
    k_function_t   *cur_function;
    void          (*log)(const char *msg);
} k_env_t;

typedef enum {
    K_ERROR_NONE = 0,
    K_ERROR_UNEXPECTED_TOKEN,
} k_compile_error_t;

/*
 *    Creates a new KAPPA environment.
 *
 *    @return k_env_t *    A pointer to the new environment.
 */
k_env_t *k_new_env(void);

/*
 *    Sets the error handler for a KAPPA environment.
 *
 *    @param k_env_t *env                      The environment to set the error handler for.
 *    @param void (*error)(const char *msg)    The error handler.
 */
void k_set_log_handler(k_env_t *env, void (*error)(const char *msg));

/*
 *    Parses a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to parse the source in.
 *    @param const char *source    The source to parse.
 */
void k_parse(k_env_t *env, const char *source);

/*
 *    Destroys a KAPPA environment.
 *
 *    @param k_env_t *env    The environment to destroy.
 */
void k_destroy_env(k_env_t *env);

#endif /* FASTK_H  */