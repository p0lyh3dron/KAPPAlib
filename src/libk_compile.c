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

#include "libk_assemble.h"
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

/*
 *    Returns the precedence of an operator.
 *
 *    @param char *op    The operator to get the precedence of.
 * 
 *    @return char    The precedence of the operator.
 */
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

/*
 *    Swaps a child's token with its parent's token.
 *
 *    @param _k_tree_t *child    The child to swap.
 */
void _k_swap_token(_k_tree_t *child) {
    _k_tree_t *parent = child->parent;

    _k_token_t *temp = parent->token;
    parent->token = child->token;
    child->token = temp;
}

/*
 *    Prints a tree.
 *
 *    @param _k_tree_t *root     The root of the tree.
 *    @param int        depth    The depth of the tree.
 *    @param _k_tree_t *bold     The bold node.
 */
void _k_tree_print(_k_tree_t *root, int depth, _k_tree_t *bold) {
    if (root == (_k_tree_t*)0x0) return;

    if (root->child_count == 2) _k_tree_print(root->children[1], depth + 1, bold);

    for (int i = 0; i < depth; i++) fprintf(stderr, "    ");
    if (root == bold) fprintf(stderr, "\e[31m\033[1m");
    fprintf(stderr, "%s\n", root->token->str);
    if (root == bold) fprintf(stderr, "\e[0m\033[0m");

    if (root->child_count >= 1) _k_tree_print(root->children[0], depth + 1, bold);
}

/*
 *    Frees a tree.
 *
 *    @param _k_tree_t *root    The root of the tree.
 */
void _k_free_tree(_k_tree_t *root) {
    if (root == (_k_tree_t*)0x0) return;

    for (unsigned long i = 0; i < root->child_count; i++) _k_free_tree(root->children[i]);

    free(root->children);
    free(root);
}

/*
 *    Compiles a literal.
 *
 *    @param _k_tree_t **node     The node to compile.
 *    @param _k_token_t *token    The token to compile.
 */
void _k_compile_literal(_k_tree_t **node, _k_token_t *token) {
    if ((*node) != (_k_tree_t*)0x0 && ((*node)->token->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER || (*node)->token->tokenable->type == _K_TOKEN_TYPE_NUMBER)) {
        /* Literal after literal, doesn't make sense.  */
        _k_build_error = 1; return; 
    }

    if (strcmp((*node)->token->str, ".") == 0) {
        (*node) = _k_place_child((*node), token)->parent; return;
    }

    if ((*node)->token->tokenable->type == _K_TOKEN_TYPE_OPERATOR || (*node)->token->tokenable->type == _K_TOKEN_TYPE_ASSIGNMENT) {
        /* Probably unary.  */

        (*node) = _k_place_child((*node), token);

        return;
    }

    (*node) = _k_place_child((*node), token); return; 
}

/*
 *    Compiles a context (new expression, statement, etc).
 *
 *    @param _k_tree_t **root    The root of the tree.
 *    @param _k_token_t *token   The token to compile.
 */
void _k_compile_context(_k_tree_t **node, _k_token_t *token) {
    (*node) = _k_place_child((*node), token);
}

/*
 *    Compiles an end expression.
 *
 *    @param _k_tree_t **root    The root of the tree.
 *    @param _k_token_t *token   The token to compile.
 */
void _k_compile_end_expression(_k_tree_t **node, _k_token_t *token) {
    while ((*node)->token->tokenable->type != _K_TOKEN_TYPE_NEWEXPRESSION) { (*node) = (*node)->parent; }
    /* Arguments.  */
    if ((*node)->parent->token->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER) { (*node) = (*node)->parent; }
}

/*
 *    Compiles an end statement.
 *
 *    @param _k_tree_t **root    The root of the tree.
 *    @param _k_token_t *token   The token to compile.
 */
void _k_compile_end_statement(_k_tree_t **node, _k_token_t *token) {
    while ((*node)->token->tokenable->type != _K_TOKEN_TYPE_NEWSTATEMENT)    { (*node) = (*node)->parent; }
    /* Function body.  */
    if ((*node)->parent->token->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER) { (*node) = (*node)->parent; }
    if ((*node)->parent->token->tokenable->type == _K_TOKEN_TYPE_KEYWORD)    { (*node) = (*node)->parent; }
}

/*
 *    Compiles an end index.
 *
 *    @param _k_tree_t **root    The root of the tree.
 *    @param _k_token_t *token   The token to compile.
 */
void _k_compile_end_index(_k_tree_t **node, _k_token_t *token) {
    while ((*node)->token->tokenable->type != _K_TOKEN_TYPE_NEWINDEX) { (*node) = (*node)->parent; }

    (*node) = (*node)->parent;
}

/*
 *    Compiles a separator.
 *
 *    @param _k_tree_t **root    The root of the tree.
 *    @param _k_token_t *token   The token to compile.
 */
void _k_compile_separator(_k_tree_t **node, _k_token_t *token) {
    while ((*node)->token->tokenable->type != _K_TOKEN_TYPE_NEWEXPRESSION) { (*node) = (*node)->parent; }
}

