// ==========================================
// User_Setup.h — TPM408-2.8 (ST7789 + XPT2046)
// ESP32 Dev Module
// ==========================================

#define USER_SETUP_INFO "TPM408_ESP32"

// ==========================================
// SECTION 1 — Driver
// ==========================================
#define ST7789_DRIVER

// Resolution (portrait — setRotation handles landscape)
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// Color order — Keep BGR
#define TFT_RGB_ORDER TFT_BGR

// Inversion — Comment this out to turn it OFF
// #define TFT_INVERSION_ON

// ==========================================
// SECTION 2 — ESP32 pins
// ==========================================
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST   4

#define TOUCH_CS 32

// ==========================================
// SECTION 3 — Fonts
// ==========================================
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// ==========================================
// SECTION 4 — SPI speeds
// ==========================================
#define SPI_FREQUENCY        27000000
#define SPI_READ_FREQUENCY   20000000
#define SPI_TOUCH_FREQUENCY   2500000