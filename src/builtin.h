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
#ifndef _LIBK_BUILTIN_H
#define _LIBK_BUILTIN_H

#include "types.h"

extern const _k_tokenable_t _tokenables[];

extern unsigned long _tokenables_length;

extern const char *_keywords[];

extern unsigned long _keywords_length;

#endif /* _LIBK_BUILTIN_H  */