/*
 *    Compiles an endline.
 *
 *    @param _k_tree_t **node    The start node.
 *    @param _k_tree_t  *root    The root of the tree.
 *    @param _k_token_t *token   The token to compile.
 *    @param FILE       *out     The output file.
 */
void _k_compile_endline(_k_tree_t **node, _k_tree_t *root, _k_token_t *token, FILE *out) {
    /* Find next scope.  */
    while ((*node)->token->tokenable->type != _K_TOKEN_TYPE_NEWSTATEMENT && 
            (*node)->parent != (_k_tree_t*)0x0) { (*node) = (*node)->parent; }

    if ((*node)->parent == (_k_tree_t*)0x0) {
        int r = 0;
        int s = -1;

        _k_assemble_tree(root, &r, &s, out);

        //_k_free_tree(root);

        (*node) = _k_place_token(&root, ++token);
    }
}

/*
 *    Compiles a keyword.
 *
 *    @param _k_tree_t **root    The root of the tree.
 *    @param _k_token_t *token   The token to compile.
 */
void _k_compile_keyword(_k_tree_t **node, _k_token_t *token) {
    if (strcmp(token->str, "do") == 0) {
        while (strcmp((*node)->token->str, "if") != 0 && strcmp((*node)->token->str, "while") != 0) { 
            (*node) = (*node)->parent; 
        }

        return;
    }
#if 0    
    if ((*node) != (_k_tree_t*)0x0 && ((*node)->token->tokenable->type != _K_TOKEN_TYPE_NEWSTATEMENT)) {
        /* Compound keyword statements can only exist at the start of a context, within {} or in the global scope.  */
        _k_build_error = 2; return;
    }
#endif
    (*node) = _k_place_child((*node), token);
}

/*
 *    Compiles an operator.
 *
 *    @param _k_tree_t **root    The root of the tree.
 *    @param _k_token_t *token   The token to compile.
 */
void _k_compile_operator(_k_tree_t **node, _k_token_t *token) {
    /* Literal with operator parent -> Token is binary  */
    if ((*node)->token->tokenable->type == _K_TOKEN_TYPE_IDENTIFIER) {
        if ((*node)->parent != (_k_tree_t *)0x0 && (*node)->parent->child_count == 1 && (*node)->parent->token->tokenable->type == _K_TOKEN_TYPE_OPERATOR) {
            (*node) = _k_place_child((*node), token);
            _k_swap_parent((*node));
            _k_swap_token((*node));

            (*node) = (*node)->parent;

            return;
        }
    }

    if ((*node)->token->tokenable->type == _K_TOKEN_TYPE_OPERATOR || (*node)->token->tokenable->type == _K_TOKEN_TYPE_ASSIGNMENT) {
        (*node) = _k_place_child((*node), token); return;
    }

    while ((*node)->parent != (_k_tree_t *)0x0 && (_k_get_prec(token->str) < _k_get_prec((*node)->parent->token->str)) && (*node)->parent->child_count != 1) { (*node) = (*node)->parent; }

    (*node) = _k_place_child((*node), token);

    _k_swap_parent((*node));
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
void _k_compile_tree(_k_token_t *token, FILE *out, int flags) {
    _k_tree_t *root = (_k_tree_t*)0x0;
    _k_tree_t *node  = (_k_tree_t*)0x0;

    node = _k_place_token(&root, token++);

    do {
        /* Re-root the tree if it gets swapped elsewhere.  */
        while (root->parent != (_k_tree_t*)0x0) {
            root = root->parent;
        }

        if (flags) {
            fprintf(stderr, "Token: %s\n", token->str);
            fprintf(stderr, "----------\n");
            _k_tree_print(root, 0, node);
            fprintf(stderr, "----------\n");
        }

        switch (token->tokenable->type) {
            case _K_TOKEN_TYPE_IDENTIFIER:
            case _K_TOKEN_TYPE_NUMBER:              { _k_compile_literal(&node, token);             break; }
            case _K_TOKEN_TYPE_NEWEXPRESSION: 
            case _K_TOKEN_TYPE_NEWSTATEMENT: 
            case _K_TOKEN_TYPE_NEWINDEX:            { _k_compile_context(&node, token);             break; }
            case _K_TOKEN_TYPE_ENDEXPRESSION:       { _k_compile_end_expression(&node, token);      break; }
            case _K_TOKEN_TYPE_SEPARATOR:           { _k_compile_separator(&node, token);           break; }
            case _K_TOKEN_TYPE_ENDSTATEMENT:        { _k_compile_end_statement(&node, token);       break; }
            case _K_TOKEN_TYPE_ENDINDEX:            { _k_compile_end_index(&node, token);           break; }
            case _K_TOKEN_TYPE_ENDLINE:             { _k_compile_endline(&node, root, token, out);  break; }
            
            case _K_TOKEN_TYPE_KEYWORD:             { _k_compile_keyword(&node, token);             break; }
            case _K_TOKEN_TYPE_ASSIGNMENT:
            case _K_TOKEN_TYPE_OPERATOR: 
            case _K_TOKEN_TYPE_DECLARATOR:          { _k_compile_operator(&node, token);            break; }
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

    _k_compile_tree(tokens, f, flags);

    fclose(f);

    free(tokens);

    return out;
}