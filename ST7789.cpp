/***************************************************
   Subset of https://github.com/moononournation/TFT_eSPI
 ****************************************************/

#include "ST7789.h"

#include <pgmspace.h>
#include <SPI.h>

// Byte read prototype
uint8_t readByte(void);

// GPIO parallel input/output control
void busDir(uint32_t mask, uint8_t mode);

// If the SPI library has transaction support, these functions
// establish settings and protect from interference from other
// libraries.  Otherwise, they simply do nothing.

inline void ST7789::spi_begin(void)
{
  if (locked)
  {
    locked = false;
    SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, TFT_SPI_MODE));
  }
}

inline void ST7789::spi_end(void)
{
  if (!inTransaction)
  {
    if (!locked)
    {
      locked = true;
      SPI.endTransaction();
    }
  }
}

/***************************************************************************************
** Function name:           ST7789
** Description:             Constructor , we must use hardware SPI pins
***************************************************************************************/
ST7789::ST7789(int16_t w, int16_t h)
{

  // The control pins are deliberately set to the inactive state (CS high) as setup()
  // might call and initialise other SPI peripherals which would could cause conflicts
  // if CS is floating or undefined.
#if (TFT_CS >= 0)
  digitalWrite(TFT_CS, HIGH); // Chip select high (inactive)
  pinMode(TFT_CS, OUTPUT);
#endif

#ifdef TFT_WR
  digitalWrite(TFT_WR, HIGH); // Set write strobe high (inactive)
  pinMode(TFT_WR, OUTPUT);
#endif

#ifdef TFT_DC
  digitalWrite(TFT_DC, HIGH); // Data/Command high = data mode
  pinMode(TFT_DC, OUTPUT);
#endif

#if (TFT_RST >= 0)
  digitalWrite(TFT_RST, HIGH); // Set high, do not share pin with another SPI device
  pinMode(TFT_RST, OUTPUT);
#endif

  _init_width = _width = w;   // Set by specific xxxxx_Defines.h file or by users sketch
  _init_height = _height = h; // Set by specific xxxxx_Defines.h file or by users sketch
  rotation = 0;
  invertcolor = false;
  cursor_y = cursor_x = 0;
  textfont = 1;
  textsize = 1;
  textcolor = bitmap_fg = 0xFFFF;   // White
  textbgcolor = bitmap_bg = 0x0000; // Black
  padX = 0;                         // No padding
  isDigits = false;                 // No bounding box adjustment
  textwrapX = true;                 // Wrap text at end of line when using print stream
  textwrapY = false;                // Wrap text at bottom of screen when using print stream
  textdatum = TL_DATUM;             // Top Left text alignment is default
  fontsloaded = 0;

  _swapBytes = false; // Do not swap colour bytes by default

  locked = true; // ESP32 transaction mutex lock flags
  inTransaction = false;

  _booted = true;

  addr_rs = 0xFFFF;
  addr_re = 0xFFFF;
  addr_cs = 0xFFFF;
  addr_ce = 0xFFFF;

#ifdef LOAD_GLCD
  fontsloaded = 0x0002; // Bit 1 set
#endif

#ifdef LOAD_FONT2
  fontsloaded |= 0x0004; // Bit 2 set
#endif
}

/***************************************************************************************
** Function name:           begin
** Description:             Included for backwards compatibility
***************************************************************************************/
void ST7789::begin()
{
  init();
}

/***************************************************************************************
** Function name:           end
** Description:             Included for backwards compatibility
***************************************************************************************/
void ST7789::end()
{
  writecommand(ST7789_SLPIN); // Sleep in
  SPI.end();

#ifdef TFT_BL
  // Turn off the back-light LED
  digitalWrite(TFT_BL, LOW);
#endif
}

/***************************************************************************************
** Function name:           init (tc is tab colour for ST7735 displays only)
** Description:             Reset, then initialise the TFT display registers
***************************************************************************************/
void ST7789::init()
{
  if (_booted)
  {
#if defined(TFT_MOSI) && !defined(TFT_SPI_OVERLAP)
    SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, -1);
#else
    SPI.begin();
#endif

    inTransaction = false;
    locked = true;

#if (TFT_CS >= 0)
    // Set to output once again in case D6 (MISO) is used for CS
    digitalWrite(TFT_CS, HIGH); // Chip select high (inactive)
    pinMode(TFT_CS, OUTPUT);
#else
    SPI.setHwCs(1);        // Use hardware SS toggling
#endif

    // Set to output once again in case D6 (MISO) is used for DC
#ifdef TFT_DC
    digitalWrite(TFT_DC, HIGH); // Data/Command high = data mode
    pinMode(TFT_DC, OUTPUT);
#endif

    _booted = false;
  } // end of: if just _booted

  // Toggle RST low to reset
  spi_begin();

#if (TFT_RST > 0)
  digitalWrite(TFT_RST, HIGH);
  delay(5);
  digitalWrite(TFT_RST, LOW);
  delay(20);
  digitalWrite(TFT_RST, HIGH);
#else
  writecommand(TFT_SWRST); // Software reset
#endif

  spi_end();
  delay(150); // Wait for reset to complete

  spi_begin();

  // This is the command sequence that initialises the ST7789 driver
  //
  // This setup information uses simple 8 bit SPI writecommand() and writedata() functions
  //
  // See ST7735_Setup.h file for an alternative format

  {
    writecommand(ST7789_SLPOUT); // Sleep out
    delay(120);

    writecommand(ST7789_NORON); // Normal display mode on

    //------------------------------display and color format setting--------------------------------//
    writecommand(ST7789_MADCTL);
    writedata(0x00);
    writedata(0x48);

    // JLX240 display datasheet
    writecommand(0xB6);
    writedata(0x0A);
    writedata(0x82);

    writecommand(ST7789_COLMOD);
    writedata(0x55);

    //--------------------------------ST7789V Frame rate setting----------------------------------//
    writecommand(ST7789_PORCTRL);
    writedata(0x0c);
    writedata(0x0c);
    writedata(0x00);
    writedata(0x33);
    writedata(0x33);

    writecommand(ST7789_GCTRL); // Voltages: VGH / VGL
    writedata(0x35);

    //---------------------------------ST7789V Power setting--------------------------------------//
    writecommand(ST7789_VCOMS);
    writedata(0x28); // JLX240 display datasheet

    writecommand(ST7789_LCMCTRL);
    writedata(0x0C);

    writecommand(ST7789_VDVVRHEN);
    writedata(0x01);
    writedata(0xFF);

    writecommand(ST7789_VRHS); // voltage VRHS
    writedata(0x10);

    writecommand(ST7789_VDVSET);
    writedata(0x20);

    writecommand(ST7789_FRCTR2);
    writedata(0x0f);

    writecommand(ST7789_PWCTRL1);
    writedata(0xa4);
    writedata(0xa1);

    //--------------------------------ST7789V gamma setting---------------------------------------//
    writecommand(ST7789_PVGAMCTRL);
    writedata(0xd0);
    writedata(0x00);
    writedata(0x02);
    writedata(0x07);
    writedata(0x0a);
    writedata(0x28);
    writedata(0x32);
    writedata(0x44);
    writedata(0x42);
    writedata(0x06);
    writedata(0x0e);
    writedata(0x12);
    writedata(0x14);
    writedata(0x17);

    writecommand(ST7789_NVGAMCTRL);
    writedata(0xd0);
    writedata(0x00);
    writedata(0x02);
    writedata(0x07);
    writedata(0x0a);
    writedata(0x28);
    writedata(0x31);
    writedata(0x54);
    writedata(0x47);
    writedata(0x0e);
    writedata(0x1c);
    writedata(0x17);
    writedata(0x1b);
    writedata(0x1e);

    writecommand(ST7789_INVOFF);

    writecommand(ST7789_CASET); // Column address set
    writedata(0x00);
    writedata(0x00);
    writedata(0x00);
    writedata(0xE5); // 239

    writecommand(ST7789_RASET); // Row address set
    writedata(0x00);
    writedata(0x00);
    writedata(0x01);
    writedata(0x3F); // 319

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    spi_end();
    delay(120);
    spi_begin();

    writecommand(ST7789_DISPON); //Display on
  }

  spi_end();

#ifdef TFT_BL
  // Turn on the back-light LED
  digitalWrite(TFT_BL, HIGH);
  pinMode(TFT_BL, OUTPUT);
#endif

  setRotation(rotation);
}

