#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <atari.h>
#include <conio.h>
#include <peekpoke.h>
#include <string.h>

#include "qr.h"

#define QR_ENCODE_TIMEOUT 3
#define PM_BASE 0xA000

uint8_t qrData[1024];
uint8_t defaultDL[3 + 2 + 24 + 3];
uint8_t hiresDL[3 + 2 + 192 + 3];

void siov(void)
{
  __asm__ ("JSR $E459");
}

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

// void setupHiRes(void)
// {
//   uint8_t i = 0;
//   hiresDL[0] = DL_BLK8;
//   hiresDL[1] = DL_BLK8;
//   hiresDL[2] = DL_BLK8;
//   hiresDL[3] = DL_LMS(DL_MAP320x1x1);
//   hiResDL[4] = PM_BASE;
//   hiResDL[5] = PM_BASE >> 8;
//   for (i = 7; i < 199; i++) {
//     hiresDL[i] = 0x0F;
//   }
//   hiresDL[200] = DL_JVB;
//   hiresDL[201] = &hiresDL;
//   hiresDL[202] = &hiresDL >> 8;
//   POKE()
//   memset(PM_BASE, 0, 0x2000);
// }

uint8_t fuji_qr_input(char *text) {
  OS.dcb.ddevic = 0x70;
  OS.dcb.dunit = 1;
  OS.dcb.dcomnd = 0xBC; // QR Input
  OS.dcb.dstats = 0x80; // Computer->Peripheral
  OS.dcb.dbuf = text;
  OS.dcb.dtimlo = QR_ENCODE_TIMEOUT;
  OS.dcb.dbyt = strlen(text);
  OS.dcb.daux = strlen(text);
  siov();

  return OS.dcb.dstats;
}

uint8_t fuji_qr_encode(char *data, uint8_t version, uint8_t ecc) {
  OS.dcb.ddevic = 0x70;
  OS.dcb.dunit = 1;
  OS.dcb.dcomnd = 0xBD; // QR Encode
  OS.dcb.dstats = 0x00;
  OS.dcb.dbuf = data;
  OS.dcb.dtimlo = QR_ENCODE_TIMEOUT;
  OS.dcb.dbyt = 0;
  OS.dcb.daux = version | (ecc<<8);
  siov();

  return OS.dcb.dstats;
}

uint8_t fuji_qr_length(uint8_t *length, uint8_t output_mode) {
  OS.dcb.ddevic = 0x70;
  OS.dcb.dunit = 1;
  OS.dcb.dcomnd = 0xBE; // QR length
  OS.dcb.dstats = 0x40; // Peripheral->Computer
  OS.dcb.dbuf = length;
  OS.dcb.dtimlo = QR_ENCODE_TIMEOUT;
  OS.dcb.dbyt = sizeof(&length);
  OS.dcb.daux = output_mode;
  siov();

  return OS.dcb.dstats;
}

uint8_t fuji_qr_output(char *output, uint16_t len) {
  OS.dcb.ddevic = 0x70;
  OS.dcb.dunit = 1;
  OS.dcb.dcomnd = 0xBF; // QR output
  OS.dcb.dstats = 0x40; // Peripheral->Computer
  OS.dcb.dbuf = output;
  OS.dcb.dtimlo = QR_ENCODE_TIMEOUT;
  OS.dcb.dbyt = len;
  OS.dcb.daux = len;
  siov();

  return OS.dcb.dstats;
}

bool qr_encode_text(char *text, uint8_t version, uint8_t ecc, bool shorten, uint8_t output_mode, char *result) {
  uint8_t status;
  uint16_t length;
  uint16_t i = 0;
  uint8_t size = 0;
  uint8_t b = 0;
  uint8_t x = 0;
  uint8_t y = 0;
  uint8_t cols = 3;
  uint8_t col = 0;
  uint8_t row = 0;

  status = fuji_qr_input(text);
  if (status == 144) {
    printf("Failed to send data to FujiNet");
    cgetc();
    return false;
  }

  status = fuji_qr_encode(qrData, version, ecc | (shorten<<4));
  if (status == 144) {
    printf("Failed to encode data");
    cgetc();
    return false;
  }

  status = fuji_qr_length(&length, output_mode);
  if (status == 144) {
    printf("Failed to get output length");
    cgetc();
    return false;
  }

  status = fuji_qr_output(qrData, length);
  if (status == 144) {
    printf("Failed to get QR output");
    cgetc();
    return false;
  }

  printf("in: %s | out: %d bytes\n", text, length);

  // size = 17 + version * 4;
  size = qrData[0];
  --length;

  // 0x00 or 0x01 bytes
  if (output_mode == 0) {
    for (i = 0; i<length; i++) {
      if (i % 21 == 0) printf("\n ");
      if (qrData[i+1]) cputc(' '|128);
      else cputc(' ');
    }
  }
  // Bits
  else if (output_mode == 1) {
    // setupHiRes();
    // i = 0;
    // b = 0;
    // for (y = 0; y < size; y++) {
    //   for (x = 0; x < size; x++) {
    //     b = i % 8;
    //     if (b == 0 && x > 0) {
    //       i++;
    //     }
    //     val = (qrData[i+1] << b) && 1;
    //   }
    // }
    puts("(display not implemented yet)");
  }
  // ready-to-print ATASCII
  else if (output_mode == 2) {
    for (i = 0; i<length; i++) {
      putchar(qrData[i+1]);
    }
  }
  // 1-bit bitmap
  else if (output_mode == 3) {
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
