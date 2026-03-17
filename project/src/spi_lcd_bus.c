#include "spi_lcd_bus.h"

#include "at32f435_437_spi.h"
#if SPI_LCD_BUS_USE_DMA
#include "at32f435_437_dma.h"
#include "wk_dma.h"
#endif
#include "wk_spi.h"
#include "wk_system.h"

#define SPI_LCD_BUS_TIMEOUT 2000000U
#define SPI_LCD_BUS_DMA_TIMEOUT 2000000U
#define SPI_LCD_BUS_DMA_BUF_BYTES 2048U

#if SPI_LCD_BUS_USE_HW_CS
#define SPI_LCD_CS_LOW()  ((void)0)
#define SPI_LCD_CS_HIGH() ((void)0)
#else
#define SPI_LCD_CS_LOW()  gpio_bits_reset(SPI1_CS_GPIO_PORT, SPI1_CS_PIN)
#define SPI_LCD_CS_HIGH() gpio_bits_set(SPI1_CS_GPIO_PORT, SPI1_CS_PIN)
#endif
#define SPI_LCD_DCX_CMD() gpio_bits_reset(SPI1_DCX_GPIO_PORT, SPI1_DCX_PIN)
#define SPI_LCD_DCX_DATA() gpio_bits_set(SPI1_DCX_GPIO_PORT, SPI1_DCX_PIN)

static int spi_wait_flag(uint32_t flag)
{
  uint32_t timeout = SPI_LCD_BUS_TIMEOUT;
  while(spi_i2s_flag_get(SPI_LCD_BUS_SPI, flag) == RESET)
  {
    if(timeout-- == 0U)
    {
      return 0;
    }
  }
  return 1;
}

static int spi_wait_not_busy(void)
{
  uint32_t timeout = SPI_LCD_BUS_TIMEOUT;
  while(spi_i2s_flag_get(SPI_LCD_BUS_SPI, SPI_I2S_BF_FLAG) == SET)
  {
    if(timeout-- == 0U)
    {
      return 0;
    }
  }
  return 1;
}

#if SPI_LCD_BUS_USE_DMA
static int spi_dma_tx(const uint8_t *data, uint32_t len)
{
  uint32_t timeout = SPI_LCD_BUS_DMA_TIMEOUT;

  if((data == 0) || (len == 0U) || (len > 0xFFFFU))
  {
    return 0;
  }

  dma_channel_enable(DMA1_CHANNEL1, FALSE);
  dma_flag_clear(DMA1_GL1_FLAG | DMA1_FDT1_FLAG | DMA1_HDT1_FLAG | DMA1_DTERR1_FLAG);
  wk_dma_channel_config(DMA1_CHANNEL1, (uint32_t)&SPI_LCD_BUS_SPI->dt, (uint32_t)data, (uint16_t)len);

  spi_i2s_dma_transmitter_enable(SPI_LCD_BUS_SPI, TRUE);
  dma_channel_enable(DMA1_CHANNEL1, TRUE);

  while(dma_flag_get(DMA1_FDT1_FLAG) == RESET)
  {
    if(timeout-- == 0U)
    {
      dma_channel_enable(DMA1_CHANNEL1, FALSE);
      return 0;
    }
  }

  dma_channel_enable(DMA1_CHANNEL1, FALSE);
  dma_flag_clear(DMA1_GL1_FLAG | DMA1_FDT1_FLAG | DMA1_HDT1_FLAG | DMA1_DTERR1_FLAG);

  return spi_wait_not_busy();
}
#endif

