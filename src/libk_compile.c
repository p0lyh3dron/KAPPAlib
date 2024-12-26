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

#include "builtin.h"

#include "libk_parse.h"

int _k_build_error = 0;

/*
 *    Gets the error code.
 *
 *    @return int    The error code.
 */
int _k_get_error_code() {
    return _k_build_error;
}

char _k_get_prec(char *op) {
    if (strcmp(op, ",") == 0)  return 1;
    if (strcmp(op, "=") == 0)  return 2;
    if (strcmp(op, "<") == 0)  return 3; if (strcmp(op, ">") == 0)  return 3;
    if (strcmp(op, "<=") == 0) return 3; if (strcmp(op, ">=") == 0) return 3; if (strcmp(op, "==") == 0) return 3;
    if (strcmp(op, "+") == 0)  return 4; if (strcmp(op, "-") == 0)  return 4;
    if (strcmp(op, "*") == 0)  return 5; if (strcmp(op, "/") == 0)  return 5; 
    if (strcmp(op, "^") == 0)  return 6;
    if (strcmp(op, ".") == 0)  return 7;

    return 0;
}

void _k_compile_bin_op(_k_token_t *token, int *r, FILE *out) {
    if (strcmp(token->str, "<") == 0)  { fprintf(out, "\tlesrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; }
    if (strcmp(token->str, ">") == 0)  { fprintf(out, "\tgrerr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; }
    if (strcmp(token->str, "<=") == 0) { fprintf(out, "\tleqrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; }
    if (strcmp(token->str, ">=") == 0) { fprintf(out, "\tgeqrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; }
    if (strcmp(token->str, "==") == 0) { fprintf(out, "\tequrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; }
    if (strcmp(token->str, "+") == 0)  { fprintf(out, "\taddrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; }
    if (strcmp(token->str, "-") == 0)  { fprintf(out, "\tsubrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; }
    if (strcmp(token->str, "*") == 0)  { fprintf(out, "\tmulrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; }
    if (strcmp(token->str, "/") == 0)  { fprintf(out, "\tdivrr: r%d r%d r%d\n", *r - 1, *r - 1, *r); --*r; }
    if (strcmp(token->str, ",") == 0)  { fprintf(out, "\tpushr: r%d\n", *r); --*r; }
}

void _k_compile_un_op(_k_token_t *token, int *r, FILE *out) {
    if (strcmp(token->str, "-") == 0) { fprintf(out, "\tnegrr: r%d r%d\n", *r, *r); }
    if (strcmp(token->str, "*") == 0) { fprintf(out, "\tderef: r%d r%d\n", *r, *r); }
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
void _k_swap_parent(_k_tree_t *node) {
    _k_tree_t *parent      = node->parent;
    _k_tree_t *grandparent = parent->parent;

    if (grandparent != (_k_tree_t*)0x0) {
        for (unsigned long i = 0; i < grandparent->child_count; i++) {
            if (grandparent->children[i] == parent) {
                grandparent->children[i] = node;
            }
        }
    }

    parent->children[parent->child_count - 1] = (_k_tree_t*)0x0;
    parent->child_count--;
    parent->parent = node;

    node->parent   = grandparent;
    node->children = (_k_tree_t**)realloc(node->children, (node->child_count + 1) * sizeof(_k_tree_t*));

    node->children[node->child_count++] = parent;
}

void _k_tree_print(_k_tree_t *root, int depth, _k_tree_t *bold) {
    if (root == (_k_tree_t*)0x0) return;

    if (root->child_count == 2) _k_tree_print(root->children[1], depth + 1, bold);

    for (int i = 0; i < depth; i++) fprintf(stderr, "    ");
    if (root == bold) fprintf(stderr, "\e[31m\033[1m");
    fprintf(stderr, "%s\n", root->token->str);
    if (root == bold) fprintf(stderr, "\e[0m\033[0m");

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
void _k_compile_tree(_k_tree_t *root, int *r, int *s, FILE *out) {
    if (root == (_k_tree_t*)0x0) return;

    switch (root->token->tokenable->type) {
        case _K_TOKEN_TYPE_DECLARATOR: {
            char type[32];
            memset(type, 0, 32);
            _k_tree_t *node = root->children[0];

            while (strcmp(node->token->str, "*") == 0) {
                strcat(type, "*");
                node = node->children[0];
            }

            strcat(type, node->token->str);

            if (strcmp(root->children[0]->token->str, "type") == 0) {
                fprintf(out, "%s: \n", root->children[1]->token->str);

                for (unsigned long i = 0; i < root->children[1]->child_count; i++) {
                    _k_compile_tree(root->children[1]->children[i], r, s, out);
                }

                break;
            }

            if (root->child_count > 1 && root->children[1]->child_count > 0 && root->children[1]->children[0]->token->tokenable->type == _K_TOKEN_TYPE_NEWINDEX) {
                char *name  = root->children[1]->token->str;
                char *count = root->children[1]->children[0]->children[0]->token->str;

                fprintf(out, "\tnewav: %s %s %d\n", type, name, atoi(count));

                break;
            }
            
            if (root->children[1]->child_count > 0 && root->children[1]->children[0]->token->tokenable->type == _K_TOKEN_TYPE_NEWEXPRESSION) {
                char *name = root->children[1]->token->str;

                fprintf(out, "\n%s: \n", name);

                for (unsigned long i = 0; i < root->children[1]->children[0]->child_count; i++) {
                    fprintf(out, "\tpoprr: r%d\n", ++*r);
                }

                for (unsigned long i = 0; i < root->children[1]->children[0]->child_count; i++) {
                    _k_compile_tree(root->children[1]->children[0]->children[i], r, s, out);
                    fprintf(out, "\tsaver: %s r%d\n", root->children[1]->children[0]->children[i]->children[1]->token->str, (*r)--);
                }

                for (unsigned long i = 0; i < root->children[1]->children[1]->child_count; i++) {
                    _k_compile_tree(root->children[1]->children[1]->children[i], r, s, out);
                }

                break;
            }

            if (root->child_count > 1 && root->children[1]->token->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER) {
                char *name = root->children[1]->token->str;

                fprintf(out, "\tnewsv: %s %s\n", type, name);

                break;
            }

            if (root->child_count > 1 && (root->children[1]->token->tokenable->type == _K_TOKEN_TYPE_OPERATOR || root->children[1]->token->tokenable->type == _K_TOKEN_TYPE_ASSIGNMENT)) {
                char *name = root->children[1]->children[0]->token->str;

                fprintf(out, "\tnewsv: %s %s\n", type, name);

                _k_compile_tree(root->children[1], r, s, out);

                break;
            }

            break;
        }

        case _K_TOKEN_TYPE_IDENTIFIER: {
            if (root->child_count > 0 && root->children[0]->token->tokenable->type == _K_TOKEN_TYPE_NEWEXPRESSION) {
                for (unsigned long i = 0; i < root->children[0]->child_count; i++) {
                    _k_compile_tree(root->children[0]->children[i], r, s, out);
                    fprintf(out, "\tpushr: r%d\n", (*r)--);
                }

                fprintf(out, "\tcallf: %s\n", root->token->str);
                fprintf(out, "\tmovrr: r%d r0\n", ++*r);

                break;
            }

            if (root->child_count > 0 && root->children[0]->token->tokenable->type == _K_TOKEN_TYPE_NEWINDEX) {
                fprintf(out, "\tloadr: r%d %s\n", ++*r, root->token->str);
                _k_compile_tree(root->children[0]->children[0], r, s, out);
                fprintf(out, "\taddrr: r%d r%d r%d\n", *r - 1, *r - 1, *r);
                fprintf(out, "\tderef: r%d r%d\n", *r - 1, *r - 1);

                *r -= 1;

                break;
            }

            fprintf(out, "\tloadr: r%d %s\n", ++*r, root->token->str);

            break;
        }

        case _K_TOKEN_TYPE_NUMBER: {
            fprintf(out, "\tmovrn: r%d %s\n", ++*r, root->token->str);

            break;
        }

        case _K_TOKEN_TYPE_ASSIGNMENT: {
            _k_tree_t *temp = root->children[0];
            int        ptrcnt = 0;
            int        memcnt = 0;
            int        arrcnt = 0;

            if (temp->child_count > 0 && strcmp(temp->children[0]->token->str, "[") == 0) {
                fprintf(out, "\tloadr: r%d %s\n", ++(*r), temp->token->str);

                _k_compile_tree(temp->children[0]->children[0], r, s, out);

                fprintf(out, "\taddrr: r%d r%d r%d\n", *r - 1, *r - 1, *r);

                *r -= 1;

                arrcnt = 1;
            }

            while (strcmp(temp->token->str, ".") == 0) {
                temp = temp->children[0];

                memcnt++;
            }

            if (memcnt > 0) {
                fprintf(out, "\tloadr: r%d %s\n", ++(*r), temp->token->str);
            }

            for (int i = 0; i < memcnt; i++) {
                temp = temp->parent;

                fprintf(out, "\tadszr: r%d r%d %s\n", *r, *r, temp->children[1]->token->str);
            }

            while (strcmp(temp->token->str, "*") == 0) {
                temp = temp->children[0];

                ptrcnt++;
            }

            if (ptrcnt > 0) fprintf(out, "\tloadr: r%d %s\n", ++(*r), temp->token->str);

            for (int i = 0; i < ptrcnt - 1; i++) {
                fprintf(out, "\tderef: r%d r%d\n", *r, *r);
            }
            
            _k_compile_tree(root->children[1], r, s, out);

            if (ptrcnt > 0) { fprintf(out, "\tsavea: r%d r%d\n", *r - 1, *r); *r -= 2; break; }

            if (memcnt > 0) { fprintf(out, "\tsavea: r%d r%d\n", *r - 1, *r); *r -= 2; break; }

            if (arrcnt > 0) { fprintf(out, "\tsavea: r%d r%d\n", *r - 1, *r); *r -= 2; break; }

            fprintf(out, "\tsaver: %s r%d\n", root->children[0]->token->str, (*r)--);

            break;
        }

        case _K_TOKEN_TYPE_OPERATOR: {
            if (root->child_count > 1) {
                if (strcmp(root->token->str, ".") == 0) {
                    if (root->children[0]->token->tokenable->type == _K_TOKEN_TYPE_NUMBER) {
                        fprintf(out, "\tmovrf: r%d %s.%s\n", ++*r, root->children[0]->token->str, root->children[1]->token->str);

                        break;
                    }

                    _k_compile_tree(root->children[0], r, s, out);

                    fprintf(out, "\tadszr: r%d r%d %s\n", *r, *r, root->children[1]->token->str);
                    fprintf(out, "\tderef: r%d r%d\n", *r, *r);

                    break;
                }

                _k_compile_tree(root->children[0], r, s, out);
                _k_compile_tree(root->children[1], r, s, out);
                _k_compile_bin_op(root->token, r, out);
            } else {
                if (strcmp(root->token->str, "&") == 0) {
                    fprintf(out, "\trefsv: r%d %s\n", ++(*r), root->children[0]->token->str);

                    break;
                }
                _k_compile_tree(root->children[0], r, s, out);
                _k_compile_un_op(root->token, r, out);
            }

            break;
        }

        case _K_TOKEN_TYPE_NEWEXPRESSION: {
            for (unsigned long i = 0; i < root->child_count; i++) {
                _k_compile_tree(root->children[i], r, s, out);
            }

            break;
        }

        case _K_TOKEN_TYPE_NEWSTATEMENT: {
            for (unsigned long i = 0; i < root->child_count; i++) {
                _k_compile_tree(root->children[i], r, s, out);
            }

            break;
        }

        case _K_TOKEN_TYPE_KEYWORD: {
            if (strcmp(root->token->str, "return") == 0) {
                if (root->child_count > 0) {
                    _k_compile_tree(root->children[0], r, s, out);
                    fprintf(out, "\tmovrr: r0 r%d\n", *r);
                }

                fprintf(out, "\tleave: \n", (*r)--);

                break;
            }

            if (strcmp(root->token->str, "if") == 0) {
                _k_compile_tree(root->children[0], r, s, out);

                fprintf(out, "\tcmprd: r%d 0\n\tjmpeq: S%d\n", (*r)--, ++*s, out);

                _k_compile_tree(root->children[1], r, s, out);

                fprintf(out, "S%d: \n", *s, out);

                break;
            }

            if (strcmp(root->token->str, "while") == 0) {
                fprintf(out, "S%d: \n", ++*s, out);

                _k_compile_tree(root->children[0], r, s, out);

                fprintf(out, "\tcmprd: r%d 0\n\tjmpeq: S%d\n", (*r)--, ++*s, out);

                _k_compile_tree(root->children[1], r, s, out);

                fprintf(out, "\tjmpal: S%d\nS%d: \n", *s - 1, *s, out);

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
void _k_tree(_k_token_t *token, FILE *out, int flags) {
    _k_tree_t *root = (_k_tree_t*)0x0;
    _k_tree_t *node  = (_k_tree_t*)0x0;
    int        s     = -1;

    int after_operator = 0;

    node = _k_place_token(&root, token++);

    do {
        if (flags) {
            fprintf(stderr, "Token: %s\n", token->str);
            fprintf(stderr, "----------\n");
            _k_tree_print(root, 0, node);
            fprintf(stderr, "----------\n");
        }

        /* Re-root the tree if it gets swapped elsewhere.  */
        while (root->parent != (_k_tree_t*)0x0) {
            root = root->parent;
        }

        switch (token->tokenable->type) {
            case _K_TOKEN_TYPE_IDENTIFIER:
            case _K_TOKEN_TYPE_NUMBER: {
                if (node != (_k_tree_t*)0x0 && 
                   (node->token->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER || node->token->tokenable->type == _K_TOKEN_TYPE_NUMBER)) {
                    /* Literal after literal, doesn't make sense.  */
                    _k_build_error = 1; return; 
                }

                if (strcmp(node->token->str, ".") == 0) {
                    node = _k_place_child(node, token)->parent; after_operator = 0; break;
                }

                if (node->token->tokenable->type == _K_TOKEN_TYPE_OPERATOR || node->token->tokenable->type == _K_TOKEN_TYPE_ASSIGNMENT) {
                    /* Probably unary.  */

                    node = _k_place_child(node, token);

                    while (node->parent != (_k_tree_t*)0x0 && node->parent->token->tokenable->type == _K_TOKEN_TYPE_OPERATOR && node->parent->child_count == 1) { node = node->parent; }

                    after_operator = 0; break;
                }

                node = _k_place_child(node, token); after_operator = 0; break; 
            }
            case _K_TOKEN_TYPE_NEWEXPRESSION: 
            case _K_TOKEN_TYPE_NEWSTATEMENT: 
            case _K_TOKEN_TYPE_NEWINDEX: { 
                node = _k_place_child(node, token); after_operator = 1; break;
            }
            case _K_TOKEN_TYPE_ENDEXPRESSION: {
                while (node->token->tokenable->type != _K_TOKEN_TYPE_NEWEXPRESSION) { node = node->parent; }
                /* Arguments.  */
                if (node->parent->token->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER) { node = node->parent; }

                break;
            }

            case _K_TOKEN_TYPE_SEPARATOR: {
                while (node->token->tokenable->type != _K_TOKEN_TYPE_NEWEXPRESSION) { node = node->parent; }

                after_operator = !after_operator;

                break;
            }

            case _K_TOKEN_TYPE_ENDSTATEMENT: {
                while (node->token->tokenable->type != _K_TOKEN_TYPE_NEWSTATEMENT) { node = node->parent; }
                /* Function body.  */
                if (node->parent->token->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER) { node = node->parent; }
                if (node->parent->token->tokenable->type == _K_TOKEN_TYPE_KEYWORD) { node = node->parent; }

                break;
            }

            case _K_TOKEN_TYPE_ENDINDEX: {
                while (node->token->tokenable->type != _K_TOKEN_TYPE_NEWINDEX) { node = node->parent; }

                node = node->parent;

                break;
            }

            case _K_TOKEN_TYPE_ENDLINE: {
                /* Find next scope.  */
                while (node->token->tokenable->type != _K_TOKEN_TYPE_NEWSTATEMENT && 
                       node->parent != (_k_tree_t*)0x0) { node = node->parent; }

                if (node->parent == (_k_tree_t*)0x0) {
                    int r = 0;

                    _k_compile_tree(root, &r, &s, out);

                    //_k_free_tree(root);

                    node = _k_place_token(&root, ++token);

                    after_operator = 0;
                }

                else after_operator = 1;

                break;
            }
            
            case _K_TOKEN_TYPE_KEYWORD: {
                if (strcmp(token->str, "do") == 0) {
                    while (strcmp(node->token->str, "if") != 0 && strcmp(node->token->str, "while") != 0) { node = node->parent; }

                    break;
                }
#if 0
                if (node != (_k_tree_t*)0x0 && (node->token->tokenable->type != _K_TOKEN_TYPE_NEWSTATEMENT)) {
                    /* Compound keyword statements can only exist at the start of a context, within {} or in the global scope.  */
                    _k_build_error = 2; return;
                }
#endif
                node = _k_place_child(node, token);

                break;
            }
            case _K_TOKEN_TYPE_ASSIGNMENT:
            case _K_TOKEN_TYPE_OPERATOR: 
            case _K_TOKEN_TYPE_DECLARATOR: {
                if (after_operator) {
                    /* Probably unary.  */

                    node = _k_place_child(node, token);

                    break;
                }

                while (node->parent != (_k_tree_t *)0x0 && (_k_get_prec(token->str) < _k_get_prec(node->parent->token->str)) && node->parent->child_count != 1) { node = node->parent; }

                node = _k_place_child(node, token);

                _k_swap_parent(node);

                after_operator = 1;

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
char *_k_compile(_k_token_t *tokens, int flags) {
    char   *out;
    size_t  size;
    FILE   *f = open_memstream(&out, &size);

    _k_tree(tokens, f, flags);

    fclose(f);

    free(tokens);

    return out;
}