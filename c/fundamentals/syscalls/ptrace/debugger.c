/**
 * Simple debugger REPL exploration to tinker with fork, wait, signal trapping, and ptrace.
 * 
 * Example usage:
 *  * First open up a second terminal and use `objdump -d {my_executable}` for address referencing
 *  * Run with `debugger ./my_executable`
 *  * In the repl, use addressing found in the objdump to peek/poke/set breakpoints/dump registers.
 *  * Addresses below correspond to the "hello.c" program in the same directory compiled with gcc 
 * version 11.4.0 on my machine. 0x401176 = the "foobar" function entry point on line 4 for example.
 * I'm providing these as an example, the addresses will most likely be different and require objdump
 * to get right.
 *  * peek 0x401176                 // These are the original instructions before we add a breakpoint.
 *  * bp 0x401176                   // This breakpoint is right between printing hello and world.
 *  * peek 0x401176                 // The instructions should now include a 0xcc for the INT3 break point.
 *  * c                             // Continuing should print "hello" but stop at the bp before we print "world"".
 *  * regs                          // Verify the RIP and other regs updated.
 *  * c                             // continuing should print "world" and exit.
 *  * (once you continue far enough and the child program exits, the debugger exits too)
 */

#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>
#include <sys/personality.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>

typedef struct arguments_s {
    bool continue_on_start;
    char child_program[256];
    char argv[256][256];
} arguments_t;

typedef struct breakpoint {
    uint64_t address;
    int64_t original_opcodes;
} breakpoint_t;


const char *help_string = "debugger\n \
Simple toy intel x64 debugger exploration made to understand\n \
ptfork, wait, signal trapping, and ptrace better. CLI argument\n \
parsing is a bit janky (not the goal of this exploration), optional \n \
arguments need to come first. The REPL is also janky. This was a 1 day build, \n \
there are lots of bugs and missing features, but I learned what I was looking \n \
to learn. \n \
\n \
Optional Arguments:\n \
   -h: Print this help menu.\n \
   -c: Continue on start.\n \
Positional Arguments:\n \
   [0] - Program to execute and log signals for.\n  \
Example: signal_log.out ./hello_world.out\n \
\n \
REPL commands:\n \
 * c - Continue\n \
 * bp 0x1234 - Create a breakpoint at address.\n \
 * s - Single step.\n \
 * regs - Dump the process registers.\n \
 * peek 0x1234 - Dump 8 bytes of memory at the specified address. \n \
 * poke 0x1234 0x00 - Write 8 bytes of memory to the specified address. \n \
 * k - Send a SIGKILL to kill the child process.\n \
 * detach - Detach the debugger.\n \
 * exit - Exit the debugger (sends a sigkill to the debuggee process).\n \
 * h - Print this help menu. \n \
";

void sigchld_handler(int signal) {
    //printf("Child process of %ld has stopped or exited.\n", (long)getpid());
    // TODO: exit the application if the child process is killed externally.
}

void process_arguments(int argc, char* argv[], arguments_t* arguments) {
    int current_argument_index = 1;
    if (argc < 2) {
        printf("%s", help_string);
        exit(1);
    } else if (argv[1][0] == '-') {
        if (argv[1][1] == 'h') {
            printf("%s", help_string);
            exit(1);
        } else if (argv[1][1] == 'c') {
            arguments->continue_on_start = true;
            current_argument_index += 1;
        }
    }
    strncpy(arguments->child_program, argv[current_argument_index], sizeof(arguments->child_program));
    current_argument_index += 1;
    arguments->child_program[strlen(arguments->child_program)] = '\0';
    if (access(arguments->child_program, F_OK) != 0) {
        printf("Program doesn't exist: %s\n", arguments->child_program);
        exit(1);
    }
    printf("argc: %d, parsed args: %d\n", argc, current_argument_index);
    printf("arguments: c: %d, child_program: '%s'\n", 
        arguments->continue_on_start, arguments->child_program);
}

void check_wait_status(int wait_status) {
    if (WIFEXITED(wait_status)) {
        printf("WIFEXITED - Child process exited normally. exit code: %d\n", WEXITSTATUS(wait_status));
    } else if (WIFSIGNALED(wait_status)) {
        printf("WIFSIGNALED - Child process terminated by a signal. signal: %d, core_dump: %d\n", 
            WTERMSIG(wait_status), WCOREDUMP(wait_status));
    } else if (WIFSTOPPED(wait_status)) {
        if (WSTOPSIG(wait_status) == SIGTRAP) {
            printf("WIFSTOPPED - Child stopped by SIGTRAP.\n");
        } else if (WSTOPSIG(wait_status) == SIGSEGV) {
            printf("WIFSTOPPED - Child stopped by SEGFAULT! Debugging can't continue. You can still dump memory though.\n");
        } else {
            printf("WIFSTOPPED - Child stopped by a ptrace signal or WUNTRACED. stop signal: %d\n", WSTOPSIG(wait_status));
        }
    } else if (WIFCONTINUED(wait_status)) {
        printf("WIFCONTINUED - Child continued from a stopped state.\n");
    }
}

