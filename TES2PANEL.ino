/*************************************************************************
   PROGRAM COUNTER 0 - 199 (PANEL 2 DIPUTAR 180 DERAJAT)
   Panel: 32x32 (2 Panel Vertikal)
   Driver: FM6126A
 **************************************************************************/

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// --- KONFIGURASI FISIK ---
#define PANEL_RES_X    32      
#define PANEL_RES_Y    16      
#define PANEL_CHAIN    2       

MatrixPanel_I2S_DMA *dma_display = nullptr;

// ================================================================
// 1. MANUAL MAPPING (DENGAN ROTASI PANEL BAWAH)
// ================================================================
void drawPixel32x32(int x, int y, uint16_t color) {
  // Cek batas aman
  if (x < 0 || x >= 32 || y < 0 || y >= 32) return;

  if (y < 16) {
    // --- PANEL 1 (ATAS) ---
    // Normal (Tidak diputar)
    dma_display->drawPixel(x, y, color); 
  } 
  else {
    // --- PANEL 2 (BAWAH) ---
    // DIPUTAR 180 DERAJAT
    // Rumus:
    // Fisik X (32-63) = 63 - x
    // Fisik Y (0-15)  = 31 - y
    dma_display->drawPixel(63 - x, 31 - y, color); 
  }
}

// ================================================================
// 2. FUNGSI GAMBAR GARIS MIRING (BEVEL)
// ================================================================
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

// ================================================================
// 3. LOGIKA ANGKA UTAMA (PULUHAN & SATUAN)
// ================================================================
void drawDuaDigit(int num, int x, int y, uint16_t color) {
  int h_len = 10;   
  int v_len = 13;  
  int thick = 3;   

  if (num!=1 && num!=4) drawBeveledBar(x + 2, y - 1, h_len, thick, 0, color); // A atas
  if (num!=5 && num!=6) drawBeveledBar(x + h_len + 1, y + 1, v_len, thick, 1, color); // B kanan atas
  if (num!=2) drawBeveledBar(x + h_len + 1, y + v_len + 2, v_len, thick, 1, color); // C kanan bawah 
  if (num!=1 && num!=4 && num!=7) drawBeveledBar(x + 2, y + (v_len * 2) + 1, h_len, thick, 0, color); // D bawah
  if (num==0 || num==2 || num==6 || num==8) drawBeveledBar(x, y + v_len + 2, v_len, thick, 1, color); // E kiri bawah
  if (num!=1 && num!=2 && num!=3 && num!=7) drawBeveledBar(x, y + 1, v_len, thick, 1, color); // F kiri atas
  if (num!=0 && num!=1 && num!=7) drawBeveledBar(x + 2, y + v_len, h_len, thick, 0, color); // G tengah
}

void drawTigaDigit(int num, int x, int y, uint16_t color) {
  int h_len = 9;   
  int v_len = 13;  
  int thick = 3;   

  if (num!=1 && num!=4) drawBeveledBar(x + 2, y - 1, h_len, thick, 0, color); // A
  if (num!=5 && num!=6) drawBeveledBar(x + h_len + 1, y + 1, v_len, thick, 1, color); // B
  if (num!=2) drawBeveledBar(x + h_len + 1, y + v_len + 2, v_len, thick, 1, color); // C
  if (num!=1 && num!=4 && num!=7) drawBeveledBar(x + 2, y + (v_len * 2) + 1, h_len, thick, 0, color); // D
  if (num==0 || num==2 || num==6 || num==8) drawBeveledBar(x, y + v_len + 2, v_len, thick, 1, color); // E
  if (num!=1 && num!=2 && num!=3 && num!=7) drawBeveledBar(x, y + 1, v_len, thick, 1, color); // F
  if (num!=0 && num!=1 && num!=7) drawBeveledBar(x + 2, y + v_len, h_len, thick, 0, color); // G
}

void drawDigitSeratusan(int x, int y, uint16_t color) {
  int v_len = 13; 
  int thick = 3;  
  drawBeveledBar(x, y + v_len + 2, v_len, thick, 1, color); // E
  drawBeveledBar(x, y + 1, v_len, thick, 1, color);         // F
}

// ================================================================
// SETUP
// ================================================================
void setup() {
  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN);
  mxconfig.driver = HUB75_I2S_CFG::FM6126A; 
  mxconfig.clkphase = false;       
  mxconfig.latch_blanking = 4;       
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M; 

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(60); 
  dma_display->clearScreen();
}

// ================================================================
// LOOP UTAMA
// ================================================================
void loop() {
  
  // Loop dari 0 sampai 199
  for (int i = 0; i <= 199; i++) {
    dma_display->clearScreen();

    uint16_t warna = dma_display->color565(0, 255, 0); // Hijau

    if (i < 100) {
      // === 2 DIGIT (Tengah) ===
      int puluhan = i / 10;
      int satuan = i % 10;
      drawDuaDigit(puluhan, 1, 1, warna);   
      drawDuaDigit(satuan, 17, 1, warna);   
    } 
    else {
      // === 3 DIGIT (Rapat Kiri) ===
      int sisa = i % 100;
      int puluhan = sisa / 10;
      int satuan = sisa % 10;
      drawDigitSeratusan(0, 1, warna);   
      drawTigaDigit(puluhan, 4, 1, warna);   
      drawTigaDigit(satuan, 18, 1, warna);   
    }

    delay(1000); 
  }
}
