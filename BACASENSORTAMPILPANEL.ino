#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// ================================================================
// 1. KONFIGURASI PIN & HARDWARE
// ================================================================

// --- KONFIGURASI SERIAL RADAR ---
// PENTING: Pin dipindah ke 32/33 agar tidak bentrok dengan Panel P10 (Pin 16/17)
#define RX_RADAR 32  
#define TX_RADAR 33  
HardwareSerial TSR(2); 

// --- KONFIGURASI PANEL LED ---
#define PANEL_RES_X    32      
#define PANEL_RES_Y    16      
#define PANEL_CHAIN    2        

MatrixPanel_I2S_DMA *dma_display = nullptr;

// Variabel Global Data Radar
uint8_t frame[20];
uint8_t idx = 0;
int currentSpeedInt = -1; // Inisialisasi -1 agar layar update saat pertama kali 0

// ================================================================
// 2. FUNGSI MAPPING & GRAFIS (DARI PROGRAM 1)
// ================================================================

// Mapping Manual (Panel Bawah Diputar 180 Derajat)
void drawPixel32x32(int x, int y, uint16_t color) {
  if (x < 0 || x >= 32 || y < 0 || y >= 32) return;

  if (y < 16) {
    // Panel Atas (Normal)
    dma_display->drawPixel(x, y, color); 
  } 
  else {
    // Panel Bawah (Rotasi 180)
    dma_display->drawPixel(63 - x, 31 - y, color); 
  }
}

// Fungsi Garis Bevel
void drawBeveledBar(int x, int y, int length, int thickness, int type, uint16_t color) {
  if (type == 0) { // HORIZONTAL
    for (int i = 0; i < length; i++) drawPixel32x32(x + i, y + 1, color);
    for (int i = 1; i < length - 1; i++) {
      drawPixel32x32(x + i, y, color);
      drawPixel32x32(x + i, y + 2, color);
    }
  } else { // VERTIKAL
    for (int i = 0; i < length; i++) drawPixel32x32(x + 1, y + i, color);
    for (int i = 1; i < length - 1; i++) {
      drawPixel32x32(x, y + i, color);
      drawPixel32x32(x + 2, y + i, color);
    }
  }
}

// Logika Angka 2 Digit (Tengah)
void drawDuaDigit(int num, int x, int y, uint16_t color) {
  int h_len = 10;   
  int v_len = 13;  
  int thick = 3;    

  if (num!=1 && num!=4) drawBeveledBar(x + 2, y - 1, h_len, thick, 0, color); 
  if (num!=5 && num!=6) drawBeveledBar(x + h_len + 1, y + 1, v_len, thick, 1, color); 
  if (num!=2) drawBeveledBar(x + h_len + 1, y + v_len + 2, v_len, thick, 1, color); 
  if (num!=1 && num!=4 && num!=7) drawBeveledBar(x + 2, y + (v_len * 2) + 1, h_len, thick, 0, color); 
  if (num==0 || num==2 || num==6 || num==8) drawBeveledBar(x, y + v_len + 2, v_len, thick, 1, color); 
  if (num!=1 && num!=2 && num!=3 && num!=7) drawBeveledBar(x, y + 1, v_len, thick, 1, color); 
  if (num!=0 && num!=1 && num!=7) drawBeveledBar(x + 2, y + v_len, h_len, thick, 0, color); 
}

// Logika Angka 3 Digit (Rapat Kiri)
void drawTigaDigit(int num, int x, int y, uint16_t color) {
  int h_len = 9;    
  int v_len = 13;   
  int thick = 3;    

  if (num!=1 && num!=4) drawBeveledBar(x + 2, y - 1, h_len, thick, 0, color); 
  if (num!=5 && num!=6) drawBeveledBar(x + h_len + 1, y + 1, v_len, thick, 1, color); 
  if (num!=2) drawBeveledBar(x + h_len + 1, y + v_len + 2, v_len, thick, 1, color); 
  if (num!=1 && num!=4 && num!=7) drawBeveledBar(x + 2, y + (v_len * 2) + 1, h_len, thick, 0, color); 
  if (num==0 || num==2 || num==6 || num==8) drawBeveledBar(x, y + v_len + 2, v_len, thick, 1, color); 
  if (num!=1 && num!=2 && num!=3 && num!=7) drawBeveledBar(x, y + 1, v_len, thick, 1, color); 
  if (num!=0 && num!=1 && num!=7) drawBeveledBar(x + 2, y + v_len, h_len, thick, 0, color); 
}

