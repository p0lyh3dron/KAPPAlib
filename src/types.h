/*
 *    types.h    --    Header for KAPPA types
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 9, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 * 
 *    This file declares the types used in KAPPA.
 */
#ifndef _LIBK_TYPES_H
#define _LIBK_TYPES_H

typedef enum {
    _K_TOKEN_TYPE_UNKNOWN = 0,
    _K_TOKEN_TYPE_EOF,
    _K_TOKEN_TYPE_IDENTIFIER,
    _K_TOKEN_TYPE_NUMBER,
    _K_TOKEN_TYPE_STRING,
    _K_TOKEN_TYPE_OPERATOR,
    _K_TOKEN_TYPE_COMMENT,
    _K_TOKEN_TYPE_NEWSTATEMENT,
    _K_TOKEN_TYPE_ENDSTATEMENT,
    _K_TOKEN_TYPE_NEWEXPRESSION,
    _K_TOKEN_TYPE_ENDEXPRESSION,
    _K_TOKEN_TYPE_NEWINDEX,
    _K_TOKEN_TYPE_ENDINDEX,
    _K_TOKEN_TYPE_DECLARATOR,
    _K_TOKEN_TYPE_KEYWORD,
    _K_TOKEN_TYPE_ENDLINE,
    _K_TOKEN_TYPE_SEPARATOR,
} _k_token_type_e;

typedef enum {
    _K_TOKEN_TERMINATABLE_UNKNOWN = 0,
    _K_TOKEN_TERMINATABLE_SINGLE,
    _K_TOKEN_TERMINATABLE_MULTIPLE,
    _K_TOKEN_TERMINATABLE_REOCCUR,
} _k_token_terminatable_e;

typedef struct {
    _k_token_type_e         type;
    const char            *chars;
    _k_token_terminatable_e terminatable;
} _k_tokenable_t;

typedef struct {
    const _k_tokenable_t  *tokenable;

    unsigned long          line;
    unsigned long          column;
    unsigned long          index;
    char                  *str;
} _k_token_t;

typedef struct {
    _k_token_t *tokens;
    unsigned long token_count;
    unsigned long line;
    unsigned long column;
    unsigned long index;
} _k_lexer_t;

typedef struct {
    char          *name;
    char          *source;
    unsigned long  size;
} _k_function_t;

typedef struct {
    char          *data;
    char          *ops;
    char          *mem;
    unsigned long  size;
    _k_function_t  *function_table;
    unsigned long  function_count;
} _k_runtime_t;

typedef struct {
    _k_lexer_t      *lexer;
    _k_runtime_t    *runtime;
    _k_token_t      *cur_token;
    _k_token_type_e  cur_type;
    _k_function_t   *cur_function;
    int           (*log)(const char *msg);
} k_env_t;

typedef enum {
    K_ERROR_NONE = 0,
    K_ERROR_UNEXPECTED_TOKEN,
} k_compile_error_t;

#endif /* _LIBK_TYPES_H  */