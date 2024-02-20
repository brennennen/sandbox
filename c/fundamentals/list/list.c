

#include <stdio.h>

#include "list.h"

void list_initialize(list_t* list, list_node_alloc_callback_t node_alloc_cb)
{
    list->count = 0;
    list->node_alloc_callback = node_alloc_cb;
}

void list_push(list_t* list, void* data)
{
    node_t* new_node = (*list->node_alloc_callback)();
    new_node->data = data;
    list_push_node(list, new_node);
}

void list_push_node(list_t* list, node_t* node)
{
    if (list->head == NULL)
    {
        list->head = node;
        list->tail = node;
    }
    else if (list->head == list->tail)
    {
        list->head->next = node;
        list->tail = node;
        node->previous = list->head;
    }
    else
    {
        node->previous = list->tail;
        node->previous->next = node;
        list->tail = node;
    }
    list->count += 1;
}

void* list_pop(list_t* list)
{
    node_t* popped_node = list_pop_node(list);
    if (popped_node == NULL)
        return NULL;
    return popped_node->data;
}

node_t* list_pop_node(list_t* list)
{
    // If the list is empty, nothing to pop
    if (list->head == NULL)
        return NULL;
    node_t* result = list->tail;
    // If the count is 0, clear the head and tail pointers
    list->count -= 1;
    if (list->count == 0)
    {
        list->head = NULL;
        list->tail = NULL;
    }
    // If the tail is now the head, clean up next/prev pointers
    else if (list->count == 1)
    {
        list->tail = list->head;
        list->tail->next = NULL;
        list->tail->previous = NULL;
    }
    // For all other scenarios, just move the tail back 1 entry
    else
    {
        list->tail = list->tail->previous;
        list->tail->next = NULL;
    }
    // Clear the popped elements list pointers
    result->next = NULL;
    result->previous = NULL;
    return result;
}

void list_push_front(list_t* list, void* data)
{
    node_t* new_node = (*list->node_alloc_callback)();
    new_node->data = data;
    list_push_front_node(list, new_node);
}

void list_push_front_node(list_t* list, node_t* node)
{
    if (list->head == NULL)
    {
        list->head = node;
        list->tail = node;
    }
    else if (list->head == list->tail)
    {
        list->tail->previous = node;
        list->head = node;
        node->next = list->tail;
    }
    else
    {
        node->next = list->head;
        node->next->previous = node;
        list->head = node;
    }
    list->count += 1;
}

void list_enqueue(list_t* list, void* data)
{
    list_push(list, data);
}

void list_enqueue_node(list_t* list, node_t* node)
{
    list_push_node(list, node);
}

void* list_dequeue(list_t* list)
{
    node_t* dequeued_node = list_dequeue_node(list);
    if (dequeued_node == NULL)
        return NULL;
    return dequeued_node->data;
}

node_t* list_dequeue_node(list_t* list)
{
    if (list->head == NULL)
        return NULL;
    node_t* result = list->head;
    // If the count is 0, clear the head and tail pointers
    list->count -= 1;
    if (list->count == 0)
    {
        list->head = NULL;
        list->tail = NULL;
    }
    // If the head is now the tail, clean up next/prev pointers
    else if (list->count == 1)
    {
        list->head = list->tail;
        list->tail->next = NULL;
        list->tail->previous = NULL;
    }
    // For all other scenarios, just move the head back 1 entry
    else
    {
        list->head = list->head->next;
        list->head->previous = NULL;
    }
    // Clear the popped elements list pointers
    result->next = NULL;
    result->previous = NULL;
    return result;
}
