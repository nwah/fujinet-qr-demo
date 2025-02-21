#include <conio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "fujinet-fuji.h"

char *version = "1.0.0";

static void readline(char *s)
{
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

  initGraphics();

  while (1) {
    clrscr();

    puts("FujiNet QR Code Demo\n");
    puts("Text to encode:");
    readline(text);

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
    puts("  0 = Bytes");
    puts("  1 = Binary");
    puts("  2 = Printable text");
    puts("  3 = Bitmap");
    c = cgetc();
    if (c >= '0' && c <= '3') {
      output_mode = c - 48;
    } else {
      output_mode = 1;
    }
    putchar(output_mode + 48);
    putchar('\n');

    clrscr();
    ok = qr_encode_text(text, version, ecc, shorten, output_mode, code);
  }

  return 0;
}
