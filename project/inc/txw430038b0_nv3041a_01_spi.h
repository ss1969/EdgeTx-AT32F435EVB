#ifndef __TXW430038B0_NV3041A_01_SPI_H
#define __TXW430038B0_NV3041A_01_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "edgetx_lcd_compat.h"

coord_t txw430038b0_nv3041a_01_getWidth(void);
coord_t txw430038b0_nv3041a_01_getHeight(void);
void txw430038b0_nv3041a_01_spi_setWindow(coord_t x, coord_t y, coord_t w, coord_t h);
/* EdgeTX-facing bitmap entry point. Prefer this over calling panel primitives
   directly when integrating with the UI layer. */
void txw430038b0_nv3041a_01_spi_drawBitmap(BitmapBuffer *dc, coord_t x, coord_t y);
/* Stub for future EdgeTX text rendering integration. */
void txw430038b0_nv3041a_01_spi_drawString(BitmapBuffer *dc, const char *s, coord_t x, coord_t y, LcdFlags flags);
void txw430038b0_nv3041a_01_spi_sleep(void);
void txw430038b0_nv3041a_01_spi_wakeup(void);

#ifdef __cplusplus
}
#endif

#endif
