#include "data_structures/queue.h"

/* Public member function prototypes */

void push(struct Queue *queue, void *data, unsigned long size);
void *peek(struct Queue *queue);
void pop(struct Queue *queue, void (*free_data)(void *data));

/* Constructor */

/**
 * It creates a new Queue struct, instantiates a new LinkedList struct, and assigns the Queue's push, peek, and pop
 * functions to the appropriate functions
 *
 * @return A struct Queue.
 */
struct Queue queue_constructor()
{
  struct Queue queue;
  // Instantiate the queue's LinkedList via the constructor.
  queue.list = linked_list_constructor();

  queue.push = push;
  queue.peek = peek;
  queue.pop = pop;

  return queue;
}

/**
 * Destructs the queue by destructing the list.
 * 
 * @param queue The queue to be destructed.
 * @param free_data a function pointer to a function that frees the data in the queue.
 */
void queue_destructor(struct Queue *queue, void (*free_data)(void *data))
{
  linked_list_destructor(&queue->list, free_data);
}

/* public methods */

/**
 * The push method adds an item to the end of the list.
 *
 * @param queue The queue to push to.
 * @param data The data to be inserted into the queue.
 * @param size The size of the data to be inserted.
 */
void push(struct Queue *queue, void *data, unsigned long size)
{
  // Utilize insert from LinkedList with enforced parameters.
  queue->list.insert(&queue->list, queue->list.length, data, size);
}

/**
 * The peek function returns the data from the first item in the chain.
 *
 * @param queue The queue to peek at.
 *
 * @return The data from the first node in the queue.
 */
void *peek(struct Queue *queue)
{
  // Utilize the retrieve function from LinkedList with enforced parameters.
  void *data = queue->list.retrieve(&queue->list, 0);
  return data;
}

/**
 * Remove the first element from the queue and free the data if the free_data function is not NULL.
 * 
 * @param queue The queue to pop from.
 * @param free_data A function pointer to a function that frees the data.
 */
void pop(struct Queue *queue, void (*free_data)(void *data))
{
  // Utilize the remove function from LinkedList with enforced parameters.
  queue->list.remove(&queue->list, 0, free_data);
}
