#include <Arduino.h>

HardwareSerial TSR(2);   // UART2 untuk TSR20

#define RXD2 16          // RX ESP32
#define TXD2 17          // TX ESP32 (Ke modul Auto RS485)

// Buffer disesuaikan: Protocol manual hanya 4 byte (bukan 14)
uint8_t frame[10]; 
uint8_t idx = 0;

// Variabel untuk Auto Reset ke 0
int currentSpeed = 0;
unsigned long lastPacketTime = 0;
const int timeoutDuration = 1000; // 1 detik tidak ada data = 0 km/h

void setup() {
  Serial.begin(115200);
  
  // Sesuai Manual PDF[cite: 148]: Baudrate 9600
  TSR.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial.println("=== TSR20 Speed Reader (Buffer Parsing Style) ===");
}

void loop() {

  // --- 1. LOGIKA PARSING (Gaya Buffer seperti kode Anda) ---
  while (TSR.available()) {
    uint8_t b = TSR.read();

    // simpan byte ke buffer (Persis kode Anda)
    frame[idx++] = b;

    // kalau sudah 4 byte -> proses 
    // (Diubah dari 14 ke 4 karena manual  bilang paketnya cuma 4 byte)
    if (idx == 4) {

      // Validasi Header & Tail
      // Header Mendekat: FC FA 
      // Header Menjauh:  FB FD [cite: 150]
      // Tail: 00 
      
      bool isComing = (frame[0] == 0xFC && frame[1] == 0xFA);
      bool isLeaving = (frame[0] == 0xFB && frame[1] == 0xFD);
      bool isTailValid = (frame[3] == 0x00);

      if ((isComing || isLeaving) && isTailValid) {
        
        // Ambil data kecepatan
        // Di manual, byte ke-3 (index 2) adalah speed langsung.
        // Tidak perlu rumus matematika ribet.
        uint8_t speedByte = frame[2];

        currentSpeed = speedByte;
        lastPacketTime = millis(); // Reset timer timeout

        // Tampilkan
        Serial.print("Speed: ");
        Serial.print(currentSpeed);
        Serial.println(" km/h");
      }

      // reset index untuk terima paket berikutnya (Persis kode Anda)
      idx = 0;
    }
  }

  // --- 2. LOGIKA AUTO RESET KE 0 (Jika Data Berhenti) ---
  if (millis() - lastPacketTime > timeoutDuration) {
    if (currentSpeed != 0) {
      currentSpeed = 0;
      Serial.println("Speed: 0 km/h (Target Hilang)");
    }
  }
}