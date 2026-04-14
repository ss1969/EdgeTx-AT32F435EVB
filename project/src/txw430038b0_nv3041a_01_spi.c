#include "txw430038b0_nv3041a_01_spi.h"

#include "wk_system.h"

#if TXW430038B0_LCD_BUS_MODE == TXW430038B0_LCD_BUS_MODE_QSPI2
#include "qspi2_lcd_bus.h"
#elif TXW430038B0_LCD_BUS_MODE == TXW430038B0_LCD_BUS_MODE_SPI1
#include "spi_lcd_bus.h"
#else
#error "Unsupported TXW430038B0_LCD_BUS_MODE"
#endif

typedef struct
{
  uint8_t cmd;
  const uint8_t *data;
  uint8_t data_bytes;
  uint16_t delay_ms;
} nv3041a01_init_cmd_t;

static void txw430038b0_vendor_reset_sequence(void)
{
  /* ---------Reset LCD Driver ------- */
  wk_delay_ms(60);
  gpio_bits_set(QSPI2_RESET_GPIO_PORT, QSPI2_RESET_PIN);
  wk_delay_ms(60);
  gpio_bits_reset(QSPI2_RESET_GPIO_PORT, QSPI2_RESET_PIN);
  wk_delay_ms(250);
  gpio_bits_set(QSPI2_RESET_GPIO_PORT, QSPI2_RESET_PIN);
  wk_delay_ms(180);
}

