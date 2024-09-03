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
    {_K_TOKEN_TYPE_UNKNOWN,       (const char*)0x0,                                                   _K_TOKEN_TERMINATABLE_UNKNOWN},
    {_K_TOKEN_TYPE_EOF,           "\0",                                                               _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_IDENTIFIER,    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.", _K_TOKEN_TERMINATABLE_MULTIPLE},
    {_K_TOKEN_TYPE_NUMBER,        "0123456789.",                                                      _K_TOKEN_TERMINATABLE_MULTIPLE},
    {_K_TOKEN_TYPE_STRING,        "\"",                                                               _K_TOKEN_TERMINATABLE_REOCCUR},
    {_K_TOKEN_TYPE_OPERATOR,      "+-*/%&|!^~<>=",                                                    _K_TOKEN_TERMINATABLE_MULTIPLE},
    {_K_TOKEN_TYPE_COMMENT,       "$",                                                                _K_TOKEN_TERMINATABLE_REOCCUR},
    {_K_TOKEN_TYPE_NEWSTATEMENT,  "{",                                                                _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_ENDSTATEMENT,  "}",                                                                _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_NEWEXPRESSION, "(",                                                                _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_ENDEXPRESSION, ")",                                                                _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_NEWINDEX,      "[",                                                                _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_ENDINDEX,      "]",                                                                _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_DECLARATOR,    ":",                                                                _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_KEYWORD,       (const char*)0x0,                                                   _K_TOKEN_TERMINATABLE_UNKNOWN},
    {_K_TOKEN_TYPE_ENDLINE,       ";",                                                                _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_SEPARATOR,     ",",                                                                _K_TOKEN_TERMINATABLE_SINGLE},
    {_K_TOKEN_TYPE_ASSIGNMENT,    (const char*)0x0,                                                   _K_TOKEN_TERMINATABLE_UNKNOWN},
};

unsigned long _tokenables_length = 18;

const char *_keywords[] = {
    "if",
    "else",
    "while",
    "return",
    "do",
};

unsigned long _keywords_length = 5;

const _k_type_t _types[] = {
    {"s8",  1, 0},
    {"s16", 2, 0},
    {"s32", 4, 0},
    {"s64", 8, 0},
    {"u8",  1, 0},
    {"u16", 2, 0},
    {"u32", 4, 0},
    {"u64", 8, 0},
    {"f32", 4, 1},
    {"f64", 8, 1},
};

unsigned long _types_length = 10;