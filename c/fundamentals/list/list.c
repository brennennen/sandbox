

#include <stdio.h>

#include "list.h"

void list_initialize(list_t* list)
{
    list->count = 0;
}

void list_append(list_t* list, node_t* node)
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

node_t* list_pop(list_t* list)
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