static const nv3041a01_init_cmd_t nv3041a01_init_cmds[] = {
  /* NV3041A-01+BOE4.3IPS(GV043WQQ-N10-8QP0)480x272_16MCU_AN_V1.2 20240118 */
  /* ---------Start Initial Code ------ */
  {0xFF, (const uint8_t[]){0xA5}, 1, 0},
  {0xE7, (const uint8_t[]){0x10}, 1, 0}, /* TE_output_en */
  {0x35, (const uint8_t[]){0x01}, 1, 0}, /* TE_interface_en */
  {0x36, (const uint8_t[]){0x00}, 1, 0}, /* MACTL b3=1 for BGR*/
  {0x3A, (const uint8_t[]){0x01}, 1, 0},
  {0x40, (const uint8_t[]){0x01}, 1, 0}, /* 01:IPS/00:TN */
  {0x41, (const uint8_t[]){0x03}, 1, 0}, /* 01--8bit//03--16bit */
  {0x43, (const uint8_t[]){0x00}, 1, 0}, /* QSPI_DCTL fixed: bgr=0, sbyte=0 for RAM32 RGB565 Red 1st */
  {0x55, (const uint8_t[]){0x01}, 1, 0},
  {0x44, (const uint8_t[]){0x15}, 1, 0},
  {0x45, (const uint8_t[]){0x15}, 1, 0},
  {0x7D, (const uint8_t[]){0x03}, 1, 0}, /* vdds_trim[2:0] */
  {0xC1, (const uint8_t[]){0xAB}, 1, 0}, /* avdd_clp_en avdd_clp[1:0] avcl_clp_en avcl_clp[1:0] */
  {0xC2, (const uint8_t[]){0x17}, 1, 0}, /* vgh_clp_en vgl_clp[2:0] */
  {0xC3, (const uint8_t[]){0x10}, 1, 0}, /* vgl_clp_en vgl_clp[2:0] */
  {0xC6, (const uint8_t[]){0x3A}, 1, 0}, /* avdd_ratio_sel avcl_ratio_sel vgh_ratio_sel[1:0] vgl_ratio_sel[1:0] */
  {0xC7, (const uint8_t[]){0x25}, 1, 0}, /* mv_clk_sel[1:0] avdd_clk_sel[1:0] avcl_clk_sel[1:0] */
  {0xC8, (const uint8_t[]){0x11}, 1, 0},
  {0x6F, (const uint8_t[]){0x2F}, 1, 0}, /* user_gvdd */
  {0x78, (const uint8_t[]){0x4B}, 1, 0}, /* user_gvcl */
  {0x7A, (const uint8_t[]){0x49}, 1, 0}, /* user_vgsp */
  {0xC9, (const uint8_t[]){0x00}, 1, 0},
  {0x51, (const uint8_t[]){0x20}, 1, 0}, /* gate_st_o[7:0] */
  {0x52, (const uint8_t[]){0x7C}, 1, 0}, /* gate_ed_o[7:0] */
  {0x53, (const uint8_t[]){0x1C}, 1, 0}, /* gate_st_e[7:0] */
  {0x54, (const uint8_t[]){0x77}, 1, 0}, /* gate_ed_e[7:0] */
  {0x46, (const uint8_t[]){0x0A}, 1, 0}, /* fsm_hbp_o[5:0] */
  {0x47, (const uint8_t[]){0x2A}, 1, 0}, /* fsm_hfp_o[5:0] */
  {0x48, (const uint8_t[]){0x0A}, 1, 0}, /* fsm_hbp_e[5:0] */
  {0x49, (const uint8_t[]){0x1A}, 1, 0}, /* fsm_hfp_e[5:0] */
  {0x56, (const uint8_t[]){0x43}, 1, 0},
  {0x57, (const uint8_t[]){0x42}, 1, 0},
  {0x58, (const uint8_t[]){0x3C}, 1, 0},
  {0x59, (const uint8_t[]){0x64}, 1, 0},
  {0x5A, (const uint8_t[]){0x41}, 1, 0},
  {0x5B, (const uint8_t[]){0x3C}, 1, 0},
  {0x5C, (const uint8_t[]){0x02}, 1, 0},
  {0x5D, (const uint8_t[]){0x3C}, 1, 0},
  {0x5E, (const uint8_t[]){0x1F}, 1, 0},
  {0x60, (const uint8_t[]){0x80}, 1, 0},
  {0x61, (const uint8_t[]){0x3F}, 1, 0},
  {0x62, (const uint8_t[]){0x21}, 1, 0},
  {0x63, (const uint8_t[]){0x07}, 1, 0},
  {0x64, (const uint8_t[]){0xE0}, 1, 0},
  {0x65, (const uint8_t[]){0x01}, 1, 0}, /* chopper */
  {0xCA, (const uint8_t[]){0x20}, 1, 0},
  {0xCB, (const uint8_t[]){0x52}, 1, 0},
  {0xCC, (const uint8_t[]){0x10}, 1, 0},
  {0xCD, (const uint8_t[]){0x42}, 1, 0},
  {0xD0, (const uint8_t[]){0x20}, 1, 0},
  {0xD1, (const uint8_t[]){0x52}, 1, 0},
  {0xD2, (const uint8_t[]){0x10}, 1, 0},
  {0xD3, (const uint8_t[]){0x42}, 1, 0},
  {0xD4, (const uint8_t[]){0x0A}, 1, 0},
  {0xD5, (const uint8_t[]){0x32}, 1, 0},
  {0xE5, (const uint8_t[]){0x05}, 1, 0}, /* DVDD_TRIM */
  {0xE6, (const uint8_t[]){0x00}, 1, 0}, /* ESD_CTRL */
  {0x6E, (const uint8_t[]){0x14}, 1, 0}, /* LVD_en */
  /* gammma 01 */
  {0x80, (const uint8_t[]){0x04}, 1, 0}, /* gam_vrp0 */
  {0xA0, (const uint8_t[]){0x00}, 1, 0}, /* gam_VRN0 */
  {0x81, (const uint8_t[]){0x07}, 1, 0}, /* gam_vrp1 */
  {0xA1, (const uint8_t[]){0x05}, 1, 0}, /* gam_VRN1 */
  {0x82, (const uint8_t[]){0x06}, 1, 0}, /* gam_vrp2 */
  {0xA2, (const uint8_t[]){0x04}, 1, 0}, /* gam_VRN2 */
  {0x83, (const uint8_t[]){0x39}, 1, 0}, /* gam_vrp3 */
  {0xA3, (const uint8_t[]){0x39}, 1, 0}, /* gam_VRN3 */
  {0x84, (const uint8_t[]){0x3A}, 1, 0}, /* gam_vrp4 */
  {0xA4, (const uint8_t[]){0x3A}, 1, 0}, /* gam_VRN4 */
  {0x85, (const uint8_t[]){0x3F}, 1, 0}, /* gam_vrp5 */
  {0xA5, (const uint8_t[]){0x3F}, 1, 0}, /* gam_VRN5 */
  {0x86, (const uint8_t[]){0x2C}, 1, 0}, /* gam_prp0 */
  {0xA6, (const uint8_t[]){0x2A}, 1, 0}, /* gam_PRN0 */
  {0x87, (const uint8_t[]){0x43}, 1, 0}, /* gam_prp1 */
  {0xA7, (const uint8_t[]){0x47}, 1, 0}, /* gam_PRN1 */
  {0x88, (const uint8_t[]){0x08}, 1, 0}, /* gam_pkp0 */
  {0xA8, (const uint8_t[]){0x08}, 1, 0}, /* gam_PKN0 */
  {0x89, (const uint8_t[]){0x0F}, 1, 0}, /* gam_pkp1 */
  {0xA9, (const uint8_t[]){0x0F}, 1, 0}, /* gam_PKN1 */
  {0x8A, (const uint8_t[]){0x17}, 1, 0}, /* gam_pkp2 */
  {0xAA, (const uint8_t[]){0x17}, 1, 0}, /* gam_PKN2 */
  {0x8B, (const uint8_t[]){0x10}, 1, 0}, /* gam_PKP3 */
  {0xAB, (const uint8_t[]){0x10}, 1, 0}, /* gam_PKN3 */
  {0x8C, (const uint8_t[]){0x16}, 1, 0}, /* gam_PKP4 */
  {0xAC, (const uint8_t[]){0x16}, 1, 0}, /* gam_PKN4 */
  {0x8D, (const uint8_t[]){0x14}, 1, 0}, /* gam_PKP5 */
  {0xAD, (const uint8_t[]){0x14}, 1, 0}, /* gam_PKN5 */
  {0x8E, (const uint8_t[]){0x11}, 1, 0}, /* gam_PKP6 */
  {0xAE, (const uint8_t[]){0x11}, 1, 0}, /* gam_PKN6 */
  {0x8F, (const uint8_t[]){0x14}, 1, 0}, /* gam_PKP7 */
  {0xAF, (const uint8_t[]){0x14}, 1, 0}, /* gam_PKN7 */
  {0x90, (const uint8_t[]){0x06}, 1, 0}, /* gam_PKP8 */
  {0xB0, (const uint8_t[]){0x06}, 1, 0}, /* gam_PKN8 */
  {0x91, (const uint8_t[]){0x0F}, 1, 0}, /* gam_PKP9 */
  {0xB1, (const uint8_t[]){0x0F}, 1, 0}, /* gam_PKN9 */
  {0x92, (const uint8_t[]){0x16}, 1, 0}, /* gam_PKP10 */
  {0xB2, (const uint8_t[]){0x16}, 1, 0}, /* gam_PKN10 */
  {0xFF, (const uint8_t[]){0x00}, 1, 0},
  {0x11, 0, 0, 350},
  {0x38, 0, 0, 20},         /* IDMOFF: exit 8-color idle mode */
  {0x29, 0, 0, 180},
};

