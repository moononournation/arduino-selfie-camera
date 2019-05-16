/***************************************************
 * Subset of https://github.com/moononournation/TFT_eSPI
 ****************************************************/

// Stop fonts etc being loaded multiple times
#ifndef _ST7789H_
#define _ST7789H_

#define TFT_WIDTH 240
#define TFT_HEIGHT 240

#define TFT_MISO 22
#define TFT_MOSI 19
#define TFT_SCLK 21
#define TFT_CS 12
#define TFT_DC 15
#define TFT_RST -1
#define TFT_BL 2

// Define the SPI clock frequency, this affects the graphics rendering speed. Too
// fast and the TFT driver will not keep up and display corruption appears.
#define SPI_FREQUENCY 40000000

#define LOAD_GLCD  // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2 // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters

#define TFT_SPI_MODE SPI_MODE3

#if (TFT_HEIGHT != 320) || (TFT_WIDTH != 240)
#define CGRAM_OFFSET
#endif

// Delay between some initialisation commands
#define TFT_INIT_DELAY 0x80 // Not used unless commandlist invoked

// Generic commands used by ST7789.cpp
#define TFT_SWRST 0x01

#define TFT_SLPIN 0x10
#define TFT_SLPOUT 0x11
#define TFT_NORON 0x13

#define TFT_INVOFF 0x20
#define TFT_INVON 0x21
#define TFT_DISPOFF 0x28
#define TFT_DISPON 0x29
#define TFT_CASET 0x2A
#define TFT_PASET 0x2B
#define TFT_RAMWR 0x2C
#define TFT_RAMRD 0x2E
#define TFT_MADCTL 0x36
#define TFT_COLMOD 0x3A

// Flags for TFT_MADCTL
#define TFT_MAD_MY 0x80
#define TFT_MAD_MX 0x40
#define TFT_MAD_MV 0x20
#define TFT_MAD_ML 0x10
#define TFT_MAD_RGB 0x00
#define TFT_MAD_BGR 0x08
#define TFT_MAD_MH 0x04
#define TFT_MAD_SS 0x02
#define TFT_MAD_GS 0x01

// ST7789 specific commands used in init
#define ST7789_NOP 0x00
#define ST7789_SWRESET 0x01
#define ST7789_RDDID 0x04
#define ST7789_RDDST 0x09

#define ST7789_RDDPM 0x0A      // Read display power mode
#define ST7789_RDD_MADCTL 0x0B // Read display MADCTL
#define ST7789_RDD_COLMOD 0x0C // Read display pixel format
#define ST7789_RDDIM 0x0D      // Read display image mode
#define ST7789_RDDSM 0x0E      // Read display signal mode
#define ST7789_RDDSR 0x0F      // Read display self-diagnostic result (ST7789V)

#define ST7789_SLPIN 0x10
#define ST7789_SLPOUT 0x11
#define ST7789_PTLON 0x12
#define ST7789_NORON 0x13

#define ST7789_INVOFF 0x20
#define ST7789_INVON 0x21
#define ST7789_GAMSET 0x26 // Gamma set
#define ST7789_DISPOFF 0x28
#define ST7789_DISPON 0x29
#define ST7789_CASET 0x2A
#define ST7789_RASET 0x2B
#define ST7789_RAMWR 0x2C
#define ST7789_RGBSET 0x2D // Color setting for 4096, 64K and 262K colors
#define ST7789_RAMRD 0x2E

#define ST7789_PTLAR 0x30
#define ST7789_VSCRDEF 0x33 // Vertical scrolling definition (ST7789V)
#define ST7789_TEOFF 0x34   // Tearing effect line off
#define ST7789_TEON 0x35    // Tearing effect line on
#define ST7789_MADCTL 0x36  // Memory data access control
#define ST7789_IDMOFF 0x38  // Idle mode off
#define ST7789_IDMON 0x39   // Idle mode on
#define ST7789_RAMWRC 0x3C  // Memory write continue (ST7789V)
#define ST7789_RAMRDC 0x3E  // Memory read continue (ST7789V)
#define ST7789_COLMOD 0x3A

