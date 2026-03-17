#ifndef __SPI1_LCD_BUS_H
#define __SPI1_LCD_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "at32f435_437_wk_config.h"

#ifndef SPI_LCD_BUS_SPI
#define SPI_LCD_BUS_SPI SPI1
#endif

#ifndef SPI_LCD_BUS_CRM_CLOCK
#define SPI_LCD_BUS_CRM_CLOCK CRM_SPI1_PERIPH_CLOCK
#endif

#ifndef SPI_LCD_BUS_USE_HW_CS
#define SPI_LCD_BUS_USE_HW_CS 1
#endif

#ifndef SPI_LCD_BUS_USE_DMA
#define SPI_LCD_BUS_USE_DMA 1
#endif

void spi_lcd_bus_init(void);
void spi_lcd_bus_reset_pulse(uint32_t low_ms, uint32_t high_ms);
void spi_lcd_bus_write_cmd(uint8_t cmd);
void spi_lcd_bus_write_cmd_data(uint8_t cmd, const uint8_t *data, uint32_t len);
int spi_lcd_bus_write_pixels_rgb565(uint8_t cmd, const uint16_t *pixels, uint32_t pixel_count);

#ifdef __cplusplus
}
#endif

#endif
