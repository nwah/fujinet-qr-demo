#include <string.h>
#include <conio.h>
#include <6502.h>

#include "qr.h"

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

static uint8_t bitmap[SIZE * SIZE / 8 + SIZE];

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
  struct regs regs;
  register uint8_t* bitmap;

  // to be replaced by getting the bitmap from the FujiNet
  bitmap = qr_to_bitmap(size);

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

  cputsxy(0, 0, "https://www.youtube.com/watch?v=dQw4w9WgXcQ");
  cputsxy(0, 3, "Press any key to continue");

  cgetc();

  regs.pc = 0xFB39; // SETTXT
  _sys(&regs);

  return true;
}
