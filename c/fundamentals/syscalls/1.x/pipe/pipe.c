/*
gcc ./pipe.c && ./a.out

Similar to "fork", pipe is an early iteration on sharing data to facilitate 
concurrent programming. It's usage require the processes wishing to communicate
both already have a pointer setup to the pipe. This is really only feasible with
fork, where you can create a pipe and then fork afterwards to copy the pointer to
both processes. Because of this akward usage issue, most folks recommend using more
modern constructs, such as "named pipes" or "FIFOs" instead. A "FIFO" is a just a
file system node with a special flag on it, so you can use the standard open/read/write
to interact with it.

Toy program below creates a parent and child process with a pipe between them. The child
writes a series of messages to the parent in a loop, and the parent listens in a loop. Finally
the child sends 'EXIT' and the parent and child exit.
*/

#include <stdio.h> // printf
#include <stdlib.h> // exit
#include <string.h> // strncmp
#include <sys/wait.h> // wait
#include <unistd.h> // pipe

int main(int argc, char* argv[]) {
    printf("[%d] Entering ipc main...\n", getpid());
    int pipe_fds[2] = {0};
    
    pipe(pipe_fds);

    // child is spawned and starts executing from this point onward. any memory defined before this point is shared.
    if (!fork()) {
        char test_string_template[] = "test - %d";
        char test_string_buffer[32] = { 0 };
        for (int i = 0; i < 3; i++) {
            int test_string_size = snprintf(test_string_buffer, sizeof(test_string_buffer), "test - %d", i);
            printf("   [%d] child write: '%s' (%d)\n", getpid(), test_string_buffer, test_string_size);
            write(pipe_fds[1], test_string_buffer, test_string_size);
            sleep(1);
        }
        printf("   [%d] child write: %s\n", getpid(), "EXIT");
        write(pipe_fds[1], "EXIT", sizeof("EXIT"));
        exit(0); // exit will fire the associated signal to notify the parent to stop waiting.
    }
    else {
        char read_buffer[32] = {0};
        while(1) {
            int read_size = read(pipe_fds[0], read_buffer, 32); // blocking call! waits until data is available to read!
            read_buffer[32] = '\0';
            printf("   [%d] parent read: '%s' (%d)\n", getpid(), read_buffer, read_size);
            if (strncmp(read_buffer, "EXIT", sizeof("EXIT")) == 0) {
                printf("   [%d] parent exiting read loop!\n", getpid());
                break;
            }
        }
        wait(NULL); // Wait until the child exits/returns
    }
    printf("[%d] Exiting ipc main...\n", getpid());
    return 0;
}
