#ifndef RESULT_H
#define RESULT_H

/**
 * "0 = Success, Non-zero = Failure" is the standard convention.
 * Other result types that have "success"ish or "failure"ish things should try to match
 * this too.
 *
 * NOTE: Parallel array with "result_strings"! If you update this enum, you need to
 * also update this array too!
 */
typedef enum result_s {
    SUCCESS = 0,            /** We've finished all work successfully */
    FAILURE,                /** An error occurred and needs to be handled.  */
} result_t;

/**
 * NOTE: Parallel array with "result_t"!
 */
static char result_strings[][16] = {
    "Success",
    "Failure",
};

/**
 * A result type for functions that are called iteratively. For example the "decode8086"
 * library can decode an assembled input file with many instructions in it. It does so
 * by iterating over all instructions in the file. Each call to "next" processes a single
 * instruction and returns if that effort was successful and we should "continue" to the
 * next instruction, or if we hit some kind of "failure" while decoding, or if we've finished
 * all work in the list and are "done".
 *
 * NOTE: Parallel array with "result_iter_strings"! If you update this enum, you need to
 * also update this array too!
 */
typedef enum result_iter_s {
    RI_CONTINUE = 0,        /** Continue processing. */
    RI_FAILURE,             /** An error occurred and needs to be handled.  */
    RI_DONE,                /** We've finished processing all work. */
} result_iter_t;

/**
 * NOTE: Parallel static array with "result_iter_t"!
 */
static char result_iter_strings[][16] = {
    "Continue",
    "Failure",
    "Done",
};

typedef enum {
    ER_SUCCESS,
    ER_FAILURE,
    ER_OUT_OF_BOUNDS,
    ER_UNKNOWN_OPCODE,
    ER_UNIMPLEMENTED_INSTRUCTION,
} emu_result_t;



#endif // RESULT_H
