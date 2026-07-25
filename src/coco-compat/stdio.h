#ifndef QRDEMO_STDIO_H
#define QRDEMO_STDIO_H
#include <cmoc.h>   /* printf, sprintf, putchar, putstr */
#ifndef puts
#define puts(s) printf("%s\n", (s))
#endif
#endif
