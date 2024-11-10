/*
gcc ./dup.c && ./a.out

`dup` is a syscall used to duplicate file descriptors.

The original "dup" (`int dup(int oldfd)`) duplicated the oldfd to "the lowest open
fd". The standard io file descriptors are 0, 1, and 2 (stdin, stdout, and stderr). Using
this information, you could close the normal `stderr` and then call `dup(1)` and all instances
that write to `stderr` would now be writing to a `stdout`. See `original_dup_usecase()` below.

The original `dup` interface is clunky and so dup2, and dup3 are preferred over `dup`. These newer
dup calls take the newfd as an argument instead of using "the lowest open fd".

The program below provides an example use case for `dup` and `dup2`. It first stores the file stream
of the original stdout using dup, then redirects stdout to an arbitrary file for some time, then
restores the original stdout stream.
*/

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

void original_dup_usecase() {
    close(2); // 2 = stderr, writes to stderr now go nowhere.
    dup(1); // 1 = stdout. duplicates stdout to the lowest free fd, which is 2 (stderr), because we 
            // just closed it. which means we duplicated stdout to where stderr normally 
            // is and all writes to stderr will go to stdout.
    // stderr is normally duplicated to stdout anyway, so run with `./a.out 2>/dev/null` to 
    // redirect stderr to somewhere else first.
    fprintf(stderr, "stderr test\n"); // prints to stdout because the dup call.
}

int redirect_stdio(char* new_file_path) {
    int old_stdout_fd = dup(STDOUT_FILENO);
    close(STDOUT_FILENO);

    int new_stdio = open(new_file_path, O_RDWR | O_CREAT | O_TRUNC);
    dup2(new_stdio, STDOUT_FILENO);

    return old_stdout_fd;
} 

void restore_stdio(int old_stdout_fd) {
    dup2(old_stdout_fd, STDOUT_FILENO);
}


int main(int argc, char* argv[]) {
    printf("starting dup sandbox...\n");

    char temp_file_path[] = "/tmp/dup_stdio_redirect_test";

    int old_stdout_fd = redirect_stdio(temp_file_path);
    printf("text sent to stdio that gets redirected to a temp file!\n");
    printf("cat `%s`   to see redirected content.\n", temp_file_path);

    restore_stdio(old_stdout_fd);
    printf("`cat %s`   to see temporarily redirected content.\n", temp_file_path);
    printf("exiting dup sandbox...\n");
}
