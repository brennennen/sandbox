/*
gcc ./signal_explorer.c && ./a.out

Simple signal catching REPL exploration to tinker with catching and raising
the common (and not so common) signals.

References:
* Signal definitions: `arch\x86\include\uapi\asm\signal.h` (replace x86 with
your appropriate architecture).
* Useful table: https://faculty.cs.niu.edu/~hutchins/csci480/signals.htm
*/

#include <stdbool.h> // bool
#include <signal.h> // sigaction
#include <stdio.h> // printf
#include <stdlib.h> // exit
#include <string.h> // strnlen, strncmp, etc.

#define MIN(a,b) ((a) < (b) ? (a) : (b))

const char *help_string = "signal_explorer\n \
Simple toy program to explore linux signaling.\n \
Sets up a signal handler for each signal and then\n \
Enters a REPL to allow the user to trigger a signal\n \
and print some information around how it behaves by\n \
default.\n \
\n \
Optional Arguments:\n \
   -h: Print this help menu.\n \
   -s: Skip custom signal handler registration.\n \
Positional Arguments:\n \
   N/A  \
Example: signal_explorer.out -s\n \
\n \
REPL commands:\n \
 * 1 - 31: Signal number to send.\n \
 * {signal_name}: String name of the signal to send (ex: SIGKILL, SIGUSR1, etc.).\n \
";

#define MAX_SIGNAL_NAME_LENGTH 32
typedef struct signal_handler_s {
    int signal;
    char name[MAX_SIGNAL_NAME_LENGTH];
    int flags;
    void (* callback)(int);
} signal_handler_t;

void sighup_handler(int signum) {
    printf("SIGHUP (1): Sent to report the user has 'hung up' the connection \
(like a phone). If you spawn daemons/background tasks in a shell and terminate \
the shell, this signal might be sent. The default signal handler terminates \
the process.\n");
    exit(1);
}

void sigint_handler(int signum) {
    printf("SIGINT (2): Sent when a user inputs a special 'interrupt' character. \
On modern OSes/keyboards this is the 'ctrl + c' combo. The default signal handler \
terminates the process.\n");
    exit(1);
}

void sigquit_handler(int signum) {
    printf("SIGQUIT (3): Sent when a user inputs a special 'quit' character. \
On modern OSes/keyboards this is the 'ctrl + \\' combo. The default signal handler \
creates a core dump and terminates the process.\n");
    exit(1);
}

void sigill_handler(int signum) {
    printf("SIGILL (4): Sent when your process asks the processor to execute an illegal \
instruction. The default signal handler creates a core dump and terminates the process.\n");
    exit(1);
}

void sigtrap_handler(int signum) {
    printf("SIGTRAP (5): Sent to temporarily halt a process during debugging sessions. \
Primarily used to implement debuggers. See 'ptrace' for more details. The default signal \
handler creates a core dump and terminates the process. TODO: When ptrace is attached to a \
process, see how this handler is modified.\n");
    exit(1);
}

void sigabrt_handler(int signum) {
    printf("SIGABRT (6): Raised by a process itself when it detects an error. \
The default signal handler creates a core dump and terminates the process. \
(SIGIOT falls through to SIGABRT on most machines, SIGIOT was originally a \
pdp-11 specific instruction related signal)\n");
    exit(1);
}

void sigbus_handler(int signum) {
    printf("SIGBUS (7): Raised by hardware when a process accesses memory that \
the CPU cannot physically access (invalid address on the address bus). Only raised \
for physical memory address. Virtual address misuse raises a segfault. The default \
signal handler creates a core dump and terminates the process.\n");
    exit(1);
}

void sigfpe_handler(int signum) {
    printf("SIGFPE (8): (Floating Point Exception) Raised by hardware when a fatal \
math error occurs. Can't be ignored through flags. The default signal handler \
creates a core dump and terminates the process.\n");
    exit(1);
}

void sigusr1_handler(int signum) {
    printf("SIGUSR1 (10): User defined signal for interprocess communication. The OS \
will never send this. User land programs send this to communicate with each other. \
The default signal handler terminates the process.\n");
    exit(1);
}

void sigsegv_handler(int signum) {
    printf("SIGSEGV (11): 'Segmentation fault' - Sent when a process attempts to access \
memory outside of where it is allowed to access. The default signal handler creates a \
core dump and terminates the process. \n");
    exit(1);
}

void sigusr2_handler(int signum) {
    printf("SIGUSR2 (12): (Same as SIGUSR1) User defined signal for interprocess \
communication. The OS will never send this. User land programs send this to \
communicate with each other. The default signal handler terminates the process.\n");
    exit(1);
}

void sigpipe_handler(int signum) {
    printf("SIGPIPE (13): Raised when a 'broken pipe' is detected. If the receiving \
process of a pipe terminates or never starts, if the sending process writes to the pipe \
a SIGPIPE will be raised. The default signal handler terminates the process.\n");
    exit(1);
}

void sigalrm_handler(int signum) {
    printf("SIGALRM (14): Raised when a 'alarm' (syscall) times out. See 'alarm' for more \
details. The default signal handler terminates the process.\n");
    exit(1);
}

