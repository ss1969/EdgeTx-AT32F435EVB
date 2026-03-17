#include "txw430038b0_nv3041a_01_spi1.h"

#include "spi1_lcd_bus.h"
#include "wk_system.h"

typedef struct
{
  uint8_t cmd;
  const uint8_t *data;
  uint8_t data_bytes;
  uint16_t delay_ms;
} nv3041a01_init_cmd_t;

static const uint8_t nv3041a01_gamma_p[] = {
  0x00, 0x07, 0x02, 0x37, 0x35, 0x3F, 0x11, 0x27, 0x0B, 0x14, 0x1A, 0x0A, 0x14, 0x17, 0x16, 0x1B, 0x04, 0x0A, 0x16
};
static const uint8_t nv3041a01_gamma_n[] = {
  0x00, 0x06, 0x01, 0x37, 0x35, 0x3F, 0x10, 0x27, 0x0B, 0x14, 0x1A, 0x0A, 0x08, 0x07, 0x06, 0x07, 0x04, 0x0A, 0x15
};

static const nv3041a01_init_cmd_t nv3041a01_init_cmds[] = {
  {0xFF, (const uint8_t[]){0xA5}, 1, 0},
  {0x36, (const uint8_t[]){0xC0}, 1, 0},
  {0x3A, (const uint8_t[]){0x01}, 1, 0},
  {0x41, (const uint8_t[]){0x03}, 1, 0},
  {0x44, (const uint8_t[]){0x15}, 1, 0},
  {0x45, (const uint8_t[]){0x15}, 1, 0},
  {0x7D, (const uint8_t[]){0x03}, 1, 0},
  {0xC1, (const uint8_t[]){0xBB}, 1, 0},
  {0xC2, (const uint8_t[]){0x05}, 1, 0},
  {0xC3, (const uint8_t[]){0x10}, 1, 0},
  {0xC6, (const uint8_t[]){0x3E}, 1, 0},
  {0xC7, (const uint8_t[]){0x25}, 1, 0},
  {0xC8, (const uint8_t[]){0x11}, 1, 0},
  {0x7A, (const uint8_t[]){0x5F}, 1, 0},
  {0x6F, (const uint8_t[]){0x44}, 1, 0},
  {0x78, (const uint8_t[]){0x70}, 1, 0},
  {0xC9, (const uint8_t[]){0x00}, 1, 0},
  {0x67, (const uint8_t[]){0x21}, 1, 0},
  {0x51, (const uint8_t[]){0x0A}, 1, 0},
  {0x52, (const uint8_t[]){0x76}, 1, 0},
  {0x53, (const uint8_t[]){0x0A}, 1, 0},
  {0x54, (const uint8_t[]){0x76}, 1, 0},
  {0x46, (const uint8_t[]){0x0A}, 1, 0},
  {0x47, (const uint8_t[]){0x2A}, 1, 0},
  {0x48, (const uint8_t[]){0x0A}, 1, 0},
  {0x49, (const uint8_t[]){0x1A}, 1, 0},
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
  {0x65, (const uint8_t[]){0x02}, 1, 0},
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
  {0x80, nv3041a01_gamma_p, (uint8_t)sizeof(nv3041a01_gamma_p), 0},
  {0xA0, nv3041a01_gamma_n, (uint8_t)sizeof(nv3041a01_gamma_n), 0},
  {0xFF, (const uint8_t[]){0x00}, 1, 0},
  {0x11, 0, 0, 120},
  {0x21, 0, 0, 0},      // INV ON
  {0x29, 0, 0, 100},
};

const char *txw430038b0_nv3041a_01_spi1_name(void)
{
  return "TXW430038B0";
}

int txw430038b0_nv3041a_01_spi_init(void)
{
  uint32_t i;

  spi_lcd_bus_init();
  spi_lcd_bus_reset_pulse(20, 20);

  for(i = 0; i < (uint32_t)(sizeof(nv3041a01_init_cmds) / sizeof(nv3041a01_init_cmds[0])); i++)
  {
    const nv3041a01_init_cmd_t *c = &nv3041a01_init_cmds[i];
    spi_lcd_bus_write_cmd_data(c->cmd, c->data, c->data_bytes);
    if(c->delay_ms != 0U)
    {
      wk_delay_ms(c->delay_ms);
    }
  }

  return txw430038b0_nv3041a_01_spi_set_window(0, 0, (uint16_t)(TXW430038B0_WIDTH - 1U), (uint16_t)(TXW430038B0_HEIGHT - 1U));
}

int txw430038b0_nv3041a_01_spi_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
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

  spi_lcd_bus_write_cmd_data(0x2A, col, (uint32_t)sizeof(col));
  spi_lcd_bus_write_cmd_data(0x2B, row, (uint32_t)sizeof(row));
  return 1;
}

int txw430038b0_nv3041a_01_spi_write_pixels_rgb565(const uint16_t *pixels, uint32_t pixel_count)
{
  return spi_lcd_bus_write_pixels_rgb565(0x2C, pixels, pixel_count);
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

  (void)txw430038b0_nv3041a_01_spi_set_window((uint16_t)x0, (uint16_t)y0, (uint16_t)x1, (uint16_t)y1);
}

void txw430038b0_nv3041a_01_spi_refresh(void)
{
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

    if(!txw430038b0_nv3041a_01_spi_set_window((uint16_t)draw_x0,
                                              (uint16_t)y_line,
                                              (uint16_t)(draw_x0 + draw_w - 1),
                                              (uint16_t)y_line))
    {
      return;
    }
    if(!txw430038b0_nv3041a_01_spi_write_pixels_rgb565(line, (uint32_t)draw_w))
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
  spi_lcd_bus_write_cmd(0x10);
  wk_delay_ms(120);
}

void txw430038b0_nv3041a_01_spi_wakeup(void)
{
  spi_lcd_bus_write_cmd(0x11);
  wk_delay_ms(120);
}