#define ST7789_RAMCTRL 0xB0   // RAM control
#define ST7789_RGBCTRL 0xB1   // RGB control
#define ST7789_PORCTRL 0xB2   // Porch control
#define ST7789_FRCTRL1 0xB3   // Frame rate control
#define ST7789_PARCTRL 0xB5   // Partial mode control
#define ST7789_GCTRL 0xB7     // Gate control
#define ST7789_GTADJ 0xB8     // Gate on timing adjustment
#define ST7789_DGMEN 0xBA     // Digital gamma enable
#define ST7789_VCOMS 0xBB     // VCOMS setting
#define ST7789_LCMCTRL 0xC0   // LCM control
#define ST7789_IDSET 0xC1     // ID setting
#define ST7789_VDVVRHEN 0xC2  // VDV and VRH command enable
#define ST7789_VRHS 0xC3      // VRH set
#define ST7789_VDVSET 0xC4    // VDV setting
#define ST7789_VCMOFSET 0xC5  // VCOMS offset set
#define ST7789_FRCTR2 0xC6    // FR Control 2
#define ST7789_CABCCTRL 0xC7  // CABC control
#define ST7789_REGSEL1 0xC8   // Register value section 1
#define ST7789_REGSEL2 0xCA   // Register value section 2
#define ST7789_PWMFRSEL 0xCC  // PWM frequency selection
#define ST7789_PWCTRL1 0xD0   // Power control 1
#define ST7789_VAPVANEN 0xD2  // Enable VAP/VAN signal output
#define ST7789_CMD2EN 0xDF    // Command 2 enable
#define ST7789_PVGAMCTRL 0xE0 // Positive voltage gamma control
#define ST7789_NVGAMCTRL 0xE1 // Negative voltage gamma control
#define ST7789_DGMLUTR 0xE2   // Digital gamma look-up table for red
#define ST7789_DGMLUTB 0xE3   // Digital gamma look-up table for blue
#define ST7789_GATECTRL 0xE4  // Gate control
#define ST7789_SPI2EN 0xE7    // SPI2 enable
#define ST7789_PWCTRL2 0xE8   // Power control 2
#define ST7789_EQCTRL 0xE9    // Equalize time control
#define ST7789_PROMCTRL 0xEC  // Program control
#define ST7789_PROMEN 0xFA    // Program mode enable
#define ST7789_NVMSET 0xFC    // NVM setting
#define ST7789_PROMACT 0xFE   // Program action

//#define ESP32 //Just used to test ESP32 options

// Include header file that defines the fonts loaded, the TFT drivers
// available and the pins to be used

// Only load the fonts defined in User_Setup.h (to save space)
// Set flag so RLE rendering code is optionally compiled
#ifdef LOAD_GLCD
#include "glcdfont.h"
#endif

#ifdef LOAD_FONT2
#include "Font16.h"
#endif

#include <Arduino.h>
#include <Print.h>

#include <pgmspace.h>

#include <SPI.h>

#ifdef SMOOTH_FONT
// Call up the SPIFFS FLASH filing system for the anti-aliased fonts
#define FS_NO_GLOBALS
#include <FS.h>

#ifdef ESP32
#include "SPIFFS.h"
#endif
#endif

#ifndef TFT_DC
#define DC_C // No macro allocated so it generates no code
#define DC_D // No macro allocated so it generates no code
#else
#if TFT_DC >= 32
#define DC_C                                 \
  GPIO.out1_w1ts.val = (1 << (TFT_DC - 32)); \
  GPIO.out1_w1tc.val = (1 << (TFT_DC - 32))
#define DC_D                                 \
  GPIO.out1_w1tc.val = (1 << (TFT_DC - 32)); \
  GPIO.out1_w1ts.val = (1 << (TFT_DC - 32))
#else
#if TFT_DC >= 0
#define DC_C                     \
  GPIO.out_w1ts = (1 << TFT_DC); \
  GPIO.out_w1tc = (1 << TFT_DC)
#define DC_D                     \
  GPIO.out_w1tc = (1 << TFT_DC); \
  GPIO.out_w1ts = (1 << TFT_DC)
#else
#define DC_C
#define DC_D
#endif
#endif
#endif

#ifndef TFT_CS
#define CS_L // No macro allocated so it generates no code
#define CS_H // No macro allocated so it generates no code
#else
#if TFT_CS >= 32
#define CS_L                                 \
  GPIO.out1_w1ts.val = (1 << (TFT_CS - 32)); \
  GPIO.out1_w1tc.val = (1 << (TFT_CS - 32))
