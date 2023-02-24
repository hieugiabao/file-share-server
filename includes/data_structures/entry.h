

#ifndef ENTRY_H
#define ENTRY_H

// Entries are the fundamental unit of a dictionary, containing a value to be stored and a key to identify it.
struct Entry
{
  void *key;
  void *value;
};

// Creating a new entry.
struct Entry entry_constructor(void *key, unsigned long key_size, void *value, unsigned long value_size);
// Freeing the memory allocated for the entry.
void entry_destructor(struct Entry *entry, void (*free_key)(void *key), void (*free_value)(void *value));

#endif /* ENTRY_H */
