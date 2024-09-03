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
    _K_TOKEN_TYPE_START,
    _K_TOKEN_TYPE_LITERAL,
    _K_TOKEN_TYPE_ASSIGNMENT,
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
    char          *type;
    unsigned long  offset;
    unsigned long  size;
    char           flags;
} _k_variable_t;

typedef struct {
    char          *name;
    char          *source;
    unsigned long  size;

    _k_variable_t  *parameters;
    unsigned long   parameter_count;

    char            flags;
} _k_function_t;

#define _K_VARIABLE_FLAG_FUNC   (1 << 0)
#define _K_VARIABLE_FLAG_GLOBAL (1 << 1)
#define _K_VARIABLE_FLAG_FLOAT  (1 << 2)

typedef enum {
    _K_OP_NONE = 0,
    _K_OP_ADD,
    _K_OP_SUB,
    _K_OP_MUL,
    _K_OP_DIV,
    _K_OP_MOD,
    _K_OP_L,
    _K_OP_LE,
    _K_OP_G,
    _K_OP_GE,
    _K_OP_E,
    _K_OP_NE,
    _K_OP_AND,
    _K_OP_OR,
    _K_OP_NOT,
    _K_OP_NEG,
    _K_OP_ASSIGN,
    _K_OP_REF,
    _K_OP_DEREF,
    _K_OP_PTR_ASSIGN,
} _k_op_type_e;

typedef struct {
    const char   *operator;
    _k_op_type_e  type;
} _k_operator_t;

typedef struct {
    char         *id;
    unsigned long size;
    char          is_float;
} _k_type_t;

typedef struct {
    _k_op_type_e               type;
    _k_token_t                *lh;
    _k_type_t                  lh_type;
    _k_token_t                *rh;
    _k_type_t                  rh_type;
} _k_operation_t;

typedef union {
    unsigned long  u;
    long           s;
    double         f;
} _k_arithmetic_u;

typedef struct {
    char          *type;
    char           is_float;
    unsigned long  size;
} _k_local_t;

typedef struct {
    char           *mem;
    unsigned long  size;
    _k_function_t  *function_table;
    unsigned long  function_count;
    _k_operation_t *operations;
    unsigned long  operation_count;
    _k_variable_t  *locals;
    unsigned long  local_count;
    unsigned long  local_offset;
    _k_variable_t  *globals;
    unsigned long  global_count;
    unsigned long  global_offset;
    _k_operation_t *current_operation;
    _k_type_t       current_type;
    _k_type_t       running_type;
    char            typed;
} _k_runtime_t;

typedef struct {
    _k_lexer_t      *lexer;
    _k_runtime_t    *runtime;
    _k_token_t      *cur_token;
    _k_token_type_e  cur_type;
    _k_function_t   *cur_function;
    int           (*log)(const char *msg);
    char *kasm;
} k_env_t;

typedef enum {
    K_ERROR_NONE = 0,
    K_ERROR_UNEXPECTED_TOKEN,
    K_ERROR_UNDECLARED_VARIABLE,
    K_ERROR_UNDECLARED_TYPE,
    K_ERROR_UNDECLARED_IDENTIFIER,
    K_ERROR_INVALID_ENDEXPRESSION,
    K_ERROR_INVALID_DECLARATION,
    K_ERROR_JUNK_AFTER_DECLARATION,
    K_ERROR_EXPECTED_ASSIGNMENT,
    K_ERROR_EXPECTED_CONSTANT,
    K_ERROR_UNALLOWED_FLOAT,
    K_ERROR_NULL_PTR,
} k_build_error_t;

typedef struct {
    _k_token_type_e     type;
    k_build_error_t (*compile)(k_env_t *env);
} _k_grammar_t;

#endif /* _LIBK_TYPES_H  */