/***************************************************************************************
** Function name:           setRotation
** Description:             rotate the screen orientation m = 0-3 or 4-7 for BMP drawing
***************************************************************************************/
void ST7789::setRotation(uint8_t m)
{

  spi_begin();

  // This is the command sequence that rotates the ST7789 driver coordinate frame

  writecommand(TFT_MADCTL);
  rotation = m % 4;
  switch (rotation)
  {
  case 0: // Portrait
    writedata(TFT_MAD_BGR);
    _width = _init_width;
    _height = _init_height;
#ifdef CGRAM_OFFSET
    colstart = 0;
    rowstart = 0;
#endif
    break;
  case 1: // Landscape (Portrait + 90)
    writedata(TFT_MAD_MX | TFT_MAD_MV | TFT_MAD_BGR);
    _width = _init_height;
    _height = _init_width;
#ifdef CGRAM_OFFSET
    colstart = 0;
    rowstart = 0;
#endif
    break;
  case 2: // Inverter portrait
    writedata(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_BGR);
    _width = _init_width;
    _height = _init_height;
#ifdef CGRAM_OFFSET
    colstart = 240 - TFT_WIDTH;
    rowstart = 320 - TFT_HEIGHT;
#endif
    break;
  case 3: // Inverted landscape
    writedata(TFT_MAD_MV | TFT_MAD_MY | TFT_MAD_BGR);
    _width = _init_height;
    _height = _init_width;
#ifdef CGRAM_OFFSET
    colstart = 320 - TFT_HEIGHT;
    rowstart = 240 - TFT_WIDTH;
#endif
    break;
  }

  delayMicroseconds(10);

  spi_end();

  addr_rs = 0xFFFF;
  addr_re = 0xFFFF;
  addr_cs = 0xFFFF;
  addr_ce = 0xFFFF;
}

/***************************************************************************************
** Function name:           commandList, used for FLASH based lists only (e.g. ST7735)
** Description:             Get initialisation commands from FLASH and send to TFT
***************************************************************************************/
void ST7789::commandList(const uint8_t *addr)
{
  uint8_t numCommands;
  uint8_t numArgs;
  uint8_t ms;

  spi_begin();

  numCommands = pgm_read_byte(addr++); // Number of commands to follow

  while (numCommands--) // For each command...
  {
    writecommand(pgm_read_byte(addr++)); // Read, issue command
    numArgs = pgm_read_byte(addr++);     // Number of args to follow
    ms = numArgs & TFT_INIT_DELAY;       // If hibit set, delay follows args
    numArgs &= ~TFT_INIT_DELAY;          // Mask out delay bit

    while (numArgs--) // For each argument...
    {
      writedata(pgm_read_byte(addr++)); // Read, issue argument
    }

    if (ms)
    {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      delay((ms == 255 ? 500 : ms));
    }
  }
  spi_end();
}

/***************************************************************************************
** Function name:           spiwrite
** Description:             Write 8 bits to SPI port (legacy support only)
***************************************************************************************/
void ST7789::spiwrite(uint8_t c)
{
  tft_Write_8(c);
}

/***************************************************************************************
** Function name:           writecommand
** Description:             Send an 8 bit command to the TFT
***************************************************************************************/
void ST7789::writecommand(uint8_t c)
{
  CS_L;

  tft_Write_C8(c);

  CS_H;
}

/***************************************************************************************
** Function name:           writedata
** Description:             Send a 8 bit data value to the TFT
***************************************************************************************/
void ST7789::writedata(uint8_t d)
{
  CS_L;

  tft_Write_8(d);

  CS_H;
}

/***************************************************************************************
** Function name:           push rectangle (for SPI Interface II i.e. IM [3:0] = "1101")
** Description:             push 565 pixel colours into a defined area
***************************************************************************************/
void ST7789::pushRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t *data)
{
  // Function deprecated, remains for backwards compatibility
  // pushImage() is better as it will crop partly off-screen image blocks
  pushImage(x, y, w, h, data);
}