#define CS_H                                 \
  GPIO.out1_w1tc.val = (1 << (TFT_CS - 32)); \
  GPIO.out1_w1ts.val = (1 << (TFT_CS - 32))
#else
#if TFT_CS >= 0
#define CS_L                     \
  GPIO.out_w1ts = (1 << TFT_CS); \
  GPIO.out_w1tc = (1 << TFT_CS)
#define CS_H                     \
  GPIO.out_w1tc = (1 << TFT_CS); \
  GPIO.out_w1ts = (1 << TFT_CS)
#else
#define CS_L
#define CS_H
#endif
#endif
#endif

#ifdef TFT_WR
#if defined(ESP32)
#define WR_L GPIO.out_w1tc = (1 << TFT_WR)
#define WR_H GPIO.out_w1ts = (1 << TFT_WR)
#else
#define WR_L GPOC = wrpinmask
#define WR_H GPOS = wrpinmask
#endif
#endif

#define tft_Write_8(C) write_8(C)
#define tft_Write_16(C) write_16(C)
#define tft_Write_16S(C) write_16(((uint16_t)C << 8) | (C >> 8))
#define tft_Write_32(C) write_32(C)

#define tft_Write_Color(C) tft_Write_16S(C)
#define tft_Write_ColorS(C) tft_Write_16(C)

#include "soc/spi_reg.h"
#define SPI_NUM 0x3

const uint32_t MASK = ((*((volatile uint32_t *)ETS_UNCACHED_ADDR(SPI_MOSI_DLEN_REG(SPI_NUM)))) & (~((SPI_USR_MOSI_DBITLEN) << (SPI_USR_MOSI_DBITLEN_S))));
const uint32_t D8_MASK = MASK | (((7) & SPI_USR_MOSI_DBITLEN) << (SPI_USR_MOSI_DBITLEN_S));
const uint32_t D16_MASK = MASK | (((15) & SPI_USR_MOSI_DBITLEN) << (SPI_USR_MOSI_DBITLEN_S));
const uint32_t D32_MASK = MASK | (((31) & SPI_USR_MOSI_DBITLEN) << (SPI_USR_MOSI_DBITLEN_S));
const uint32_t D504_MASK = MASK | (((503) & SPI_USR_MOSI_DBITLEN) << (SPI_USR_MOSI_DBITLEN_S));
const uint32_t D512_MASK = MASK | (((511) & SPI_USR_MOSI_DBITLEN) << (SPI_USR_MOSI_DBITLEN_S));

#define write_8(C)                                       \
  WRITE_PERI_REG((SPI_MOSI_DLEN_REG(SPI_NUM)), D8_MASK); \
  WRITE_PERI_REG(SPI_W0_REG(SPI_NUM), C);                \
  SET_PERI_REG_MASK(SPI_CMD_REG(SPI_NUM), SPI_USR);      \
  while (READ_PERI_REG(SPI_CMD_REG(SPI_NUM)) & SPI_USR)

#define write_16(C)                                       \
  WRITE_PERI_REG((SPI_MOSI_DLEN_REG(SPI_NUM)), D16_MASK); \
  WRITE_PERI_REG(SPI_W0_REG(SPI_NUM), C);                 \
  SET_PERI_REG_MASK(SPI_CMD_REG(SPI_NUM), SPI_USR);       \
  while (READ_PERI_REG(SPI_CMD_REG(SPI_NUM)) & SPI_USR)

#define write_32(C)                                       \
  WRITE_PERI_REG((SPI_MOSI_DLEN_REG(SPI_NUM)), D32_MASK); \
  WRITE_PERI_REG(SPI_W0_REG(SPI_NUM), C);                 \
  SET_PERI_REG_MASK(SPI_CMD_REG(SPI_NUM), SPI_USR);       \
  while (READ_PERI_REG(SPI_CMD_REG(SPI_NUM)) & SPI_USR)

#define tft_write_8_8(C, D) write_16((uint16_t)D << 8) | C)
#define tft_write_16_16(C, D) write_32((C >> 8) | (uint16_t)(C << 8) | ((uint8_t)(D >> 8) << 16 | (D << 24)))

#define tft_Write_C8(C) \
  DC_C;                 \
  write_8(C);           \
  DC_D
#define tft_write_C16(C) \
  DC_C;                  \
  write_16(C);           \
  DC_D

