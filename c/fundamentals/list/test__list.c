
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "list.h"

/* Test Utilities */
typedef struct arena_s {
    void* data;
    void* current;
    size_t element_size;
    int max_elements;
    int count;
} arena_t;

void arena_setup(arena_t* arena, size_t element_size, int max_elements) 
{
    arena->element_size = element_size;
    arena->max_elements = max_elements;
    arena->data = malloc(element_size * max_elements);
    arena->current = arena->data;
    arena->count = 0;
}

void* arena_alloc(arena_t* arena)
{
    void* current = arena->current;
    arena->current = ((char*)arena->current) + arena->element_size;
    arena->count += 1;
    return current;
}

void print_node_uint64(node_t* node, const char* prefix, const char* postfix)
{
    printf("%s%p <- %ld (%p) -> %p%s",
        prefix,
        node->previous,
        *(uint64_t*)node->data,
        node,
        node->next,
        postfix);
}

void print_list_uint64(list_t* list)
{
    printf("list (count: %d, head: %p, tail: %p):\n",
        list->count,
        list->head,
        list->tail);
    node_t* current = list->head;
    while(current != NULL)
    {
        print_node_uint64(current, "   ", "\n");
        current = current->next;
    }
}

static arena_t node_arena = { 0 };
static arena_t data_arena = { 0 };

/* Tests */
void test__append()
{
    printf("%s\n", __func__);
    // Arrange
    arena_setup(&node_arena, sizeof(node_t), 100);
    arena_setup(&data_arena, sizeof(uint64_t), 100); // could be any type.
    node_t* node1 = arena_alloc(&node_arena);
    node1->data = arena_alloc(&data_arena);
    *((uint64_t*)node1->data) = 12;
    node_t* node2 = arena_alloc(&node_arena);
    node2->data = arena_alloc(&data_arena);
    *((uint64_t*)node2->data) = 34;
    node_t* node3 = arena_alloc(&node_arena);
    node3->data = arena_alloc(&data_arena);
    *((uint64_t*)node3->data) = 56;
    list_t list = { 0 };
    list_initialize(&list);
    // Act
    list_append(&list, node1);
    list_append(&list, node2);
    list_append(&list, node3);
    // Assert
    print_list_uint64(&list);
    assert(*((uint64_t*)list.head->data) == 12);
    assert(*((uint64_t*)list.head->next->data) == 34);
    assert(*((uint64_t*)list.head->next->next->data) == 56);
    assert(*((uint64_t*)list.tail->data) == 56);
    assert(*((uint64_t*)list.tail->previous->data) == 34);
    assert(*((uint64_t*)list.tail->previous->previous->data) == 12);
    assert(list.count == 3);
}

void test__pop()
{
    printf("%s\n", __func__);
    // Arrange
    arena_setup(&node_arena, sizeof(node_t), 100);
    arena_setup(&data_arena, sizeof(uint64_t), 100); // could be any type.
    node_t* node1 = arena_alloc(&node_arena);
    node1->data = arena_alloc(&data_arena);
    *((uint64_t*)node1->data) = 1234;
    node_t* node2 = arena_alloc(&node_arena);
    node2->data = arena_alloc(&data_arena);
    *((uint64_t*)node2->data) = 5678;
    node_t* node3 = arena_alloc(&node_arena);
    node3->data = arena_alloc(&data_arena);
    *((uint64_t*)node3->data) = 9999;
    list_t list = { 0 };
    list_initialize(&list);
    list_append(&list, node1);
    list_append(&list, node2);
    list_append(&list, node3);
    print_list_uint64(&list);
    // Act
    node_t* popped_node = list_pop(&list);
    // Assert
    print_node_uint64(popped_node, "   popped: ", "\n");
    print_list_uint64(&list);
    assert(*((uint64_t*)popped_node->data) == 9999);
    assert(list.count == 2);
    // Act
    node_t* popped_node2 = list_pop(&list);
    // Assert
    print_node_uint64(popped_node2, "   popped2: ", "\n");
    print_list_uint64(&list);
    assert(*((uint64_t*)popped_node2->data) == 5678);
    assert(list.count == 1);
    // Act
    node_t* popped_node3 = list_pop(&list);
    // Assert
    print_node_uint64(popped_node3, "   popped3: ", "\n");
    print_list_uint64(&list);
    assert(*((uint64_t*)popped_node3->data) == 1234);
    assert(list.count == 0);
}

/* Main */
int main(int argc, char* argv[])
{
    printf("starting linked list tests\n");
    test__append();
    test__pop();
    printf("done with linked list tests\n");
}