/***************************************************************************************
** Function name:           pushImage
** Description:             plot 16 bit colour sprite or image onto TFT
***************************************************************************************/
void ST7789::pushImage(int32_t x, int32_t y, uint32_t w, uint32_t h, uint16_t *data)
{
  if ((x >= (int32_t)_width) || (y >= (int32_t)_height))
    return;

  int32_t dx = 0;
  int32_t dy = 0;
  int32_t dw = w;
  int32_t dh = h;

  if (x < 0)
  {
    dw += x;
    dx = -x;
    x = 0;
  }
  if (y < 0)
  {
    dh += y;
    dy = -y;
    y = 0;
  }

  if ((x + w) > _width)
    dw = _width - x;
  if ((y + h) > _height)
    dh = _height - y;

  if (dw < 1 || dh < 1)
    return;

  spi_begin();
  inTransaction = true;

  setAddrWindow(x, y, x + dw - 1, y + dh - 1); // Sets CS low and sent RAMWR

  data += dx + dy * w;

  pushColors(data, dw * dh, _swapBytes);

  CS_H;

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           pushImage
** Description:             plot 16 bit sprite or image with 1 colour being transparent
***************************************************************************************/
void ST7789::pushImage(int32_t x, int32_t y, uint32_t w, uint32_t h, uint16_t *data, uint16_t transp)
{
  if ((x >= (int32_t)_width) || (y >= (int32_t)_height))
    return;

  int32_t dx = 0;
  int32_t dy = 0;
  int32_t dw = w;
  int32_t dh = h;

  if (x < 0)
  {
    dw += x;
    dx = -x;
    x = 0;
  }
  if (y < 0)
  {
    dh += y;
    dy = -y;
    y = 0;
  }

  if ((x + w) > _width)
    dw = _width - x;
  if ((y + h) > _height)
    dh = _height - y;

  if (dw < 1 || dh < 1)
    return;

  spi_begin();
  inTransaction = true;

  data += dx + dy * w;

  int32_t xe = x + dw - 1, ye = y + dh - 1;

  uint16_t lineBuf[dw];

  if (!_swapBytes)
    transp = transp >> 8 | transp << 8;

  while (dh--)
  {
    int32_t len = dw;
    uint16_t *ptr = data;
    int32_t px = x;
    boolean move = true;
    uint16_t np = 0;

    while (len--)
    {
      if (transp != *ptr)
      {
        if (move)
        {
          move = false;
          setAddrWindow(px, y, xe, ye);
        }
        lineBuf[np] = *ptr;
        np++;
      }
      else
      {
        move = true;
        if (np)
        {
          pushColors((uint16_t *)lineBuf, np, _swapBytes);
          np = 0;
        }
      }
      px++;
      ptr++;
    }
    if (np)
      pushColors((uint16_t *)lineBuf, np, _swapBytes);

    y++;
    data += w;
  }

  CS_H;

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           pushImage - for FLASH (PROGMEM) stored images
** Description:             plot 16 bit image
***************************************************************************************/
void ST7789::pushImage(int32_t x, int32_t y, uint32_t w, uint32_t h, const uint16_t *data)
{
  if ((x >= (int32_t)_width) || (y >= (int32_t)_height))
    return;

  int32_t dx = 0;
  int32_t dy = 0;
  int32_t dw = w;
  int32_t dh = h;

  if (x < 0)
  {
    dw += x;
    dx = -x;
    x = 0;
  }
  if (y < 0)
  {
    dh += y;
    dy = -y;
    y = 0;
  }

  if ((x + w) > _width)
    dw = _width - x;
  if ((y + h) > _height)
    dh = _height - y;

  if (dw < 1 || dh < 1)
    return;

  spi_begin();
  inTransaction = true;

  data += dx + dy * w;

  uint16_t buffer[64];
  uint16_t *pix_buffer = buffer;

  setAddrWindow(x, y, x + dw - 1, y + dh - 1);

  // Work out the number whole buffers to send
  uint16_t nb = (dw * dh) / 64;

  // Fill and send "nb" buffers to TFT
  for (int i = 0; i < nb; i++)
  {
    for (int j = 0; j < 64; j++)
    {
      pix_buffer[j] = pgm_read_word(&data[i * 64 + j]);
    }
    pushColors(pix_buffer, 64, !_swapBytes);
  }

  // Work out number of pixels not yet sent
  uint16_t np = (dw * dh) % 64;

  // Send any partial buffer left over
  if (np)
  {
    for (int i = 0; i < np; i++)
    {
      pix_buffer[i] = pgm_read_word(&data[nb * 64 + i]);
    }
    pushColors(pix_buffer, np, !_swapBytes);
  }

  CS_H;

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           pushImage - for FLASH (PROGMEM) stored images
** Description:             plot 16 bit image with 1 colour being transparent
***************************************************************************************/
void ST7789::pushImage(int32_t x, int32_t y, uint32_t w, uint32_t h, const uint16_t *data, uint16_t transp)
{
  if ((x >= (int32_t)_width) || (y >= (int32_t)_height))
    return;

  int32_t dx = 0;
  int32_t dy = 0;
  int32_t dw = w;
  int32_t dh = h;

  if (x < 0)
  {
    dw += x;
    dx = -x;
    x = 0;
  }
  if (y < 0)
  {
    dh += y;
    dy = -y;
    y = 0;
  }

  if ((x + w) > _width)
    dw = _width - x;
  if ((y + h) > _height)
    dh = _height - y;

  if (dw < 1 || dh < 1)
    return;

  spi_begin();
  inTransaction = true;

  data += dx + dy * w;

  int32_t xe = x + dw - 1, ye = y + dh - 1;

  uint16_t lineBuf[dw];

  if (_swapBytes)
    transp = transp >> 8 | transp << 8;

  while (dh--)
  {
    int32_t len = dw;
    uint16_t *ptr = (uint16_t *)data;
    int32_t px = x;
    boolean move = true;

    uint16_t np = 0;

    while (len--)
    {
      uint16_t color = pgm_read_word(ptr);
      if (transp != color)
      {
        if (move)
        {
          move = false;
          setAddrWindow(px, y, xe, ye);
        }
        lineBuf[np] = color;
        np++;
      }
      else
      {
        move = true;
        if (np)
        {
          pushColors(lineBuf, np, !_swapBytes);
          np = 0;
        }
      }
      px++;
      ptr++;
    }
    if (np)
      pushColors(lineBuf, np, !_swapBytes);

    y++;
    data += w;
  }

  CS_H;

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           pushImage
** Description:             plot 8 bit image or sprite using a line buffer
***************************************************************************************/
void ST7789::pushImage(int32_t x, int32_t y, uint32_t w, uint32_t h, uint8_t *data, bool bpp8)
{
  if ((x >= (int32_t)_width) || (y >= (int32_t)_height))
    return;

  int32_t dx = 0;
  int32_t dy = 0;
  int32_t dw = w;
  int32_t dh = h;

  if (x < 0)
  {
    dw += x;
    dx = -x;
    x = 0;
  }
  if (y < 0)
  {
    dh += y;
    dy = -y;
    y = 0;
  }

  if ((x + w) > _width)
    dw = _width - x;
  if ((y + h) > _height)
    dh = _height - y;

  if (dw < 1 || dh < 1)
    return;

  spi_begin();
  inTransaction = true;

  setAddrWindow(x, y, x + dw - 1, y + dh - 1); // Sets CS low and sent RAMWR

  // Line buffer makes plotting faster
  uint16_t lineBuf[dw];

  if (bpp8)
  {
    uint8_t blue[] = {0, 11, 21, 31}; // blue 2 to 5 bit colour lookup table

    _lastColor = -1; // Set to illegal value

    // Used to store last shifted colour
    uint8_t msbColor = 0;
    uint8_t lsbColor = 0;

    data += dx + dy * w;
    while (dh--)
    {
      uint32_t len = dw;
      uint8_t *ptr = data;
      uint8_t *linePtr = (uint8_t *)lineBuf;

      while (len--)
      {
        uint32_t color = *ptr++;

        // Shifts are slow so check if colour has changed first
        if (color != _lastColor)
        {
          //          =====Green=====     ===============Red==============
          msbColor = (color & 0x1C) >> 2 | (color & 0xC0) >> 3 | (color & 0xE0);
          //          =====Green=====    =======Blue======
          lsbColor = (color & 0x1C) << 3 | blue[color & 0x03];
          _lastColor = color;
        }

        *linePtr++ = msbColor;
        *linePtr++ = lsbColor;
      }

      pushColors(lineBuf, dw, false);

      data += w;
    }
  }
  else
  {
    while (dh--)
    {
      w = (w + 7) & 0xFFF8;

      int32_t len = dw;
      uint8_t *ptr = data;
      uint8_t *linePtr = (uint8_t *)lineBuf;
      uint8_t bits = 8;
      while (len > 0)
      {
        if (len < 8)
          bits = len;
        uint32_t xp = dx;
        for (uint16_t i = 0; i < bits; i++)
        {
          uint8_t col = (ptr[(xp + dy * w) >> 3] << (xp & 0x7)) & 0x80;
          if (col)
          {
            *linePtr++ = bitmap_fg >> 8;
            *linePtr++ = (uint8_t)bitmap_fg;
          }
          else
          {
            *linePtr++ = bitmap_bg >> 8;
            *linePtr++ = (uint8_t)bitmap_bg;
          }
          //if (col) drawPixel((dw-len)+xp,h-dh,bitmap_fg);
          //else     drawPixel((dw-len)+xp,h-dh,bitmap_bg);
          xp++;
        }
        *ptr++;
        len -= 8;
      }

      pushColors(lineBuf, dw, false);

      dy++;
    }
  }

  CS_H;

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           pushImage
** Description:             plot 8 or 1 bit image or sprite with a transparent colour
***************************************************************************************/
void ST7789::pushImage(int32_t x, int32_t y, uint32_t w, uint32_t h, uint8_t *data, uint8_t transp, bool bpp8)
{
  if ((x >= (int32_t)_width) || (y >= (int32_t)_height))
    return;

  int32_t dx = 0;
  int32_t dy = 0;
  int32_t dw = w;
  int32_t dh = h;

  if (x < 0)
  {
    dw += x;
    dx = -x;
    x = 0;
  }
  if (y < 0)
  {
    dh += y;
    dy = -y;
    y = 0;
  }

  if ((x + w) > _width)
    dw = _width - x;
  if ((y + h) > _height)
    dh = _height - y;

  if (dw < 1 || dh < 1)
    return;

  spi_begin();
  inTransaction = true;

  int32_t xe = x + dw - 1, ye = y + dh - 1;

  // Line buffer makes plotting faster
  uint16_t lineBuf[dw];

  if (bpp8)
  {
    data += dx + dy * w;

    uint8_t blue[] = {0, 11, 21, 31}; // blue 2 to 5 bit colour lookup table

    _lastColor = -1; // Set to illegal value

    // Used to store last shifted colour
    uint8_t msbColor = 0;
    uint8_t lsbColor = 0;

    //int32_t spx = x, spy = y;

    while (dh--)
    {
      int32_t len = dw;
      uint8_t *ptr = data;
      uint8_t *linePtr = (uint8_t *)lineBuf;

      int32_t px = x;
      boolean move = true;
      uint16_t np = 0;

      while (len--)
      {
        if (transp != *ptr)
        {
          if (move)
          {
            move = false;
            setAddrWindow(px, y, xe, ye);
          }
          uint8_t color = *ptr;

          // Shifts are slow so check if colour has changed first
          if (color != _lastColor)
          {
            //          =====Green=====     ===============Red==============
            msbColor = (color & 0x1C) >> 2 | (color & 0xC0) >> 3 | (color & 0xE0);
            //          =====Green=====    =======Blue======
            lsbColor = (color & 0x1C) << 3 | blue[color & 0x03];
            _lastColor = color;
          }
          *linePtr++ = msbColor;
          *linePtr++ = lsbColor;
          np++;
        }
        else
        {
          move = true;
          if (np)
          {
            pushColors(lineBuf, np, false);
            linePtr = (uint8_t *)lineBuf;
            np = 0;
          }
        }
        px++;
        ptr++;
      }

      if (np)
        pushColors(lineBuf, np, false);

      y++;
      data += w;
    }
  }
  else
  {
    w = (w + 7) & 0xFFF8;
    while (dh--)
    {
      int32_t px = x;
      boolean move = true;
      uint16_t np = 0;
      int32_t len = dw;
      uint8_t *ptr = data;
      uint8_t bits = 8;
      while (len > 0)
      {
        if (len < 8)
          bits = len;
        uint32_t xp = dx;
        uint32_t yp = (dy * w) >> 3;
        for (uint16_t i = 0; i < bits; i++)
        {
          //uint8_t col = (ptr[(xp + dy * w)>>3] << (xp & 0x7)) & 0x80;
          if ((ptr[(xp >> 3) + yp] << (xp & 0x7)) & 0x80)
          {
            if (move)
            {
              move = false;
              setAddrWindow(px, y, xe, ye);
            }
            np++;
          }
          else
          {
            if (np)
            {
              pushColor(bitmap_fg, np);
              np = 0;
              move = true;
            }
          }
          px++;
          xp++;
        }
        *ptr++;
        len -= 8;
      }
      if (np)
        pushColor(bitmap_fg, np);
      y++;
      dy++;
    }
  }

  CS_H;

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           setSwapBytes
** Description:             Used by 16 bit pushImage() to swap byte order in colours
***************************************************************************************/
void ST7789::setSwapBytes(bool swap)
{
  _swapBytes = swap;
}

/***************************************************************************************
** Function name:           getSwapBytes
** Description:             Return the swap byte order for colours
***************************************************************************************/
bool ST7789::getSwapBytes(void)
{
  return _swapBytes;
}

/***************************************************************************************
** Function name:           drawCircle
** Description:             Draw a circle outline
***************************************************************************************/
// Optimised midpoint circle algorithm
void ST7789::drawCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color)
{
  int32_t x = 0;
  int32_t dx = 1;
  int32_t dy = r + r;
  int32_t p = -(r >> 1);

  spi_begin();
  inTransaction = true;

  // These are ordered to minimise coordinate changes in x or y
  // drawPixel can then send fewer bounding box commands
  drawPixel(x0 + r, y0, color);
  drawPixel(x0 - r, y0, color);
  drawPixel(x0, y0 - r, color);
  drawPixel(x0, y0 + r, color);

  while (x < r)
  {

    if (p >= 0)
    {
      dy -= 2;
      p -= dy;
      r--;
    }

    dx += 2;
    p += dx;

    x++;

    // These are ordered to minimise coordinate changes in x or y
    // drawPixel can then send fewer bounding box commands
    drawPixel(x0 + x, y0 + r, color);
    drawPixel(x0 - x, y0 + r, color);
    drawPixel(x0 - x, y0 - r, color);
    drawPixel(x0 + x, y0 - r, color);

    drawPixel(x0 + r, y0 + x, color);
    drawPixel(x0 - r, y0 + x, color);
    drawPixel(x0 - r, y0 - x, color);
    drawPixel(x0 + r, y0 - x, color);
  }

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           drawCircleHelper
** Description:             Support function for circle drawing
***************************************************************************************/
void ST7789::drawCircleHelper(int32_t x0, int32_t y0, int32_t r, uint8_t cornername, uint32_t color)
{
  int32_t f = 1 - r;
  int32_t ddF_x = 1;
  int32_t ddF_y = -2 * r;
  int32_t x = 0;

  while (x < r)
  {
    if (f >= 0)
    {
      r--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if (cornername & 0x4)
    {
      drawPixel(x0 + x, y0 + r, color);
      drawPixel(x0 + r, y0 + x, color);
    }
    if (cornername & 0x2)
    {
      drawPixel(x0 + x, y0 - r, color);
      drawPixel(x0 + r, y0 - x, color);
    }
    if (cornername & 0x8)
    {
      drawPixel(x0 - r, y0 + x, color);
      drawPixel(x0 - x, y0 + r, color);
    }
    if (cornername & 0x1)
    {
      drawPixel(x0 - r, y0 - x, color);
      drawPixel(x0 - x, y0 - r, color);
    }
  }
}

/***************************************************************************************
** Function name:           fillCircle
** Description:             draw a filled circle
***************************************************************************************/
// Optimised midpoint circle algorithm, changed to horizontal lines (faster in sprites)
void ST7789::fillCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color)
{
  int32_t x = 0;
  int32_t dx = 1;
  int32_t dy = r + r;
  int32_t p = -(r >> 1);

  spi_begin();
  inTransaction = true;

  drawFastHLine(x0 - r, y0, dy + 1, color);

  while (x < r)
  {

    if (p >= 0)
    {
      dy -= 2;
      p -= dy;
      r--;
    }

    dx += 2;
    p += dx;

    x++;

    drawFastHLine(x0 - r, y0 + x, 2 * r + 1, color);
    drawFastHLine(x0 - r, y0 - x, 2 * r + 1, color);
    drawFastHLine(x0 - x, y0 + r, 2 * x + 1, color);
    drawFastHLine(x0 - x, y0 - r, 2 * x + 1, color);
  }

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           fillCircleHelper
** Description:             Support function for filled circle drawing
***************************************************************************************/
// Used to support drawing roundrects, changed to horizontal lines (faster in sprites)
void ST7789::fillCircleHelper(int32_t x0, int32_t y0, int32_t r, uint8_t cornername, int32_t delta, uint32_t color)
{
  int32_t f = 1 - r;
  int32_t ddF_x = 1;
  int32_t ddF_y = -r - r;
  int32_t y = 0;

  delta++;
  while (y < r)
  {
    if (f >= 0)
    {
      r--;
      ddF_y += 2;
      f += ddF_y;
    }
    y++;
    //x++;
    ddF_x += 2;
    f += ddF_x;

    if (cornername & 0x1)
    {
      drawFastHLine(x0 - r, y0 + y, r + r + delta, color);
      drawFastHLine(x0 - y, y0 + r, y + y + delta, color);
    }
    if (cornername & 0x2)
    {
      drawFastHLine(x0 - r, y0 - y, r + r + delta, color); // 11995, 1090
      drawFastHLine(x0 - y, y0 - r, y + y + delta, color);
    }
  }
}

/***************************************************************************************
** Function name:           drawEllipse
** Description:             Draw a ellipse outline
***************************************************************************************/
void ST7789::drawEllipse(int16_t x0, int16_t y0, int32_t rx, int32_t ry, uint16_t color)
{
  if (rx < 2)
    return;
  if (ry < 2)
    return;
  int32_t x, y;
  int32_t rx2 = rx * rx;
  int32_t ry2 = ry * ry;
  int32_t fx2 = 4 * rx2;
  int32_t fy2 = 4 * ry2;
  int32_t s;

  spi_begin();
  inTransaction = true;

  for (x = 0, y = ry, s = 2 * ry2 + rx2 * (1 - 2 * ry); ry2 * x <= rx2 * y; x++)
  {
    // These are ordered to minimise coordinate changes in x or y
    // drawPixel can then send fewer bounding box commands
    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + x, y0 - y, color);
    if (s >= 0)
    {
      s += fx2 * (1 - y);
      y--;
    }
    s += ry2 * ((4 * x) + 6);
  }

  for (x = rx, y = 0, s = 2 * rx2 + ry2 * (1 - 2 * rx); rx2 * y <= ry2 * x; y++)
  {
    // These are ordered to minimise coordinate changes in x or y
    // drawPixel can then send fewer bounding box commands
    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + x, y0 - y, color);
    if (s >= 0)
    {
      s += fy2 * (1 - x);
      x--;
    }
    s += rx2 * ((4 * y) + 6);
  }

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           fillEllipse
** Description:             draw a filled ellipse
***************************************************************************************/
void ST7789::fillEllipse(int16_t x0, int16_t y0, int32_t rx, int32_t ry, uint16_t color)
{
  if (rx < 2)
    return;
  if (ry < 2)
    return;
  int32_t x, y;
  int32_t rx2 = rx * rx;
  int32_t ry2 = ry * ry;
  int32_t fx2 = 4 * rx2;
  int32_t fy2 = 4 * ry2;
  int32_t s;

  spi_begin();
  inTransaction = true;

  for (x = 0, y = ry, s = 2 * ry2 + rx2 * (1 - 2 * ry); ry2 * x <= rx2 * y; x++)
  {
    drawFastHLine(x0 - x, y0 - y, x + x + 1, color);
    drawFastHLine(x0 - x, y0 + y, x + x + 1, color);

    if (s >= 0)
    {
      s += fx2 * (1 - y);
      y--;
    }
    s += ry2 * ((4 * x) + 6);
  }

  for (x = rx, y = 0, s = 2 * rx2 + ry2 * (1 - 2 * rx); rx2 * y <= ry2 * x; y++)
  {
    drawFastHLine(x0 - x, y0 - y, x + x + 1, color);
    drawFastHLine(x0 - x, y0 + y, x + x + 1, color);

    if (s >= 0)
    {
      s += fy2 * (1 - x);
      x--;
    }
    s += rx2 * ((4 * y) + 6);
  }

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           fillScreen
** Description:             Clear the screen to defined colour
***************************************************************************************/
void ST7789::fillScreen(uint32_t color)
{
  fillRect(0, 0, _width, _height, color);
}

/***************************************************************************************
** Function name:           drawRect
** Description:             Draw a rectangle outline
***************************************************************************************/
// Draw a rectangle
void ST7789::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
{
  spi_begin();
  inTransaction = true;

  drawFastHLine(x, y, w, color);
  drawFastHLine(x, y + h - 1, w, color);
  drawFastVLine(x, y, h, color);
  drawFastVLine(x + w - 1, y, h, color);

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           drawRoundRect
** Description:             Draw a rounded corner rectangle outline
***************************************************************************************/
// Draw a rounded rectangle
void ST7789::drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color)
{
  spi_begin();
  inTransaction = true;

  // smarter version
  drawFastHLine(x + r, y, w - r - r, color);         // Top
  drawFastHLine(x + r, y + h - 1, w - r - r, color); // Bottom
  drawFastVLine(x, y + r, h - r - r, color);         // Left
  drawFastVLine(x + w - 1, y + r, h - r - r, color); // Right
  // draw four corners
  drawCircleHelper(x + r, y + r, r, 1, color);
  drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
  drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
  drawCircleHelper(x + r, y + h - r - 1, r, 8, color);

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           fillRoundRect
** Description:             Draw a rounded corner filled rectangle
***************************************************************************************/
// Fill a rounded rectangle, changed to horizontal lines (faster in sprites)
void ST7789::fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color)
{
  spi_begin();
  inTransaction = true;

  // smarter version
  fillRect(x, y + r, w, h - r - r, color);

  // draw four corners
  fillCircleHelper(x + r, y + h - r - 1, r, 1, w - r - r - 1, color);
  fillCircleHelper(x + r, y + r, r, 2, w - r - r - 1, color);

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           drawTriangle
** Description:             Draw a triangle outline using 3 arbitrary points
***************************************************************************************/
// Draw a triangle
void ST7789::drawTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
  spi_begin();
  inTransaction = true;

  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           fillTriangle
** Description:             Draw a filled triangle using 3 arbitrary points
***************************************************************************************/
// Fill a triangle - original Adafruit function works well and code footprint is small
void ST7789::fillTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
  int32_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1)
  {
    swap_coord(y0, y1);
    swap_coord(x0, x1);
  }
  if (y1 > y2)
  {
    swap_coord(y2, y1);
    swap_coord(x2, x1);
  }
  if (y0 > y1)
  {
    swap_coord(y0, y1);
    swap_coord(x0, x1);
  }

  if (y0 == y2)
  { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if (x1 < a)
      a = x1;
    else if (x1 > b)
      b = x1;
    if (x2 < a)
      a = x2;
    else if (x2 > b)
      b = x2;
    drawFastHLine(a, y0, b - a + 1, color);
    return;
  }

  spi_begin();
  inTransaction = true;

  int32_t
      dx01 = x1 - x0,
      dy01 = y1 - y0,
      dx02 = x2 - x0,
      dy02 = y2 - y0,
      dx12 = x2 - x1,
      dy12 = y2 - y1,
      sa = 0,
      sb = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if (y1 == y2)
    last = y1; // Include y1 scanline
  else
    last = y1 - 1; // Skip it

  for (y = y0; y <= last; y++)
  {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;

    if (a > b)
      swap_coord(a, b);
    drawFastHLine(a, y, b - a + 1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for (; y <= y2; y++)
  {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;

    if (a > b)
      swap_coord(a, b);
    drawFastHLine(a, y, b - a + 1, color);
  }

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           drawBitmap
** Description:             Draw an image stored in an array on the TFT
***************************************************************************************/
void ST7789::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  spi_begin();
  inTransaction = true;

  int32_t i, j, byteWidth = (w + 7) / 8;

  for (j = 0; j < h; j++)
  {
    for (i = 0; i < w; i++)
    {
      if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7)))
      {
        drawPixel(x + i, y + j, color);
      }
    }
  }

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           drawXBitmap
** Description:             Draw an image stored in an XBM array onto the TFT
***************************************************************************************/
void ST7789::drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  spi_begin();
  inTransaction = true;

  int32_t i, j, byteWidth = (w + 7) / 8;

  for (j = 0; j < h; j++)
  {
    for (i = 0; i < w; i++)
    {
      if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (1 << (i & 7)))
      {
        drawPixel(x + i, y + j, color);
      }
    }
  }

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           drawXBitmap
** Description:             Draw an XBM image with foreground and background colors
***************************************************************************************/
void ST7789::drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bgcolor)
{
  spi_begin();
  inTransaction = true;

  int32_t i, j, byteWidth = (w + 7) / 8;

  for (j = 0; j < h; j++)
  {
    for (i = 0; i < w; i++)
    {
      if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (1 << (i & 7)))
        drawPixel(x + i, y + j, color);
      else
        drawPixel(x + i, y + j, bgcolor);
    }
  }

  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           setCursor
** Description:             Set the text cursor x,y position
***************************************************************************************/
void ST7789::setCursor(int16_t x, int16_t y)
{
  cursor_x = x;
  cursor_y = y;
}

/***************************************************************************************
** Function name:           setCursor
** Description:             Set the text cursor x,y position and font
***************************************************************************************/
void ST7789::setCursor(int16_t x, int16_t y, uint8_t font)
{
  textfont = font;
  cursor_x = x;
  cursor_y = y;
}

/***************************************************************************************
** Function name:           getCursorX
** Description:             Get the text cursor x position
***************************************************************************************/
int16_t ST7789::getCursorX(void)
{
  return cursor_x;
}

/***************************************************************************************
** Function name:           getCursorY
** Description:             Get the text cursor y position
***************************************************************************************/
int16_t ST7789::getCursorY(void)
{
  return cursor_y;
}

/***************************************************************************************
** Function name:           setTextSize
** Description:             Set the text size multiplier
***************************************************************************************/
void ST7789::setTextSize(uint8_t s)
{
  if (s > 7)
    s = 7;                    // Limit the maximum size multiplier so byte variables can be used for rendering
  textsize = (s > 0) ? s : 1; // Don't allow font size 0
}

/***************************************************************************************
** Function name:           setTextColor
** Description:             Set the font foreground colour (background is transparent)
***************************************************************************************/
void ST7789::setTextColor(uint16_t c)
{
  // For 'transparent' background, we'll set the bg
  // to the same as fg instead of using a flag
  textcolor = textbgcolor = c;
}

/***************************************************************************************
** Function name:           setTextColor
** Description:             Set the font foreground and background colour
***************************************************************************************/
void ST7789::setTextColor(uint16_t c, uint16_t b)
{
  textcolor = c;
  textbgcolor = b;
}

/***************************************************************************************
** Function name:           setBitmapColor
** Description:             Set the foreground foreground and background colour
***************************************************************************************/
void ST7789::setBitmapColor(uint16_t c, uint16_t b)
{
  if (c == b)
    b = ~c;
  bitmap_fg = c;
  bitmap_bg = b;
}

/***************************************************************************************
** Function name:           setTextWrap
** Description:             Define if text should wrap at end of line
***************************************************************************************/
void ST7789::setTextWrap(boolean wrapX, boolean wrapY)
{
  textwrapX = wrapX;
  textwrapY = wrapY;
}

/***************************************************************************************
** Function name:           setTextDatum
** Description:             Set the text position reference datum
***************************************************************************************/
void ST7789::setTextDatum(uint8_t d)
{
  textdatum = d;
}

/***************************************************************************************
** Function name:           setTextPadding
** Description:             Define padding width (aids erasing old text and numbers)
***************************************************************************************/
void ST7789::setTextPadding(uint16_t x_width)
{
  padX = x_width;
}

/***************************************************************************************
** Function name:           getRotation
** Description:             Return the rotation value (as used by setRotation())
***************************************************************************************/
uint8_t ST7789::getRotation(void)
{
  return rotation;
}

/***************************************************************************************
** Function name:           getTextDatum
** Description:             Return the text datum value (as used by setTextDatum())
***************************************************************************************/
uint8_t ST7789::getTextDatum(void)
{
  return textdatum;
}

/***************************************************************************************
** Function name:           width
** Description:             Return the pixel width of display (per current rotation)
***************************************************************************************/
// Return the size of the display (per current rotation)
int16_t ST7789::width(void)
{
  return _width;
}

/***************************************************************************************
** Function name:           height
** Description:             Return the pixel height of display (per current rotation)
***************************************************************************************/
int16_t ST7789::height(void)
{
  return _height;
}

/***************************************************************************************
** Function name:           textWidth
** Description:             Return the width in pixels of a string in a given font
***************************************************************************************/
int16_t ST7789::textWidth(const String &string)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return textWidth(buffer, textfont);
}

int16_t ST7789::textWidth(const String &string, int font)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return textWidth(buffer, font);
}

int16_t ST7789::textWidth(const char *string)
{
  return textWidth(string, textfont);
}

int16_t ST7789::textWidth(const char *string, int font)
{
  int str_width = 0;

  unsigned char uniCode;
  char *widthtable;

  if (font > 1 && font < 9)
  {
    widthtable = (char *)pgm_read_dword(&(fontdata[font].widthtbl)) - 32; //subtract the 32 outside the loop

    while (*string)
    {
      uniCode = *(string++);
      if (uniCode > 31 && uniCode < 128)
        str_width += pgm_read_byte(widthtable + uniCode); // Normally we need to subract 32 from uniCode
      else
        str_width += pgm_read_byte(widthtable + 32); // Set illegal character = space width
    }
  }
  else
  {
#ifdef LOAD_GLCD
    while (*string++)
      str_width += 6;
#endif
  }
  isDigits = false;
  return str_width * textsize;
}

/***************************************************************************************
** Function name:           fontsLoaded
** Description:             return an encoded 16 bit value showing the fonts loaded
***************************************************************************************/
// Returns a value showing which fonts are loaded (bit N set =  Font N loaded)

uint16_t ST7789::fontsLoaded(void)
{
  return fontsloaded;
}

/***************************************************************************************
** Function name:           fontHeight
** Description:             return the height of a font (yAdvance for free fonts)
***************************************************************************************/
int16_t ST7789::fontHeight(int16_t font)
{
  return pgm_read_byte(&fontdata[font].height) * textsize;
}

int16_t ST7789::fontHeight(void)
{
  return fontHeight(textfont);
}

/***************************************************************************************
** Function name:           drawChar
** Description:             draw a single character in the Adafruit GLCD font
***************************************************************************************/
void ST7789::drawChar(int32_t x, int32_t y, unsigned char c, uint32_t color, uint32_t bg, uint8_t size)
{
  if ((x >= (int16_t)_width) ||   // Clip right
      (y >= (int16_t)_height) ||  // Clip bottom
      ((x + 6 * size - 1) < 0) || // Clip left
      ((y + 8 * size - 1) < 0))   // Clip top
    return;

  if (c < 32)
    return;

#ifdef LOAD_GLCD
  boolean fillbg = (bg != color);

  if ((size == 1) && fillbg)
  {
    uint8_t column[6];
    uint8_t mask = 0x1;
    spi_begin();
    //inTransaction = true;
    setAddrWindow(x, y, x + 5, y + 8);
    for (int8_t i = 0; i < 5; i++)
      column[i] = pgm_read_byte(font + (c * 5) + i);
    column[5] = 0;

    for (int8_t j = 0; j < 8; j++)
    {
      for (int8_t k = 0; k < 5; k++)
      {
        if (column[k] & mask)
        {
          tft_Write_Color(color);
        }
        else
        {
          tft_Write_Color(bg);
        }
      }
      mask <<= 1;
      tft_Write_Color(bg);
    }

    CS_H;
    //inTransaction = false;
    spi_end();
  }
  else
  {
    spi_begin();
    inTransaction = true;
    for (int8_t i = 0; i < 6; i++)
    {
      uint8_t line;
      if (i == 5)
        line = 0x0;
      else
        line = pgm_read_byte(font + (c * 5) + i);

      if (size == 1) // default size
      {
        for (int8_t j = 0; j < 8; j++)
        {
          if (line & 0x1)
            drawPixel(x + i, y + j, color);
          line >>= 1;
        }
      }
      else
      { // big size
        for (int8_t j = 0; j < 8; j++)
        {
          if (line & 0x1)
            fillRect(x + (i * size), y + (j * size), size, size, color);
          else if (fillbg)
            fillRect(x + i * size, y + j * size, size, size, bg);
          line >>= 1;
        }
      }
    }
    inTransaction = false;
    spi_end();
  }
#endif
}

/***************************************************************************************
** Function name:           setWindow
** Description:             define an area to receive a stream of pixels
***************************************************************************************/
// Chip select is high at the end of this function
void ST7789::setWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
  spi_begin();
  setAddrWindow(x0, y0, x1, y1);
  CS_H;
  spi_end();
}

/***************************************************************************************
** Function name:           setAddrWindow
** Description:             define an area to receive a stream of pixels
***************************************************************************************/
// Chip select stays low, use setWindow() from sketches

inline void ST7789::setAddrWindow(int32_t xs, int32_t ys, int32_t xe, int32_t ye)
{
  //spi_begin();

#ifdef CGRAM_OFFSET
  xs += colstart;
  xe += colstart;
  ys += rowstart;
  ye += rowstart;
#endif

  CS_L;

  setAddrWindowCore(xs, ys, xe, ye);

  //spi_end();
}

inline void ST7789::setAddrWindowCore(int32_t xs, int32_t ys, int32_t xe, int32_t ye)
{
  if ((addr_cs != xs) || (addr_ce != xe))
  {
    // Column addr set
    tft_Write_C8(TFT_CASET);

    // Load the two coords as a 32 bit value and shift in one go
    tft_write_16_16(xs, xe);

    addr_cs = xs;
    addr_ce = xe;
  }

  if ((addr_rs != ys) || (addr_re != ye))
  {
    // Row addr set
    tft_Write_C8(TFT_PASET);

    // Load the two coords as a 32 bit value and shift in one go
    tft_write_16_16(ys, ye);

    addr_rs = ys;
    addr_re = ye;
  }

  // write to RAM
  tft_Write_C8(TFT_RAMWR);
}

/***************************************************************************************
** Function name:           drawPixel
** Description:             push a single pixel at an arbitrary position
***************************************************************************************/
void ST7789::drawPixel(uint32_t x, uint32_t y, uint32_t color)
{
  // Faster range checking, possible because x and y are unsigned
  if ((x >= _width) || (y >= _height))
    return;

#ifdef CGRAM_OFFSET
  x += colstart;
  y += rowstart;
#endif

  spi_begin();

  CS_L;

  setAddrWindowCore(x, y, x, y);
  tft_Write_Color(color);

  CS_H;

  spi_end();
}

/***************************************************************************************
** Function name:           pushColor
** Description:             push a single pixel
***************************************************************************************/
void ST7789::pushColor(uint16_t color)
{
  spi_begin();

  CS_L;

  tft_Write_Color(color);

  CS_H;

  spi_end();
}

/***************************************************************************************
** Function name:           pushColor
** Description:             push a single colour to "len" pixels
***************************************************************************************/
void ST7789::pushColor(uint16_t color, uint32_t len)
{
  spi_begin();

  CS_L;

  writeBlock(color, len);

  CS_H;

  spi_end();
}

/***************************************************************************************
** Function name:           pushColors
** Description:             push an array of pixels, for image drawing
***************************************************************************************/
void ST7789::pushColors(uint16_t *data, uint32_t len, bool swap)
{
  spi_begin();

  CS_L;

  if (swap)
    SPI.writePixels(data, len << 1);
  else
    SPI.writeBytes((uint8_t *)data, len << 1);

  CS_H;

  spi_end();
}

/***************************************************************************************
** Function name:           drawLine
** Description:             draw a line between 2 arbitrary points
***************************************************************************************/
// Bresenham's algorithm - thx wikipedia - speed enhanced by Bodmer to use
// an efficient FastH/V Line draw routine for line segments of 2 pixels or more
void ST7789::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color)
{
  spi_begin();
  inTransaction = true;
  boolean steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep)
  {
    swap_coord(x0, y0);
    swap_coord(x1, y1);
  }

  if (x0 > x1)
  {
    swap_coord(x0, x1);
    swap_coord(y0, y1);
  }

  int32_t dx = x1 - x0, dy = abs(y1 - y0);
  ;

  int32_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;

  if (y0 < y1)
    ystep = 1;

  // Split into steep and not steep for FastH/V separation
  if (steep)
  {
    for (; x0 <= x1; x0++)
    {
      dlen++;
      err -= dy;
      if (err < 0)
      {
        err += dx;
        if (dlen == 1)
          drawPixel(y0, xs, color);
        else
          drawFastVLine(y0, xs, dlen, color);
        dlen = 0;
        y0 += ystep;
        xs = x0 + 1;
      }
    }
    if (dlen)
      drawFastVLine(y0, xs, dlen, color);
  }
  else
  {
    for (; x0 <= x1; x0++)
    {
      dlen++;
      err -= dy;
      if (err < 0)
      {
        err += dx;
        if (dlen == 1)
          drawPixel(xs, y0, color);
        else
          drawFastHLine(xs, y0, dlen, color);
        dlen = 0;
        y0 += ystep;
        xs = x0 + 1;
      }
    }
    if (dlen)
      drawFastHLine(xs, y0, dlen, color);
  }
  inTransaction = false;
  spi_end();
}

