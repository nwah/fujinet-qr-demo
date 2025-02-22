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

uint8_t qrData[1024];

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
  // PAYLOAD:
  // 0,1 : length
  // 2   : Output mode (0-3)
  if (sp_get_fuji_id() == 0) {
    return false;
  }

  sp_error = sp_status(sp_fuji_id, FUJICMD_QRCODE_LENGTH);
  if (sp_error == 1) {
    return false;
  }

  length = (uint16_t *)sp_payload[2];

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

  memcpy(output, &sp_payload[2], len);

  return true;
}

#define VERSION 3
#define SIZE VERSION * 4 + 17

static const char qr[SIZE][SIZE] =
 {{"*******  *    * ***** *******"},
  {"*     * ** *****    * *     *"},
  {"* *** *  ***  ***  ** * *** *"},
  {"* *** * ** * * *****  * *** *"},
  {"* *** *   * ***  **** * *** *"},
  {"*     * * *    ***  * *     *"},
  {"******* * * * * * * * *******"},
  {"            * * *  *         "},
  {"***** ***** * ****   * * * * "},
  {"  * ** * *    * *  ** ***   *"},
  {" *  ***  * *******   **      "},
  {"*        ***  **   **  * * * "},
  {" *** **  * * ***** *     **  "},
  {" ** *    ** *  ** ** ** *   *"},
  {" *** ***     ***  * * ** **  "},
  {"** **   *   * ***   * ***  * "},
  {"    *** *** * ** *     * **  "},
  {"* **    * *  ** *  ** *** * *"},
  {"*  *  * *****  **      *  *  "},
  {"*   *   ** *   ** **   **  * "},
  {"*   * * *  * ** **  ***** ***"},
  {"        *** *     * *   *****"},
  {"******* * *  * * **** * ***  "},
  {"*     *   * * * *  **   *   *"},
  {"* *** * * * * ****  ***** ***"},
  {"* *** * **    * ****   * ****"},
  {"* *** * ** **  ** **  ****** "},
  {"*     * *  *  * * ***  * * * "},
  {"******* ** ** * **** * ****  "}};

// static uint8_t bitmap[SIZE * SIZE / 8 + SIZE];
static uint8_t bitmap[1024];

/**
* to_bitmap - Convert QR code to compact binary format
*
* Replaces data in out_buf, with data suitable for copying directly into
* bitmap graphics memory. Each QR module (pixel) is represented by 1 bit.
* Most significant bit is leftmost pixel. The last bits of the final byte
* of each row are returned as 0s. A 21x21 QR code will be 63 bytes (3 bytes
* per row of 21 bits (= 24 bits with 3 unused) * 21 rows).
*/

static uint8_t *qr_to_bitmap(uint8_t size) {
  uint8_t x, y;
  uint8_t* p = bitmap;

  bzero(bitmap, sizeof(bitmap));

  for (y = 0; y < size; ++y) {
    for (x = 0; x < size; ++x) {
      if (qr[y][x] == '*')
        *p |= 1;
      if (x % 8 != 7)
        *p <<= 1;
      else
        ++p;
    }
    *p <<= 7 - x % 8;
    ++p;
  }

  return bitmap;
}

/////////////////////////////////////////////////////////////////////////
// everything above this comment is supposed to be done on the FujiNet //
/////////////////////////////////////////////////////////////////////////

#pragma static-locals(on)

bool qr_encode_text(char *text, uint8_t version, uint8_t ecc, bool shorten, uint8_t output_mode, char *result) {
  uint8_t size = VERSION * 4 + 17;
  uint8_t offset = (40 - size) / 2; // center
  uint8_t x, y;
  uint16_t length;
  struct regs regs;
  register uint8_t* bitmap;

  // to be replaced by getting the bitmap from the FujiNet
  // bitmap = qr_to_bitmap(size);

  fuji_qr_input(text, strlen(text));
  fuji_qr_encode(version, ecc, shorten);
  fuji_qr_set_output_mode(output_mode);
  fuji_qr_length(&length);
  fuji_qr_output(bitmap, length);

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
