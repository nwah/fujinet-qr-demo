#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <6502.h>
#include <stdbool.h>
#include <stdint.h>
#include "fujinet-fuji.h"

#include "qr.h"

static uint8_t qrData[1024];

#pragma static-locals(on)

bool qr_encode_text(char *text, uint8_t version, uint8_t ecc, bool shorten, uint8_t output_mode, char *result) {
  uint8_t size = version * 4 + 17;
  uint8_t offset = (40 - size) / 2; // center
  uint8_t x, y;
  unsigned long length;
  struct regs regs;
  register uint8_t* bitmap = &qrData[1];

  fuji_qrcode_input(text, strlen(text));
  fuji_qrcode_encode(version, ecc, shorten);
  fuji_qrcode_length(output_mode, &length);
  fuji_qrcode_output((char *)qrData, (uint16_t)length);

  regs.pc = 0xFB40; // SETGR
  _sys(&regs);
  regs.pc = 0xF864; // SETCOL
  regs.a = 0x0F; // COLOR = WHITE
  _sys(&regs);

  for (y = 0; y < size; ++y) {
    for (x = 0; x < size; ++x) {
      if (*bitmap & 0x80) {
        regs.pc = 0xF800; // PLOT
        regs.y = x + offset; // X
        regs.a = y + offset; // Y
        _sys(&regs);
      }
      if (x % 8 != 7)
        *bitmap <<= 1;
      else
        ++bitmap;
    }
    ++bitmap;
  }

  cputsxy(0, 0, text);
  cputsxy(0, 3, "Press any key to continue");

  cgetc();

  regs.pc = 0xFB39; // SETTXT
  _sys(&regs);

  return true;
}
