#include <Arduino.h>

HardwareSerial TSR(2);   // UART2 untuk TSR20

#define RXD2 16          // RX ESP32
#define TXD2 17          // TX tidak dipakai

uint8_t frame[20];
uint8_t idx = 0;

void setup() {
  Serial.begin(115200);
  TSR.begin(115200, SERIAL_8N1, RXD2, TXD2);

  Serial.println("=== TSR20 Speed Reader (KM/H) ===");
}

void loop() {

  while (TSR.available()) {
    uint8_t b = TSR.read();

    // simpan byte ke buffer
    frame[idx++] = b;

    // kalau sudah 14 byte → proses
    if (idx == 14) {

      // validasi header & tail
      if (frame[0] == 0xAA && frame[1] == 0xAA &&
          frame[12] == 0x55 && frame[13] == 0x55) {

        // ambil 2 byte kecepatan
        uint16_t rawSpeed = (frame[9] << 8) | frame[10];

        float speed_ms = rawSpeed / 10.0;
        float speed_kmh = speed_ms * 3.6;

        Serial.print("Speed: ");
        Serial.print(speed_kmh, 1);
        Serial.println(" km/h");
      }

      // reset index untuk terima paket berikutnya
      idx = 0;
    }
  }
}
