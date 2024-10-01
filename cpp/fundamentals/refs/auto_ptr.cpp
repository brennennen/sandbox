
/*
g++ -std=c++03 ./auto_ptr.cpp

NOTE: auto_ptr is deprecated, and if you're using c++11 or greater, you should use unique_ptr.

'auto_ptr' was designed to be a way to safely allocate an object, use it like a normal
pointer, and have peace of mind it would be de-allocated/freed when it is done being used,
even if control flow was interrupted with an exception.

With a standard pointer, you need to manually call free. If an exception is raised before
free is called, the memory allocated to that pointer will never get freed.

Below is a small test program to demo a traditional pointer vs the 'auto_ptr'. Below 
both types of pointers are allocated, and there is an explicit line to free the
traditional pointer, however, an exception is thrown before this line is reached. The
auto_ptr's destructor is properly called and is freed when this happens. The traditional
pointer however is not freed and the memory it consumed is never released back to the 
OS (memory leak). Valgrind can be used to detect the memory leak (valgrind ./a.out).
*/

#include <cstdio>
#include <memory>
#include <stdexcept>

using namespace std;

struct my_struct_t {
    int foo;
    int bar;
    int baz;
};

void do_work(my_struct_t* a, auto_ptr<my_struct_t> b) {
    // ...
    if (a->foo == b->foo) {
        throw std::runtime_error("arbitrary test exception");
    }
    // ...
}

void foo_bar() {
    my_struct_t* my_var = new my_struct_t;
    auto_ptr<my_struct_t> my_auto_var(new my_struct_t);

    my_var->foo = 5;
    my_auto_var->foo = 5;

    do_work(my_var, my_auto_var); // May raise exception?

    printf("Deleting my_var! (not called if an exception is raised in 'do_work')\n");
    delete my_var;
}

int main(int argc, char* argv[]) {
    printf("Starting auto_ptr sandbox main...\n");
    try {
        foo_bar();
    } catch (const std::runtime_error &e) {
        printf("got exception: '%s' before my_var and my_auto_var were freed! oh no!\n", e.what());
    }
    printf("Use valgrind to see if any memory was leaked!\n");
    printf("Exiting auto_ptr sandbox main...\n");
}
