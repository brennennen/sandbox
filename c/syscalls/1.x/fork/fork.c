/*
gcc ./fork.c && ./a.out

Fork is an early iteration on concurrent programming. You fork (creat a clone of) your 
current running process to create a second clone of that process (data is cloned to). 
There are a lot of caveats and cleanup with this model and the general advice 
is that folks should use `thread` or other more modern solutions. It's a fun call
to mess around with though, see `syscalls/ptrace/debugger.c` for a ~300 loc toy 
debugger relying on fork.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // getpid()
#include <sys/wait.h> // wait()

int main(int argc, char* argv[]) {
    printf("Entering fork main (pid: %d)...\n", getpid());
    // child is spawned and starts executing from this point onward.
    pid_t child_pid = fork();
    if (!child_pid) {
        // It can take some time for the forked process to be started, and generally the child
        // block executes after the parent block.
        printf("   [%d] child\n", getpid());
        exit(0); // exit will fire the associated signal to notify the parent to stop waiting.
    }
    else {
        printf("   [%d] parent\n", getpid());
        wait(NULL); // Wait until the child exits/returns
    }
    printf("Exiting fork main (pid: %d)...\n", getpid());
    return 0;
}
