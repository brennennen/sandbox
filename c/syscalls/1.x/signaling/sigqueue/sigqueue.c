/*
gcc ./sigqueue.c && ./a.out

"sigqueue" sends a signal to a process like 'kill', but can also send some data as
an argument with the signal. The user defined data shared is an integer or a pointer.
Along with the user defined data is a suite of various system info such as the process
that raised the signal, the time the signal was raised, any errors that are assoicated
with the signal, etc.

I don't see a use case for a process to signal itself in a non-multi-threaded context. 
When passing a pointer through signalling in a multi-threaded context, you'll need to 
use some form of shared memory or shared static addresses.

Below is a simple toy program to experiment with using 'sigqueue' to communicate 
and send some additional data between 2 processes. The parent process sets up some
shared memory, spawns the child process, and configures a signal handler for 'SIGUSR1'.
The child process sends 'SIGUSR1' with some data and exits.
*/

#include <errno.h> // errno
#include <signal.h> // sigqueue, sigaction, siginfo_t, sigval, etc.
#include <stdio.h> // printf
#include <stdlib.h> // exit
#include <string.h> // strerror
#include <unistd.h> // getpid()
#include <sys/wait.h> // wait()
#include <sys/mman.h> // mmap()

typedef struct sigusr1_data_s {
    int my_num;
    char my_string[64];
} sigusr1_data_t;

void parent_sigusr1_handler(int signo, siginfo_t *info, void *context) {
    printf("received SIGUSR1!\n");
    sigusr1_data_t* my_data = (sigusr1_data_t*)info->si_value.sival_ptr;
    printf("my_data: my_num: %d, my_string: %s\n", my_data->my_num, my_data->my_string);
    // Not all fields are defined or useful for all signals.
    printf("other siginfo:\n" \
"   signo: %d,\n" \
"   errno: %d,\n" \
"   code: %d, \n" \
"   pid: %d, \n" \
"   uid: %d, \n" \
"   status: %d, \n" \
"   utime: %ld, \n" \
"   stime: %ld, \n" \
"   value.int: %d, \n" \
"   value.ptr: %p, \n" \
"   int: %d, \n" \
"   ptr: %p, \n" \
"   overrun: %d, \n" \
"   timerid: %d, \n" \
"   addr: %p, \n" \
"   band: %ld, \n" \
"   fd: %d, \n",
        info->si_signo, info->si_errno, info->si_code, info->si_pid, info->si_uid,
        info->si_status, info->si_utime, info->si_stime, info->si_value.sival_int,
        info->si_value.sival_ptr, info->si_int, info->si_ptr, info->si_overrun,
        info->si_timerid, info->si_addr, info->si_band, info->si_fd);
}

int main(int argc, char* argv[]) {
    printf("Starting 'sigqueue' sandbox...\n");
    pid_t parent_pid = getpid();
    // Sigqueue sends a pointer/address. Using shared memory we can make sure the pointer can
    // be accessed on both the child and the parent.
    void* shared_memory = mmap(NULL, sizeof(sigusr1_data_t), (PROT_READ | PROT_WRITE), 
        (MAP_SHARED | MAP_ANONYMOUS), -1, 0);
    sigusr1_data_t* sigusr1_data = (sigusr1_data_t*) shared_memory;

    pid_t child_pid = fork(); // child is spawned and starts executing from this point onward.
    if (!child_pid) {
        memset(sigusr1_data, 0, sizeof(sigusr1_data));
        sigusr1_data->my_num = 42;
        strncpy(sigusr1_data->my_string, "the answer to the universe", 64);
        printf("child sending 'SIGUSR1' with: my_num: %d, my_string: '%s'\n", 
            sigusr1_data->my_num, sigusr1_data->my_string);
        union sigval sigusr_value;
        sigusr_value.sival_ptr = sigusr1_data;
        sigqueue(parent_pid, SIGUSR1, sigusr_value);
        exit(0); // exit will fire the associated signal to notify the parent to stop waiting.
    }
    else {
        struct sigaction new_sigusr1_action;
        sigemptyset(&new_sigusr1_action.sa_mask);
        // Setting 'SA_SIGINFO' allows the handler to access the additional information 
        // sent in sigqueue.
        new_sigusr1_action.sa_flags = SA_SIGINFO; 
        // Note that 'sa_sigaction' is used instead of 'sa_handler' for the callback when
        // 'SA_SIGINFO' is set (it has a different function prototype as well).
        new_sigusr1_action.sa_sigaction = parent_sigusr1_handler;
        sigaction(SIGUSR1, &new_sigusr1_action, NULL);
        wait(NULL); // Wait until the child exits/returns
    }
    printf("Exiting 'sigqueue' sandbox...\n");
    return 0;
}
