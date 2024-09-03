/*
 *    libk_compile.c    --    Source for KAPPA compilation
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on July 1, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    The KAPPA compiler is contained in this file, the stage
 *    after the parsing and lexing of the KAPPA source code.
 */
#include "libk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "tree.h"

#include "builtin.h"
#include "util.h"

#include "libk_assemble.h"
#include "libk_operator.h"
#include "libk_parse.h"

k_build_error_t _err = K_ERROR_NONE;

char _k_get_prec(char *op) {
    if (strcmp(op, ",") == 0)  return 1;
    if (strcmp(op, "=") == 0)  return 2;
    if (strcmp(op, "<") == 0)  return 3; if (strcmp(op, ">") == 0)  return 3;
    if (strcmp(op, "<=") == 0) return 3; if (strcmp(op, ">=") == 0) return 3; if (strcmp(op, "==") == 0) return 3;
    if (strcmp(op, "+") == 0)  return 4; if (strcmp(op, "-") == 0)  return 4;
    if (strcmp(op, "*") == 0)  return 5; if (strcmp(op, "/") == 0)  return 5; 
    if (strcmp(op, "^") == 0)  return 6;

    return 0;
}

int _k_compile_bin_op(_k_token_t *token, int *r) {
    if (strcmp(token->str, "<") == 0)  { printf("\tlesrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; return 0; }
    if (strcmp(token->str, ">") == 0)  { printf("\tgrerr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; return 0; }
    if (strcmp(token->str, "<=") == 0) { printf("\tleqrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; return 0; }
    if (strcmp(token->str, ">=") == 0) { printf("\tgeqrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; return 0; }
    if (strcmp(token->str, "==") == 0) { printf("\tequrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; return 0; }
    if (strcmp(token->str, "+") == 0)  { printf("\taddrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; return 0; }
    if (strcmp(token->str, "-") == 0)  { printf("\tsubrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; return 0; }
    if (strcmp(token->str, "*") == 0)  { printf("\tmulrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; return 0; }
    if (strcmp(token->str, "/") == 0)  { printf("\tdivrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; return 0; }
    if (strcmp(token->str, ",") == 0)  { printf("\tpushr: r%d\n", *r); --*r; return 0; }

    return 0;
}

int _k_graph_edge(_k_token_t *it, _k_token_t *ft, _k_token_type_e itt, _k_token_type_e ftt) {
    return it->tokenable->type == itt && ft->tokenable->type == ftt;
}

_k_token_t *_k_expression(k_env_t *env, _k_token_t *token) {
    _k_token_t      stack[1024] = {0};
    _k_token_t     *stackp      = stack;
    int             last        = -1;
    int             r           = -1;
    int             line        = -1;

    do {
        if (stackp > stack) last = (stackp-1)->index;
        /* Expression graph edges, and their associated operations.  */

        /* Declarator -> Identifier.  */
        if (_k_graph_edge(stackp - 2, stackp - 1, _K_TOKEN_TYPE_DECLARATOR, _K_TOKEN_TYPE_IDENTIFIER)) {
            if (_k_graph_edge(stackp - 1, token, _K_TOKEN_TYPE_IDENTIFIER, _K_TOKEN_TYPE_NEWEXPRESSION)) {
                if ((stackp - 3)->tokenable->type != _K_TOKEN_TYPE_IDENTIFIER) return (_k_token_t*)0x0;

                printf("%s: \n", (stackp-1)->str);

                /* Pop the declarator and identifier off the stack.  */
                stackp -= 3;

                continue;
            }

            else {
                /* Error if token preceding declarator is not an identifier.  */
                if ((stackp - 3)->tokenable->type != _K_TOKEN_TYPE_IDENTIFIER) return (_k_token_t*)0x0;

                printf("\tnewsv: %s %s\n", (stackp-3)->str, (stackp-1)->str);

                /* Remove the top two elements of stack, and push identifier back on.  */
                *(stackp-3) = *(stackp-1);

                stackp -= 2;
            }
        }

        /* Identifier -> ).  */
        /* Identifier -> Operator.  */
        /* Identifier -> ,.  */
        else if (_k_graph_edge(stackp - 1, token, _K_TOKEN_TYPE_IDENTIFIER, _K_TOKEN_TYPE_ENDEXPRESSION) ||
                 _k_graph_edge(stackp - 1, token, _K_TOKEN_TYPE_IDENTIFIER, _K_TOKEN_TYPE_OPERATOR)      ||
                 _k_graph_edge(stackp - 1, token, _K_TOKEN_TYPE_IDENTIFIER, _K_TOKEN_TYPE_SEPARATOR)) {
            printf("\tloadr: r%d, %s\n", ++r, (stackp-1)->str);

            --stackp;
        }

        /* Number -> ).  */
        /* Number -> Operator.  */
        /* Number -> ,.  */
        else if (_k_graph_edge(stackp - 1, token, _K_TOKEN_TYPE_NUMBER, _K_TOKEN_TYPE_ENDEXPRESSION) ||
                 _k_graph_edge(stackp - 1, token, _K_TOKEN_TYPE_NUMBER, _K_TOKEN_TYPE_OPERATOR)      ||
                 _k_graph_edge(stackp - 1, token, _K_TOKEN_TYPE_NUMBER, _K_TOKEN_TYPE_SEPARATOR)) {
            printf("\tmovrn: r%d, %s\n", ++r, (stackp-1)->str);

            --stackp;
        }

        /* Standalone states.  */

        /* End of nested expression.  */
        if (token->tokenable->type == _K_TOKEN_TYPE_ENDEXPRESSION) {
            /* Pop operators off the stack.  */
            while (stackp > stack && (--stackp)->tokenable->type != _K_TOKEN_TYPE_NEWEXPRESSION) {
                stackp -= _k_compile_bin_op(stackp, &r);
            }

            /* Mismatched parenteses.  */
            if (stackp < stack) return (_k_token_t *)0x0;

            /* Delayed check of graph edge for Identifier -> (.  */
            if (_k_graph_edge(stackp - 1, stackp, _K_TOKEN_TYPE_IDENTIFIER, _K_TOKEN_TYPE_NEWEXPRESSION)) {
                /* Push an argument if we didnt void out the call.  */
                if (!_k_graph_edge(token - 1, token, _K_TOKEN_TYPE_NEWEXPRESSION, _K_TOKEN_TYPE_ENDEXPRESSION)) {
                    printf("\tpushr: r%d\n", r--);
                }

                /* Call the function and pop the function name off.  */
                printf("\tcallf: %s\n", (--stackp)->str);

                /* Return register into working stack.  */
                printf("\tmovrr: r%d, rr\n", ++r);
            }
        }

        /* Operations.  */
        if (token->tokenable->type == _K_TOKEN_TYPE_OPERATOR || 
            token->tokenable->type == _K_TOKEN_TYPE_SEPARATOR) {
            /* Pop precedent operators off stack.  */
            while (stackp > stack && _k_get_prec(token->str) <= _k_get_prec((stackp-1)->str)) {
                stackp -= _k_compile_bin_op(--stackp, &r);
            }
        }

        /* Cases in which we add the token to the stack.  */
        if      (token->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER)    *stackp++ = *token;
        else if (token->tokenable->type == _K_TOKEN_TYPE_NUMBER)        *stackp++ = *token;
        else if (token->tokenable->type == _K_TOKEN_TYPE_NEWEXPRESSION) *stackp++ = *token;
        else if (token->tokenable->type == _K_TOKEN_TYPE_ASSIGNMENT)    *stackp++ = *token;
        else if (token->tokenable->type == _K_TOKEN_TYPE_DECLARATOR)    *stackp++ = *token;
        else if (token->tokenable->type == _K_TOKEN_TYPE_OPERATOR)      *stackp++ = *token;
        else if (token->tokenable->type == _K_TOKEN_TYPE_SEPARATOR)     *stackp++ = *token;
    } while (last != (stackp-1)->index && token++);

    if (stackp > stack && (stackp-1)->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER) {
        printf("\tloadr: r%d, %s\n", ++r, (stackp-1)->str);

        stackp--;
    }

    if (stackp > stack && (stackp-1)->tokenable->type == _K_TOKEN_TYPE_NUMBER) {
        printf("\tmovrn: r%d, %s\n", ++r, (stackp-1)->str);

        stackp--;
    }

    do {
        stackp -= _k_compile_bin_op(--stackp, &r);
    } while (stackp >= stack);

    return token;
}

_k_token_t *_k_statement(k_env_t *env, _k_token_t *token) {
    _k_token_t  stack[1024];
    _k_token_t *stackp = stack;
    int         s      = 0;

    do {
        if (token->tokenable->type == _K_TOKEN_TYPE_KEYWORD) {
            if (strcmp(token->str, "return") == 0) {
                token = _k_expression(env, ++token);

                printf("\tmovrr: rr, r0\n\tleave:\n");

                if (strcmp((stackp - 1)->str, "if") == 0) {
                    printf("S%d: \n", s++);

                    stackp--;
                }

                if (strcmp((stackp - 1)->str, "while") == 0) {
                    printf("\tjmpal: S%d\nS%d: \n", s - 2, s++);

                    stackp--;
                }
            }

            else if (strcmp(token->str, "if") == 0) {
                *stackp++ = *token;

                token = _k_expression(env, ++token);

                printf("\tcmprd: r0, 0\n\tjmpeq: S%d\n", s);

                if (strcmp(token->str, "do") != 0) return (_k_token_t *)0x0;
            }

            else if (strcmp(token->str, "while") == 0) {
                *stackp++ = *token;

                printf("S%d: \n", s++);

                token = _k_expression(env, ++token);

                printf("\tcmprd: r0, 0\n\tjmpeq: S%d\n", s);

                if (strcmp(token->str, "do") != 0) return (_k_token_t *)0x0;
            }
        }

        else if (token->tokenable->type == _K_TOKEN_TYPE_NEWSTATEMENT) {
            *stackp++ = *token;
        }

        else if (token->tokenable->type == _K_TOKEN_TYPE_ENDSTATEMENT) {
            if (stackp == stack) return token;

            if (strcmp((stackp - 2)->str, "if") == 0) {
                printf("S%d: \n", s++);

                stackp -= 2;
            }

            if (strcmp((stackp - 2)->str, "while") == 0) {
                printf("\tjmpal: S%d\nS%d: \n", s - 2, s++);

                stackp -= 2;
            }
        }

        /* Expression.  */
        else {
            token = _k_expression(env, token);

            if (strcmp((stackp - 1)->str, "if") == 0) {
                printf("S%d: \n", s++);

                stackp--;
            }

            if (strcmp((stackp - 1)->str, "while") == 0) {
                printf("\tjmpal: S%d\nS%d: \n", s - 2, s++);

                stackp--;
            }
        }
    } while (stackp > stack);

    return token;
}

typedef struct _k_tree_s {
    _k_token_t *token;
    struct _k_tree_s **children;
    unsigned long      child_count;

    struct _k_tree_s *parent;
    struct _k_tree_s *guardian;
} _k_tree_t;

/*
 *    Places a token in a tree.
 *
 *    @param _k_tree_t **root    The root of the tree.
 *    @param _k_token_t *token   The token to place.
 * 
 *    @return _k_tree_t *    The node the token was placed in.
 */
_k_tree_t *_k_place_token(_k_tree_t **root, _k_token_t *token) {
    if (*root == (_k_tree_t *)0x0) *root = (_k_tree_t *)malloc(sizeof(_k_tree_t));

    (*root)->token       = token;
    (*root)->children    = (_k_tree_t**)0x0;
    (*root)->child_count = 0;

    (*root)->parent   = (_k_tree_t*)0x0;
    (*root)->guardian = (_k_tree_t*)0x0;

    return *root;
}

/*
 *    Places a child in a tree.
 *
 *    @param _k_tree_t  *root    The root of the tree.
 *    @param _k_token_t *token   The token to place.
 * 
 *    @return _k_tree_t *    The node the token was placed in.
 */
_k_tree_t *_k_place_child(_k_tree_t *root, _k_token_t *token) {
    root->children = (_k_tree_t**)realloc(root->children, (root->child_count + 1) * sizeof(_k_tree_t*));

    root->children[root->child_count] = (_k_tree_t*)malloc(sizeof(_k_tree_t));

    root->children[root->child_count]->token       = token;
    root->children[root->child_count]->children    = (_k_tree_t**)0x0;
    root->children[root->child_count]->child_count = 0;

    root->children[root->child_count]->parent   = root;
    root->children[root->child_count]->guardian = (_k_tree_t*)0x0;

    return root->children[root->child_count++];
}

/*
 *    Swaps a parent with its first child.
 *
 *    @param _k_tree_t *parent    The parent to swap.
 */
void _k_swap_parent(_k_tree_t *parent) {
    _k_token_t *temp = parent->children[0]->token;

    parent->children[0]->token = parent->token;
    parent->token              = temp;
}

void _k_tree_print(_k_tree_t *root, int depth, _k_tree_t *bold) {
    if (root == (_k_tree_t*)0x0) return;

    if (root->child_count == 2) _k_tree_print(root->children[1], depth + 1, bold);

    for (int i = 0; i < depth; i++) printf("    ");
    if (root == bold) printf("\e[31m\033[1m");
    printf("%s\n", root->token->str);
    if (root == bold) printf("\e[0m\033[0m");

    if (root->child_count >= 1) _k_tree_print(root->children[0], depth + 1, bold);
}

void _k_free_tree(_k_tree_t *root) {
    if (root == (_k_tree_t*)0x0) return;

    for (unsigned long i = 0; i < root->child_count; i++) _k_free_tree(root->children[i]);

    free(root->children);
    free(root);
}

/*
 *    Compiles a tree.
 *
 *    @param k_env_t    *env       The environment to compile the tree in.
 *    @param _k_tree_t *root       The root of the tree.
 */
void _k_compile_tree(k_env_t *env, _k_tree_t *root, int *r, int *s) {
    if (root == (_k_tree_t*)0x0) return;

    switch (root->token->tokenable->type) {
        case _K_TOKEN_TYPE_DECLARATOR: {
            char *type = root->children[0]->token->str;
            
            if (root->children[1]->child_count > 0 && root->children[1]->children[0]->token->tokenable->type == _K_TOKEN_TYPE_NEWEXPRESSION) {
                char *name = root->children[1]->token->str;

                printf("\n%s: \n", name);

                for (unsigned long i = 0; i < root->children[1]->children[0]->child_count; i++) {
                    printf("\tpoprr: r%d\n", ++*r);
                }

                for (unsigned long i = 0; i < root->children[1]->children[0]->child_count; i++) {
                    _k_compile_tree(env, root->children[1]->children[0]->children[i], r, s);
                    printf("\tsaver: %s r%d\n", root->children[1]->children[0]->children[i]->children[1]->token->str, (*r)--);
                }

                for (unsigned long i = 0; i < root->children[1]->children[1]->child_count; i++) {
                    _k_compile_tree(env, root->children[1]->children[1]->children[i], r, s);
                }

                break;
            }

            if (root->child_count > 1 && root->children[1]->token->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER) {
                char *name = root->children[1]->token->str;

                printf("\tnewsv: %s %s\n", type, name);

                break;
            }

            if (root->child_count > 1 && (root->children[1]->token->tokenable->type == _K_TOKEN_TYPE_OPERATOR || root->children[1]->token->tokenable->type == _K_TOKEN_TYPE_ASSIGNMENT)) {
                char *name = root->children[1]->children[0]->token->str;

                printf("\tnewsv: %s %s\n", type, name);

                _k_compile_tree(env, root->children[1], r, s);

                break;
            }

            break;
        }

        case _K_TOKEN_TYPE_IDENTIFIER: {
            if (root->child_count > 0 && root->children[0]->token->tokenable->type == _K_TOKEN_TYPE_NEWEXPRESSION) {
                for (unsigned long i = 0; i < root->children[0]->child_count; i++) {
                    _k_compile_tree(env, root->children[0]->children[i], r, s);
                    printf("\tpushr: r%d\n", (*r)--);
                }

                printf("\tcallf: %s\n", root->token->str);
                printf("\tmovrr: r%d r0\n", ++*r);

                break;
            }

            printf("\tloadr: r%d %s\n", ++*r, root->token->str);

            break;
        }

        case _K_TOKEN_TYPE_NUMBER: {
            printf("\tmovrn: r%d %s\n", ++*r, root->token->str);

            break;
        }

        case _K_TOKEN_TYPE_ASSIGNMENT: {
            _k_compile_tree(env, root->children[1], r, s);

            printf("\tsaver: %s r%d\n", root->children[0]->token->str, (*r)--);

            break;
        }

        case _K_TOKEN_TYPE_OPERATOR: {
            _k_compile_tree(env, root->children[0], r, s);
            _k_compile_tree(env, root->children[1], r, s);
            _k_compile_bin_op(root->token, r);

            break;
        }

        case _K_TOKEN_TYPE_NEWEXPRESSION: {
            for (unsigned long i = 0; i < root->child_count; i++) {
                _k_compile_tree(env, root->children[i], r, s);
            }

            break;
        }

        case _K_TOKEN_TYPE_NEWSTATEMENT: {
            for (unsigned long i = 0; i < root->child_count; i++) {
                _k_compile_tree(env, root->children[i], r, s);
            }

            break;
        }

        case _K_TOKEN_TYPE_KEYWORD: {
            if (strcmp(root->token->str, "return") == 0) {
                _k_compile_tree(env, root->children[0], r, s);

                printf("\tmovrr: r0 r%d\n\tleave: \n", (*r)--);

                break;
            }

            if (strcmp(root->token->str, "if") == 0) {
                _k_compile_tree(env, root->children[0], r, s);

                printf("\tcmprd: r%d 0\n\tjmpeq: S%d\n", (*r)--, ++*s);

                _k_compile_tree(env, root->children[1], r, s);

                printf("S%d: \n", *s);

                break;
            }

            if (strcmp(root->token->str, "while") == 0) {
                printf("S%d: \n", ++*s);

                _k_compile_tree(env, root->children[0], r, s);

                printf("\tcmprd: r%d 0\n\tjmpeq: S%d\n", (*r)--, ++*s);

                _k_compile_tree(env, root->children[1], r, s);

                printf("\tjmpal: S%d\nS%d: \n", *s - 1, *s);

                break;
            }

            break;
        
        }
    }
}

/*
 *    Parses a KAPPA source file into a tree.
 *
 *    @param k_env_t    *env       The environment to parse the source in.
 *    @param const char *source    The source to parse.
 *    @param _k_token_t *token     The token to parse.
 *
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_tree(k_env_t *env, const char *source, _k_token_t *token) {
    _k_tree_t *roots = (_k_tree_t*)0x0;
    _k_tree_t *root  = roots;
    _k_tree_t *node  = (_k_tree_t*)0x0;
    int        s     = -1;

    node = _k_place_token(&root, token++);

    do {
        //printf("Token: %s\n", token->str);
        //printf("----------\n");
        //_k_tree_print(root, 0, node);
        //printf("----------\n");
        switch (token->tokenable->type) {
            case _K_TOKEN_TYPE_IDENTIFIER:
            case _K_TOKEN_TYPE_NUMBER: {
                if (node->token->tokenable->type == _K_TOKEN_TYPE_DECLARATOR) {
                    node = _k_place_child(node, token);

                    break;
                }

                node = _k_place_child(node, token);

                break;
            }
            case _K_TOKEN_TYPE_DECLARATOR: {
                node = _k_place_child(node, token)->parent;

                _k_swap_parent(node);

                break;            
            }
            case _K_TOKEN_TYPE_NEWEXPRESSION: {
                node = _k_place_child(node, token);

                break;
            
            }
            case _K_TOKEN_TYPE_ENDEXPRESSION: {
                while (node->token->tokenable->type != _K_TOKEN_TYPE_NEWEXPRESSION) {
                    node = node->parent;
                }

                /* Arguments.  */
                if (node->parent->token->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER) {
                    node = node->parent;
                }

                break;
            }
            case _K_TOKEN_TYPE_NEWSTATEMENT: {
                node = _k_place_child(node, token);

                break;
            }
            case _K_TOKEN_TYPE_SEPARATOR: {
                while (node->token->tokenable->type != _K_TOKEN_TYPE_NEWEXPRESSION) {
                    node = node->parent;
                }

                break;
            }
            case _K_TOKEN_TYPE_ENDSTATEMENT: {
                while (node->token->tokenable->type != _K_TOKEN_TYPE_NEWSTATEMENT) {
                    node = node->parent;
                }

                /* Function body.  */
                if (node->parent->token->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER) {
                    node = node->parent;
                }

                if (node->parent->token->tokenable->type == _K_TOKEN_TYPE_KEYWORD) {
                    node = node->parent;
                }

                break;
            }
            case _K_TOKEN_TYPE_ENDLINE: {
                /* Find next scope.  */
                while (node->token->tokenable->type != _K_TOKEN_TYPE_NEWSTATEMENT && node->parent != (_k_tree_t*)0x0) {
                    node = node->parent;
                }

                if (node->parent == (_k_tree_t*)0x0) {
                    int r = 0;

                    _k_compile_tree(env, root, &r, &s);

                    //_k_free_tree(root);

                    node = _k_place_token(&root, ++token);
                }

                break;
            }
            case _K_TOKEN_TYPE_KEYWORD: {
                if (strcmp(token->str, "do") == 0) {
                    while (strcmp(node->token->str, "if") != 0 && strcmp(node->token->str, "while") != 0) {
                        node = node->parent;
                    }

                    break;
                }

                node = _k_place_child(node, token);

                break;
            }
            case _K_TOKEN_TYPE_ASSIGNMENT:
            case _K_TOKEN_TYPE_OPERATOR: {
                do {
                    node = node->parent;
                } while (_k_get_prec(token->str) < _k_get_prec(node->token->str));

                _k_tree_t *temp = node->children[node->child_count - 1];

                node->children[node->child_count - 1] = (_k_tree_t*)0x0;
                node->child_count--;

                node = _k_place_child(node, token);
                node = _k_place_child(node, temp->token)->parent;
                node->children[node->child_count - 1] = temp;

                temp->parent = node;

                break;
            }
        }
    } while (token++->tokenable->type != _K_TOKEN_TYPE_EOF);
}

/*
 *    Compiles a KAPPA source file.
 *
 *    @param k_env_t    *env       The environment to compile the source in.
 *    @param const char *source    The source to compile.
 * 
 *    @return k_build_error_t    The error code.
 */
k_build_error_t _k_compile(k_env_t *env, const char *source) {
    _k_create_runtime(env, source);

    _k_token_t *token = &env->lexer->tokens[0];

    //for (unsigned long i = 0; i < env->lexer->token_count; i++) printf("[%d]: %s\n", i, env->lexer->tokens[i].str);

#if 0
    for (_k_token_t *token       = &env->lexer->tokens[0]; 
         token->tokenable->type != _K_TOKEN_TYPE_EOF; 
         token                   = _k_expression(env, token) + 1) {
    }
#endif

    _k_tree(env, source, token + 0);

    if (env->kasm != (char*)0x0) {
        printf("%s", env->kasm);

        free(env->kasm);
    }

    return _err;
}