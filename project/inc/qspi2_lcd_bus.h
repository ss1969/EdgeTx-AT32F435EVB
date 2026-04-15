#ifndef __QSPI2_LCD_BUS_H
#define __QSPI2_LCD_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "at32f435_437_wk_config.h"

int qspi2_lcd_bus_write(uint8_t cmd, const uint8_t *data, uint32_t len);
int qspi2_lcd_bus_read(uint8_t cmd, uint8_t *data, uint32_t len, uint8_t dummy_cycles);
int qspi2_lcd_bus_write_pixels_rgb565(const uint16_t *pixels, uint32_t pixel_count);

#ifdef __cplusplus
}
#endif

#endif
