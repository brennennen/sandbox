/**
 * 3 queues (shared resources with a lock each)
 * 3 workers
 *
 * Each worker reads data from a queue, does some work to it, then writes data
 * to the next queue.
 *
 * Worker1: consume queue1, do work, produce to queue2
 * Worker2: consume queue2, do work, produce to queue3
 * Worker3: consume queue3, do work, produce to queue1
 *
 * The workers try to consume/produce data to the queues. Doing this naively
 * taking a lock when a resource is needed without any planning or schema
 * results in a deadlock. Each worker h
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define QUEUE_CAPACITY 32

typedef struct {
    int items[QUEUE_CAPACITY];
    int head;
    int tail;
    int size;
} queue_t;

queue_t* create_queue() {
    queue_t* queue = (queue_t*)malloc(sizeof(queue_t));
    queue->head = 0;
    queue->tail = -1;
    queue->size = 0;
    return(queue);
}

int enqueue(queue_t* queue, int item) {
    if (queue->size == QUEUE_CAPACITY) {
        return(0);
        // queue is full
    }
    queue->tail = (queue->tail + 1) % QUEUE_CAPACITY;
    queue->items[queue->tail] = item;
    queue->size++;
    return(1);
}

int dequeue(queue_t* queue, int* item) {
    if (queue->size == 0) {
        return(0);
        // queue is empty
    }
    *item = queue->items[queue->head];
    queue->head = (queue->head + 1) % QUEUE_CAPACITY;
    queue->size--;
    return(1);
}


queue_t *queue1;
queue_t *queue2;
queue_t *queue3;

pthread_mutex_t lock1;
pthread_mutex_t lock2;
pthread_mutex_t lock3;

void *worker1(void *arg) {
    int temp;
    printf("%s: started\n", __func__);
    while(1) {
        printf("%s: attempt aquire lock1\n", __func__);
        pthread_mutex_lock(&lock1);
        printf("%s: aquired lock1\n", __func__);

        usleep(100 * 1000); // make the deadlock easier to hit

        printf("%s: attempt aquire lock2\n", __func__);
        pthread_mutex_lock(&lock2);
        printf("%s: aquired lock2\n", __func__);

        if (dequeue(queue1, &temp)) {
            enqueue(queue2, temp);
            printf("%s: moved %d from queue1 to queue2\n", __func__, temp);
        } else {
            printf("%s: queue1 is empty\n", __func__);
        }

        pthread_mutex_unlock(&lock1);
        pthread_mutex_unlock(&lock2);
        sleep(1);
    }
    return(NULL);
}

void *worker2(void *arg) {
    int temp;
    printf("%s: started\n", __func__);
    while(1) {
        printf("%s: attempt aquire lock2\n", __func__);
        pthread_mutex_lock(&lock2);
        printf("%s: aquired lock2\n", __func__);

        usleep(100 * 1000); // make the deadlock easier to hit

        printf("%s: attempt aquire lock3\n", __func__);
        pthread_mutex_lock(&lock3);
        printf("%s: aquired lock3\n", __func__);

        if (dequeue(queue2, &temp)) {
            enqueue(queue3, temp);
            printf("%s: moved %d from queue2 to queue3\n", __func__, temp);
        } else {
            printf("%s: queue2 is empty\n", __func__);
        }

        pthread_mutex_unlock(&lock2);
        pthread_mutex_unlock(&lock3);
        sleep(1);
    }
    return(NULL);
}

void *worker3(void *arg) {
    int temp;
    printf("%s: started\n", __func__);
    while(1) {
        printf("%s: attempt aquire lock3\n", __func__);
        pthread_mutex_lock(&lock1);
        printf("%s: aquired lock3\n", __func__);

        usleep(100 * 1000); // make the deadlock easier to hit

        printf("%s: attempt aquire lock1\n", __func__);
        pthread_mutex_lock(&lock3);
        printf("%s: aquired lock1\n", __func__);

        if (dequeue(queue3, &temp)) {
            enqueue(queue1, temp);
            printf("%s: moved %d from queue3 to queue1\n", __func__, temp);
        } else {
            printf("%s: queue3 is empty\n", __func__);
        }

        pthread_mutex_unlock(&lock1);
        pthread_mutex_unlock(&lock3);
        sleep(1);
    }
    return(NULL);
}

int main(int argc, char* argv[]) {
    pthread_t t1;
    pthread_t t2;
    pthread_t t3;

    pthread_mutex_init(&lock1, NULL);
    pthread_mutex_init(&lock2, NULL);
    pthread_mutex_init(&lock3, NULL);

    queue1 = create_queue();
    queue2 = create_queue();
    queue3 = create_queue();
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

    printf("clean up\n");
    free(queue1);
    free(queue2);
    free(queue3);
    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);
    pthread_mutex_destroy(&lock3);
    return 0;
}
