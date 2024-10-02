/*
g++ ./int_array_list.cpp && ./a.out

Simple dynamically growable int array list implementation.
*/


#include <cstdio>
#include <cstdlib>
#include <cstring>

class IntArrayList {

public:
    int count;
    int* data;
    int data_capacity;

    IntArrayList(int capacity) {
        count = 0;
        data = (int*) malloc((sizeof(int) * capacity));
        data_capacity = capacity;
    }

    int get(int i) {
        if (i >= data_capacity) {
            return -1;
        }
        return data[i];
    }

    void set(int i, int n) {
        if (i >= data_capacity) {
            return;
        }
        data[i] = n;
    }

    void push(int n) {
        if (count >= data_capacity) {
            resize();
        }
        data[count] = n;
        count += 1;
    }

    int pop() {
        if (count <= 0)
            return -1;
        count -= 1;
        return data[count];
    }

    void resize() {
        int old_capacity = data_capacity;
        data_capacity = data_capacity * 2;
        int* new_data = (int*) malloc((sizeof(int) * data_capacity));
        memcpy(new_data, data, (old_capacity * sizeof(int)));
        free(data);
        data = new_data;
    }

    int getSize() {
        return count;
    }

    int getCapacity() {
        return data_capacity;
    }

    void print() {
        printf("IntArrayList (count: %d, capacity: %d): ", count, data_capacity);
        for (int i = 0; i < count; i++) {
            printf("%d, ", data[i]);
        }
        printf("\n");
    }
};


int main(int argc, char* argv[]) {
    printf("Entering IntArrayList test...\n");
    IntArrayList array = IntArrayList(5);
    array.push(1);
    array.print();
    array.push(2);
    array.push(3);
    array.push(4);
    array.push(5);
    array.set(2, -3);
    array.print();
    array.push(6);
    array.print();
    int six = array.pop();
    printf("six: %d\n", six);
    array.print();
    array.pop();
    array.pop();
    array.pop();
    array.pop();
    array.pop();
    array.print();
    printf("Exiting IntArrayList test...\n");
}
