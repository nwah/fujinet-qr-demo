#ifndef QRDEMO_STDINT_H
#define QRDEMO_STDINT_H
/* cmoc ships no <stdint.h>; define the fixed-width types (matches fujinet-fuji.h). */
#ifndef uint8_t
#define uint8_t  unsigned char
#define int8_t   signed char
#define uint16_t unsigned short
#define int16_t  signed short
#define uint32_t unsigned long
#define int32_t  signed long
#endif
#endif
