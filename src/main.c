#include <conio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "main.h"
#include "../../fujinet-lib/fujinet-fuji.h"

char *version = "1.0.0";

int main() {
  char *text = "https://fujinet.online?foo=bar&baz=%20";
  char *code;
  bool ok;

	ok = qr_encode_text(text, code);

	while (1);

	return 0;
}
