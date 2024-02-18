/**
 * Memory agnostic toy linked list (does not allocate memory for entries). 
 */
#ifndef LIST_H
#define LIST_H


#include <stdint.h>


typedef struct node_s {
    void* data;
    struct node_s* next;
    struct node_s* previous;
} node_t;

typedef struct list_s {
    node_t* head;
    node_t* tail;
    uint32_t count;
} list_t;

void list_initialize(list_t* list);
void list_append(list_t* list, node_t* data);
node_t* list_pop(list_t* list);


#endif // LIST_H
