
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

void* node_alloc()
{
    return arena_alloc(&node_arena);
}

/* Tests */
void test__push()
{
    printf("%s\n", __func__);
    // Arrange
    arena_setup(&node_arena, sizeof(node_t), 100);
    arena_setup(&data_arena, sizeof(uint64_t), 100); // could be any type.
    uint64_t* data1 = arena_alloc(&data_arena);
    uint64_t* data2 = arena_alloc(&data_arena);
    uint64_t* data3 = arena_alloc(&data_arena);
    *data1 = 12;
    *data2 = 34;
    *data3 = 56;
    list_t list = { 0 };
    list_initialize(&list, &node_alloc);
    // Act
    list_push(&list, data1);
    // Assert
    assert(*((uint64_t*)list.head->data) == 12);
    assert(*((uint64_t*)list.head->data) == *((uint64_t*)list.tail->data));
    assert(list.head->previous == NULL);
    assert(list.head->next == NULL);
    assert(list.tail->previous == NULL);
    assert(list.tail->next == NULL);
    assert(list.count == 1);
    // Act
    list_push(&list, data2);
    // Assert
    assert(*((uint64_t*)list.head->data) == 12);
    assert(*((uint64_t*)list.tail->data) == 34);
    assert(*((uint64_t*)list.head->next->data) == *((uint64_t*)list.tail->data));
    assert(*((uint64_t*)list.head->data) == *((uint64_t*)list.tail->previous->data));
    assert(list.head->previous == NULL);
    assert(list.tail->next == NULL);
    assert(list.count == 2);
    // Act
    list_push(&list, data3);
    // Assert
    print_list_uint64(&list);
    assert(*((uint64_t*)list.head->data) == 12);
    assert(*((uint64_t*)list.head->next->data) == 34);
    assert(*((uint64_t*)list.head->next->next->data) == 56);
    assert(*((uint64_t*)list.tail->data) == 56);
    assert(*((uint64_t*)list.tail->previous->data) == 34);
    assert(*((uint64_t*)list.tail->previous->previous->data) == 12);
    assert(list.head->previous == NULL);
    assert(list.tail->next == NULL);
    assert(list.count == 3);
}

void test__pop()
{
    printf("%s\n", __func__);
    // Arrange
    arena_setup(&node_arena, sizeof(node_t), 100);
    arena_setup(&data_arena, sizeof(uint64_t), 100); // could be any type.
    uint64_t* data1 = arena_alloc(&data_arena);
    uint64_t* data2 = arena_alloc(&data_arena);
    uint64_t* data3 = arena_alloc(&data_arena);
    *data1 = 12;
    *data2 = 34;
    *data3 = 56;
    list_t list = { 0 };
    list_initialize(&list, &node_alloc);
    list_push(&list, data1);
    list_push(&list, data2);
    list_push(&list, data3);
    // Act
    uint64_t* data1_popped = list_pop(&list);
    // Assert
    printf("data1_popped: %ld\n", *data1_popped);
    assert(*data1_popped == 56);
    assert(list.count == 2);
    // Act
    uint64_t* data2_popped = list_pop(&list);
    // Assert
    printf("data2_popped: %ld\n", *data2_popped);
    assert(*data2_popped == 34);
    assert(list.count == 1);
    // Act
    uint64_t* data3_popped = list_pop(&list);
    // assert
    printf("data3_popped: %ld\n", *data3_popped);
    assert(*data3_popped == 12);
    assert(list.count == 0);
}

void test__push_front()
{
    printf("%s\n", __func__);
    // Arrange
    arena_setup(&node_arena, sizeof(node_t), 100);
    arena_setup(&data_arena, sizeof(uint64_t), 100); // could be any type.
    uint64_t* data1 = arena_alloc(&data_arena);
    uint64_t* data2 = arena_alloc(&data_arena);
    uint64_t* data3 = arena_alloc(&data_arena);
    *data1 = 12;
    *data2 = 34;
    *data3 = 56;
    list_t list = { 0 };
    list_initialize(&list, &node_alloc);
    // Act
    list_push_front(&list, data1);
    // Assert
    assert(*((uint64_t*)list.head->data) == 12);
    assert(*((uint64_t*)list.head->data) == *((uint64_t*)list.tail->data));
    assert(list.head->previous == NULL);
    assert(list.head->next == NULL);
    assert(list.tail->previous == NULL);
    assert(list.tail->next == NULL);
    assert(list.count == 1);
    // Act
    list_push_front(&list, data2);
    // Assert
    assert(*((uint64_t*)list.head->data) == 34);
    assert(*((uint64_t*)list.tail->data) == 12);
    assert(*((uint64_t*)list.head->next->data) == *((uint64_t*)list.tail->data));
    assert(*((uint64_t*)list.tail->previous->data) == *((uint64_t*)list.head->data));
    assert(list.head->previous == NULL);
    assert(list.tail->next == NULL);
    assert(list.count == 2);
    // Act
    list_push_front(&list, data3);
    // Assert
    print_list_uint64(&list);
    assert(*((uint64_t*)list.head->data) == 56);
    assert(*((uint64_t*)list.head->next->data) == 34);
    assert(*((uint64_t*)list.tail->data) == 12);
    assert(*((uint64_t*)list.head->next->next->data) == *((uint64_t*)list.tail->data));
    assert(*((uint64_t*)list.tail->previous->previous->data) == *((uint64_t*)list.head->data));
    assert(list.head->previous == NULL);
    assert(list.tail->next == NULL);
    assert(list.count == 3);
}

void test__dequeue()
{
    printf("%s\n", __func__);
    // Arrange
    arena_setup(&node_arena, sizeof(node_t), 100);
    arena_setup(&data_arena, sizeof(uint64_t), 100); // could be any type.
    uint64_t* data1 = arena_alloc(&data_arena);
    uint64_t* data2 = arena_alloc(&data_arena);
    uint64_t* data3 = arena_alloc(&data_arena);
    *data1 = 12;
    *data2 = 34;
    *data3 = 56;
    list_t list = { 0 };
    list_initialize(&list, &node_alloc);
    list_enqueue(&list, data1);
    list_enqueue(&list, data2);
    list_enqueue(&list, data3);
    // Act
    uint64_t* data1_dequeued = list_dequeue(&list);
    // Assert
    printf("data1_dequeued: %ld\n", *data1_dequeued);
    assert(*data1_dequeued == 12);
    assert(list.count == 2);
    // Act
    uint64_t* data2_dequeued = list_dequeue(&list);
    // Assert
    printf("data2_dequeued: %ld\n", *data2_dequeued);
    assert(*data2_dequeued == 34);
    assert(list.count == 1);
    // Act
    uint64_t* data3_dequeued = list_dequeue(&list);
    // Assert
    printf("data3_dequeued: %ld\n", *data3_dequeued);
    assert(*data3_dequeued == 56);
    assert(list.count == 0);
}

/* Main */
int main(int argc, char* argv[])
{
    printf("starting linked list tests\n");
    test__push();
    test__pop();
    test__push_front();
    test__dequeue();
    printf("done with linked list tests\n");
}