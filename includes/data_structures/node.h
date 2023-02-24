

#ifndef NODE_H
#define NODE_H

// Nodes are used to store data of any type in a list.
struct Node
{
  void *data; // The data is stored as a void pointer - casting is required for proper access.
  // A pointer to the next node in the chain.
  struct Node *next;
  struct Node *prev;
};

struct Node node_constructor(void *data, unsigned long size);
void node_destructor(struct Node *node, void (*free_data)(void *data));

#endif /* NODE_H */
