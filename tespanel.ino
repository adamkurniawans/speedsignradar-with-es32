/*************************************************************************
   Adaptasi Kode untuk Panel P10 Outdoor (8s Scan)
   Ukuran Total: 32x32 (Disusun Atas-Bawah)
   Driver: FM6126A
 **************************************************************************/

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>

// --- 1. KONFIGURASI PANEL FISIK ---
#define PANEL_RES_X 32      // Lebar 1 panel
#define PANEL_RES_Y 16      // Tinggi 1 panel
#define PANEL_CHAIN 1       // Total jumlah panel (1 keping)

// --- 2. KONFIGURASI SUSUNAN (VIRTUAL) ---
// Kita menyusun secara VERTIKAL (Atas-Bawah)
#define NUM_ROWS 1          // 2 Baris Panel
#define NUM_COLS 1          // 1 Kolom Panel

// Hasilnya menjadi 32x32 Pixel
#define VIRTUAL_MATRIX_WIDTH  (PANEL_RES_X * NUM_COLS)
#define VIRTUAL_MATRIX_HEIGHT (PANEL_RES_Y * NUM_ROWS)

// Placeholder objects
MatrixPanel_I2S_DMA *dma_display = nullptr;
VirtualMatrixPanel  *virtualDisp = nullptr;

/******************************************************************************
   Setup!
 ******************************************************************************/
void setup()
{
  Serial.begin(115200);

  // Konfigurasi HUB75 dasar
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   
    PANEL_RES_Y,   
    PANEL_CHAIN    
  );

  // --- BAGIAN PENTING UNTUK PANEL ANDA ---
  // Mengaktifkan Driver Chip FM6126A (Wajib untuk P10 Outdoor baru)
  mxconfig.driver = HUB75_I2S_CFG::FM6126A; 
  
  // Mengatasi Ghosting/Bayangan
  mxconfig.clkphase = true; 

  // --- Inisialisasi Fisik ---
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);

  // Cek memori
  if ( not dma_display->begin() )
    Serial.println("****** !KABOOM! I2S memory allocation failed ***********");

  // Set kecerahan (Jangan terlalu terang untuk tes)
  dma_display->setBrightness8(60);    
  dma_display->clearScreen();

  // --- Inisialisasi Virtual Matrix ---
  // Tidak perlu pakai 'CustomPxBasePanel', pakai standar library terbaru.
  // Parameter: (Display, Rows, Cols, Width, Height)
  // Kita set: 2 Baris (Rows), 1 Kolom (Cols) -> Ini akan membuat urutan 32x32
  virtualDisp = new VirtualMatrixPanel((*dma_display), NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y);
  
  // Jika urutan panel terbalik (Bawah jadi Atas), uncomment baris ini:
  // virtualDisp->setRotation(2); 
}

/******************************************************************************
   Loop (Animasi Pixel by Pixel)
 ******************************************************************************/
void loop() {
  
  // Loop Baris (Y) dari 0 sampai 31
  for (int y = 0; y < virtualDisp->height(); y++)
  {
    // Loop Kolom (X) dari 0 sampai 31
    for (int x = 0; x < virtualDisp->width(); x++)
    {
      // Menggambar pixel Merah (Red)
      // Format: (x, y, warna)
      virtualDisp->drawPixel(x, y, virtualDisp->color565(255, 0, 0));
      
      // Delay 30ms (Sesuai kode asli Anda)
      delay(30);
    }
  }

  // Tunggu 2 detik setelah layar penuh
  delay(2000);
  
  // Bersihkan layar dan ulangi
  virtualDisp->clearScreen();
}