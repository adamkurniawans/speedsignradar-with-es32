/*************************************************************************
   PROGRAM COUNTER: REALISTIC 7-SEGMENT STYLE
   Panel: 32x32 (2 Panel Vertikal)
   Driver: FM6126A
   Fitur: Bentuk segmen tidak kotak polos, tapi ada sudut miring & gap.
 **************************************************************************/

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// --- KONFIGURASI FISIK ---
#define PANEL_RES_X    32      
#define PANEL_RES_Y    16      
#define PANEL_CHAIN    2       

MatrixPanel_I2S_DMA *dma_display = nullptr;

// ================================================================
// 1. MANUAL MAPPING (Wajib untuk kabel panel Anda)
// ================================================================
void drawPixel32x32(int x, int y, uint16_t color) {
  if (x < 0 || x >= 32 || y < 0 || y >= 32) return;
  if (y < 16) {
    dma_display->drawPixel(x, y, color); // Panel Atas
  } else {
    dma_display->drawPixel(x + 32, y - 16, color); // Panel Bawah
  }
}

// ================================================================
// 2. FUNGSI MENGGAMBAR BENTUK SEGMEN (KHAS DIGITAL)
// ================================================================
// Fungsi ini menggambar garis dengan ujung runcing/miring
// type: 0 = Horizontal (A, G, D), 1 = Vertikal (F, B, E, C)
void drawBeveledBar(int x, int y, int length, int thickness, int type, uint16_t color) {
  
  if (type == 0) { // HORIZONTAL (Mendatar)
    // Baris tengah (Panjang penuh)
    for (int i = 0; i < length; i++) drawPixel32x32(x + i, y + 1, color);
    // Baris atas & bawah (Lebih pendek biar runcing)
    for (int i = 1; i < length - 1; i++) {
      drawPixel32x32(x + i, y, color);
      drawPixel32x32(x + i, y + 2, color);
    }
  } 
  else { // VERTIKAL (Tegak)
    // Kolom tengah (Tinggi penuh)
    for (int i = 0; i < length; i++) drawPixel32x32(x + 1, y + i, color);
    // Kolom kiri & kanan (Lebih pendek biar runcing)
    for (int i = 1; i < length - 1; i++) {
      drawPixel32x32(x, y + i, color);
      drawPixel32x32(x + 2, y + i, color);
    }
  }
}

// ================================================================
// 3. LOGIKA ANGKA 7-SEGMENT
// ================================================================
void drawRealDigit(int num, int x, int y, uint16_t color) {
  // Ukuran Segmen
  int h_len = 9; // Panjang segmen horizontal
  int v_len = 13; // Panjang segmen vertikal
  int thick = 3;  // Ketebalan (Fixed di fungsi drawBeveledBar)

  // Posisi Koordinat Segmen (Ada Gap antar segmen)
  // A (Atas)
  if (num!=1 && num!=4) 
    drawBeveledBar(x + 2, y - 1, h_len, thick, 0, color);
  
  // B (Kanan Atas)
  if (num!=5 && num!=6) 
    drawBeveledBar(x + h_len + 1, y + 1, v_len, thick, 1, color);

  // C (Kanan Bawah)
  if (num!=2) 
    drawBeveledBar(x + h_len + 1, y + v_len + 2, v_len, thick, 1, color);

  // D (Bawah)
  if (num!=1 && num!=4 && num!=7) 
    drawBeveledBar(x + 2, y + (v_len * 2) + 1, h_len, thick, 0, color);

  // E (Kiri Bawah)
  if (num==0 || num==2 || num==6 || num==8) 
    drawBeveledBar(x, y + v_len + 2, v_len, thick, 1, color);

  // F (Kiri Atas)
  if (num!=1 && num!=2 && num!=3 && num!=7) 
    drawBeveledBar(x, y + 1, v_len, thick, 1, color);

  // G (Tengah)
  if (num!=0 && num!=1 && num!=7) 
    drawBeveledBar(x + 2, y + v_len, h_len, thick, 0, color);
}
void drawDigitSeratusan(int x, int y, uint16_t color) {
  // Ukuran Segmen
  int h_len = 9; // Panjang segmen horizontal
  int v_len = 13; // Panjang segmen vertikal
  int thick = 3;  // Ketebalan (Fixed di fungsi drawBeveledBar)

  // E (Kiri Bawah)
  drawBeveledBar(x, y + v_len + 2, v_len, thick, 1, color);

  // F (Kiri Atas)
  drawBeveledBar(x, y + 1, v_len, thick, 1, color);

}

// ================================================================
// SETUP
// ================================================================
void setup() {
  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN);

  // Konfigurasi P10 Outdoor FM6126A
  mxconfig.driver = HUB75_I2S_CFG::FM6126A; 
  mxconfig.clkphase = false;       
  mxconfig.latch_blanking = 4;       
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M; 

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(20); 
  dma_display->clearScreen();
}

// ================================================================
// LOOP
// ================================================================
void loop() {
  
// Kita set batas sampai 199
  for (int i = 100; i <= 199; i++) {
    dma_display->clearScreen();

    uint16_t warna = dma_display->color565(0, 255, 0); // Hijau

    // --- LOGIKA POSISI (LAYOUT) ---
    
    if (i < 100) {
      // === KASUS 1: ANGKA 0 - 99 (Layout Standar/Tengah) ===
      int puluhan = i / 10;
      int satuan = i % 10;

      // Posisi agak renggang biar estetik
      drawRealDigit(puluhan, 2, 1, warna);   // Kiri
      drawRealDigit(satuan, 17, 1, warna);   // Kanan
    } 
    else {
      // === KASUS 2: ANGKA 100 - 199 (Layout Padat) ===
      int ratusan = 1;
      int sisa = i % 100;
      int puluhan = sisa / 10;
      int satuan = sisa % 10;

      // Posisi agak rapat biar estetik
      drawDigitSeratusan(0, 1, warna);   // Kiri
      drawRealDigit(puluhan, 4, 1, warna);   // tengah
      drawRealDigit(satuan, 18, 1, warna);   // Kanan
    }
    delay(1000); // Kecepatan
  }
}
