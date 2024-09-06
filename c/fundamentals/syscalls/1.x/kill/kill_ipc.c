/*
gcc ./kill_ipc.c && ./a.out

Simple toy program to experiment with using kill to communicate between 2
processes. More explicitly, this program spawns a parent and child process,
then the child sends a "SIGUSR1" to the parent, which catches and prints 
a notification it received it.
*/
#include <errno.h> // errno
#include <signal.h> // signal
#include <stdio.h> // printf
#include <stdlib.h> // exit
#include <string.h> // strerror
#include <unistd.h> // getpid()
#include <sys/wait.h> // wait()

void parent_sigusr1_handler(int signal) {
    printf("[%d] parent received SIGUSR1 from child!\n", getpid());
}

int main(int argc, char* argv[]) {
    printf("Entering fork main (pid: %d)...\n", getpid());
    // child is spawned and starts executing from this point onward.
    pid_t parent_pid = getpid();
    pid_t child_pid = fork();
    if (!child_pid) {
        printf("   [%d] child\n", getpid());
        // Send a signal SIGUSR1 to the parent.
        if (kill(parent_pid, SIGUSR1)) {
            printf("kill failed. errno: %s\n", strerror(errno));
        }
        exit(0); // exit will fire the associated signal to notify the parent to stop waiting.
    }
    else {
        printf("   [%d] parent\n", getpid());
        if(signal(SIGUSR1, parent_sigusr1_handler) == SIG_ERR) {
            printf("signal failed. errno: %s\n", strerror(errno));
        }
        wait(NULL); // Wait until the child exits/returns
    }
    printf("Exiting fork main (pid: %d)...\n", getpid());
    return 0;
}
