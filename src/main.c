#include <conio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "fujinet-fuji.h"

char *version = "1.0.0";

#ifndef CH_DEL
#define CH_DEL 0x7F
#endif

static void read_line(char *s)
{
#ifdef __CC65__
  uint16_t i = 0;
  uint8_t c;

  cursor(1);

  do {
#ifdef __APPLE2__
    gotox(i);
#endif

    c = cgetc();

    if (isprint(c)) {
      putchar(c);
      s[i++] = c;
    }
    else if ((c == CH_CURS_LEFT) || (c == CH_DEL)) {
      if (i) {
        putchar(CH_CURS_LEFT);
        putchar(' ');
        putchar(CH_CURS_LEFT);
        --i;
      }
    }
  } while (c != CH_ENTER);
  putchar('\n');
  s[i] = '\0';

  cursor(0);
#elif defined(_CMOC_VERSION_)
  /* cmoc has no stdin; read keys directly. */
  uint16_t i = 0;
  char c;

  do {
    c = (char)waitkey(0);
    if (c == '\r' || c == '\n')
      break;
    if ((unsigned char)c >= 0x20 && i < 30) {
      putchar(c);
      s[i++] = c;
    }
  } while (1);
  putchar('\n');
  s[i] = '\0';
#else
  /* Standard line-buffered console (z88dk, Open Watcom). */
  size_t n;
  if (fgets(s, 30, stdin) == NULL) {
    s[0] = '\0';
    return;
  }
  n = strlen(s);
  if (n && s[n - 1] == '\n')
    s[n - 1] = '\0';
#endif
}

int main() {
  // char *text;
  char text[32];
  uint8_t version;
  uint8_t ecc;
  bool shorten;
  uint8_t output_mode;
  char *code;
  char c;
  bool ok;

#ifdef __ATARI__
  initGraphics();
#endif

  while (1) {
    clrscr();

    puts("FujiNet QR Code Demo\n");
    puts("Text to encode:");
    read_line(text);

    puts("Version? (1-9)");
    // puts("  0 = auto");
    puts("  1 = 21x21");
    puts("  2 = 25x25");
    puts("  3 = 29x29");
    puts("  ...etc");
    c = cgetc();
    if (c >= '1' && c <= '9') {
      version = c - 48;
    } else {
      version = 1;
    }
    putchar(version + 48);
    putchar('\n');

    puts("Error correction level?");
    puts("  0 = Low");
    puts("  1 = Medium");
    puts("  2 = Quartile");
    puts("  3 = High");
    c = cgetc();
    if (c >= '0' && c <= '3') {
      ecc = c - 48;
    } else {
      ecc = 1;
    }
    putchar(ecc + 48);
    putchar('\n');

    puts("Shorten URL? (Y/N)");
    c = cgetc();
    shorten = c == 'y' || c == 'Y';
    putchar(shorten ? 'Y' : 'N');
    putchar('\n');

    puts("Output format?");
#ifdef __ATARI__
    puts("  0 = Binary");
    puts("  1 = ATASCII");
    puts("  2 = Bitmap");
    c = cgetc();
    switch (c) {
      case '1': output_mode = QR_OUTPUT_MODE_ATASCII; break;
      case '2': output_mode = QR_OUTPUT_MODE_BITMAP; break;
      default:  output_mode = QR_OUTPUT_MODE_BINARY; break;
    }
#endif
#ifdef __APPLE2__
    puts("  0 = Bitmap");
    c = cgetc();
    output_mode = QR_OUTPUT_MODE_BITMAP;
#endif
#if !defined(__ATARI__) && !defined(__APPLE2__)
    puts("  0 = Text");
    c = cgetc();
    output_mode = QR_OUTPUT_MODE_BINARY;
#endif
    putchar(c);
    putchar('\n');

    clrscr();
    ok = qr_encode_text(text, version, ecc, shorten, output_mode, code);
  }

  return 0;
}