// Angka 1 Ratusan
void drawDigitSeratusan(int x, int y, uint16_t color) {
  int v_len = 13; 
  int thick = 3;  
  drawBeveledBar(x, y + v_len + 2, v_len, thick, 1, color); 
  drawBeveledBar(x, y + 1, v_len, thick, 1, color);         
}

// Fungsi Update Layar (Dipanggil saat kecepatan berubah)
void updateDisplay(int speedVal) {
  dma_display->clearScreen();
  
  // Tentukan Warna Berdasarkan Kecepatan (Opsional, contoh logika)
  // Hijau: < 60, Kuning: 60-99, Merah: >= 100
  uint16_t warna;
  if (speedVal < 10) warna = dma_display->color565(0, 255, 0);       // Hijau
  else warna = dma_display->color565(255, 0, 0);                     // Merah

  if (speedVal < 100) {
    // === 2 DIGIT (Tengah) ===
    int puluhan = speedVal / 10;
    int satuan = speedVal % 10;
    
    // Jika speed < 10 (misal 0-9), apakah puluhan "0" mau ditampilkan?
    // Kode asli Anda menampilkan '0' di puluhan. Jika ingin blank, tambahkan kondisi.
    drawDuaDigit(puluhan, 1, 1, warna);    
    drawDuaDigit(satuan, 17, 1, warna);    
  } 
  else {
    // === 3 DIGIT (Rapat Kiri) ===
    // Batasi max display 199 (karena keterbatasan font seratusan Anda)
    if (speedVal > 199) speedVal = 199; 

    int sisa = speedVal % 100;
    int puluhan = sisa / 10;
    int satuan = sisa % 10;
    
    drawDigitSeratusan(0, 1, warna);    
    drawTigaDigit(puluhan, 4, 1, warna);    
    drawTigaDigit(satuan, 18, 1, warna);    
  }
}

// ================================================================
// 3. SETUP & LOOP
// ================================================================

void setup() {
  // 1. Init Serial Monitor (Debug)
  Serial.begin(115200);
  
  // 2. Init Serial Radar (Gunakan Pin 32/33!)
  TSR.begin(115200, SERIAL_8N1, RX_RADAR, TX_RADAR);
  Serial.println("=== TSR20 Radar + P10 Matrix Started ===");

  // 3. Init Panel
  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN);
  mxconfig.driver = HUB75_I2S_CFG::FM6126A; 
  mxconfig.clkphase = false;        
  mxconfig.latch_blanking = 4;        
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M; 

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(20); // Kecerahan
  
  // Test Nyala Awal (Tampilkan 00 sebentar)
  updateDisplay(0);
}

void loop() {
  // Cek apakah ada data masuk dari Radar
  while (TSR.available()) {
    uint8_t b = TSR.read();
    frame[idx++] = b;

    // Jika buffer penuh (14 byte)
    if (idx == 14) {
      // Validasi Header & Tail
      if (frame[0] == 0xAA && frame[1] == 0xAA &&
          frame[12] == 0x55 && frame[13] == 0x55) {

        // Hitung Speed
        uint16_t rawSpeed = (frame[9] << 8) | frame[10];
        float speed_ms = rawSpeed / 10.0;
        float speed_kmh = speed_ms * 3.6;
        
        int displaySpeed = (int)speed_kmh; // Ubah ke integer untuk display

        // Debug ke Serial Monitor
        Serial.printf("Raw: %d | KM/H: %.1f | Disp: %d\n", rawSpeed, speed_kmh, displaySpeed);

        // Update ke Panel HANYA jika angka berubah (untuk efisiensi)
        if (displaySpeed != currentSpeedInt) {
          currentSpeedInt = displaySpeed;
          updateDisplay(currentSpeedInt);
        }
      }
      // Reset index
      idx = 0;
    }
  }
}