const char *txw430038b0_nv3041a_01_name(void)
{
#if TXW430038B0_LCD_BUS_MODE == TXW430038B0_LCD_BUS_MODE_QSPI2
  return "TXW430038B0(QSPI2)";
#else
  return "TXW430038B0(SPI1)";
#endif
}

int txw430038b0_nv3041a_01_init(void)
{
  uint32_t i;

#if TXW430038B0_LCD_BUS_MODE == TXW430038B0_LCD_BUS_MODE_SPI1
  spi_lcd_bus_init();
#endif
  txw430038b0_vendor_reset_sequence();

  for(i = 0; i < (uint32_t)(sizeof(nv3041a01_init_cmds) / sizeof(nv3041a01_init_cmds[0])); i++)
  {
    const nv3041a01_init_cmd_t *c = &nv3041a01_init_cmds[i];
#if TXW430038B0_LCD_BUS_MODE == TXW430038B0_LCD_BUS_MODE_QSPI2
    if(!qspi2_lcd_bus_write(c->cmd, c->data, c->data_bytes))
    {
      return 0;
    }
#else
    spi_lcd_bus_write_cmd_data(c->cmd, c->data, c->data_bytes);
#endif
    if(c->delay_ms != 0U)
    {
      wk_delay_ms(c->delay_ms);
    }
  }

  if(!txw430038b0_nv3041a_01_set_window(0, 0, (uint16_t)(TXW430038B0_WIDTH - 1U), (uint16_t)(TXW430038B0_HEIGHT - 1U)))
  {
    return 0;
  }

  wk_delay_ms(20);
  return 1;
}

