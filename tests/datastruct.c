#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
 * It frees the memory allocated for the data and the node itself
 *
 * @param node The node to destroy.
 */
void node_destructor(struct Node *node)
{
  free(node->data);
  free(node);
}

/* Private member methods prototypes */

struct Node *create_node_ll(void *data, unsigned long size);
void destroy_node_ll(struct Node *node);

/* Public member methods prototypes */

struct Node *iterate_ll(struct LinkedList *list, int index);
void insert_ll(struct LinkedList *list, int index, void *data, unsigned long size);
void remove_node_ll(struct LinkedList *list, int index);
void *retrieve_ll(struct LinkedList *list, int index);
void bubble_sort_ll(struct LinkedList *list, int (*compare)(void *a, void *b));
short binary_search_ll(struct LinkedList *list, void *query, int (*compare)(void *a, void *b));

/* Constructor */

/**
 * It creates a new linked list and returns it
 *
 * @return A struct LinkedList
 */
struct LinkedList linked_list_constructor()
{
  struct LinkedList new_list;
  // init data
  new_list.head = NULL;
  new_list.length = 0;

  // init methods
  new_list.insert = insert_ll;
  new_list.remove = remove_node_ll;
  new_list.retrieve = retrieve_ll;
  new_list.sort = bubble_sort_ll;
  new_list.search = binary_search_ll;

  return new_list;
}

/**
 * This function removes all the elements from the list.
 *
 * @param list The list to be destructed.
 */
void linked_list_destructor(struct LinkedList *list)
{
  for (int i = 0; i < list->length; i++)
  {
    list->remove(list, 0);
  }
}

/* Private methods */

/**
 * The create_node function creates a new node to add to the chain
 * by allocating space on the heap and calling the node constructor.
 *
 * @param data The data that you want to store in the node.
 * @param size The size of the data that you want to store in the node.
 *
 * @return A pointer to a Node struct.
 */
struct Node *create_node_ll(void *data, unsigned long size)
{
  struct Node *new_node = (struct Node *)malloc(sizeof(struct Node));
  *new_node = node_constructor(data, size);

  return new_node;
}

/**
 * The destroy_node function removes a node by deallocating it's memory address.
 * This simply renames the node destructor function.
 *
 * @param node The node to be destroyed.
 */
void destroy_node_ll(struct Node *node)
{
  node_destructor(node);
}

/**
 * Iterate through the linked list until you reach the index specified
 * by the user, then return the node at that index.
 *
 * @param list The linked list to iterate over
 * @param index The index of the node you want to retrieve.
 *
 * @return A pointer to the node at the specified index.
 */
struct Node *iterate_ll(struct LinkedList *list, int index)
{
  // Confirm the user has specified a valid index
  if (index < 0 || index >= list->length)
  {
    return NULL;
  }

  // Create a cursor node for iteration
  struct Node *cursor = list->head;
  for (int i = 0; i < index; i++)
  {
    cursor = cursor->next;
  }

  return cursor;
}

/* Public methods */

/**
 * The insert function puts a new node in the chain.
 *
 * @param list the list to insert into
 * @param index the index at which to insert the new node
 * @param data the data to be inserted into the list
 * @param size the size of the data to be stored in the node
 */
void insert_ll(struct LinkedList *list, int index, void *data, unsigned long size)
{
  // Create a new node to be inserted
  struct Node *node_to_insert = create_node_ll(data, size);
  // Check if the node will be the new head of the list
  if (index == 0)
  {
    // insert to head
    node_to_insert->next = list->head;
    list->head = node_to_insert;
  }
  else
  {
    // find the item in the list immediately before the desired index
    struct Node *cursor = iterate_ll(list, index - 1);
    if (!cursor)
    {
      return;
    }
    // set the node's next pointer to the node at the desired index
    node_to_insert->next = cursor->next;
    // set the cursor's next to the new node
    cursor->next = node_to_insert;
  }
  // increment the length of the list
  list->length++;
}

/**
 * The remove function removes a node from the linked list.
 *
 * @param list The list to remove the node from
 * @param index the index of the item to remove
 *
 * @return The node at the given index
 */
void remove_node_ll(struct LinkedList *list, int index)
{
  // Check if the item being removed is the head
  if (index == 0)
  {
    struct Node *node_to_remove = list->head;
    if (node_to_remove)
    {
      list->head = node_to_remove->next;
      destroy_node_ll(node_to_remove);
    }
    else
    {
      return;
    }
  }
  else
  {
    // find the item in the list immediately before the desired index
    struct Node *cursor = iterate_ll(list, index - 1);
    if (!cursor)
    {
      return;
    }
    struct Node *node_to_remove = cursor->next;
    cursor->next = node_to_remove->next;
    destroy_node_ll(node_to_remove);
  }
  // decrement the length of the list
  list->length--;
}

/**
 * Iterate to the desired node, and return the data.
 *
 * @param list the linked list to retrieve from
 * @param index the index of the node to retrieve
 *
 * @return The data stored in the node at the given index.
 */
void *retrieve_ll(struct LinkedList *list, int index)
{
  // find the desired node
  struct Node *cursor = iterate_ll(list, index);
  if (cursor)
  {
    return cursor->data;
  }
  return NULL;
}

/**
 * The sort function is used to sort data in the list.
 * Note that this is a permanent change and items added after sorting will not themselves be sorted.
 * Bubble sort algorithm is used.
 *
 * @param list The list to sort.
 * @param compare A function that takes two void pointers and returns an integer 1, 0 or -1.
 */
void bubble_sort_ll(struct LinkedList *list, int (*compare)(void *a, void *b))
{
  for (struct Node *i = list->retrieve(list, 0); i; i = i->next)
  {
    for (struct Node *j = i->next; j; j = j->next)
    {
      if (compare(i->data, j->data) > 0)
      {
        void *temp = i->data;
        i->data = j->data;
        j->data = temp;
      }
    }
  }
}

/**
 * It takes a linked list, a query, and a comparison function, and returns 1 if the query is in the list, and 0 if it is
 * not
 *
 * @param list The linked list to search.
 * @param query The data to search for.
 * @param compare a function that takes two void pointers and returns 1 if the first is greater than the second, -1 if the
 * first is less than the second, and 0 if they are equal.
 *
 * @return A short int.
 */
short binary_search_ll(struct LinkedList *list, void *query, int (*compare)(void *a, void *b))
{
  int position = list->length / 2;
  int min_checked = 0;
  int max_checked = list->length;
  while (max_checked > min_checked)
  {
    void *data = list->retrieve(list, position);
    if (compare(data, query) == 1)
    {
      max_checked = position;
      if (position != (min_checked + position) / 2)
      {
        position = (min_checked + position) / 2;
      }
      else
      {
        break;
      }
    }
    else if (compare(data, query) == -1)
    {
      min_checked = position;
      if (position != (max_checked + position) / 2)
      {
        position = (max_checked + position) / 2;
      }
      else
      {
        break;
      }
    }
    else
    {
      return 1;
    }
  }
  return 0;
}

int main()
{
  struct LinkedList list = linked_list_constructor();
  list.insert(&list, 0, "Hello", 6);
  list.insert(&list, 1, "World", 6);

  printf("%s %s\n", list.retrieve(&list, 0), list.retrieve(&list, 1));
}