//These enumerate the text plotting alignment (reference datum point)
#define TL_DATUM 0    // Top left (default)
#define TC_DATUM 1    // Top centre
#define TR_DATUM 2    // Top right
#define ML_DATUM 3    // Middle left
#define CL_DATUM 3    // Centre left, same as above
#define MC_DATUM 4    // Middle centre
#define CC_DATUM 4    // Centre centre, same as above
#define MR_DATUM 5    // Middle right
#define CR_DATUM 5    // Centre right, same as above
#define BL_DATUM 6    // Bottom left
#define BC_DATUM 7    // Bottom centre
#define BR_DATUM 8    // Bottom right
#define L_BASELINE 9  // Left character baseline (Line the 'A' character would sit on)
#define C_BASELINE 10 // Centre character baseline
#define R_BASELINE 11 // Right character baseline

// New color definitions use for all my libraries
#define TFT_BLACK 0x0000       /*   0,   0,   0 */
#define TFT_NAVY 0x000F        /*   0,   0, 128 */
#define TFT_DARKGREEN 0x03E0   /*   0, 128,   0 */
#define TFT_DARKCYAN 0x03EF    /*   0, 128, 128 */
#define TFT_MAROON 0x7800      /* 128,   0,   0 */
#define TFT_PURPLE 0x780F      /* 128,   0, 128 */
#define TFT_OLIVE 0x7BE0       /* 128, 128,   0 */
#define TFT_LIGHTGREY 0xC618   /* 192, 192, 192 */
#define TFT_DARKGREY 0x7BEF    /* 128, 128, 128 */
#define TFT_BLUE 0x001F        /*   0,   0, 255 */
#define TFT_GREEN 0x07E0       /*   0, 255,   0 */
#define TFT_CYAN 0x07FF        /*   0, 255, 255 */
#define TFT_RED 0xF800         /* 255,   0,   0 */
#define TFT_MAGENTA 0xF81F     /* 255,   0, 255 */
#define TFT_YELLOW 0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE 0xFFFF       /* 255, 255, 255 */
#define TFT_ORANGE 0xFDA0      /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0 /* 180, 255,   0 */
#define TFT_PINK 0xFC9F

// Next is a special 16 bit colour value that encodes to 8 bits
// and will then decode back to the same 16 bit value.
// Convenient for 8 bit and 16 bit transparent sprites.
#define TFT_TRANSPARENT 0x0120

// Swap any type
template <typename T>
static inline void
swap_coord(T &a, T &b)
{
  T t = a;
  a = b;
  b = t;
}

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

// This structure allows sketches to retrieve the user setup parameters at runtime
// by calling getSetup(), zero impact on code size unless used, mainly for diagnostics
typedef struct
{
  int16_t esp;
  uint8_t trans;
  uint8_t serial;
  uint8_t overlap;

  uint16_t tft_driver; // Hexadecimal code
  uint16_t tft_width;  // Rotation 0 width and height
  uint16_t tft_height;

  uint8_t r0_x_offset; // Offsets, not all used yet
  uint8_t r0_y_offset;
  uint8_t r1_x_offset;
  uint8_t r1_y_offset;
  uint8_t r2_x_offset;
  uint8_t r2_y_offset;
  uint8_t r3_x_offset;
  uint8_t r3_y_offset;

  int8_t pin_tft_mosi;
  int8_t pin_tft_miso;
  int8_t pin_tft_clk;
  int8_t pin_tft_cs;

  int8_t pin_tft_dc;
  int8_t pin_tft_rd;
  int8_t pin_tft_wr;
  int8_t pin_tft_rst;

  int8_t pin_tft_d0;
  int8_t pin_tft_d1;
  int8_t pin_tft_d2;
  int8_t pin_tft_d3;
  int8_t pin_tft_d4;
  int8_t pin_tft_d5;
  int8_t pin_tft_d6;
  int8_t pin_tft_d7;

  int8_t pin_tch_cs;

  int16_t tft_spi_freq;
  int16_t tch_spi_freq;
} setup_t;

// This is a structure to conveniently hold information on the default fonts
// Stores pointer to font character image address table, width table and height

// Create a null set in case some fonts not used (to prevent crash)
const uint8_t widtbl_null[1] = {0};
PROGMEM const uint8_t chr_null[1] = {0};
PROGMEM const uint8_t *const chrtbl_null[1] = {chr_null};

