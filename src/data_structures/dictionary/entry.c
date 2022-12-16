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
  entry.key = malloc(key_size);
  memcpy(entry.key, key, key_size);
  entry.value = malloc(value_size);
  memcpy(entry.value, value, value_size);
  return entry;
}

/**
 * It frees the memory allocated for the key, value, and entry itself
 *
 * @param entry The entry to be destroyed.
 */
void entry_destructor(struct Entry *entry)
{
  free(entry->key);
  free(entry->value);
  free(entry);
}