/***************************************************************************************
** Function name:           drawFastVLine
** Description:             draw a vertical line
***************************************************************************************/
void ST7789::drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color)
{
  // Rudimentary clipping
  if ((x >= _width) || (y >= _height) || (h < 1))
    return;
  if ((y + h - 1) >= _height)
    h = _height - y;

  spi_begin();

  setAddrWindow(x, y, x, y + h - 1);

  writeBlock(color, h);

  CS_H;

  spi_end();
}

/***************************************************************************************
** Function name:           drawFastHLine
** Description:             draw a horizontal line
***************************************************************************************/
void ST7789::drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color)
{
  // Rudimentary clipping
  if ((x >= _width) || (y >= _height) || (w < 1))
    return;
  if ((x + w - 1) >= _width)
    w = _width - x;

  spi_begin();
  setAddrWindow(x, y, x + w - 1, y);

  writeBlock(color, w);

  CS_H;

  spi_end();
}

/***************************************************************************************
** Function name:           fillRect
** Description:             draw a filled rectangle
***************************************************************************************/
void ST7789::fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
{
  // rudimentary clipping (drawChar w/big text requires this)
  if ((x > _width) || (y > _height) || (w < 1) || (h < 1))
    return;
  if ((x + w - 1) > _width)
    w = _width - x;
  if ((y + h - 1) > _height)
    h = _height - y;

  spi_begin();
  setAddrWindow(x, y, x + w - 1, y + h - 1);

  uint32_t n = (uint32_t)w * (uint32_t)h;

  writeBlock(color, n);

  CS_H;

  spi_end();
}