typedef struct
{
  const uint8_t *chartbl;
  const uint8_t *widthtbl;
  uint8_t height;
  uint8_t baseline;
} fontinfo;

// Now fill the structure
const PROGMEM fontinfo fontdata[] = {
#ifdef LOAD_GLCD
    {(const uint8_t *)font, widtbl_null, 0, 0},
#else
    {(const uint8_t *)chrtbl_null, widtbl_null, 0, 0},
#endif
    // GLCD font (Font 1) does not have all parameters
    {(const uint8_t *)chrtbl_null, widtbl_null, 8, 7},

#ifdef LOAD_FONT2
    {(const uint8_t *)chrtbl_f16, widtbl_f16, chr_hgt_f16, baseline_f16},
#else
    {(const uint8_t *)chrtbl_null, widtbl_null, 0, 0},
#endif
};

// Class functions and variables
class ST7789 : public Print
{

public:
  ST7789(int16_t _W = TFT_WIDTH, int16_t _H = TFT_HEIGHT);

  void init(), begin(), end();

  // These are virtual so the TFT_eSprite class can override them with sprite specific functions
  virtual void drawPixel(uint32_t x, uint32_t y, uint32_t color),
      drawChar(int32_t x, int32_t y, unsigned char c, uint32_t color, uint32_t bg, uint8_t size),
      drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color),
      drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color),
      drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color),
      fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);

  virtual int16_t drawChar(unsigned int uniCode, int x, int y, int font),
      drawChar(unsigned int uniCode, int x, int y),
      height(void),
      width(void);

  // The TFT_eSprite class inherits the following functions
  void setWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1),
      pushColor(uint16_t color),
      pushColor(uint16_t color, uint32_t len),
      pushColors(uint16_t *data, uint32_t len, bool swap = true), // With byte swap option
      pushColors(uint8_t *data, uint32_t len),

      fillScreen(uint32_t color);

  void drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color),
      drawRoundRect(int32_t x0, int32_t y0, int32_t w, int32_t h, int32_t radius, uint32_t color),
      fillRoundRect(int32_t x0, int32_t y0, int32_t w, int32_t h, int32_t radius, uint32_t color),

      setRotation(uint8_t r),
      invertDisplay(boolean i),

      drawCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color),
      drawCircleHelper(int32_t x0, int32_t y0, int32_t r, uint8_t cornername, uint32_t color),
      fillCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color),
      fillCircleHelper(int32_t x0, int32_t y0, int32_t r, uint8_t cornername, int32_t delta, uint32_t color),

      drawEllipse(int16_t x0, int16_t y0, int32_t rx, int32_t ry, uint16_t color),
      fillEllipse(int16_t x0, int16_t y0, int32_t rx, int32_t ry, uint16_t color),

      drawTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color),
      fillTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color),

      drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color),
      drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color),
      drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t fgcolor, uint16_t bgcolor),
      setBitmapColor(uint16_t fgcolor, uint16_t bgcolor), // For 1bpp sprites

      setCursor(int16_t x, int16_t y),
      setCursor(int16_t x, int16_t y, uint8_t font),
      setTextColor(uint16_t color),
      setTextColor(uint16_t fgcolor, uint16_t bgcolor),
      setTextSize(uint8_t size),

      setTextWrap(boolean wrapX, boolean wrapY = false),
      setTextDatum(uint8_t datum),
      setTextPadding(uint16_t x_width),

      setFreeFont(uint8_t font),
      setTextFont(uint8_t font),

      spiwrite(uint8_t),
      writecommand(uint8_t c),
      writedata(uint8_t d),

      commandList(const uint8_t *addr);

  // Write a block of pixels to the screen
  void pushRect(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint16_t *data);

  // These are used to render images or sprites stored in RAM arrays
  void pushImage(int32_t x0, int32_t y0, uint32_t w, uint32_t h, uint16_t *data);
  void pushImage(int32_t x0, int32_t y0, uint32_t w, uint32_t h, uint16_t *data, uint16_t transparent);

  // These are used to render images stored in FLASH (PROGMEM)
  void pushImage(int32_t x0, int32_t y0, uint32_t w, uint32_t h, const uint16_t *data, uint16_t transparent);
  void pushImage(int32_t x0, int32_t y0, uint32_t w, uint32_t h, const uint16_t *data);

  // These are used by pushSprite for 1 and 8 bit colours
  void pushImage(int32_t x0, int32_t y0, uint32_t w, uint32_t h, uint8_t *data, bool bpp8 = true);
  void pushImage(int32_t x0, int32_t y0, uint32_t w, uint32_t h, uint8_t *data, uint8_t transparent, bool bpp8 = true);

  // Swap the byte order for pushImage() - corrects endianness
  void setSwapBytes(bool swap);
  bool getSwapBytes(void);

  // This next function has been used successfully to dump the TFT screen to a PC for documentation purposes
  // It reads a screen area and returns the RGB 8 bit colour values of each pixel
  // Set w and h to 1 to read 1 pixel's colour. The data buffer must be at least w * h * 3 bytes
  void readRectRGB(int32_t x0, int32_t y0, int32_t w, int32_t h, uint8_t *data);

  uint8_t getRotation(void),
      getTextDatum(void),
      color16to8(uint16_t color565); // Convert 16 bit colour to 8 bits

  int16_t getCursorX(void),
      getCursorY(void);

  uint16_t fontsLoaded(void);
  uint16_t color565(uint8_t red, uint8_t green, uint8_t blue), // Convert 8 bit red, green and blue to 16 bits
      color8to16(uint8_t color332);                            // Convert 8 bit colour to 16 bits

  int16_t drawNumber(long long_num, int poX, int poY, int font),
      drawNumber(long long_num, int poX, int poY),
      drawFloat(float floatNumber, int decimal, int poX, int poY, int font),
      drawFloat(float floatNumber, int decimal, int poX, int poY),

      // Handle char arrays
      drawString(const char *string, int poX, int poY, int font),
      drawString(const char *string, int poX, int poY),
      drawCentreString(const char *string, int dX, int poY, int font), // Deprecated, use setTextDatum() and drawString()
      drawRightString(const char *string, int dX, int poY, int font),  // Deprecated, use setTextDatum() and drawString()

      // Handle String type
      drawString(const String &string, int poX, int poY, int font),
      drawString(const String &string, int poX, int poY),
      drawCentreString(const String &string, int dX, int poY, int font), // Deprecated, use setTextDatum() and drawString()
      drawRightString(const String &string, int dX, int poY, int font);  // Deprecated, use setTextDatum() and drawString()

  int16_t textWidth(const char *string, int font),
      textWidth(const char *string),
      textWidth(const String &string, int font),
      textWidth(const String &string),
      fontHeight(int16_t font),
      fontHeight(void);

  void setAddrWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye);

  size_t write(uint8_t);

  void getSetup(setup_t &tft_settings); // Sketch provides the instance to populate

  int32_t cursor_x, cursor_y, padX;
  uint32_t textcolor, textbgcolor;

  uint32_t bitmap_fg, bitmap_bg;

  uint8_t textfont, // Current selected font
      textsize,     // Current font size multiplier
      textdatum,    // Text reference datum
      rotation;     // Display rotation (0-3)

  bool invertcolor; // invert display color

  inline void spi_begin() __attribute__((always_inline));
  inline void spi_end() __attribute__((always_inline));

