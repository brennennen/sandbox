/*
g++ ./155_min_stack.cpp && ./a.out

Design a stack that supports push, pop, top, and retrieving the minimum element 
in constant time.

Implement the `MinStack` class:
 * `MinStack()` initializes the stack object.
 * `void push(int val)` pushes the element `val` onto the stack.
 * `void pop()` removes the element on the top of the stack.
 * `int top()` gets the top element of the stack.
 * `int getMin()` retrieves the minimum element in the stack.

You must implement a solution with `O(1)` time complexity for each function.

Scratchpad:
 * To make the question harder, I limited myself to not use the standard library.
 * Use 2 stacks (memory used is O(2n)). Add to the first whenever push is called. Then
add to the the "min_stack" when the element pushed is lower than the current minimum
element.
 * Got faster than 77% and uses less memory than 72%. Looks like faster solutions
did the same thing as the below but used 'vector', which probably has some optimizations
to skip allocs and just be stack based for low sizes.
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>

class MinStack {
private:
    static const int START_CAPACITY = 16;

    int* data = nullptr;
    int data_capacity = 0;
    int count = 0;

    int *min_data = nullptr;
    int min_data_capacity = 0;
    int min_count = 0;
    

    void min_push(int val) {
        if (min_count >= min_data_capacity) {
            min_resize();
        }
        min_data[min_count] = val;
        min_count += 1;
    }

    int min_pop() {
        if (count <= 0)
            return -1;
        min_count -= 1;
        return min_data[min_count];
    }

    void min_resize() {
        int old_min_capacity = min_data_capacity;
        min_data_capacity = min_data_capacity * 2;
        int* new_min_data = (int*) malloc((sizeof(int) * min_data_capacity));
        memcpy(new_min_data, min_data, (old_min_capacity * sizeof(int)));
        free(min_data);
        min_data = new_min_data;
    }

    int min_get_top() {
        printf("min_get_top: %d (min_count: %d)\n", min_data[min_count - 1], min_count);
        return min_data[min_count - 1];
    }

    void min_print() {
        printf("MinStack.min (count: %d, capacity: %d): ", 
            min_count, min_data_capacity);
        for (int i = 0; i < min_count; i++) {
            printf("%d, ", min_data[i]);
        }
        printf("\n");
    }

    void resize() {
        int old_capacity = data_capacity;
        data_capacity = data_capacity * 2;
        int* new_data = (int*) malloc((sizeof(int) * data_capacity));
        memcpy(new_data, data, (old_capacity * sizeof(int)));
        free(data);
        data = new_data;
    }

public:

    MinStack() {
        data = (int*) malloc((sizeof(int) * START_CAPACITY));
        data_capacity = START_CAPACITY;
        count = 0;

        min_data = (int*) malloc((sizeof(int) * START_CAPACITY));
        min_data_capacity = START_CAPACITY;
        min_count = 0;
    }

    void push(int val) {
        if (count >= data_capacity) {
            resize();
        }
        if (count == 0 || val <= min_get_top()) {
            min_push(val);
        }
        data[count] = val;
        count += 1;
    }

    void pop() {
        int data_top = top();
        if (data_top == min_get_top()) {
            min_pop();
        }
        count -= 1;
    }

    int top() {
        if (count == 0)
            return -1;
        return data[count - 1];
    }

    int getMin() {
        if (min_count == 0)
            return -1;
        return min_data[min_count - 1];
    }

    void print() {
        printf("MinStack (count: %d, capacity: %d, min: %d): ", 
            count, data_capacity, getMin());
        for (int i = 0; i < count; i++) {
            printf("%d, ", data[i]);
        }
        printf("\n");
        min_print();
    }
};


int main(int argc, char* argv[]) {
    printf("Entering 155_min_stack test...\n");

    MinStack min_stack = MinStack();
    min_stack.push(10);
    min_stack.push(20);
    min_stack.push(5);
    min_stack.push(30);
    min_stack.print();
    min_stack.pop();
    min_stack.print();
    min_stack.pop();
    min_stack.print();

    printf("Exiting 155_min_stack test...\n");
    return 1;
}
