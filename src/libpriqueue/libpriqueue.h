/** @file libpriqueue.h
 */

#ifndef LIBPRIQUEUE_H_
#define LIBPRIQUEUE_H_


// Priority Node - What we will insert into the Priority Queue
typedef struct Node
{
    void* job;
    struct Node* next;
} node_t;

/**
  Priqueue Data Structure
  - Takes a Prioriy Node as defined above for its front pointer.
*/
typedef struct _priqueue_t
{
    int size;
    int (*comparer) (const void *, const void *)
    struct Node* front;
} priqueue_t;

struct Node* createNode(void* job);


void   priqueue_init     (priqueue_t *q, int(*comparer)(const void *, const void *));

int    priqueue_offer    (priqueue_t *q, void *ptr);
void * priqueue_peek     (priqueue_t *q);
void * priqueue_poll     (priqueue_t *q);
void * priqueue_at       (priqueue_t *q, int index);
int    priqueue_remove   (priqueue_t *q, void *ptr);
void * priqueue_remove_at(priqueue_t *q, int index);
int    priqueue_size     (priqueue_t *q);

void   priqueue_destroy  (priqueue_t *q);
void   destroy_recurse(struct Node* current);

#endif /* LIBPQUEUE_H_ */
