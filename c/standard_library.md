# C STL
## Overview
```c
// C23
/* New headers */
#include <stdbit.h> // Various bit fiddling functions, ex: count_ones, count_zeros, etc.
#include <stdckint.h> // Checked integer arithmetic, ex: ckd_add, ckd_sub, etc. all return false on error.
/*
 * Notable New Language Features:
 *  * nullptr and nullptr_t - Null pointer type alternative for commonly used "#define NULL ((void*)0)"
 *  * "%b" printf/scanf format family of functions for binary values
 *  * 'true' and 'false' - new boolean keywords (true = 1, false = 0)
 *  * constexpr - compile time constant expressions
 *  * [[attributes]] - flags to be used during pre-processing/compile time.
 *  * char8_t and u8 constants - utf8 byte type and constant definition ex: u8'ö'
 */
// ISO: https://open-std.org/JTC1/SC22/WG14/www/docs/n3096.pdf
// Further Reading: https://en.cppreference.com/w/c/23

// C17
// No notable changes since C11

// C11
/* New headers */
#include <stdalign.h> // Functions used to specify or query the alignment of data. alignas(), alignof()
#include <stdatomic.h> // Part of the new concurrency support library providing concurrency safe types and operations. ex: atomic_bool, atomic_fetch_add, etc.
#include <stdnoreturn.h> // Deprecated in c23 for [[noreturn]] attribute. Specify the function doesn't return (ex: it longjmps out of the function).
#include <threads.h> // New concurrency support library. "pthreads" (posix standard threads.h was based on) should still be considered to be used in place.
#include <uchar.h> // Utf8 c-string support "null-terminated multibyte string". char8_t, char16_t, char32_t.
/*
 * Notable New Language Features:
 * * Thread local errno - Making the errno value associated with the thread executed on.
 * * aligned_alloc(alignment, size) - stdlib.h addition for allocating memory with a specified alignment.
 * * quick_exit() - exit without cleaning up resources (let the os deal with it)
 * * timespec - time (seconds/nanoseconds) structure and methods.
 * * Removed "gets()"
 */
// ISO: https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1548.pdf
// Further Reading: https://en.cppreference.com/w/c/11

// C99
/* New headers */
#include <stdint.h> // Fixed width int types: int8_t, uint32_t, etc.
#include <inttypes.h> // Includes stdint.h and additional features such as fixed length int printf format tokens and various functions.
#include <stdbool.h> // Defines a "bool" type: true = 1, false = 0
#include <complex.h> // "imaginary" and "complex" number types. Used for very complex math operations (most folks still just use IEEE floats/doubles.).
#include <fenv.h> // "Floating point environment" - part of complex number support.
#include <tgmath.h> // "Type-Generic Math" - part of complex number support.
// ISO: https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf
// Further Reading: https://en.cppreference.com/w/c/99

// C95
/* New headers */
#include <iso646.h> // Alternative tokens for some c tokens in non-english locales, ex: using "<%" instead of "{".
#include <wctype.h> // Pre-ut8 multi-byte character support. Used in legacy encodings: Latin-1, etc.
#include <wchar.h> // Pre-ut8 multi-byte character support. Used in legacy encodings: Latin-1, etc.

// C89/ANSI C and lower
/* New headers */
#include <assert.h> // Conditionally compiled macros to help validate the state of a program during development.
#include <ctype.h> // "Character Type" functions: "isupper", "isspace", etc.
#include <stdio.h> // Standard Input/Output - Input/output (print, printf) to stdin/stdout/stderr and () to file streams.
#include <errno.h> // Macro to access Where standard library functions write errors to.
#include <float.h> // Various floating point arithmetic macros
#include <limits.h> // Max/min macro defines for various sized types (UCHAR_MAX 255, etc.).
#include <locale.h> // Functions to set and read the system locale
#include <math.h> // Various math functions (trig sin/cos/tan, sqrt, abs, etc.)
#include <setjmp.h> // Defines setjmp/longjmp for non-standard control flow.
#include <signal.h> // Defines various signal, setting up signal handling callbacks, and rasing signals.
#include <stddef.h> // "Standard Type Definitions" - Defines NULL, size_t, wchar_t, ptrdiff_t
#include <stdlib.h> // "Standard Library" - Core c standard library include. Defines a wide arrange of functions, ex: atoi, strod, rand, malloc, free, exit, etc.
#include <string.h> // Functions for manipulating c strings and memory in general, ex: memcpy, strncpy, strchr, strstr, strok, etc.
#include <time.h> // Various time related functions.
```
