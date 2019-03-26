/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"


struct Node* createNode(void* job) {
	struct Node* newNode;
	newNode->job = job;
	newNode->next = NULL;
	return newNode;
}


/**
  Initializes the priqueue_t data structure.

  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
	q->size = 0;
	q->comparer = comparer;
	q->front = NULL;			//Set front pointer and comparer-function attributes of the priqueue
}


/**
  Inserts the specified element into this priority queue
  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
	struct Node* current = q->front;
	struct Node* newNode = createNode(ptr);

	if (q->size == 0) { 	//Empty case
		q->front = newNode;
		q->size++;
		return 0;
	}

	//Find the insertion point
	int index = 0;
	for (; q->comparer(newNode,current) > 0; index++) current = current->next;

	if (q->comparer(newNode,current) <= 0) {	//New node is highest priority, insert front
		struct Node* temp = current;
		current = newNode;
		newNode->next = temp;
	} else {	//Equivalence case - determine tiebreaker
		//TODO: check case
	}

	return index; //Return the position that was inserted to.
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
	return q->front;
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
	return priqueue_remove_at(q, 0)->job;
}



/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
	if(size < index) return NULL; // index does not exist

	struct Node* nodeAt = q->front;
	int step = 0;
	for(; step < index; step++) nodeAt = nodeAt->next; //step through nodes to index

	return nodeAt;
}


/**
  Removes all instances of ptr from the queue.

  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
	int removed = 0;
	for(; removed < q->size; removed++) priqueue_remove_at(q, removed);

	return removed;
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
	if(size < index) return NULL; //index does not exist

	struct Node* nodeBefore = priqueue_at(q, (index-1));
	struct Node* nodeAt = priqueue_at(q, (index));
	nodeBefore->next = nodeAt->next;


	return 0;
}


/**
  Returns the number of elements in the queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return q->size;
}


/**
  Destroys and frees all the memory associated with q.

  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{
	destroy_recurse(q->front)
}

void destroy_recurse(struct Node* current)
{
	if (current->next) destroy_recurse(current->next);
	free(current);
}
