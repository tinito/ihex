/*
 * kk_ihex_write.h: A simple library for writing Intel HEX data. 
 * See the accompanying kk_ihex_read.h for read support, and the
 * main header kk_ihex.h for the shared parts.
 *
 *
 *      WRITING BINARY DATA AS INTEL HEX
 *      --------------------------------
 *
 * In order to write out data, the `ihex_write_at_address` or
 * `ihex_write_at_segment` functions are used to set the data location,
 * and then the binary bytes are written with `ihex_write_byte` and/or
 * `ihex_write_bytes`. The writing functions will then call the function
 * `ihex_flush_buffer` whenever the internal write buffer needs to be
 * cleared - it is up to the caller to provide an implementation of
 * `ihex_flush_buffer` to do the actual writing. See below for details
 * and an example implementation.
 *
 * See the declaration further down for an example implementation.
 *
 * The sequence to write data in IHEX format is:
 *      struct ihex_state ihex;
 *      ihex_init(&ihex);
 *      ihex_write_at_address(&ihex, 0);
 *      ihex_write_bytes(&ihex, my_data, length_of_my_data);
 *      ihex_end_write(&ihex);
 *
 * For outputs larger than 64KiB, 32-bit linear addresses are output. Normally
 * the initial linear extended address record of zero is NOT written - it can
 * be forced by setting `ihex->flags |= IHEX_FLAG_ADDRESS_OVERFLOW` before
 * writing the first byte.
 *
 * Gaps in the data may be created by calling `ihex_write_at_address` with the
 * new starting address without calling `ihex_end_write` in between.
 *
 *
 * The same `struct ihex_state` may be used either for reading or writing,
 * but NOT both at the same time. Furthermore, a global output buffer is
 * used for writing, i.e., multiple threads must not write simultaneously
 * (but multiple writes may be interleaved).
 *
 *
 *      CONSERVING MEMORY
 *      -----------------
 *
 * If you are using only the write support, you should define
 * `IHEX_LINE_MAX_LENGTH` as the length of your output line. This makes
 * both the `struct ihex_state' and the internal write buffer smaller.
 * For example, 32 or even 16 can be used instead of the default 255.
 *
 * If you are reading IHEX as well, then you'll end up limiting the
 * maximum length of line that can be read. In that case you may wish to
 * define `IHEX_MAX_OUTPUT_LINE_LENGTH` as smaller to decrease the
 * write buffer size, but keep `IHEX_LINE_MAX_LENGTH` at 255 to support
 * reading any IHEX file.
 *
 *
 * Copyright (c) 2013 Kimmo Kulovesi, http://arkku.com/
 * Provided with absolutely no warranty, use at your own risk only.
 * Use and distribute freely, mark modified copies as such.
 */

#ifndef KK_IHEX_WRITE_H
#define KK_IHEX_WRITE_H

#include "kk_ihex.h"

// Default number of data bytes written per line
#if IHEX_LINE_MAX_LENGTH >= 32
#define IHEX_DEFAULT_OUTPUT_LINE_LENGTH 32
#else
#define IHEX_DEFAULT_OUTPUT_LINE_LENGTH IHEX_LINE_MAX_LENGTH
#endif

#ifndef IHEX_MAX_OUTPUT_LINE_LENGTH
#define IHEX_MAX_OUTPUT_LINE_LENGTH IHEX_LINE_MAX_LENGTH
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Initialise the structure `ihex` for writing
void ihex_init(struct ihex_state * const ihex);

// Begin writing at the given 32-bit `address` after writing any
// pending data at the current address.
//
// This can also be used to skip to a new address without calling
// `ihex_end_write`; this allows writing sparse output.
//
void ihex_write_at_address(struct ihex_state *ihex, ihex_address_t address);

// Write a single byte
void ihex_write_byte(struct ihex_state *ihex, unsigned int b);

// Write `count` bytes from `data`
void ihex_write_bytes(struct ihex_state *ihex, uint8_t *data, unsigned int count);

// End writing (flush buffers, write end of file record)
void ihex_end_write(struct ihex_state *ihex);

// Called whenever the global, internal write buffer needs to be flushed by
// the write functions. The implementation is NOT provided by this library;
// this must be implemented to perform the actual output, i.e., write out
// `(eptr - buffer)` bytes from `buffer` (which is not NUL-terminated, but
// may be modified to make it thus).
//
// Example implementation:
//
//      void ihex_flush_buffer(char *buffer, char *eptr) {
//          *eptr = '\0';
//          (void) fputs(buffer, stdout);
//      }
//
// Note that the contents of `buffer` can become invalid immediately after
// this function returns - the data must be copied if it needs to be preserved!
//
extern void ihex_flush_buffer(struct ihex_state *ihex,
                              char *buffer, char *eptr);

// As `ihex_write_at_address`, but specify a segment selector. Note that
// segments are not automatically incremented when the 16-bit address
// overflows (the default is to use 32-bit linear addressing). For segmented
// 20-bit addressing you must manually ensure that a write does not overflow
// the segment boundary, and call `ihex_write_at_segment` to every time the
// segment needs to be changed.
//
#ifndef IHEX_DISABLE_SEGMENTS
void ihex_write_at_segment(struct ihex_state *ihex, ihex_segment_t segment,
                           ihex_address_t address);
#endif

// Set the output line length to `length` - may be safely called only right
// after `ihex_write_at_address` or `ihex_write_at_segment`. The maximum
// is IHEX_LINE_MAX_LENGTH (which may be changed at compile time).
void ihex_set_output_line_length(struct ihex_state *ihex, uint8_t line_length);

#ifdef __cplusplus
}
#endif

#endif // !KK_IHEX_H