void read_registers(pid_t pid, struct user_regs_struct* registers) {
    if (ptrace(PTRACE_GETREGS, pid, NULL, registers) == -1) {
        printf("Failed to read debuggee process registers.\n");
    }
}

void set_registers(pid_t pid, struct user_regs_struct* registers) {
    if (ptrace(PTRACE_SETREGS, pid, NULL, registers) == -1) {
        printf("Failed to set debuggee process registers.\n");
    }
}

void print_registers(struct user_regs_struct* registers) {
    printf("Registers:\n \
   rip: %llx\n \
   rsp: %llx\n \
   rax: %llx\n \
   rbx: %llx\n \
   rcx: %llx\n \
   rdx: %llx\n \
   rsi: %llx\n \
   rdi: %llx\n \
", registers->rip, registers->rsp, registers->rax, registers->rbx, registers->rcx, registers->rdx,
    registers->rsi, registers->rdi);
}

void set_breakpoint(pid_t pid, uint64_t address, breakpoint_t* breakpoints, int breakpoints_size, int* bp_index) {
    breakpoints[*bp_index].address = address;
    breakpoints[*bp_index].original_opcodes = ptrace(PTRACE_PEEKTEXT, pid, (void *)address, NULL);
    long breakpoint_opcode = (breakpoints[*bp_index].original_opcodes & ~0xFF) | 0xCC; // 0xCC = INT3 (breakpoint opcode)
    ptrace(PTRACE_POKETEXT, pid, (void*)address, breakpoint_opcode);
    printf("Added breakpoint[%d] addr: %#lx, opcode: %#lx\n", *bp_index, breakpoints[*bp_index].address, breakpoints[*bp_index].original_opcodes);
    *bp_index += 1;
}

void print_breakpoints(breakpoint_t* breakpoints, int breakpoints_size, int* bp_index) {
    printf("breakpoints size: %d", *bp_index);
    for (int i = 0; i < *bp_index && i < breakpoints_size; i++) {
        printf("[%d] addr: %#lx, data: %#lx\n", *bp_index, breakpoints[i].address, breakpoints[i].original_opcodes);
    }
}

// If we continue but were at a breakpoint, we missed an opcode because we replaced one with INT3 to
// cause the break point to stop the process.
void continue_from_stop(pid_t pid, breakpoint_t* breakpoints, int breakpoints_size, int* bp_index) {
    struct user_regs_struct registers = {0};
    read_registers(pid, &registers);

    printf("trying to continue, checking bps first (%d). rip: %#llx\n", *bp_index, registers.rip);
    for (int i = 0; i < *bp_index && i < breakpoints_size; i++) {
        printf("   i: %d, addr: %#lx\n", i, breakpoints[i].address);
        if (registers.rip == (long) (breakpoints[i].address + 1)) {
            printf("   found breakpoint we were stopped at. removing bp, restoring original opcode, and fixing rip.\n");
            // Restore the original opcode we replaced with an INT3 (0xCC).
            ptrace(PTRACE_POKETEXT, pid, (void*)breakpoints[i].address, breakpoints[i].original_opcodes);
            // Fix the RIP.
            registers.rip = breakpoints[i].address;
            set_registers(pid, &registers);
            // Remove the breakpoint from the list.
            if (*bp_index == 1) {
                *bp_index = 0;
            } else {
                memcpy(&breakpoints[i], &breakpoints[*bp_index - 1], sizeof(breakpoint_t));
                *bp_index -= 1;
            }
            printf("   new bp count: %d\n", *bp_index);
            break;
        }
    }
    ptrace(PTRACE_CONT, pid, NULL, NULL);
}

long peek(pid_t pid, uintptr_t address) {
    errno = 0;
    long data = ptrace(PTRACE_PEEKDATA, pid, (void*)address, NULL);
    if (data == -1 || errno != 0) {
        printf("Failed to peek debuggee process data at address: %" PRIxPTR "\n", address);
    }
    return data;
}

void poke(pid_t pid, uintptr_t address, unsigned long data) {
    printf("poke: address: %#lx, data: %lu\n", address, data);
    ptrace(PTRACE_POKEDATA, pid, (void*)address, data);
    if (errno != 0) {
        printf("Failed to poke debuggee process data at address: %" PRIxPTR "\n", address);
    }
}

void debuggee_main(int argc, char* argv[], arguments_t* arguments) {
    //printf("[%d] child\n", getpid());
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    char* child_argv[] = {arguments->child_program};
    execvp(arguments->child_program, argv); // Exec started processes with ptrace on them are immediately sent SIGTRAP (5) - Breakpoint for debugging.
}