void sigterm_handler(int signum) {
    printf("SIGTERM (15): Generic signal used to terminate a process. Sent by default when \
no signal is provided to the shell 'kill()' command. The default signal handler terminates \
the process.\n");
    exit(1);
}

void sigstkflt_handler(int signum) {
    printf("SIGSTKFLT (16): Stack fault on coprocessor. Doesn't appear to still be a thing \
that happens on modern processor architectures (x86 at least). The default signal handler \
terminates the process.\n");
    exit(1);
}

void sigchld_handler(int signum) {
    printf("SIGCHLD (17): Raised when a child process is terminated or stops. The default \
signal handler ignores the signal.\n");
    exit(1);
}

void sigcont_handler(int signum) {
    printf("SIGCONT (18): Raised by the user to continue a process previously stopped by \
SIGSTOP. In shells, to send a SIGCONT to continue a process stoped with ctrl + z, you use \
the 'fg' command. Not to be confused with PTRACE_CONT, which continues SIGTRAP. The default \
signal handler continues the halted process.\n");
    exit(1);
}

void sigtstp_handler(int signum) {
    printf("SIGTSTP (20): Raised by the user to stop a process. In most shells you can send \
a SIGTSTP to the currently executing process with 'Ctrl + Z'. See SIGCONT for how to continue \
the process. The default signal handler stops/halts the process in a way it can be continued \
later.\n");
    exit(1);
}

void sigttin_handler(int signum) {
    printf("SIGTTIN (21): (TTIN = 'TeleType IN'?) Raised by the OS when a process tries to \
read from stdin while it is a background task without a shell attached. The default signal \
handler stops/halts the process in a way it can be continued later.\n");
    exit(1);
}

void sigttou_handler(int signum) {
    printf("SIGTTOU (22): (TTOU = 'TeleType OUT'?) Raised by the OS when a process tries to \
write to stdout while it is a background task without a shell attached. The default signal \
handler stops/halts the process in a way it can be continued later.\n");
    exit(1);
}

void sigurg_handler(int signum) {
    printf("SIGURG (23): Raised by the OS when a socket has 'URGent' data available to read. \
The default signal handler ignores the signal.\n");
    exit(1);
}

void sigxcpu_handler(int signum) {
    printf("SIGXCPU (24): Raised when the process exceeds the maximum allowed \
cpu time limit. See 'setrlimit', specifically the 'RLIMIT_CPU' resource, for \
how to set file size limits. The default signal handler creates a core dump \
and terminates the process.\n");
    exit(1);
}

void sigxfsz_handler(int signum) {
    printf("SIGXFSZ (25): Raised when the process attempts to extend a file \
such that it exceeds the file size limit. See 'setrlimit', specifically the \
'RLIMIT_FSIZE' resource, for how to set file size limits. The default signal \
handler creates a core dump and terminates the process.\n");
    exit(1);
}

void sigvtalrm_handler(int signum) {
    printf("SIGVTALRM (26): (VTALRM = Virtual Timer ALaRM) Raised when a virtual alarm \
set by alarm or setitimer with type 'ITIMER_VIRTUAL' expires. Virtual timers are \
cpu time based ('working time') instead of second/time based. The default signal \
handler terminates the process.\n");
    exit(1);
}

void sigprof_handler(int signum) {
    printf("SIGPROF (27): (PROF = PROFiler) Raised when a profiler alarm \
set by alarm or setitimer with type 'ITIMER_PROF' expires. Profile timers measure \
cpu time based ('working time') and include time spent in OS kernal land processing \
syscalls (virtual timers do not). The default signal handler terminates the process.\n");
    exit(1);
}

void sigwinch_handler(int signum) {
    printf("SIGWINCH (28): Sent when a terminal window is resized. Also seems to be \
hijacked for 'graceful shutdown' by apache httpd. The default signal handler ignores \
the signal.\n");
    exit(1);
}

void sigio_handler(int signum) {
    printf("SIGIO (29): Raised when IO is available on a socket or terminal being read \
from or written to is ready. Used for asynchronous I/O.The default signal handler \
terminates the process.\n");
    exit(1);
}

void sigpwr_handler(int signum) {
    printf("SIGPWR (30): Raised when the hardware detects a power supply failure. \
The default signal handler terminates the process.\n");
    exit(1);
}

void sigsys_handler(int signum) {
    printf("SIGSYS (31): Raised when a process makes a syscall with a bad argument. \
The default signal handler creates a core dump and terminates the process.\n");
    exit(1);
}

