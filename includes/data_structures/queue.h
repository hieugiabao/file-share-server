

#ifndef QUEUE_H
#define QUEUE_H

#include "linked_list.h"

// Queues are used to access a linked list in a controlled manner.
struct Queue
{
  /* Public variables */

  struct LinkedList list; // A refrerence to the embedded linked list.

  /* Public methods */

  // Adds a node to the end of the chain
  void (*push)(struct Queue *queue, void *data, unsigned long size);
  // Retrieves the data from the first node in the chain.
  void *(*peek)(struct Queue *queue);
  // Removes the first node in the chain.
  void (*pop)(struct Queue *queue);
};

// Creating a new queue.
struct Queue queue_constructor(void);
// Freeing the memory allocated for the queue.
void queue_destructor(struct Queue *queue);

#endif /* QUEUE_H */
