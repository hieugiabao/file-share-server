#include "data_structures/queue.h"

/* Public member function prototypes */

void push(struct Queue *queue, void *data, unsigned long size);
void *peek(struct Queue *queue);
void pop(struct Queue *queue);

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
 * This function frees the memory allocated to the queue.
 *
 * @param queue The queue to be destructed.
 */
void queue_destructor(struct Queue *queue)
{
  linked_list_destructor(&queue->list);
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
 * Remove the first element from the queue.
 *
 * @param queue The queue to pop from.
 */
void pop(struct Queue *queue)
{
  // Utilize the remove function from LinkedList with enforced parameters.
  queue->list.remove(&queue->list, 0);
}
