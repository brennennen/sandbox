/*
gcc ./sigpending.c && ./a.out

"sigpending" is used to access signals sent by the OS that have not been processed
yet. Calls "rt_sigpending" (127) on Linux.

Most of the time signals should immediately be processed, and "sigpending" will return
nothing. However, if the process's main execution thread is in an "uninterruptible sleep"
(TASK_UNINTERRUPTIBLE), or a signal is set to be blocked through "sigprocmask", then 
it will not process signals and "sigpending" can be useful.

Below is a toy/demo program that spawns 3 threads. The main thread that is doing important
work and has set "SIGUSR1" signals to be ignored (this important work is mocked by just
performing a blocking read call on a pipe). The first child periodically prints any pending 
signals. The second child sends "SIGUSR1" signals to the main thread for 10 seconds, then 
writes to the pipe to unblock and every thread exits.
*/

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void my_sigusr1_handler(int signum) {
    printf("my_sigusr1_handler: %d\n", signum);
}

void* signal_watcher_child_main(void* vargp) {
    for (int i = 0; i < 3; i++) {
        sleep(1);
        printf("signal_watcher_child_main poll: %d...\n", i);
        sigset_t sigset = {};
        sigemptyset(&sigset);
        sigpending(&sigset);
        int sigusr1_present = sigismember(&sigset, SIGUSR1);
        printf("signal_watcher_child_main poll: %d. sigusr1_present: %d\n", i, sigusr1_present);
    }
    printf("Exiting signal watcher child main...\n");
    pthread_exit(NULL);
}

void* signal_sender_main(void* vargp) {
    printf("sender child: %d\n", getpid());
    int* pipe_write_fd = (int*) vargp;
    for (int i = 0; i < 3; i++) {
        sleep(1);
        printf("signal_sender poll: %d...\n", i);
        kill(getpid(), SIGUSR1);
    }
    printf("writing data to parent to unblock/end read call: pipe fd: %d\n", *pipe_write_fd);
    write(*pipe_write_fd, "EXIT", sizeof("EXIT"));
    sleep(1);
    printf("Exiting signal sender child main...\n");
    pthread_exit(NULL);
}

void setup_sigusr1_handler() {
    struct sigaction old_sigint_action;
    struct sigaction new_sigint_action;

    sigemptyset(&new_sigint_action.sa_mask);
    new_sigint_action.sa_flags = SA_NODEFER;
    new_sigint_action.sa_handler = my_sigusr1_handler;
    sigaction(SIGUSR1, &new_sigint_action, &old_sigint_action);
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigprocmask(SIG_SETMASK, &sigset, NULL);
}

void block_sigusr1(sigset_t* old_sigset) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigprocmask(SIG_SETMASK, &sigset, old_sigset);
}

void unblock_sigusr1(sigset_t* old_sigset) {
    sigprocmask(SIG_SETMASK, old_sigset, NULL);
}

int main(int argc, char* argv[]) {
    printf("Starting 'sigpending' sandbox...\n");
    printf("parent: %d\n", getpid());

    // Setup a signal handler for SIGUSR1. If SIGUSR1 doesn't have a handler, it 
    // terminates the process, which is a path that unblocks and exits even if reading.
    // If it does have a handler, it'll take a different path and NOT unblock a read call.
    sigset_t old_sigset;
    setup_sigusr1_handler(); 
    block_sigusr1(&old_sigset);

    // Create a pipe between the threads to start a blocking read call to get to a state
    // where signals are not processed and 'sigpending' can be populated.
    int pipe_fds[2] = {0};
    pipe(pipe_fds);

    // Create the signal watcher thread that periodically calls 'sigpending'.
    pthread_t signal_watcher_thread_id;
    pthread_create(&signal_watcher_thread_id, NULL, signal_watcher_child_main, NULL);

    // Create another thread to periodically send 'SIGUSR1' signals to accumulate in the
    // 'sigpending' result set and then eventually write to the pipe and unblock the parent
    // threads read call.
    pthread_t signal_sender_thread_id;
    pthread_create(&signal_sender_thread_id, NULL, signal_sender_main, &pipe_fds[1]);

    // Enter a read call to block and have sigpending result signal set accumulate.
    char read_buffer[32] = {0};
    printf("parent starting read: read fd: %d write fd: %d\n", pipe_fds[0], pipe_fds[1]);
    int read_size = read(pipe_fds[0], read_buffer, 32);
    printf("parent thread read: %s (%d)\n", read_buffer, read_size);
    unblock_sigusr1(&old_sigset);
    pthread_join(signal_watcher_thread_id, NULL);
    pthread_join(signal_sender_thread_id, NULL);
    printf("Exiting 'sigpending' sandbox...\n");
}
