

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "node.h"

// LinkedLists are used to move between and manipulate related nodes in an organized fashion.
struct LinkedList
{
  /* Public variables */

  struct Node *head; // Head points to the first node in the chain.
  int length;        // Length refers to the number of nodes in the chain.

  /* Public methods */

  // Insert adds new items to the chain at a specified location - this function creates the new nodes.
  void (*insert)(struct LinkedList *list, int index, void *data, unsigned long size);
  // Remove an item from the chain at a specified location and deal allocation of memory.
  void (*remove)(struct LinkedList *list, int index);
  // Get the data from a node at a specified location.
  void *(*retrieve)(struct LinkedList *list, int index);
  // Sorting and searching the list (bubble sort).
  void (*sort)(struct LinkedList *list, int (*compare)(void *a, void *b));
  // Binary search. (requires sorted list)
  short (*search)(struct LinkedList *list, void *query, int (*compare)(void *a, void *b));
};

// Creating a new linked list.
struct LinkedList linked_list_constructor(void);
// Freeing the memory allocated for the linked list.
void linked_list_destructor(struct LinkedList *list);

#endif /* LINKED_LIST_H */