/***************************************************************************************
** Function name:           color565
** Description:             convert three 8 bit RGB levels to a 16 bit colour value
***************************************************************************************/
uint16_t ST7789::color565(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/***************************************************************************************
** Function name:           color16to8
** Description:             convert 16 bit colour to an 8 bit 332 RGB colour value
***************************************************************************************/
uint8_t ST7789::color16to8(uint16_t c)
{
  return ((c & 0xE000) >> 8) | ((c & 0x0700) >> 6) | ((c & 0x0018) >> 3);
}

/***************************************************************************************
** Function name:           color8to16
** Description:             convert 8 bit colour to a 16 bit 565 colour value
***************************************************************************************/
uint16_t ST7789::color8to16(uint8_t color)
{
  uint8_t blue[] = {0, 11, 21, 31}; // blue 2 to 5 bit colour lookup table
  uint16_t color16 = 0;

  //        =====Green=====     ===============Red==============
  color16 = (color & 0x1C) << 6 | (color & 0xC0) << 5 | (color & 0xE0) << 8;
  //        =====Green=====    =======Blue======
  color16 |= (color & 0x1C) << 3 | blue[color & 0x03];

  return color16;
}

/***************************************************************************************
** Function name:           invertDisplay
** Description:             invert the display colours i = 1 invert, i = 0 normal
***************************************************************************************/
void ST7789::invertDisplay(boolean i)
{
  invertcolor = i;

  spi_begin();
#if TFT_INVON
  // Send the command twice as otherwise it does not always work!
  writecommand(i ? TFT_INVON : TFT_INVOFF);
  writecommand(i ? TFT_INVON : TFT_INVOFF);
#endif
  spi_end();
}

/***************************************************************************************
** Function name:           write
** Description:             draw characters piped through serial stream
***************************************************************************************/
size_t ST7789::write(uint8_t utf8)
{
  if (utf8 == '\r')
    return 1;

  uint8_t uniCode = utf8; // Work with a copy
  if (utf8 == '\n')
    uniCode += 22; // Make it a valid space character to stop errors
  else if (utf8 < 32)
    return 0;

  uint16_t width = 0;
  uint16_t height = 0;

#ifdef LOAD_FONT2
  if (textfont == 2)
  {
    if (utf8 > 127)
      return 0;
    // This is 20us faster than using the fontdata structure (0.443ms per character instead of 0.465ms)
    width = pgm_read_byte(widtbl_f16 + uniCode - 32);
    height = chr_hgt_f16;
    // Font 2 is rendered in whole byte widths so we must allow for this
    width = (width + 6) / 8; // Width in whole bytes for font 2, should be + 7 but must allow for font width change
    width = width * 8;       // Width converted back to pixels
  }
#endif

#ifdef LOAD_GLCD
  if (textfont == 1)
  {
    width = 6;
    height = 8;
  }
#else
  if (textfont == 1)
    return 0;
#endif

  height = height * textsize;

  if (utf8 == '\n')
  {
    cursor_y += height;
    cursor_x = 0;
  }
  else
  {
    if (textwrapX && (cursor_x + width * textsize > _width))
    {
      cursor_y += height;
      cursor_x = 0;
    }
    if (textwrapY && (cursor_y >= _height))
      cursor_y = 0;
    cursor_x += drawChar(uniCode, cursor_x, cursor_y, textfont);
  }

  return 1;
}

/***************************************************************************************
** Function name:           drawChar
** Description:             draw a Unicode onto the screen
***************************************************************************************/
int16_t ST7789::drawChar(unsigned int uniCode, int x, int y)
{
  return drawChar(uniCode, x, y, textfont);
}

int16_t ST7789::drawChar(unsigned int uniCode, int x, int y, int font)
{

  if (font == 1)
  {
#ifdef LOAD_GLCD
    drawChar(x, y, uniCode, textcolor, textbgcolor, textsize);
    return 6 * textsize;
#else
    return 0;
#endif
  }

  if ((font > 1) && (font < 9) && ((uniCode < 32) || (uniCode > 127)))
    return 0;

  int width = 0;
  int height = 0;
  uint32_t flash_address = 0;
  uniCode -= 32;

#ifdef LOAD_FONT2
  if (font == 2)
  {
    // This is faster than using the fontdata structure
    flash_address = pgm_read_dword(&chrtbl_f16[uniCode]);
    width = pgm_read_byte(widtbl_f16 + uniCode);
    height = chr_hgt_f16;
  }
#endif

  int w = width;
  int pX = 0;
  int pY = y;
  uint8_t line = 0;

#ifdef LOAD_FONT2 // chop out code if we do not need it
  if (font == 2)
  {
    w = w + 6; // Should be + 7 but we need to compensate for width increment
    w = w / 8;
    if (x + width * textsize >= (int16_t)_width)
      return width * textsize;

    if (textcolor == textbgcolor || textsize != 1)
    {
      spi_begin();
      inTransaction = true;

      for (int i = 0; i < height; i++)
      {
        if (textcolor != textbgcolor)
          fillRect(x, pY, width * textsize, textsize, textbgcolor);

        for (int k = 0; k < w; k++)
        {
          line = pgm_read_byte((uint8_t *)flash_address + w * i + k);
          if (line)
          {
            if (textsize == 1)
            {
              pX = x + k * 8;
              if (line & 0x80)
                drawPixel(pX, pY, textcolor);
              if (line & 0x40)
                drawPixel(pX + 1, pY, textcolor);
              if (line & 0x20)
                drawPixel(pX + 2, pY, textcolor);
              if (line & 0x10)
                drawPixel(pX + 3, pY, textcolor);
              if (line & 0x08)
                drawPixel(pX + 4, pY, textcolor);
              if (line & 0x04)
                drawPixel(pX + 5, pY, textcolor);
              if (line & 0x02)
                drawPixel(pX + 6, pY, textcolor);
              if (line & 0x01)
                drawPixel(pX + 7, pY, textcolor);
            }
            else
            {
              pX = x + k * 8 * textsize;
              if (line & 0x80)
                fillRect(pX, pY, textsize, textsize, textcolor);
              if (line & 0x40)
                fillRect(pX + textsize, pY, textsize, textsize, textcolor);
              if (line & 0x20)
                fillRect(pX + 2 * textsize, pY, textsize, textsize, textcolor);
              if (line & 0x10)
                fillRect(pX + 3 * textsize, pY, textsize, textsize, textcolor);
              if (line & 0x08)
                fillRect(pX + 4 * textsize, pY, textsize, textsize, textcolor);
              if (line & 0x04)
                fillRect(pX + 5 * textsize, pY, textsize, textsize, textcolor);
              if (line & 0x02)
                fillRect(pX + 6 * textsize, pY, textsize, textsize, textcolor);
              if (line & 0x01)
                fillRect(pX + 7 * textsize, pY, textsize, textsize, textcolor);
            }
          }
        }
        pY += textsize;
      }

      inTransaction = false;
      spi_end();
    }
    else
    // Faster drawing of characters and background using block write
    {
      spi_begin();
      setAddrWindow(x, y, (x + w * 8) - 1, y + height - 1);

      uint8_t mask;
      for (int i = 0; i < height; i++)
      {
        for (int k = 0; k < w; k++)
        {
          line = pgm_read_byte((uint8_t *)flash_address + w * i + k);
          pX = x + k * 8;
          mask = 0x80;
          while (mask)
          {
            if (line & mask)
            {
              tft_Write_Color(textcolor);
            }
            else
            {
              tft_Write_Color(textbgcolor);
            }
            mask = mask >> 1;
          }
        }
        pY += textsize;
      }

      CS_H;
      spi_end();
    }
  }
#endif //FONT2

  return width * textsize; // x +
}

/***************************************************************************************
** Function name:           drawString (with or without user defined font)
** Description :            draw string with padding if it is defined
***************************************************************************************/
// Without font number, uses font set by setTextFont()
int16_t ST7789::drawString(const String &string, int poX, int poY)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return drawString(buffer, poX, poY, textfont);
}
// With font number
int16_t ST7789::drawString(const String &string, int poX, int poY, int font)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return drawString(buffer, poX, poY, font);
}

