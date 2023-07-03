/*
 *    builtin.h    --    Header for builtin types and operators
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 2, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    Builtin types include integers, floats, strings, etc
 *    Operators include most of the C operators
 */
#ifndef K_BUILTIN_H
#define K_BUILTIN_H

#include "fastK.h"

extern const k_tokenable_t _tokenables[];

extern unsigned long _tokenables_length;

extern const char *_keywords[];

extern unsigned long _keywords_length;

extern const char *_types[];

extern unsigned long _types_length;

extern const unsigned long _type_lengths[];

#endif /* K_BUILTIN_H  */