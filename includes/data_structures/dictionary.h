

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "linked_list.h"
#include "entry.h"
#include "binary_search_tree.h"

// The dictionary is a collection of entries stored in a BinarySearchTree.
struct Dictionary
{
  /* Public variables */

  struct BinarySearchTree bst; // Inheriting from the BinarySearchTree object.
  struct LinkedList keys;      // A linked list to store the dictionary's keys for easy iteration.

  /* Public methods */

  // The search function finds an key in the dictionary, returning its value or NULL if not found.
  void *(*search)(struct Dictionary *dictionary, void *key, unsigned long key_size);
  // The insert function adds a new entry to the dictionary.  Since memory allocation is handled by the entry constructor, the size of this entry's key and value must be specified.
  void (*insert)(struct Dictionary *dictionary, void *key, unsigned long key_size, void *value, unsigned long value_size);
};

// Creating a new dictionary.
struct Dictionary dictionary_constructor(int (*compare)(void *key_one, void *key_two));
// Freeing the memory allocated for the dictionary.
void dictionary_destructor(struct Dictionary *dictionary);

// The compare_string_keys function is used to compare the keys of two entries.
int compare_string_keys(void *entry_one, void *entry_two);

#endif /* DICTIONARY_H */
