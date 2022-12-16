#include "data_structures/binary_search_tree.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Private member methods prototypes */

struct Node *create_node_bst(void *data, unsigned long size);
void destroy_node_bst(struct Node *node);
struct Node *iterate_bst(struct BinarySearchTree *tree, struct Node *cursor, void *data, int *direction);
void recursive_tree_destruction(struct Node *node);

/* Public member methods prototypes */

void *search_bst(struct BinarySearchTree *tree, void *query);
void insert_bst(struct BinarySearchTree *tree, void *data, unsigned long size);

/* Constructor */

/**
 * It creates a new binary search tree and returns it
 *
 * @param compare A function that takes two void pointers and returns an integer. This function is used to compare two data
 * elements in the tree.
 *
 * @return A struct BinarySearchTree
 */
struct BinarySearchTree binary_search_tree_constructor(int (*compare)(void *data_one, void *data_two))
{
  struct BinarySearchTree tree;
  tree.head = NULL;
  tree.compare = compare;
  tree.search = search_bst;
  tree.insert = insert_bst;
  return tree;
}

/**
 * It recursively destroys the tree by freeing the memory of the left and right subtrees, then freeing the memory of the
 * current node
 *
 * @param tree The tree to be destroyed.
 */
void binary_search_tree_destructor(struct BinarySearchTree *tree)
{
  recursive_tree_destruction(tree->head);
}

/* Private member methods implementation */

/**
 * The create_node allocates space on the heap for a node and uses the Node constructor to instantiate it.
 *
 * @param data the data you want to store in the node
 * @param size The size of the data you want to store in the node.
 *
 * @return A pointer to a node.
 */
struct Node *create_node_bst(void *data, unsigned long size)
{
  struct Node *node = malloc(sizeof(struct Node));
  *node = node_constructor(data, size);
  return node;
}

/**
 * The destroy_node function removes a node by deallocating it's memory address.
 * This simply renames the node destructor function.
 *
 * @param node The node to be destroyed.
 */
void destroy_node_bst(struct Node *node)
{
  node_destructor(node);
}

/**
 * The iterate function is a recursive algorithm that traverses the branches of a tree.
 * It utilizes the compare function to determine if it should move left or right,
 * and returns the cursor once there is nowhere left for the iterator to move.
 * The user must take care to insrue this function returns the node they are actually looking for.
 * The function takes a reference to the BinarySearchTree, the current position, desired data, and an int pointer as arguments.
 * The int pointer becomes 1 if the desired data is greater than the returned node, -1 if it is less than, and 0 if they are equal.
 *
 * @param tree The binary search tree.
 * @param cursor The current node being tested.
 * @param data The data to be inserted.
 * @param direction A pointer to an integer that will be set to -1, 0, or 1 depending on the direction the cursor should
 * move.
 *
 * @return a pointer to a struct Node.
 */
struct Node *iterate_bst(struct BinarySearchTree *tree, struct Node *cursor, void *data, int *direction)
{
  if (cursor == NULL)
  {
    *direction = 1;
    return NULL;
  }
  // Compare the cursor's data to the desired data.
  if (tree->compare(cursor->data, data) == 1)
  {
    // Check if there is another node in the chain to be tested
    if (cursor->next)
    {
      // Recursively test the next (right) node.
      return iterate_bst(tree, cursor->next, data, direction);
    }
    else
    {
      // Set the direction pointer to reflect the next position is desired (moving right).
      *direction = 1;
      return cursor;
    }
  }
  // Alternative outcome of the compare.
  else if (tree->compare(cursor->data, data) == -1)
  {
    // Check if there is another node in the chain to be tested
    if (cursor->prev)
    {
      // Recursively test the previous (left) node.
      return iterate_bst(tree, cursor->prev, data, direction);
    }
    else
    {
      // Set the direction pointer to reflect the next position is desired (moving left).
      *direction = -1;
      return cursor;
    }
  }
  // The two data values are equal.
  else
  {
    *direction = 0;
    return cursor;
  }
}