int txw430038b0_nv3041a_01_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  uint8_t col[4];
  uint8_t row[4];

  if((x1 < x0) || (y1 < y0))
  {
    return 0;
  }
  if((x1 >= TXW430038B0_WIDTH) || (y1 >= TXW430038B0_HEIGHT))
  {
    return 0;
  }

  col[0] = (uint8_t)(x0 >> 8);
  col[1] = (uint8_t)(x0 & 0xFFU);
  col[2] = (uint8_t)(x1 >> 8);
  col[3] = (uint8_t)(x1 & 0xFFU);

  row[0] = (uint8_t)(y0 >> 8);
  row[1] = (uint8_t)(y0 & 0xFFU);
  row[2] = (uint8_t)(y1 >> 8);
  row[3] = (uint8_t)(y1 & 0xFFU);

#if TXW430038B0_LCD_BUS_MODE == TXW430038B0_LCD_BUS_MODE_QSPI2
  if(!qspi2_lcd_bus_write(0x2A, col, (uint32_t)sizeof(col)))
  {
    return 0;
  }
  if(!qspi2_lcd_bus_write(0x2B, row, (uint32_t)sizeof(row)))
  {
    return 0;
  }
#else
  spi_lcd_bus_write_cmd_data(0x2A, col, (uint32_t)sizeof(col));
  spi_lcd_bus_write_cmd_data(0x2B, row, (uint32_t)sizeof(row));
#endif
  return 1;
}

int txw430038b0_nv3041a_01_write_pixels_rgb565(const uint16_t *pixels, uint32_t pixel_count)
{
#if TXW430038B0_LCD_BUS_MODE == TXW430038B0_LCD_BUS_MODE_QSPI2
  return qspi2_lcd_bus_write_pixels_rgb565(pixels, pixel_count);
#else
  return spi_lcd_bus_write_pixels_rgb565(0x2C, pixels, pixel_count);
#endif
}

coord_t txw430038b0_nv3041a_01_getWidth(void)
{
  return (coord_t)TXW430038B0_WIDTH;
}

coord_t txw430038b0_nv3041a_01_getHeight(void)
{
  return (coord_t)TXW430038B0_HEIGHT;
}

void txw430038b0_nv3041a_01_spi_setWindow(coord_t x, coord_t y, coord_t w, coord_t h)
{
  int32_t x0 = (int32_t)x;
  int32_t y0 = (int32_t)y;
  int32_t x1;
  int32_t y1;

  if((w <= 0) || (h <= 0))
  {
    return;
  }

  x1 = x0 + (int32_t)w - 1;
  y1 = y0 + (int32_t)h - 1;

  x0 = (x0 < 0) ? 0 : x0;
  y0 = (y0 < 0) ? 0 : y0;
  x1 = (x1 >= (int32_t)TXW430038B0_WIDTH) ? ((int32_t)TXW430038B0_WIDTH - 1) : x1;
  y1 = (y1 >= (int32_t)TXW430038B0_HEIGHT) ? ((int32_t)TXW430038B0_HEIGHT - 1) : y1;

  if((x0 > x1) || (y0 > y1))
  {
    return;
  }

  (void)txw430038b0_nv3041a_01_set_window((uint16_t)x0, (uint16_t)y0, (uint16_t)x1, (uint16_t)y1);
}


