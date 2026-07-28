#ifndef QRDEMO_CONIO_H
#define QRDEMO_CONIO_H
#include <coco.h>   /* waitkey, inkey */
#ifndef CH_ENTER
#define CH_ENTER 13
#endif
#ifndef CH_CURS_LEFT
#define CH_CURS_LEFT 8
#endif
#ifndef CH_DEL
#define CH_DEL 8
#endif
#define clrscr()   ((void)0)
#define cursor(x)  ((void)(x))
#define gotox(x)   ((void)(x))
static char cgetc(void) { return (char)waitkey(0); }
#endif
