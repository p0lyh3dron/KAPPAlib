/*
 *    libk_compile.c    --    Source for KAPPA assembler
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on January 21, 2025
 * 
 *    This file is part of the KAPPA project.
 * 
 *    This file defines how the compiled KAPPA source tree is
 *    assembled into the target IR.
 */
#include "libk_assemble.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 *    Compiles a binary operation.
 *
 *    @param _k_token_t *token    The token to compile.
 *    @param int        *r        The register to compile to.
 *    @param FILE       *out      The output file.
 */
void _k_assemble_bin_op(_k_token_t *token, int *r, FILE *out) {
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

/*
 *    Compiles a unary operation.
 *
 *    @param _k_token_t *token    The token to compile.
 *    @param int        *r        The register to compile to.
 *    @param FILE       *out      The output file.
 */
void _k_assemble_un_op(_k_token_t *token, int *r, FILE *out) {
    if (strcmp(token->str, "-") == 0) { fprintf(out, "\tnegrr: r%d r%d\n", *r, *r); }
    if (strcmp(token->str, "*") == 0) { fprintf(out, "\tderef: r%d r%d\n", *r, *r); }
}

/*
 *    Compiles a tree.
 *
 *    @param k_env_t    *env       The environment to compile the tree in.
 *    @param _k_tree_t *root       The root of the tree.
 */
void _k_assemble_tree(_k_tree_t *root, int *r, int *s, FILE *out) {
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
                    _k_assemble_tree(root->children[1]->children[i], r, s, out);
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
                    _k_assemble_tree(root->children[1]->children[0]->children[i], r, s, out);
                    fprintf(out, "\tsaver: %s r%d\n", root->children[1]->children[0]->children[i]->children[1]->token->str, (*r)--);
                }

                for (unsigned long i = 0; i < root->children[1]->children[1]->child_count; i++) {
                    _k_assemble_tree(root->children[1]->children[1]->children[i], r, s, out);
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

                _k_assemble_tree(root->children[1], r, s, out);

                break;
            }

            break;
        }

        case _K_TOKEN_TYPE_IDENTIFIER: {
            if (root->child_count > 0 && root->children[0]->token->tokenable->type == _K_TOKEN_TYPE_NEWEXPRESSION) {
                for (unsigned long i = 0; i < root->children[0]->child_count; i++) {
                    _k_assemble_tree(root->children[0]->children[i], r, s, out);
                    fprintf(out, "\tpushr: r%d\n", (*r)--);
                }

                fprintf(out, "\tcallf: %s\n", root->token->str);
                fprintf(out, "\tmovrr: r%d r0\n", ++*r);

                break;
            }

            if (root->child_count > 0 && root->children[0]->token->tokenable->type == _K_TOKEN_TYPE_NEWINDEX) {
                fprintf(out, "\tloadr: r%d %s\n", ++*r, root->token->str);
                _k_assemble_tree(root->children[0]->children[0], r, s, out);
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

                _k_assemble_tree(temp->children[0]->children[0], r, s, out);

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
            
            _k_assemble_tree(root->children[1], r, s, out);

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

                    _k_assemble_tree(root->children[0], r, s, out);

                    fprintf(out, "\tadszr: r%d r%d %s\n", *r, *r, root->children[1]->token->str);
                    fprintf(out, "\tderef: r%d r%d\n", *r, *r);

                    break;
                }

                _k_assemble_tree(root->children[0], r, s, out);
                _k_assemble_tree(root->children[1], r, s, out);
                _k_assemble_bin_op(root->token, r, out);
            } else {
                if (strcmp(root->token->str, "&") == 0) {
                    fprintf(out, "\trefsv: r%d %s\n", ++(*r), root->children[0]->token->str);

                    break;
                }
                _k_assemble_tree(root->children[0], r, s, out);
                _k_assemble_un_op(root->token, r, out);
            }

            break;
        }

        case _K_TOKEN_TYPE_NEWEXPRESSION: {
            for (unsigned long i = 0; i < root->child_count; i++) {
                _k_assemble_tree(root->children[i], r, s, out);
            }

            break;
        }

        case _K_TOKEN_TYPE_NEWSTATEMENT: {
            for (unsigned long i = 0; i < root->child_count; i++) {
                _k_assemble_tree(root->children[i], r, s, out);
            }

            break;
        }

        case _K_TOKEN_TYPE_KEYWORD: {
            if (strcmp(root->token->str, "return") == 0) {
                if (root->child_count > 0) {
                    _k_assemble_tree(root->children[0], r, s, out);
                    fprintf(out, "\tmovrr: r0 r%d\n", *r);
                }

                fprintf(out, "\tleave: \n", (*r)--);

                break;
            }

            if (strcmp(root->token->str, "if") == 0) {
                _k_assemble_tree(root->children[0], r, s, out);

                fprintf(out, "\tcmprd: r%d 0\n\tjmpeq: S%d\n", (*r)--, ++*s, out);

                _k_assemble_tree(root->children[1], r, s, out);

                fprintf(out, "S%d: \n", *s, out);

                break;
            }

            if (strcmp(root->token->str, "while") == 0) {
                fprintf(out, "S%d: \n", ++*s, out);

                _k_assemble_tree(root->children[0], r, s, out);

                fprintf(out, "\tcmprd: r%d 0\n\tjmpeq: S%d\n", (*r)--, ++*s, out);

                _k_assemble_tree(root->children[1], r, s, out);

                fprintf(out, "\tjmpal: S%d\nS%d: \n", *s - 1, *s, out);

                break;
            }

            break;
        
        }
    }
}