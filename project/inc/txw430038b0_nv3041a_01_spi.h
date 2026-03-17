#ifndef __TXW430038B0_NV3041A_01_SPI1_H
#define __TXW430038B0_NV3041A_01_SPI1_H

#ifdef __cplusplus
extern "C" {
#endif

#include "at32f435_437_wk_config.h"

#define TXW430038B0_WIDTH 480U
#define TXW430038B0_HEIGHT 272U

typedef enum {
  BMP_FORMAT_RGB565 = 0,   // 16位，R(5)G(6)B(5)
  BMP_FORMAT_ARGB32,       // 32位，A(8)R(8)G(8)B(8)
  BMP_FORMAT_LCD1,         // 1位单色 (黑白)
  BMP_FORMAT_LCD4,         // 4位灰度
  BMP_FORMAT_LCD8,         // 8位灰度
} BitmapFormat;

typedef int16_t coord_t;

typedef struct BitmapBuffer {
  uint16_t width;
  uint16_t height;
  uint16_t format;  // 如：RGB565, ARGB32
  uint8_t *data;    // 指向实际像素数据的指针
} BitmapBuffer;

typedef uint32_t LcdFlags;

const char *txw430038b0_nv3041a_01_spi1_name(void);
int txw430038b0_nv3041a_01_spi_init(void);
int txw430038b0_nv3041a_01_spi_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
int txw430038b0_nv3041a_01_spi_write_pixels_rgb565(const uint16_t *pixels, uint32_t pixel_count);
coord_t txw430038b0_nv3041a_01_getWidth();
coord_t txw430038b0_nv3041a_01_getHeight();
void txw430038b0_nv3041a_01_spi_setWindow(coord_t x, coord_t y, coord_t w, coord_t h);
void txw430038b0_nv3041a_01_spi_refresh();
void txw430038b0_nv3041a_01_spi_drawBitmap(BitmapBuffer *dc, coord_t x, coord_t y);
void txw430038b0_nv3041a_01_spi_drawString(BitmapBuffer *dc, const char *s, coord_t x, coord_t y, LcdFlags flags);
void txw430038b0_nv3041a_01_spi_sleep();
void txw430038b0_nv3041a_01_spi_wakeup();
#ifdef __cplusplus
}
#endif

#endif