// can omit calling, spi & gpio is inited by structure codes.
void spi_lcd_bus_init(void)
{
  gpio_init_type gpio_init_struct;

  crm_periph_clock_enable(SPI_LCD_BUS_CRM_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOE_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

  gpio_default_para_init(&gpio_init_struct);
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;

  wk_spi1_init();

#if SPI_LCD_BUS_USE_DMA
  crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
  wk_dma1_channel1_init();
  dma_interrupt_enable(DMA1_CHANNEL1, DMA_FDT_INT | DMA_HDT_INT | DMA_DTERR_INT, FALSE);
  dma_channel_enable(DMA1_CHANNEL1, FALSE);
  dma_flag_clear(DMA1_GL1_FLAG | DMA1_FDT1_FLAG | DMA1_HDT1_FLAG | DMA1_DTERR1_FLAG);
#endif

#if !SPI_LCD_BUS_USE_HW_CS
  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_pins = SPI1_CS_PIN;
  gpio_init(SPI1_CS_GPIO_PORT, &gpio_init_struct);
#endif

  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_pins = SPI1_DCX_PIN;
  gpio_init(SPI1_DCX_GPIO_PORT, &gpio_init_struct);

  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_pins = QSPI2_RESET_PIN;
  gpio_init(QSPI2_RESET_GPIO_PORT, &gpio_init_struct);

  SPI_LCD_CS_HIGH();
  SPI_LCD_DCX_DATA();
}


void spi_lcd_bus_reset_pulse(uint32_t low_ms, uint32_t high_ms)
{
  gpio_bits_reset(QSPI2_RESET_GPIO_PORT, QSPI2_RESET_PIN);
  wk_delay_ms(low_ms);
  gpio_bits_set(QSPI2_RESET_GPIO_PORT, QSPI2_RESET_PIN);
  wk_delay_ms(high_ms);
}

void spi_lcd_bus_write_cmd(uint8_t cmd)
{
  SPI_LCD_CS_LOW();
  SPI_LCD_DCX_CMD();

  (void)spi_wait_flag(SPI_I2S_TDBE_FLAG);
  spi_i2s_data_transmit(SPI_LCD_BUS_SPI, cmd);
  (void)spi_wait_not_busy();

  SPI_LCD_CS_HIGH();
}

void spi_lcd_bus_write_cmd_data(uint8_t cmd, const uint8_t *data, uint32_t len)
{
  uint32_t i;

  SPI_LCD_CS_LOW();
  SPI_LCD_DCX_CMD();

  (void)spi_wait_flag(SPI_I2S_TDBE_FLAG);
  spi_i2s_data_transmit(SPI_LCD_BUS_SPI, cmd);
  (void)spi_wait_not_busy();

  if((data != 0) && (len != 0U))
  {
    SPI_LCD_DCX_DATA();
    for(i = 0; i < len; i++)
    {
      (void)spi_wait_flag(SPI_I2S_TDBE_FLAG);
      spi_i2s_data_transmit(SPI_LCD_BUS_SPI, data[i]);
      (void)spi_wait_not_busy();
    }
  }

  SPI_LCD_CS_HIGH();
}

int spi_lcd_bus_write_pixels_rgb565(uint8_t cmd, const uint16_t *pixels, uint32_t pixel_count)
{
  uint32_t i;

  if((pixels == 0) && (pixel_count != 0U))
  {
    return 0;
  }

  SPI_LCD_CS_LOW();
  SPI_LCD_DCX_CMD();

  if(!spi_wait_flag(SPI_I2S_TDBE_FLAG))
  {
    SPI_LCD_CS_HIGH();
    return 0;
  }
  spi_i2s_data_transmit(SPI_LCD_BUS_SPI, cmd);
  if(!spi_wait_not_busy())
  {
    SPI_LCD_CS_HIGH();
    return 0;
  }

  SPI_LCD_DCX_DATA();

#if SPI_LCD_BUS_USE_DMA
  if(pixel_count != 0U)
  {
    uint32_t bytes = pixel_count * 2U;
    static uint8_t buf[SPI_LCD_BUS_DMA_BUF_BYTES];

    if(bytes <= (uint32_t)sizeof(buf))
    {
      for(i = 0; i < pixel_count; i++)
      {
        uint16_t px = pixels[i];
        buf[i * 2U] = (uint8_t)(px >> 8);
        buf[i * 2U + 1U] = (uint8_t)(px & 0xFFU);
      }

      if(!spi_dma_tx(buf, bytes))
      {
        SPI_LCD_CS_HIGH();
        return 0;
      }

      SPI_LCD_CS_HIGH();
      return 1;
    }
  }
#endif

  for(i = 0; i < pixel_count; i++)
  {
    uint16_t px = pixels[i];
    uint8_t hi = (uint8_t)(px >> 8);
    uint8_t lo = (uint8_t)(px & 0xFFU);

    if(!spi_wait_flag(SPI_I2S_TDBE_FLAG))
    {
      SPI_LCD_CS_HIGH();
      return 0;
    }
    spi_i2s_data_transmit(SPI_LCD_BUS_SPI, hi);
    if(!spi_wait_not_busy())
    {
      SPI_LCD_CS_HIGH();
      return 0;
    }

    if(!spi_wait_flag(SPI_I2S_TDBE_FLAG))
    {
      SPI_LCD_CS_HIGH();
      return 0;
    }
    spi_i2s_data_transmit(SPI_LCD_BUS_SPI, lo);
    if(!spi_wait_not_busy())
    {
      SPI_LCD_CS_HIGH();
      return 0;
    }
  }

  SPI_LCD_CS_HIGH();
  return 1;
}
