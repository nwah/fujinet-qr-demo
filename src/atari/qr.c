#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <atari.h>
#include <conio.h>
#include <peekpoke.h>
#include <string.h>
#include "fujinet-fuji.h"

#include "qr.h"

#define PM_BASE 0xA000

uint8_t qrData[1024];
uint8_t defaultDL[3 + 2 + 24 + 3];
uint8_t hiresDL[3 + 2 + 192 + 3];

void initGraphics(void)
{
  OS.sdmctl = OS.sdmctl | 8;
  POKE(0xD407, PM_BASE/256);
  // Turn on P+M
  POKE(0xD01D, 3);
}

void setupPMG(void)
{
  memset(PM_BASE, 0, 0x800);
  GTIA_WRITE.sizep0 = PMG_SIZE_NORMAL;
  GTIA_WRITE.sizep1 = PMG_SIZE_NORMAL;
  GTIA_WRITE.sizep2 = PMG_SIZE_NORMAL;
  GTIA_WRITE.sizep3 = PMG_SIZE_NORMAL;
  GTIA_WRITE.hposp0 = 56;
  GTIA_WRITE.hposp1 = 64;
  GTIA_WRITE.hposp2 = 72;
  GTIA_WRITE.hposp3 = 80;
  OS.pcolr0 = 0x0F;
  OS.pcolr1 = 0x0F;
  OS.pcolr2 = 0x0F;
  OS.pcolr3 = 0x0F;
}

bool qr_encode_text(char *text, uint8_t version, uint8_t ecc, bool shorten, uint8_t output_mode, char *result) {
  unsigned long length;
  uint16_t i = 0;
  uint8_t cols = 3;
  uint8_t col = 0;
  uint8_t row = 0;

  if (!fuji_qrcode_input(text, strlen(text))) {
    printf("Failed to send data to FujiNet");
    cgetc();
    return false;
  }

  if (!fuji_qrcode_encode(version, ecc, shorten)) {
    printf("Failed to encode data");
    cgetc();
    return false;
  }

  if (!fuji_qrcode_length(output_mode, &length)) {
    printf("Failed to get output length");
    cgetc();
    return false;
  }

  if (!fuji_qrcode_output((char *)qrData, (uint16_t)length)) {
    printf("Failed to get QR output");
    cgetc();
    return false;
  }

  printf("in: %s | out: %u bytes\n", text, (uint16_t)length);

  // qrData[0] holds the QR size; module data follows from qrData[1]
  --length;

  // one 0x00/0x01 byte per module
  if (output_mode == QR_OUTPUT_MODE_BINARY) {
    for (i = 0; i<length; i++) {
      if (i % 21 == 0) printf("\n ");
      if (qrData[i+1]) cputc(' '|128);
      else cputc(' ');
    }
  }
  // ready-to-print ATASCII
  else if (output_mode == QR_OUTPUT_MODE_ATASCII) {
    for (i = 0; i<length; i++) {
      putchar(qrData[i+1]);
    }
  }
  // 1-bit bitmap
  else if (output_mode == QR_OUTPUT_MODE_BITMAP) {
    setupPMG();
    cols = 2 + version;
    for (i = 0; i<length; i++) {
      col = i % cols;
      if (col == 0 && i > 0) {
        row++;
      }
      POKE(PM_BASE + 0x200 + col * 0x80 + row + 24, qrData[i+1]);
    }
  }

  cgetc();

  memset(PM_BASE, 0, 0x800);

  return true;
}
