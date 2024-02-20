/**
 * Memory agnostic toy linked list (does not allocate memory for entries). 
 */
#ifndef LIST_H
#define LIST_H


#include <stdint.h>
#include <stdbool.h>

typedef void* (*list_node_alloc_callback_t)(void);
typedef bool (*list_data_compare_callback_t)(void* data1, void* data2);

typedef struct node_s {
    void* data;
    struct node_s* next;
    struct node_s* previous;
} node_t;

typedef struct list_s {
    node_t* head;
    node_t* tail;
    uint32_t count;
    list_node_alloc_callback_t node_alloc_callback;
    list_data_compare_callback_t compare_callback;
} list_t;

void list_initialize(list_t* list, list_node_alloc_callback_t node_alloc);
void list_push(list_t* list, void* data);
void list_push_node(list_t* list, node_t* node);
void* list_pop(list_t* list);
node_t* list_pop_node(list_t* list);

void list_push_front(list_t* list, void* data);
void list_push_front_node(list_t* list, node_t* node);
void list_push_front(list_t* list, void* data);
void list_enqueue(list_t* list, void* data);
void list_enqueue_node(list_t* list, node_t* node);
void* list_dequeue(list_t* list);
node_t* list_dequeue_node(list_t* list);

#endif // LIST_H
