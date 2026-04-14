#ifndef __QSPI2_LCD_BUS_H
#define __QSPI2_LCD_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "at32f435_437_wk_config.h"
#include "at32f435_437_qspi.h"
#include "wk_system.h"

typedef struct
{
  uint32_t cmd_w0;
  uint32_t cmd_w1;
  uint32_t cmd_w2;
  uint32_t cmd_w3;
  uint32_t ctrl;
  uint32_t fifosts;
  uint32_t ctrl2;
  uint32_t cmdsts;
  uint32_t rsts;
  uint32_t fsize;
  uint32_t xip_cmd_w0;
  uint32_t xip_cmd_w1;
  uint32_t xip_cmd_w2;
  uint32_t xip_cmd_w3;
  uint32_t ctrl3;
} qspi2_lcd_bus_diag_t;

void qspi2_lcd_bus_reset_pulse(uint32_t low_ms, uint32_t high_ms);
int qspi2_lcd_bus_write(uint8_t cmd, const uint8_t *data, uint32_t len);
int qspi2_lcd_bus_read(uint8_t cmd, uint8_t *data, uint32_t len, uint8_t dummy_cycles);
int qspi2_lcd_bus_write_pixels_rgb565(const uint16_t *pixels, uint32_t pixel_count);
int qspi2_lcd_bus_wait_te_rise(uint32_t timeout);
int qspi2_lcd_bus_wait_te_fall(uint32_t timeout);
int qspi2_lcd_bus_read_03h(uint8_t addr, uint8_t *data, uint32_t len, uint8_t dummy_cycles);
void qspi2_lcd_bus_diag_capture(qspi2_lcd_bus_diag_t *out);
void qspi2_lcd_bus_diag_last_get(qspi2_lcd_bus_diag_t *after_kick, qspi2_lcd_bus_diag_t *before_exit);

#ifdef __cplusplus
}
#endif

#endif
