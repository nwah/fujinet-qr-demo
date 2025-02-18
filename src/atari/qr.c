#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <atari.h>
#include <conio.h>
#include <string.h>

#include "qr.h"

#define QR_ENCODE_TIMEOUT 3

uint8_t qrData[625];

void siov(void)
{
    __asm__ ("JSR $E459");
}

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
// bool qr_encode_text(char *text, char *result) {
  uint8_t status;
  uint16_t length;
  uint16_t i = 0;

  // uint8_t version = 3;
  // uint8_t ecc = 0;
  // uint8_t shorten = true << 4;
  // uint8_t output_mode = 2;

	status = fuji_qr_input(text);
	status = fuji_qr_encode(qrData, version, ecc | (shorten<<4));
	status = fuji_qr_length(&length, output_mode);
	status = fuji_qr_output(qrData, length);
	printf("in: %s | out: %d bytes\n", text, length);

	cputc('\n');

	if (output_mode == 0) { // 0x00 or 0x01 bytes
    for (i = 0; i<length; i++) {
      if (i % 21 == 0) printf("\n ");
      if (qrData[i]) cputc(' '|128);
      else cputc(' ');
  	}
	}
	else if (output_mode == 1) { // Bits
  	puts("(display not implemented yet)");
	}
	else if (output_mode == 2) { // ATASCII
  	for (i = 0; i<length; i++) {
      putchar(qrData[i]);
  	}
	}
	else if (output_mode == 3) { // Bitmap
  	puts("(display not implemented yet)");
	}

	cputc('\n');

	return true;
}
