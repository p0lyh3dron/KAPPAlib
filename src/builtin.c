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