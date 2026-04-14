#ifndef __TXW430038B0_NV3041A_01_SPI_H
#define __TXW430038B0_NV3041A_01_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "at32f435_437_wk_config.h"
#include "at32f435_437_qspi.h"

#define TXW430038B0_WIDTH 480U
#define TXW430038B0_HEIGHT 272U

#define TXW430038B0_LCD_BUS_MODE_SPI1  1
#define TXW430038B0_LCD_BUS_MODE_QSPI2 2

#ifndef TXW430038B0_LCD_BUS_MODE
#define TXW430038B0_LCD_BUS_MODE TXW430038B0_LCD_BUS_MODE_QSPI2
#endif

typedef enum {
  BMP_FORMAT_RGB565 = 0,
  BMP_FORMAT_ARGB32,
  BMP_FORMAT_LCD1,
  BMP_FORMAT_LCD4,
  BMP_FORMAT_LCD8
} BitmapFormat;

typedef int16_t coord_t;

typedef struct BitmapBuffer {
  uint16_t width;
  uint16_t height;
  uint16_t format;
  uint8_t *data;
} BitmapBuffer;

typedef uint32_t LcdFlags;

typedef enum {
  TXW430038B0_QSPI_GRAM_MODE_RAM32
} txw430038b0_qspi_gram_mode_t;

const char *txw430038b0_nv3041a_01_name(void);
int txw430038b0_nv3041a_01_init(void);
int txw430038b0_nv3041a_01_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
int txw430038b0_nv3041a_01_write_pixels_rgb565(const uint16_t *pixels, uint32_t pixel_count);
coord_t txw430038b0_nv3041a_01_getWidth(void);
coord_t txw430038b0_nv3041a_01_getHeight(void);
void txw430038b0_nv3041a_01_spi_setWindow(coord_t x, coord_t y, coord_t w, coord_t h);
void txw430038b0_nv3041a_01_spi_drawBitmap(BitmapBuffer *dc, coord_t x, coord_t y);
void txw430038b0_nv3041a_01_spi_drawString(BitmapBuffer *dc, const char *s, coord_t x, coord_t y, LcdFlags flags);
void txw430038b0_nv3041a_01_spi_sleep(void);
void txw430038b0_nv3041a_01_spi_wakeup(void);

#ifdef __cplusplus
}
#endif

#endif
