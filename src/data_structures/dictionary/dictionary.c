#include "data_structures/dictionary.h"

#include <stdlib.h>
#include <string.h>

/* Private member methods prototypes */

void recursive_dictionary_destroy(struct Node *cursor);

/* Public member methods prototypes */

void insert_dict(struct Dictionary *dictionary, void *key, unsigned long key_size, void *value, unsigned long value_size);
void *search_dict(struct Dictionary *dictionary, void *key, unsigned long key_size);
void iterate_dict(struct Dictionary *dictionary, size_t (*key_size)(void *key), void (*callback)(void *key, void *value, void *arg), void *arg);

/* Constructor */

/**
 * It creates a dictionary by creating a binary search tree and a linked list, and then it returns the dictionary
 *
 * @param compare a function that takes two keys and returns an integer. If the integer is less than 0, then the first key
 * is less than the second key. If the integer is greater than 0, then the first key is greater than the second key. If the
 * integer is 0, then the two keys are
 *
 * @return A dictionary.
 */
struct Dictionary dictionary_constructor(int (*compare)(void *key_one, void *key_two))
{
  struct Dictionary dictionary;
  dictionary.bst = binary_search_tree_constructor(compare);
  dictionary.keys = linked_list_constructor();
  dictionary.insert = insert_dict;
  dictionary.search = search_dict;
  dictionary.iterate = iterate_dict;
  return dictionary;
}

/**
 * It destroys the linked list of keys, and then it destroys the binary search tree of values
 *
 * @param dictionary The dictionary to be destroyed.
 */
void dictionary_destructor(struct Dictionary *dictionary)
{
  linked_list_destructor(&dictionary->keys);
  recursive_dictionary_destroy(dictionary->bst.head);
}

/* Private member methods implementation */

/**
 * If the current node has a previous node, recursively destroy the previous node. If the current node has a next node,
 * recursively destroy the next node. Then, free the data of the current node and free the current node
 *
 * @param cursor The current node in the tree.
 */
void recursive_dictionary_destroy(struct Node *cursor)
{
  if (cursor && cursor->prev)
  {
    recursive_dictionary_destroy(cursor->prev);
  }
  if (cursor && cursor->next)
  {
    recursive_dictionary_destroy(cursor->next);
  }
  if (cursor)
  {
    entry_destructor((struct Entry *)cursor->data);
    free(cursor);
  }
}

/* Public member methods implementation */

/**
 * It creates a dummy entry with the given key and value, and then uses the search function of the BinarySearchTree to find
 * the desired element
 *
 * @param dictionary The dictionary to search in.
 * @param key The key to search for.
 * @param key_size The size of the key in bytes.
 *
 * @return The value of the entry that was found.
 */
void *search_dict(struct Dictionary *dictionary, void *key, unsigned long key_size)
{
  int dummy_value = 0;
  struct Entry searchable = entry_constructor(key, key_size, &dummy_value, sizeof(dummy_value));
  // Use the iterate function of the BinarySearchTree to find the desired element.
  void *result = dictionary->bst.search(&dictionary->bst, &searchable);

  if (result)
  {
    return ((struct Entry *)result)->value;
  }
  else
  {
    return NULL;
  }
}

/**
 * It creates a new entry, inserts it into the dictionary's binary search tree, and then inserts the key into the
 * dictionary's key list
 *
 * @param dictionary The dictionary to insert into.
 * @param key The key to insert into the dictionary.
 * @param key_size The size of the key in bytes.
 * @param value The value to be inserted into the dictionary.
 * @param value_size The size of the value in bytes.
 */
void insert_dict(struct Dictionary *dictionary, void *key, unsigned long key_size, void *value, unsigned long value_size)
{
  struct Entry new_entry = entry_constructor(key, key_size, value, value_size);
  dictionary->bst.insert(&dictionary->bst, &new_entry, sizeof(new_entry));
  dictionary->keys.insert(&dictionary->keys, dictionary->keys.length, key, key_size);
}

#include <stdio.h>

/* public helper function implementation */

/**
 * If the first string is greater than the second string, return 1. If the first string is less than the second string,
 * return -1. If the strings are equal, return 0
 *
 * @param entry_one The first entry to compare.
 * @param entry_two The second entry to compare.
 *
 * @return the result of the strcmp function.
 */
int compare_string_keys(void *entry_one, void *entry_two)
{
  if (strcmp((char *)(((struct Entry *)entry_one)->key), (char *)(((struct Entry *)entry_two)->key)) > 0)
  {
    return 1;
  }
  else if (strcmp((char *)(((struct Entry *)entry_one)->key), (char *)(((struct Entry *)entry_two)->key)) < 0)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

/**
 * It iterates through the dictionary and calls a callback function on each entry
 *
 * @param dictionary The dictionary to iterate over.
 * @param callback a function pointer to a function that takes a pointer to an Entry and a void pointer as parameters.
 * @param arg The argument to pass to the callback function.
 */
void iterate_dict(struct Dictionary *dictionary, size_t (*key_size)(void *key), void (*callback)(void *key, void *value, void *arg), void *arg)
{
  for (int i = 0; i < dictionary->keys.length; i++)
  {
    void *key = dictionary->keys.retrieve(&dictionary->keys, i);
    void *value = dictionary->search(dictionary, key, key_size(key));
    callback(key, value, arg);
  }
}
