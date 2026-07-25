#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "qr.h"
#include "fujinet-fuji.h"

/* CP437 solid block; two per module gives a roughly square, scannable code. */
#define QR_ON  0xDB

bool qr_encode_text(char *text, uint8_t version, uint8_t ecc, bool shorten,
                    uint8_t output_mode, char *result)
{
  static uint8_t qrData[1024];
  unsigned long length;
  uint8_t size, on;
  uint16_t i, total;

  (void)result;

  if (!fuji_qrcode_input(text, strlen(text))) {
    printf("Failed to send data to FujiNet\n");
    return false;
  }
  if (!fuji_qrcode_encode(version, ecc, shorten)) {
    printf("Failed to encode data\n");
    return false;
  }
  if (!fuji_qrcode_length(output_mode, &length)) {
    printf("Failed to get output length\n");
    return false;
  }
  if (!fuji_qrcode_output((char *)qrData, (uint16_t)length)) {
    printf("Failed to get QR output\n");
    return false;
  }

  /* BINARY format: qrData[0] = module count per side, then one bit per module,
     row-major, LSB first. */
  size = qrData[0];
  total = (uint16_t)size * size;

  for (i = 0; i < total; i++) {
    if (i % size == 0)
      putchar('\n');
    on = (qrData[1 + (i >> 3)] & (1 << (i & 7))) ? 1 : 0;
    putchar(on ? QR_ON : ' ');
    putchar(on ? QR_ON : ' ');
  }
  putchar('\n');

  printf("\n%s\n", text);
  printf("Press Enter to continue\n");
  getchar();
  return true;
}
