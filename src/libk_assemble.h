/*
 *    libk_compile.h    --    Header for KAPPA assembler
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on January 19, 2025
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This file declares the functions used to take compiled
 *    KAPPA code and assemble it to the target IR.
 */
#ifndef _LIBK_ASSEMBLE_H
#define _LIBK_ASSEMBLE_H

#include <stdio.h>

#include "types.h"

/*
 *    Compiles a tree.
 *
 *    @param k_env_t    *env       The environment to compile the tree in.
 *    @param _k_tree_t *root       The root of the tree.
 */
void _k_assemble_tree(_k_tree_t *root, int *r, int *s, FILE *out);

#endif /* _LIBK_ASSEMBLE_H  */