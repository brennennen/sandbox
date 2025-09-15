/**
 * In a non-trivial system, it is hard to figure out which exact workers and shared resources
 * have a "circular wait" problem. Most problems that involve locks in the spaces I've worked
 * can be re-formatted to use lock-free solutions like atomic CAS queues or "mailbox" style messaging tools for example. "Communicate to share, don't communicate by sharing."
 *
 * An example using lock-free atomic queues to avoid the deadlock/livelock problems without
 * complicated lock ordering schemas that are easy to make mistakes with. CAS - compare and
 * swap with atomics. potential issues: aba problem.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h> // c11
#include <unistd.h>


typedef struct node_s{
    int value;
    _Atomic(struct node_s*) next;
} node_t;

typedef struct {
    _Atomic(node_t*) head;
    _Atomic(node_t*) tail;
} lf_queue_t;

lf_queue_t* create_lf_queue() {
    lf_queue_t* queue = (lf_queue_t*)malloc(sizeof(lf_queue_t));
    node_t* dummy_node = (node_t*)malloc(sizeof(node_t));
    dummy_node->next = NULL;
    atomic_init(&queue->head, dummy_node);
    atomic_init(&queue->tail, dummy_node);
    return(queue);
}

void enqueue(lf_queue_t* queue, int value) {
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    new_node->value = value;
    atomic_init(&new_node->next, NULL);

    node_t* last;
    node_t* next;

    while(1) {
        last = atomic_load(&queue->tail);
        next = atomic_load(&last->next);
        if (last == atomic_load(&queue->tail)) {
            if (next == NULL) {
                if (atomic_compare_exchange_weak(&last->next, &next, new_node)) {
                    break;
                }
            } else {
                atomic_compare_exchange_weak(&queue->tail, &last, next);
            }
        }
    }
    atomic_compare_exchange_weak(&queue->tail, &last, new_node);
}

int dequeue(lf_queue_t* queue, int* value) {
    node_t* first;
    node_t* last;
    node_t* next;

    while(1) {
        first = atomic_load(&queue->head);
        last = atomic_load(&queue->tail);
        next = atomic_load(&first->next);

        if (first == atomic_load(&queue->head)) {
            if (first == last) {
                if (next == NULL) {
                    return(0); // queue is empty
                }
                atomic_compare_exchange_weak(&queue->tail, &last, next);
            } else {
                *value = next->value;
                if (atomic_compare_exchange_weak(&queue->head, &first, next)) {
                    break;
                }
            }
        }
    }
    free(first);
    return(1);
}

lf_queue_t* queue1;
lf_queue_t* queue2;
lf_queue_t* queue3;

void* worker1(void* arg) {
    int item;
    printf("%s: started\n", __func__);
    while(1) {
        if (dequeue(queue1, &item)) {
            enqueue(queue2, item);
            printf("%s: moved %d from queue1 to queue2\n", __func__, item);
        } else {
            printf("%s: queue1 is empty\n", __func__);
        }
        sleep(1);
    }
    return(NULL);
}

void* worker2(void* arg) {
    int item;
    printf("%s: started\n", __func__);
    while(1) {
        if (dequeue(queue2, &item)) {
            enqueue(queue3, item);
            printf("%s: moved %d from queue2 to queue3\n", __func__, item);
        } else {
            printf("%s: queue2 is empty\n", __func__);
        }
        sleep(1);
    }
    return(NULL);
}

void* worker3(void* arg) {
    int item;
    printf("%s: started\n", __func__);
    while(1) {
        if (dequeue(queue3, &item)) {
            enqueue(queue1, item);
            printf("%s: moved %d from queue3 to queue1\n", __func__, item);
        } else {
            printf("%s: queue3 is empty\n", __func__);
        }
        sleep(1);
    }
    return(NULL);
}

int main(int argc, char* argv[]) {
    pthread_t t1;
    pthread_t t2;
    pthread_t t3;

    queue1 = create_lf_queue();
    queue2 = create_lf_queue();
    queue3 = create_lf_queue();

    enqueue(queue1, 111);
    enqueue(queue2, 222);
    enqueue(queue3, 333);

    pthread_create(&t1, NULL, worker1, NULL);
    pthread_create(&t2, NULL, worker2, NULL);
    pthread_create(&t3, NULL, worker3, NULL);
    printf("all workers started\n");

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    // never reached, deadlocked before any joins occur
    printf("clean up\n");
    free(queue1);
    free(queue2);
    free(queue3);
    return 0;
}