// Without font number, uses font set by setTextFont()
int16_t ST7789::drawString(const char *string, int poX, int poY)
{
  return drawString(string, poX, poY, textfont);
}

// With font number
int16_t ST7789::drawString(const char *string, int poX, int poY, int font)
{
  int16_t sumX = 0;
  uint8_t padding = 1, baseline = 0;
  uint16_t cwidth = textWidth(string, font); // Find the pixel width of the string in the font
  uint16_t cheight = 8 * textsize;

  if (font != 1)
  {
    baseline = pgm_read_byte(&fontdata[font].baseline) * textsize;
    cheight = fontHeight(font);
  }

  if (textdatum || padX)
  {

    switch (textdatum)
    {
    case TC_DATUM:
      poX -= cwidth / 2;
      padding += 1;
      break;
    case TR_DATUM:
      poX -= cwidth;
      padding += 2;
      break;
    case ML_DATUM:
      poY -= cheight / 2;
      //padding += 0;
      break;
    case MC_DATUM:
      poX -= cwidth / 2;
      poY -= cheight / 2;
      padding += 1;
      break;
    case MR_DATUM:
      poX -= cwidth;
      poY -= cheight / 2;
      padding += 2;
      break;
    case BL_DATUM:
      poY -= cheight;
      //padding += 0;
      break;
    case BC_DATUM:
      poX -= cwidth / 2;
      poY -= cheight;
      padding += 1;
      break;
    case BR_DATUM:
      poX -= cwidth;
      poY -= cheight;
      padding += 2;
      break;
    case L_BASELINE:
      poY -= baseline;
      //padding += 0;
      break;
    case C_BASELINE:
      poX -= cwidth / 2;
      poY -= baseline;
      padding += 1;
      break;
    case R_BASELINE:
      poX -= cwidth;
      poY -= baseline;
      padding += 2;
      break;
    }
    // Check coordinates are OK, adjust if not
    if (poX < 0)
      poX = 0;
    if (poX + cwidth > width())
      poX = width() - cwidth;
    if (poY < 0)
      poY = 0;
    if (poY + cheight - baseline > height())
      poY = height() - cheight;
  }

  int8_t xo = 0;

  while (*string)
    sumX += drawChar(*(string++), poX + sumX, poY, font);

  if ((padX > cwidth) && (textcolor != textbgcolor))
  {
    int16_t padXc = poX + cwidth + xo;
    switch (padding)
    {
    case 1:
      fillRect(padXc, poY, padX - cwidth, cheight, textbgcolor);
      break;
    case 2:
      fillRect(padXc, poY, (padX - cwidth) >> 1, cheight, textbgcolor);
      padXc = (padX - cwidth) >> 1;
      if (padXc > poX)
        padXc = poX;
      fillRect(poX - padXc, poY, (padX - cwidth) >> 1, cheight, textbgcolor);
      break;
    case 3:
      if (padXc > padX)
        padXc = padX;
      fillRect(poX + cwidth - padXc, poY, padXc - cwidth, cheight, textbgcolor);
      break;
    }
  }

  return sumX;
}

