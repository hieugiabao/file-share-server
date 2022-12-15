
#ifndef BINARY_SEARCH_TREE_H
#define BINARY_SEARCH_TREE_H

#include "node.h"

struct BinarySearchTree
{
  /* Public variables */

  struct Node *head;

  /* Public methods */

  // The compare function is specified by the user and is used to compare the data of two nodes.
  // It must take two void pointers as arguments and return 1, -1, or 0.
  int (*compare)(void *data_one, void *data_two);
  // The search function finds a node in the tree, returning its data or NULL if not found.
  void *(*search)(struct BinarySearchTree *tree, void *query);
  // The insert function adds a new node to the tree.  Since memory allocation is handled by the node constructor, the size of this node's data must be specified.
  void (*insert)(struct BinarySearchTree *tree, void *data, unsigned long size);
};

// Creating a new binary search tree.
struct BinarySearchTree binary_search_tree_constructor(int (*compare)(void *data_one, void *data_two));
// Freeing the memory allocated for the binary search tree.
void binary_search_tree_destructor(struct BinarySearchTree *tree);

// MARK: PUBLIC HELPER FUNCTIONS

int binary_search_tree_int_compare(void *data_one, void *data_two);
int binary_search_tree_float_compare(void *data_one, void *data_two);
int binary_search_tree_char_compare(void *data_one, void *data_two);
int binary_search_tree_str_compare(void *data_one, void *data_two);

#endif /* BINARY_SEARCH_TREE_H */
