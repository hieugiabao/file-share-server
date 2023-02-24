#include "data_structures/linked_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Private member methods prototypes */

struct Node *create_node_ll(void *data, unsigned long size);
void destroy_node_ll(struct Node *node, void (*free_data)(void *data));

/* Public member methods prototypes */

struct Node *iterate_ll(struct LinkedList *list, int index);
void insert_ll(struct LinkedList *list, int index, void *data, unsigned long size);
void remove_node_ll(struct LinkedList *list, int index, void (*free_data)(void *data));
void *retrieve_ll(struct LinkedList *list, int index);
void bubble_sort_ll(struct LinkedList *list, int (*compare)(void *a, void *b));
short binary_search_ll(struct LinkedList *list, void *query, int (*compare)(void *a, void *b));
char *to_json_ll(struct LinkedList *list, char *(*to_json)(void *data));

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
  new_list.to_json = to_json_ll;

  return new_list;
}


/**
 * It removes all the elements from the list, starting from the first element, and calls the free_data
 * function on each element's data
 * 
 * @param list The list to be destructed.
 * @param free_data A function pointer to a function that frees the data in the list.
 */
void linked_list_destructor(struct LinkedList *list, void (*free_data)(void *data))
{
  for (int i = 0; i < list->length; i++)
  {
    list->remove(list, 0, free_data);
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
 * This function destroys a node and frees the memory associated with it.
 * 
 * @param node The node to be destroyed.
 * @param free_data a function pointer to a function that frees the data in the node.
 */
void destroy_node_ll(struct Node *node, void (*free_data)(void *data))
{
  node_destructor(node, free_data);
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
 * We find the node immediately before the node we want to remove, and then we set the next pointer of
 * that node to the next pointer of the node we want to remove
 * 
 * @param list The list to remove the node from
 * @param index the index of the item to remove
 * @param free_data a function pointer to a function that frees the data in the node.
 * 
 * @return A pointer to the node at the specified index.
 */
void remove_node_ll(struct LinkedList *list, int index, void (*free_data)(void *data))
{
  // Check if the item being removed is the head
  if (index == 0)
  {
    struct Node *node_to_remove = list->head;
    if (node_to_remove)
    {
      list->head = node_to_remove->next;
      destroy_node_ll(node_to_remove, free_data);
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
    destroy_node_ll(node_to_remove, free_data);
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

/**
 * It takes a linked list and a function that converts a single element of the linked list to JSON, and
 * returns a string that represents the entire linked list in JSON
 * 
 * @param list The LinkedList to convert to JSON
 * @param to_json a function that takes a pointer to a data structure and returns a JSON string
 * 
 * @return A string containing the JSON representation of the list.
 */
char *to_json_ll(struct LinkedList *list, char *(*to_json)(void *data))
{
  char *json = malloc(sizeof(char) * 2);
  json[0] = '[';
  json[1] = '\0';
  for (int i = 0; i < list->length; i++)
  {
    void *data = list->retrieve(list, i);
    char *data_json = to_json(data);
    json = realloc(json, sizeof(char) * (strlen(json) + strlen(data_json) + 2));
    strcat(json, data_json);
    if (i != list->length - 1)
    {
      strcat(json, ",");
    }
    free(data_json);
  }
  json = realloc(json, sizeof(char) * (strlen(json) + 2));
  strcat(json, "]");
  return json;
}