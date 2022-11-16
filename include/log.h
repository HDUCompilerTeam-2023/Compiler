#ifndef __LOG__
#define __LOG__

#include <stdio.h>

/* In stdio.h
 * Write formatted output to STREAM.
 * This function is a possible cancellation point and therefore not
 * marked with __THROW.
 * ```
 * extern int fprintf (FILE *__restrict __stream,
 *                     const char *__restrict __format, ...);
 * ```
 * Write formatted output to stdout.
 */
#define yyerror(fmt, ...) fprintf(stderr, fmt"\n", ##__VA_ARGS__)

#ifdef DEBUG
#define yydebug(fmt, ...) printf(fmt"\n", ##__VA_ARGS__)
#else
#define yydebug(fmt, ...)
#endif

#endif