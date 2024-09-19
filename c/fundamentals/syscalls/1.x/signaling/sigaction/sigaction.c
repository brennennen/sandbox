/*
gcc ./sigaction.c && ./a.out

"sigaction" is used to set or get the current signal handler action for a signal.
You can read. Calls signal "rt_sigaction" (13) on Linux.

Below is a small program that sets a custom "SIGINT" (interrupt) signal handler
to print when a interrupt signal is received, and test this functionality by 
immediately raising a SIGINT.
*/


#include <stdio.h>
#include <signal.h>

void my_sigint_handler(int signum) {
    printf("my_sigint_handler: %d\n", signum);
}

int main(int argc, char* argv[]) {
    printf("starting sigaction sandbox...\n");

    struct sigaction old_sigint_action;
    struct sigaction new_sigint_action;

    sigemptyset(&new_sigint_action.sa_mask);
    new_sigint_action.sa_flags = 0;
    new_sigint_action.sa_handler = my_sigint_handler;

    sigaction(SIGINT, &new_sigint_action, &old_sigint_action);
    printf("old_sigint_action: flags: %d, ignored: %d\n", 
        old_sigint_action.sa_flags, 
        (old_sigint_action.sa_handler == SIG_IGN));

    printf("Raising SIGINT...\n");
    raise(SIGINT);
 
    printf("exiting sigaction sandbox...\n");
}
