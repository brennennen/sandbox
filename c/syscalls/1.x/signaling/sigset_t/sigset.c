/*
gcc ./sigset.c && ./a.out

The `sigset_t` type is used to represent a list of signals. This is used to create masks to
ignore certain signals or provide a list of signals that have been raised and unprocessed.
Although on most architures `sigset_t` is implemented as just a simple 64 bit mask with each
bit corresponding to a signal (i.e. SIGHUP = bit 0, SIGINT = bit 1, etc.), it is undefined
behavior to manipulate this mask yourself and you should use the following syscalls:
* `sigemptyset(sigset_t* sigset)` - Sets each signal to not be present in the set.
* `sigfillset(sigset_t* sigset)` - Sets each signal to be present in the set.
* `sigaddset(sigset_t* sigset, int sig_num)` - Sets a specific signal to be present in the 
set.
* `sigdelset(sigset_t* sigset, int sig_num)` - Sets a specific signal to not be present in 
the set.
* `int sigismember(sigset_t* sigset, int sig_num)` - Returns 1 if the specified signal is
present, 0 if not present, and -1 on error.

Below is a small toy program to explore the above mentioned interface. See other signal
related syscall toy programs in this directory for more complex examples.
*/


#include <signal.h>
#include <stdio.h>


int main(int argc, char* argv[]) {
    printf("Starting 'sigset' sandbox...\n");

    sigset_t sigset;
    sigemptyset(&sigset);
    printf("Is 'SIGUSR1' raised on an empty set? %d\n", sigismember(&sigset, SIGUSR1));
    printf("Is 'SIGUSR2' raised on an empty set? %d\n", sigismember(&sigset, SIGUSR2));

    sigfillset(&sigset);
    printf("Is 'SIGUSR1' raised on an filled set? %d\n", sigismember(&sigset, SIGUSR1));
    printf("Is 'SIGUSR2' raised on an filled set? %d\n", sigismember(&sigset, SIGUSR2));

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    printf("Is 'SIGUSR1' raised on an empty set after it is added? %d\n", 
        sigismember(&sigset, SIGUSR1));
    printf("Is 'SIGUSR2' raised on an empty set after it is not added? %d\n", 
        sigismember(&sigset, SIGUSR2));

    sigdelset(&sigset, SIGUSR1);
    printf("Is 'SIGUSR1' raised on an empty set after it is added and removed? %d\n", 
        sigismember(&sigset, SIGUSR1));

    printf("Exiting 'sigset' sandbox...\n");
}