/**
 * It recursively destroys the tree by destroying the next and previous nodes of the current node
 *
 * @param node The node to be destroyed.
 */
void recursive_tree_destruction(struct Node *node)
{
  if (node->next)
  {
    recursive_tree_destruction(node->next);
  }
  if (node->prev)
  {
    recursive_tree_destruction(node->prev);
  }
  destroy_node_bst(node);
}

/* Public member methods implementation */

/**
 * The search function utilizes the iterate function to test if a given node exists in the tree.
 * If the node is found, its data is returned.  Otherwise, NULL is returned.
 *
 * @param tree The tree to search.
 * @param query The data to search for.
 *
 * @return The data of the node that is found.
 */
void *search_bst(struct BinarySearchTree *tree, void *query)
{
  int direction = 0;
  // Utilize iterate to find the desired position.
  struct Node *cursor = iterate_bst(tree, tree->head, query, &direction);
  // Check if the found node is the desired node, or an adjacent one.
  if (direction == 0)
  {
    return cursor->data;
  }
  else
  {
    return NULL;
  }
}

/**
 * The insert function adds new nodes to the tree by finding their proper position.
 *
 * @param tree The tree to insert the data into.
 * @param data The data to be inserted into the tree.
 * @param size The size of the data.
 */
void insert_bst(struct BinarySearchTree *tree, void *data, unsigned long size)
{
  // Check if has the first node in the tree
  if (!tree->head)
  {
    tree->head = create_node_bst(data, size);
  }
  else
  {
    // Set the direction int pointer.
    int direction = 0;
    // Find the desired position.
    struct Node *cursor = iterate_bst(tree, tree->head, data, &direction);
    // Check if the desired position is to the left or right of the cursor.
    if (direction == 1)
    {
      // Create a new node and set the cursor's next pointer to it.
      cursor->next = create_node_bst(data, size);
    }
    else if (direction == -1)
    {
      // Create a new node and set the cursor's previous pointer to it.
      cursor->prev = create_node_bst(data, size);
    }
    // If the direction is 0, the data is already in the tree.
  }
}

/**
 * If the first integer is greater than the second integer, return 1; if the first integer is less than the second integer,
 * return -1; otherwise, return 0
 *
 * @param data_one The first data to compare.
 * @param data_two The data to compare against.
 *
 * @return the difference between the two integers.
 */
int binary_search_tree_int_compare(void *data_one, void *data_two)
{
  if (*(int *)data_one > *(int *)data_two)
  {
    return 1;
  }
  else if (*(int *)data_one < *(int *)data_two)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

/**
 * If the first float is greater than the second float, return 1; if the first float is less than the second float, return
 * -1; otherwise, return 0
 *
 * @param data_one The first data to compare.
 * @param data_two The data to compare against.
 *
 * @return the difference between the two values.
 */
int binary_search_tree_float_compare(void *data_one, void *data_two)
{
  if (*(float *)data_one > *(float *)data_two)
  {
    return 1;
  }
  else if (*(float *)data_one < *(float *)data_two)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

/**
 * If the first character is greater than the second character, return 1; if the first character is less than the second
 * character, return -1; otherwise, return 0
 *
 * @param data_one The first data to compare.
 * @param data_two The data to compare against.
 *
 * @return the difference between the two characters.
 */
int binary_search_tree_char_compare(void *data_one, void *data_two)
{
  if (*(char *)data_one > *(char *)data_two)
  {
    return 1;
  }
  else if (*(char *)data_one < *(char *)data_two)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

/**
 * It compares two strings and returns 1 if the first string is greater than the second, -1 if the first string is less
 * than the second, and 0 if the two strings are equal
 *
 * @param data_one The first data to compare.
 * @param data_two The data to compare against.
 *
 * @return The comparison of the two strings.
 */
int binary_search_tree_str_compare(void *data_one, void *data_two)
{
  int comparison = strcmp(data_one, data_two);

  if (comparison > 0)
  {
    return 1;
  }
  else if (comparison < 0)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}
