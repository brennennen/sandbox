/*
gcc ./kill.c && ./a.out

Sends a signal to a process. Originally (in UNIX) it was created to terminate
processes (similar to SIGKILL today). Over time, other "termination" related
signals were added. Then non-termination (SIGUSR1, etc) related signals were
added and now we have are stuck with a poorly named syscall/function wrapper.'
Further reading: pubs.opengroup.org/onlinepubs/009695399/functions/kill.html#tag_03_323_08

Also see "raise()", helper function implemented in glibc as just 
`kill(getpid(), sig);` in naive scenarios. 
*/

#include <signal.h> // SIGKILL
#include <stdio.h> // printf
#include <unistd.h> // getpid()

int main(int argc, char* argv[]) {
    printf("Entering kill main...\n");
    kill(getpid(), SIGKILL); // Kills self, essentially exits here and no further code is executed.
    printf("Exiting kill main...\n"); // Never called because we un-alived ourselves.
}
