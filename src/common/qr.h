#include <stdbool.h>
#include <stdint.h>

#ifndef QR_H
#define QR_H

bool qr_encode_text(char *text, uint8_t version, uint8_t ecc, bool shorten, uint8_t output_mode, char *result);

#endif // QR_H