static const int signal_handler_table_size = 31;
static const signal_handler_t signal_handler_table[] = {
    {SIGHUP, "SIGHUP", 0, sighup_handler},
    {SIGINT, "SIGINT", 0, sigint_handler},
    {SIGQUIT, "SIGQUIT", 0, sigquit_handler},
    {SIGILL, "SIGILL", 0, sigill_handler},
    {SIGTRAP, "SIGTRAP", 0, sigtrap_handler},
    {SIGABRT, "SIGABRT", 0, sigabrt_handler},
    {SIGBUS, "SIGBUS", 0, sigbus_handler},
    {SIGFPE, "SIGFPE", 0, sigfpe_handler},
    {SIGKILL, "SIGKILL", 0, NULL}, // Can't be caught
    {SIGUSR1, "SIGUSR1", 0, sigusr1_handler},
    {SIGSEGV, "SIGSEGV", 0, sigsegv_handler},
    {SIGUSR2, "SIGUSR2", 0, sigusr2_handler},
    {SIGPIPE, "SIGPIPE", 0, sigpipe_handler},
    {SIGALRM, "SIGALRM", 0, sigalrm_handler},
    {SIGTERM, "SIGTERM", 0, sigterm_handler},
    {SIGSTKFLT, "SIGSTKFLT", 0, sigstkflt_handler},
    {SIGCHLD, "SIGCHLD", 0, sigchld_handler},
    {SIGCONT, "SIGCONT", 0, sigcont_handler},
    {SIGSTOP, "SIGSTOP", 0, NULL}, // Can't be caught
    {SIGTSTP, "SIGTSTP", 0, sigtstp_handler},
    {SIGTTIN, "SIGTTIN", 0, sigttin_handler},
    {SIGTTOU, "SIGTTOU", 0, sigttou_handler},
    {SIGURG, "SIGURG", 0, sigurg_handler},
    {SIGXCPU, "SIGXCPU", 0, sigxcpu_handler},
    {SIGXFSZ, "SIGXFSZ", 0, sigxfsz_handler},
    {SIGVTALRM, "SIGVTALRM", 0, sigvtalrm_handler},
    {SIGPROF, "SIGPROF", 0, sigprof_handler},
    {SIGWINCH, "SIGWINCH", 0, sigwinch_handler},
    {SIGIO, "SIGIO", 0, sigio_handler},
    {SIGPWR, "SIGPWR", 0, sigpwr_handler},
    {SIGSYS, "SIGSYS", 0, sigsys_handler},
};

int signal_name_to_int(char* signal_name, int signal_name_length) {
    for (int i = 0; i < signal_handler_table_size; i++) {
        if (strncmp(signal_name, signal_handler_table[i].name, MIN(MAX_SIGNAL_NAME_LENGTH, signal_name_length)) == 0) {
            return signal_handler_table[i].signal;
        }
    }
    return -1;
}

void register_signal_handlers() {
    for (int i = 0; i < signal_handler_table_size; i++) {
        struct sigaction old_sigint_action;
        struct sigaction new_sigint_action;

        sigemptyset(&new_sigint_action.sa_mask);
        new_sigint_action.sa_flags = signal_handler_table[i].flags;
        new_sigint_action.sa_handler = signal_handler_table[i].callback;

        printf("Registering custom signal handler for: %s (%d)\n", signal_handler_table[i].name, signal_handler_table[i].signal);
        sigaction(signal_handler_table[i].signal, &new_sigint_action, &old_sigint_action);
    }
}

void process_signal_input(int signal) { 
    if (signal == SIGKILL) {
        printf("SIGKILL (9): Forces a process to terminate immediately. Can't be raised by \
a program on itself (ex 'raise(SIGKILL)' does not work) and can't have a custom signal handler \
defined or be ignored through flags.\n");
        exit(1);
    } else if (signal == SIGSTOP) {
        printf("SIGSTOP (19): Stops/halts a process. Can't have a custom signal handler \
defined or be ignored through flags. See SIGCONT for how to continue the process. The \
default signal handler stops/halts the process in a way it can be continued later.\n");
        exit(1);
    }
    raise(signal);
}

int main(int argc, char* argv[]) {
    printf("starting signal_explorer sandbox...\n");

    bool skip_register_signal_handlers = false;
    if (argc >= 2) {
        char* arg1 = argv[1];
        size_t arg1_len = strnlen(arg1, 32);
        printf("arg1: '%s' (%lu)\n", arg1, arg1_len);
        if (strncmp(arg1, "-h", MIN(arg1_len, 2)) == 0) {
            printf("%s", help_string);
            exit(1);
        } else if (strncmp(arg1, "-s", MIN(arg1_len, 2)) == 0) {
            skip_register_signal_handlers = true;
        }
    }

    if (!skip_register_signal_handlers) {
        register_signal_handlers();
    }

    do {
        char* line = NULL; // getline() auto mallocs, user needs to free. not a fan of this api.
        size_t line_buffer_size = 0;
        if (getline(&line, &line_buffer_size, stdin) != -1) {
            size_t line_size = strnlen(line, line_buffer_size);
            line[line_size - 1] = '\0';
            printf("input: '%s' (%lu) (%lu)\n", line, line_buffer_size, line_size);

            if (line_size >= 1 && '0' <= line[0] && line[0] <= '9') {
                int signal = atoi(line);
                process_signal_input(signal);
            } else {
                int signal = signal_name_to_int(line, line_size);
                process_signal_input(signal);
            }
        }
        free(line);
    } while(1);
 
    printf("exiting signal_explorer sandbox...\n");
}
