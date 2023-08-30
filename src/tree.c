/*
 *    tree.h    --    header for tree operations
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on August 29, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    The declarations for functions related to trees.
 */
#include "tree.h"

#include <stdlib.h>
#include <string.h>

/*
 *    Creates a new node with the given data.
 *
 *    @param unsigned long size    The size of the data to be stored.
 *    @param unsigned long id      The id of the node.
 *    @param void *data            The data to be stored.
 * 
 *    @return node_t *             A pointer to the new node.
 */
node_t *new_node(unsigned long size, unsigned long id, void *data) {
    node_t *node = (node_t*)malloc(sizeof(node_t));

    if (node == (node_t*)0x0)
        return (node_t*)0x0;

    node->size   = size;
    node->id     = id;
    node->data   = malloc(size);
    node->parent = (node_t*)0x0;
    node->left   = (node_t*)0x0;
    node->right  = (node_t*)0x0;

    if (node->data == (void*)0x0) {
        free(node);
        return (node_t*)0x0;
    }

    memcpy(node->data, data, size);

    return node;
}

/*
 *    Frees a tree from memory.
 *
 *    @param node_t *root    The root of the tree to be freed.
 */
void free_tree(node_t *root) {
    if (root == (node_t*)0x0)
        return;

    free_tree(root->left);
    free_tree(root->right);

    free(root->data);
    free(root);
}