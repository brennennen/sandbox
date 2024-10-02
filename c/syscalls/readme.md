# Syscalls 
A directory of explorations and examples of using Unix/Linux/Posix syscalls in c (so 
not exactly calling syscalls directly, calling libc functions that call syscalls).

## Notes
 * Most c files either have the command to build them at the top of the file or a 
 "build.sh" file for anything non-trivial.
 * See all 411 (as of 2024) syscalls in the kernel at: `arch\x86\entry\syscalls\syscall_64.tbl`
 * Signal handling was reworked around Linux ~2.2 to add "real-time" signals. Many
 signal handingling related syscalls now have a "rt_{old_name}" prefix on them as
 part of this work.

