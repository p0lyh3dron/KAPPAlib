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
    _K_TOKEN_TYPE_MEMBER,
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

typedef struct _k_tree_s {
    _k_token_t *token;
    struct _k_tree_s **children;
    unsigned long      child_count;

    struct _k_tree_s *parent;
    struct _k_tree_s *guardian;
} _k_tree_t;

#endif /* _LIBK_TYPES_H  */