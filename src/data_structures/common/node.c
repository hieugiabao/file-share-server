#include "data_structures/node.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Constructor */

/**
 * It takes a pointer to some data and the size of that data, and returns a node with that data
 *
 * @param data The data to be stored in the node.
 * @param size The size of the data you want to store in the node.
 *
 * @return A node.
 */
struct Node node_constructor(void *data, unsigned long size)
{
  if (size < 1)
  {
    fprintf(stderr, "Error: Invalid data size for node...\n");
    exit(1);
  }

  struct Node node;
  node.data = malloc(size);
  memcpy(node.data, data, size);
  node.next = NULL;
  node.prev = NULL;

  return node;
}


/**
 * `node_destructor` frees the data in the node and then frees the node itself
 * 
 * @param node The node to be freed.
 * @param free_data a function pointer to a function that frees the data in the node.
 */
void node_destructor(struct Node *node, void (*free_data)(void *data))
{
  if (free_data == NULL)
    free(node->data);
  else
    free_data(node->data);
  free(node);
}
