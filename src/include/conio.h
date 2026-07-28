#ifndef CONIO_H
#define CONIO_H

/*
 * Minimal subset of cc65's <conio.h> used by main.c, backed by standard stdio
 * for toolchains that don't ship conio (cmoc, z88dk, Open Watcom). This header
 * is only placed on the include path for those platforms (EXTRA_INCLUDE_* in
 * the Makefile); cc65 targets use their real conio.h.
 */

#include <stdio.h>

#ifndef CH_ENTER
#define CH_ENTER '\n'
#endif
#ifndef CH_CURS_LEFT
#define CH_CURS_LEFT 8
#endif
#ifndef CH_DEL
#define CH_DEL 127
#endif

/* Screen/cursor control is a no-op on a scrolling text console. */
#define clrscr()   ((void)0)
#define cursor(x)  ((void)(x))
#define gotox(x)   ((void)(x))

/* Read one line and return its first character (menus expect a single key). */
static char cgetc(void)
{
    char line[16];
    if (fgets(line, sizeof(line), stdin) == NULL)
        return CH_ENTER;
    return line[0] ? line[0] : CH_ENTER;
}

#endif /* CONIO_H */
