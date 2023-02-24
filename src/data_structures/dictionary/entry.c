#include "data_structures/entry.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Constructor */

/**
 * It takes a key and a value, and returns a new dictionary entry
 *
 * @param key The key to be stored in the entry.
 * @param key_size The size of the key in bytes.
 * @param value The value to be stored in the hash table.
 * @param value_size The size of the value in bytes.
 *
 * @return A struct Entry.
 */
struct Entry entry_constructor(void *key, unsigned long key_size, void *value, unsigned long value_size)
{
  struct Entry entry;
  entry.key = calloc(key_size, key_size);
  memcpy(entry.key, key, key_size);
  entry.value = calloc(value_size, value_size);
  memcpy(entry.value, value, value_size);
  return entry;
}

/**
 * It frees the memory allocated for the key, value, and entry itself
 *
 * @param entry The entry to be destroyed.
 * @param free_key A function pointer to a function that frees the key in the entry.
 * @param free_value A function pointer to a function that frees the value in the entry.
 */
void entry_destructor(struct Entry *entry, void (*free_key)(void *key), void (*free_value)(void *value))
{
  if (free_key != NULL)
    free_key(entry->key);
  else
    free(entry->key);
  if (free_value != NULL)
    free_value(entry->value);
  else
    free(entry->value);
  free(entry);
}
