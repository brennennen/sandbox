/*
gcc ./sigsuspend.c && ./a.out

"sigsuspend" is used to halt a process until a specific signal is received.

Below is a toy program that halts with sigsuspend until 'SIGUSR1' is received. Before
halting, the program spawns a child thread to send a 'SIGUSR1' after 10 seconds.
*/

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void my_sigusr1_handler(int signum) {
    printf("my_sigusr1_handler: %d\n", signum);
}

void* signal_sender_main(void* vargp) {
    printf("signal sender child main. Sending 'SIGUSR1' after 10 seconds. \n");
    sleep(10);
    printf("Sending 'SIGUSR1' signal\n");
    kill(getpid(), SIGUSR1);
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
}

int main(int argc, char* argv[]) {
    printf("Starting 'sigpending' sandbox...\n");
    printf("parent: %d\n", getpid());

    sigset_t old_sigset;
    setup_sigusr1_handler(); 

    pthread_t signal_sender_thread_id;
    pthread_create(&signal_sender_thread_id, NULL, signal_sender_main, NULL);

    // The 'sigsuspend' first argument is an ignore mask. So setting each signal bit except 
    // 'SIGUSR1' states to only wake back up if 'SIGUSR1' is received.
    sigset_t suspend_ignore_mask;
    sigfillset(&suspend_ignore_mask); // Set all bits in the mask high
    sigdelset(&suspend_ignore_mask, SIGUSR1); // Set 'SIGUSR1' low

    printf("Suspending until we receive a 'SIGUSR1'...\n");
    sigsuspend(&suspend_ignore_mask);

    pthread_join(signal_sender_thread_id, NULL);
    printf("Exiting 'sigpending' sandbox...\n");
}
