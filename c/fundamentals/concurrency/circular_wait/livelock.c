/**
 * An example of a system with 3 workers and 3 shared resources, each worker needing 2 specific
 * shared resources and a naive concurrency implementation that causes starvation using mutexes
 * with non-blocking aquires.
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
pthread_mutex_t queue1_lock;
queue_t *queue2;
pthread_mutex_t queue2_lock;
queue_t *queue3;
pthread_mutex_t queue3_lock;

void *worker1(void *arg) {
    int temp;
    printf("%s: started\n", __func__);
    while(1) {
        printf("%s: attempt aquire queue1_lock\n", __func__);
        if (pthread_mutex_trylock(&queue1_lock) == 0) {
            printf("%s: aquired queue1_lock\n", __func__);

            usleep(100 * 1000); // make the deadlock easier to hit

            printf("%s: attempt aquire queue2_lock\n", __func__);
            if (pthread_mutex_trylock(&queue2_lock) == 0) {
                printf("%s: aquired queue2_lock\n", __func__);
                if (dequeue(queue1, &temp)) {
                    enqueue(queue2, temp);
                    printf("%s: moved %d from queue1 to queue2\n", __func__, temp);
                } else {
                    printf("%s: queue1 is empty\n", __func__);
                }
                pthread_mutex_unlock(&queue1_lock);
                pthread_mutex_unlock(&queue2_lock);
            } else {
                pthread_mutex_unlock(&queue1_lock);
            }
        }
        sleep(1);
    }
    return(NULL);
}

void *worker2(void *arg) {
    int temp;
    printf("%s: started\n", __func__);
    while(1) {
        printf("%s: attempt aquire queue2_lock\n", __func__);
        if (pthread_mutex_trylock(&queue2_lock) == 0) {
            printf("%s: aquired queue2_lock\n", __func__);

            usleep(100 * 1000); // make the deadlock easier to hit

            printf("%s: attempt aquire queue3_lock\n", __func__);
            if (pthread_mutex_trylock(&queue3_lock) == 0) {
                printf("%s: aquired queue3_lock\n", __func__);
                if (dequeue(queue2, &temp)) {
                    enqueue(queue3, temp);
                    printf("%s: moved %d from queue2 to queue3\n", __func__, temp);
                } else {
                    printf("%s: queue2 is empty\n", __func__);
                }
                pthread_mutex_unlock(&queue2_lock);
                pthread_mutex_unlock(&queue3_lock);
            } else {
                pthread_mutex_unlock(&queue2_lock);
            }
        }
        sleep(1);
    }
    return(NULL);
}

void *worker3(void *arg) {
    int temp;
    printf("%s: started\n", __func__);
    while(1) {
        printf("%s: attempt aquire queue3_lock\n", __func__);
        if (pthread_mutex_trylock(&queue3_lock) == 0) {
            printf("%s: aquired queue3_lock\n", __func__);
            usleep(100 * 1000); // make the deadlock easier to hit
            printf("%s: attempt aquire queue1_lock\n", __func__);
            if (pthread_mutex_trylock(&queue1_lock) == 0) {
                printf("%s: aquired queue1_lock\n", __func__);
                if (dequeue(queue3, &temp)) {
                    enqueue(queue1, temp);
                    printf("%s: moved %d from queue3 to queue1\n", __func__, temp);
                } else {
                    printf("%s: queue3 is empty\n", __func__);
                }
                pthread_mutex_unlock(&queue3_lock);
                pthread_mutex_unlock(&queue1_lock);
            } else {
                pthread_mutex_unlock(&queue3_lock);
            }
        }
        sleep(1);
    }
    return(NULL);
}

int main(int argc, char* argv[]) {
    pthread_t t1;
    pthread_t t2;
    pthread_t t3;

    pthread_mutex_init(&queue1_lock, NULL);
    pthread_mutex_init(&queue2_lock, NULL);
    pthread_mutex_init(&queue3_lock, NULL);

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

    // never reached, deadlocked before any joins occur
    printf("clean up\n");
    free(queue1);
    free(queue2);
    free(queue3);
    pthread_mutex_destroy(&queue1_lock);
    pthread_mutex_destroy(&queue2_lock);
    pthread_mutex_destroy(&queue3_lock);
    return 0;
}
