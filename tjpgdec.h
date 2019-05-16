#ifndef _TJPGDECH_
#define _TJPGDECH_

#include <SD.h>
#include <FS.h>

// Buffer is created during jpeg decode for sending data
// Total size of the buffer is  2 * (JPG_IMAGE_LINE_BUF_SIZE * 3)
// The size must be multiple of 256 bytes !!
#define JPG_IMAGE_LINE_BUF_SIZE 512
#define WORK_BUF_SIZE 3800 // Size of the working buffer (must be power of 2)

// 24-bit color type structure
typedef struct __attribute__((__packed__))
{
    //typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

// ================ JPG SUPPORT ================================================
// User defined device identifier
typedef struct
{
    File f;             // File handler for input function
    int x;              // image top left point X position
    int y;              // image top left point Y position
    uint8_t *membuff;   // memory buffer containing the image
    uint32_t bufsize;   // size of the memory buffer
    uint32_t bufptr;    // memory buffer current position
    color_t *linbuf[2]; // memory buffer used for display output
    uint8_t linbuf_idx;
} JPGIODEV;

#endif
