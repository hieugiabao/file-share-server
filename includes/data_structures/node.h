

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

// Creating a new node with the data and size passed in.
struct Node node_constructor(void *data, unsigned long size);
// Freeing the memory allocated for the node.
void node_destructor(struct Node *node);

#endif /* NODE_H */
