/*
 *    tree.h    --    header for tree operations
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on August 29, 2023
 * 
 *    This file is part of the KAPPA project.
 * 
 *    The declarations for functions related to trees.
 */
#ifndef _LIBK_TREE_H
#define _LIBK_TREE_H

typedef struct node_s {
    unsigned long  size;
    unsigned long  id;
    void          *data;

    struct node_s *parent;
    struct node_s *left;
    struct node_s *right;
} node_t;

/*
 *    Creates a new node with the given data.
 *
 *    @param unsigned long size    The size of the data to be stored.
 *    @param unsigned long id      The id of the node.
 *    @param void *data            The data to be stored.
 * 
 *    @return node_t *             A pointer to the new node.
 */
node_t *new_node(unsigned long size, unsigned long id, void *data);

/*
 *    Frees a tree from memory.
 *
 *    @param node_t *root    The root of the tree to be freed.
 */
void free_tree(node_t *root);

#endif /* _LIBK_TREE_H  */