#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <6502.h>
#include <stdbool.h>
#include <stdint.h>
#include "../../../fujinet-lib/apple2/src/include/fujinet-bus-apple2.h"

#include "qr.h"

#define FUJICMD_QRCODE_INPUT               0xBC
#define FUJICMD_QRCODE_ENCODE              0xBD
#define FUJICMD_QRCODE_LENGTH              0xBE
#define FUJICMD_QRCODE_OUTPUT              0xBF

static uint8_t qrData[1024];

bool fuji_qr_input(char *s, uint16_t len)
{
    // send CTRL command: FUJICMD_QRCODE_INPUT
    // PAYLOAD:
    // 0,1 : length
    // 2+  : string
    //
    if (sp_get_fuji_id() == 0) {
      return false;
    }

    sp_payload[0] = len;
    sp_payload[1] = len >> 8;

    if (len > MAX_DATA_LEN) return true;
    strncpy((char *)sp_payload + 2, s, len);

    sp_error = sp_control(sp_fuji_id, FUJICMD_QRCODE_INPUT);
    return sp_error == 0;
}

bool fuji_qr_encode(uint8_t version, uint8_t ecc, bool shorten) {
  // send CTRL command: FUJICMD_QRCODE_ENCODE
  // PAYLOAD:
  // 0,1 : length
  // 2   : QR version (1-40)
  // 3   : ECC: error correction level (0-3)
  // 4   : Shorten URL (Y/N)
  if (sp_get_fuji_id() == 0) {
    return false;
  }

  sp_payload[0] = 3;
  sp_payload[1] = 0;
  sp_payload[2] = version;
  sp_payload[3] = ecc;
  sp_payload[4] = shorten;

  sp_error = sp_control(sp_fuji_id, FUJICMD_QRCODE_ENCODE);
  return sp_error == 0;
}

bool fuji_qr_set_output_mode(uint8_t output_mode) {
  // send CTRL command: FUJICMD_QRCODE_OUTPUT
  // PAYLOAD:
  // 0,1 : length
  // 2   : Output mode (0-3)
  if (sp_get_fuji_id() == 0) {
    return false;
  }

  sp_payload[0] = 3;
  sp_payload[1] = 0;
  sp_payload[2] = output_mode;

  sp_error = sp_control(sp_fuji_id, FUJICMD_QRCODE_OUTPUT);
  return sp_error == 0;
}

bool fuji_qr_length(uint16_t *length) {
  // send STAT command: FUJICMD_QRCODE_LENGTH
  if (sp_get_fuji_id() == 0) {
    return false;
  }

  sp_error = sp_status(sp_fuji_id, FUJICMD_QRCODE_LENGTH);
  if (sp_error == 1) {
    return false;
  }

  memcpy(length, sp_payload, 2);
  // *length = sp_payload[0];
  // *length = sp_payload[1];

  return true;
}

bool fuji_qr_output(uint8_t *output, uint16_t len) {
  // send STAT command: FUJICMD_QRCODE_OUTPUT
  // PAYLOAD:
  // 0,1 : length
  // 2   : Output mode (0-3)
  if (sp_get_fuji_id() == 0) {
    return false;
  }

  sp_error = sp_status(sp_fuji_id, FUJICMD_QRCODE_OUTPUT);
  if (sp_error == 1) {
    return false;
  }

  // memcpy(output, &sp_payload[2], len);
  memcpy(output, &sp_payload[0], 441);

  return true;
}

#pragma static-locals(on)

bool qr_encode_text(char *text, uint8_t version, uint8_t ecc, bool shorten, uint8_t output_mode, char *result) {
  uint8_t size = version * 4 + 17;
  uint8_t offset = (40 - size) / 2; // center
  uint8_t x, y;
  uint16_t length;
  struct regs regs;
  register uint8_t* bitmap = &qrData[0];

  // to be replaced by getting the bitmap from the FujiNet
  // bitmap = qr_to_bitmap(size);

  fuji_qr_input(text, strlen(text));
  fuji_qr_encode(version, ecc, shorten);
  fuji_qr_set_output_mode(output_mode);
  fuji_qr_length(&length);
  // printf("length: %d", length);
  // fuji_qr_output(bitmap, length);
  fuji_qr_output(qrData, size * size);

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

  // cputsxy(0, 0, "https://www.youtube.com/watch?v=dQw4w9WgXcQ");
  cputsxy(0, 0, text);
  cputsxy(0, 3, "Press any key to continue");

  cgetc();

  regs.pc = 0xFB39; // SETTXT
  _sys(&regs);

  return true;
}