void debugger_main(arguments_t* arguments, pid_t debuggee_pid) {
    //printf("[%d] parent\n", getpid());
    ptrace(PTRACE_CONT, debuggee_pid, NULL, NULL);
    int wait_status = 0;

    wait(&wait_status); // Consume the initial ptrace SIGTRAP notification.
    check_wait_status(wait_status);

    if (arguments->continue_on_start) {
        ptrace(PTRACE_CONT, debuggee_pid, NULL, NULL); // Continue after the initial SIGTRAP.
    }

    breakpoint_t breakpoints[128] = {0};
    int bp_index = 0;

    // Monitor any further start/sotp signals
    printf("Starting Debugger REPL: ('c' - continue, 'bp 0x4321' - create breakpoint, see -h for full list.)\n");
    do {
        bool should_continue = false;
        do {
            char* line = NULL; // getline() auto mallocs, user needs to free. not a fan of this api.
            size_t line_size = 0;
            if (getline(&line, &line_size, stdin) != -1) {
                line[strnlen(line, line_size) - 1] = '\0'; // Replace the ending newline with a null.
                //printf("(%d) line: '%s'\n", (int)strnlen(line, line_size), line);

                char* delimeter = " ";
                char* command_token = strtok(line, delimeter);
                if (command_token == NULL) {
                    continue; // Skip this REPL.
                }
                int command_token_length = strnlen(command_token, 128);
                //printf("command_token: '%s'\n", command_token);

                if (strncmp(command_token, "c", command_token_length) == 0 || 
                        strncmp(command_token, "continue", command_token_length) == 0)  {
                    printf("continuing!\n");
                    should_continue = true;
                    continue_from_stop(debuggee_pid, breakpoints, sizeof(breakpoints), &bp_index);
                } else if (strncmp(command_token, "bp", command_token_length) == 0) {
                    char* address_token = strtok(NULL, delimeter);
                    if (address_token != NULL) {
                        uintptr_t address = (uintptr_t)strtol(address_token, NULL, 16);
                        set_breakpoint(debuggee_pid, address, breakpoints, sizeof(breakpoints), &bp_index);
                    }
                } else if (strncmp(command_token, "s", command_token_length) == 0) {
                    ptrace(PTRACE_SINGLESTEP, debuggee_pid, 0, 0);
                    should_continue = true;
                } else if (strncmp(command_token, "k", command_token_length) == 0) {
                    kill(debuggee_pid, SIGKILL);
                    should_continue = true;
                } else if (strncmp(command_token, "regs", command_token_length) == 0) {
                    struct user_regs_struct registers = {0};
                    read_registers(debuggee_pid, &registers);
                    print_registers(&registers);
                } else if (strncmp(command_token, "peek", command_token_length) == 0) {
                    char* address_token = strtok(NULL, delimeter);
                    if (address_token != NULL) {
                        uintptr_t address = (uintptr_t)strtol(address_token, NULL, 16);
                        long data = peek(debuggee_pid, (uintptr_t)address);
                        printf("peeked data: %#lx\n", data);
                    }
                } else if (strncmp(command_token, "poke", command_token_length) == 0) {
                    char* address_token = strtok(NULL, delimeter);
                    if (address_token != NULL) {
                        uintptr_t address = (uintptr_t)strtol(address_token, NULL, 16);
                        char* data_token = strtok(NULL, delimeter);
                        printf("poke data token: %s\n", data_token);
                        if (data_token != NULL) {
                            unsigned long data = strtoul(data_token, NULL, 16);
                            printf("poke data: %lu\n", data);
                            poke(debuggee_pid, (uintptr_t)address, data);
                        }
                    }
                } else if (strncmp(command_token, "detach", command_token_length) == 0) {
                    ptrace(PTRACE_DETACH, debuggee_pid, 0, 0);
                } else if (strncmp(command_token, "h", command_token_length) == 0) {
                    printf("%s\n", help_string);
                } else if (strncmp(command_token, "exit", command_token_length) == 0) {
                    kill(debuggee_pid, SIGKILL);
                    should_continue = true;
                }
                // TOOD: look into PTRACE_PEEKUSER and PTRACE_POKEUSER
            } else {
                printf("Failed to read stdin! (max input: 4096)\n");
                should_continue = true;
            }
            free(line);
        } while(!should_continue);
        wait(&wait_status);
        check_wait_status(wait_status);
    } while (!WIFEXITED(wait_status) && !WIFSIGNALED(wait_status));
    check_wait_status(wait_status);
}

int main(int argc, char* argv[]) {
    personality(ADDR_NO_RANDOMIZE); // Disable address randomizer for the spawned sub-program.
    arguments_t arguments = {};
    process_arguments(argc, argv, &arguments);
    signal(SIGCHLD, sigchld_handler); // Setup a signal handler to be notified when the child is stopped or exits.

    pid_t debuggee_pid = fork();
    if (!debuggee_pid) { // fork child
        debuggee_main(argc, argv, &arguments);
    } else { // fork parent
        debugger_main(&arguments, debuggee_pid);
    }
}
