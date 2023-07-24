/*
 *    builtin.c    --    Definitions for builtin types and operators
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 2, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    Defines the builtin types and operators for KAPPA.
 */
#include "builtin.h"

#include "util.h"

const _k_tokenable_t _tokenables[] = {
    {_K_TOKEN_TYPE_UNKNOWN,       (const char*)0x0,                                                  _K_TOKEN_TERMINATABLE_UNKNOWN},
    {_K_TOKEN_TYPE_EOF,           "\0",                                                              _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_IDENTIFIER,    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", _K_TOKEN_TERMINATABLE_MULTIPLE},
    {_K_TOKEN_TYPE_NUMBER,        "0123456789.",                                                      _K_TOKEN_TERMINATABLE_MULTIPLE},
    {_K_TOKEN_TYPE_STRING,        "\"",                                                              _K_TOKEN_TERMINATABLE_REOCCUR},
    {_K_TOKEN_TYPE_OPERATOR,      "+-*/%&|!^~<>=",                                                   _K_TOKEN_TERMINATABLE_MULTIPLE},
    {_K_TOKEN_TYPE_COMMENT,       "$",                                                               _K_TOKEN_TERMINATABLE_REOCCUR},
    {_K_TOKEN_TYPE_NEWSTATEMENT,  "{",                                                               _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_ENDSTATEMENT,  "}",                                                               _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_NEWEXPRESSION, "(",                                                               _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_ENDEXPRESSION, ")",                                                               _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_NEWINDEX,      "[",                                                               _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_ENDINDEX,      "]",                                                               _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_DECLARATOR,    ":",                                                               _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_KEYWORD,       (const char*)0x0,                                                  _K_TOKEN_TERMINATABLE_UNKNOWN},
    {_K_TOKEN_TYPE_ENDLINE,       ";",                                                               _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_SEPARATOR,     ",",                                                               _K_TOKEN_TERMINATABLE_SINGLE},
};

unsigned long _tokenables_length = 17;

const char *_keywords[] = {
    "if",
    "else",
    "while",
    "return",
};

unsigned long _keywords_length = 4;

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

unsigned long _types_length = 10;

const unsigned long _type_lengths[] = {
    1,
    2,
    4,
    8,
    1,
    2,
    4,
    8,
    4,
    8,
};