void txw430038b0_nv3041a_01_spi_drawBitmap(BitmapBuffer *dc, coord_t x, coord_t y)
{
  const uint16_t *src;
  int32_t src_w;
  int32_t src_h;
  int32_t dst_x;
  int32_t dst_y;
  int32_t draw_x0;
  int32_t draw_y0;
  int32_t draw_w;
  int32_t draw_h;
  int32_t src_x0;
  int32_t src_y0;
  int32_t row;

  if((dc == 0) || (dc->data == 0))
  {
    return;
  }

  src_w = (int32_t)dc->width;
  src_h = (int32_t)dc->height;
  if((src_w <= 0) || (src_h <= 0))
  {
    return;
  }

  dst_x = (int32_t)x;
  dst_y = (int32_t)y;

  draw_x0 = dst_x;
  draw_y0 = dst_y;
  draw_w = src_w;
  draw_h = src_h;

  src_x0 = 0;
  src_y0 = 0;

  if(draw_x0 < 0)
  {
    src_x0 = -draw_x0;
    draw_w += draw_x0;
    draw_x0 = 0;
  }
  if(draw_y0 < 0)
  {
    src_y0 = -draw_y0;
    draw_h += draw_y0;
    draw_y0 = 0;
  }

  if((draw_x0 + draw_w) > (int32_t)TXW430038B0_WIDTH)
  {
    draw_w = (int32_t)TXW430038B0_WIDTH - draw_x0;
  }
  if((draw_y0 + draw_h) > (int32_t)TXW430038B0_HEIGHT)
  {
    draw_h = (int32_t)TXW430038B0_HEIGHT - draw_y0;
  }

  if((draw_w <= 0) || (draw_h <= 0))
  {
    return;
  }

  src = (const uint16_t *)(void *)dc->data;

  for(row = 0; row < draw_h; row++)
  {
    int32_t y_line = draw_y0 + row;
    const uint16_t *line = &src[(src_y0 + row) * src_w + src_x0];

    if(!txw430038b0_nv3041a_01_set_window((uint16_t)draw_x0,
                                          (uint16_t)y_line,
                                          (uint16_t)(draw_x0 + draw_w - 1),
                                          (uint16_t)y_line))
    {
      return;
    }
    if(!txw430038b0_nv3041a_01_write_pixels_rgb565(line, (uint32_t)draw_w))
    {
      return;
    }
  }
}

void txw430038b0_nv3041a_01_spi_drawString(BitmapBuffer *dc, const char *s, coord_t x, coord_t y, LcdFlags flags)
{
  (void)dc;
  (void)s;
  (void)x;
  (void)y;
  (void)flags;
}

void txw430038b0_nv3041a_01_spi_sleep(void)
{
#if TXW430038B0_LCD_BUS_MODE == TXW430038B0_LCD_BUS_MODE_QSPI2
  (void)qspi2_lcd_bus_write(0x10, 0, 0);
#else
  spi_lcd_bus_write_cmd(0x10);
#endif
  wk_delay_ms(120);
}

void txw430038b0_nv3041a_01_spi_wakeup(void)
{
#if TXW430038B0_LCD_BUS_MODE == TXW430038B0_LCD_BUS_MODE_QSPI2
  (void)qspi2_lcd_bus_write(0x11, 0, 0);
#else
  spi_lcd_bus_write_cmd(0x11);
#endif
  wk_delay_ms(350);
}