private:
  uint8_t colstart = 0, rowstart = 0; // some ST7735 displays need this changed

  volatile uint32_t *dcport, *csport;

  uint32_t cspinmask, dcpinmask, wrpinmask;

  uint32_t lastColor = 0xFFFF;

  inline void setAddrWindowCore(int32_t xs, int32_t ys, int32_t xe, int32_t ye) __attribute__((always_inline));

  void writeBlock(uint16_t color, uint32_t repeat);

protected:
  int32_t win_xe, win_ye;

  uint32_t _init_width, _init_height; // Display w/h as input, used by setRotation()
  uint32_t _width, _height;           // Display w/h as modified by current rotation
  uint32_t addr_rs, addr_re, addr_cs, addr_ce;
  uint32_t addr_row, addr_col;

  uint32_t fontsloaded;

  uint8_t glyph_ab, // glyph height above baseline
      glyph_bb;     // glyph height below baseline

  bool isDigits;              // adjust bounding box for numbers to reduce visual jiggling
  bool textwrapX, textwrapY;  // If set, 'wrap' text at right and optionally bottom edge of display
  bool _swapBytes;            // Swap the byte order for TFT pushImage()
  bool locked, inTransaction; // Transaction and mutex lock flags for ESP32

  bool _booted;

  uint32_t _lastColor;
}; // End of class ST7789

#endif