/***************************************************************************************
** Function name:           drawCentreString (deprecated, use setTextDatum())
** Descriptions:            draw string centred on dX
***************************************************************************************/
int16_t ST7789::drawCentreString(const String &string, int dX, int poY, int font)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return drawCentreString(buffer, dX, poY, font);
}

int16_t ST7789::drawCentreString(const char *string, int dX, int poY, int font)
{
  uint8_t tempdatum = textdatum;
  int sumX = 0;
  textdatum = TC_DATUM;
  sumX = drawString(string, dX, poY, font);
  textdatum = tempdatum;
  return sumX;
}

/***************************************************************************************
** Function name:           drawRightString (deprecated, use setTextDatum())
** Descriptions:            draw string right justified to dX
***************************************************************************************/
int16_t ST7789::drawRightString(const String &string, int dX, int poY, int font)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return drawRightString(buffer, dX, poY, font);
}

int16_t ST7789::drawRightString(const char *string, int dX, int poY, int font)
{
  uint8_t tempdatum = textdatum;
  int16_t sumX = 0;
  textdatum = TR_DATUM;
  sumX = drawString(string, dX, poY, font);
  textdatum = tempdatum;
  return sumX;
}

/***************************************************************************************
** Function name:           drawNumber
** Description:             draw a long integer
***************************************************************************************/
int16_t ST7789::drawNumber(long long_num, int poX, int poY)
{
  isDigits = true; // Eliminate jiggle in monospaced fonts
  char str[12];
  ltoa(long_num, str, 10);
  return drawString(str, poX, poY, textfont);
}

int16_t ST7789::drawNumber(long long_num, int poX, int poY, int font)
{
  isDigits = true; // Eliminate jiggle in monospaced fonts
  char str[12];
  ltoa(long_num, str, 10);
  return drawString(str, poX, poY, font);
}

/***************************************************************************************
** Function name:           drawFloat
** Descriptions:            drawFloat, prints 7 non zero digits maximum
***************************************************************************************/
// Assemble and print a string, this permits alignment relative to a datum
// looks complicated but much more compact and actually faster than using print class
int16_t ST7789::drawFloat(float floatNumber, int dp, int poX, int poY)
{
  return drawFloat(floatNumber, dp, poX, poY, textfont);
}

int16_t ST7789::drawFloat(float floatNumber, int dp, int poX, int poY, int font)
{
  isDigits = true;
  char str[14];         // Array to contain decimal string
  uint8_t ptr = 0;      // Initialise pointer for array
  int8_t digits = 1;    // Count the digits to avoid array overflow
  float rounding = 0.5; // Round up down delta

  if (dp > 7)
    dp = 7; // Limit the size of decimal portion

  // Adjust the rounding value
  for (uint8_t i = 0; i < dp; ++i)
    rounding /= 10.0;

  if (floatNumber < -rounding) // add sign, avoid adding - sign to 0.0!
  {
    str[ptr++] = '-';           // Negative number
    str[ptr] = 0;               // Put a null in the array as a precaution
    digits = 0;                 // Set digits to 0 to compensate so pointer value can be used later
    floatNumber = -floatNumber; // Make positive
  }

  floatNumber += rounding; // Round up or down

  // For error put ... in string and return (all ST7789 library fonts contain . character)
  if (floatNumber >= 2147483647)
  {
    strcpy(str, "...");
    return drawString(str, poX, poY, font);
  }
  // No chance of overflow from here on

  // Get integer part
  unsigned long temp = (unsigned long)floatNumber;

  // Put integer part into array
  ltoa(temp, str + ptr, 10);

  // Find out where the null is to get the digit count loaded
  while ((uint8_t)str[ptr] != 0)
    ptr++;       // Move the pointer along
  digits += ptr; // Count the digits

  str[ptr++] = '.'; // Add decimal point
  str[ptr] = '0';   // Add a dummy zero
  str[ptr + 1] = 0; // Add a null but don't increment pointer so it can be overwritten

  // Get the decimal portion
  floatNumber = floatNumber - temp;

  // Get decimal digits one by one and put in array
  // Limit digit count so we don't get a false sense of resolution
  uint8_t i = 0;
  while ((i < dp) && (digits < 9)) // while (i < dp) for no limit but array size must be increased
  {
    i++;
    floatNumber *= 10;  // for the next decimal
    temp = floatNumber; // get the decimal
    ltoa(temp, str + ptr, 10);
    ptr++;
    digits++;            // Increment pointer and digits count
    floatNumber -= temp; // Remove that digit
  }

  // Finally we can plot the string and return pixel length
  return drawString(str, poX, poY, font);
}

/***************************************************************************************
** Function name:           setFreeFont
** Descriptions:            Sets the GFX free font to use
***************************************************************************************/

// Alternative to setTextFont() so we don't need two different named functions
void ST7789::setFreeFont(uint8_t font)
{
  setTextFont(font);
}

/***************************************************************************************
** Function name:           setTextFont
** Description:             Set the font for the print stream
***************************************************************************************/
void ST7789::setTextFont(uint8_t f)
{
  textfont = (f > 0) ? f : 1; // Don't allow font 0
}

/***************************************************************************************
** Function name:           writeBlock
** Description:             Write a block of pixels of the same colour
***************************************************************************************/
//Clear screen test 76.8ms theoretical. 81.5ms ST7789, 967ms Adafruit_ILI9341
//Performance 26.15Mbps@26.66MHz, 39.04Mbps@40MHz, 75.4Mbps@80MHz SPI clock
//Efficiency:
//       ST7789       98.06%              97.59%          94.24%
//       Adafruit_GFX   19.62%              14.31%           7.94%
//
inline void ST7789::writeBlock(uint16_t color, uint32_t repeat)
{
  uint16_t color16 = (color >> 8) | (color << 8);
  uint32_t color32 = color16 | color16 << 16;

  if (repeat > 31)
  {
    WRITE_PERI_REG((SPI_MOSI_DLEN_REG(SPI_NUM)), D512_MASK);
    while (repeat > 31)
    {
      for (uint32_t i = 0; i < 16; i++)
        WRITE_PERI_REG((SPI_W0_REG(SPI_NUM) + (i << 2)), color32);
      SET_PERI_REG_MASK(SPI_CMD_REG(SPI_NUM), SPI_USR);
      repeat -= 32;
      while (READ_PERI_REG(SPI_CMD_REG(SPI_NUM)) & SPI_USR)
        ;
    }
  }

  if (repeat)
  {
    WRITE_PERI_REG((SPI_MOSI_DLEN_REG(SPI_NUM)), MASK | ((((repeat << 4) - 1) & SPI_USR_MOSI_DBITLEN) << (SPI_USR_MOSI_DBITLEN_S)));
    for (uint32_t i = 0; i < ((repeat + 1) >> 1); i++)
      WRITE_PERI_REG((SPI_W0_REG(SPI_NUM) + (i << 2)), color32);
    SET_PERI_REG_MASK(SPI_CMD_REG(SPI_NUM), SPI_USR);
    while (READ_PERI_REG(SPI_CMD_REG(SPI_NUM)) & SPI_USR)
      ;
  }
}
