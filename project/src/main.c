/* add user code begin Header */
/**
  **************************************************************************
  * @file     main.c
  * @brief    main program
  **************************************************************************
  * Copyright (c) 2025, Artery Technology, All rights reserved.
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include "at32f435_437_wk_config.h"
#include "wk_acc.h"
#include "wk_exint.h"
#include "wk_qspi.h"
#include "wk_spi.h"
#include "wk_usart.h"
#include "wk_usb_otgfs.h"
#include "wk_dma.h"
#include "wk_edma.h"
#include "wk_gpio.h"
#include "wk_system.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */
#include "aps6404l.h"
#include "txw430038b0_nv3041a_01_panel.h"
#include "txw430038b0_nv3041a_01_spi.h"
#if TXW430038B0_LCD_BUS_MODE == TXW430038B0_LCD_BUS_MODE_QSPI2
#include "qspi2_lcd_bus_priv.h"
#else
#include "spi_lcd_bus.h"
#endif
#include "cat_rgb565.h"
#include "uart3_printf.h"
#include <stdlib.h>
/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */
#define APS6404L_TEST_TOTAL_BYTES         (1U * 1024U * 1024U)
#define APS6404L_TEST_CHUNK_SIZE          (8U * 1024U)
#define APS6404L_TEST_PROGRESS_STEP       (256U * 1024U)

#define APS6404L_BENCH_TOTAL_BYTES        (256U * 1024U)
#define APS6404L_BENCH_CHUNK_BYTES        (64U * 1024U)

/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */

/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */
static int lcd_fill_color(uint16_t color);
static void run_psram_tests(void);

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */
static void aps_perf_init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static uint32_t aps_perf_cycles(void)
{
  return DWT->CYCCNT;
}

static void aps_print_speed(const char *tag, uint32_t bytes, uint32_t cycles)
{
  uint64_t bps64;
  uint32_t mib_x10;

  if(cycles == 0U)
  {
    UART3_Printf("%s: cycles=0\r\n", tag);
    return;
  }

  bps64 = ((uint64_t)bytes * (uint64_t)SystemCoreClock) / (uint64_t)cycles;
  mib_x10 = (uint32_t)((bps64 * 10ULL) / (1024ULL * 1024ULL));

  UART3_Printf("%s: bytes=%lu cycles=%lu speed=%lu.%lu MiB/s\r\n",
               tag,
               (unsigned long)bytes,
               (unsigned long)cycles,
               (unsigned long)(mib_x10 / 10U),
               (unsigned long)(mib_x10 % 10U));
}

static int psram_id_is_valid(const unsigned char *id, unsigned int len)
{
  unsigned int i;
  unsigned char all_ff = 1;
  unsigned char all_00 = 1;

  for(i = 0; i < len; i++)
  {
    if(id[i] != 0xFFU)
    {
      all_ff = 0;
    }
    if(id[i] != 0x00U)
    {
      all_00 = 0;
    }
  }

  return (!all_ff && !all_00) ? 1 : 0;
}

static void run_psram_read_id_test(void)
{
  int ok;
  int i;
  unsigned char id[8];

  for(i = 0; i < (int)sizeof(id); i++)
  {
    id[i] = 0;
  }

  ok = PSRAM_ReadID(id, (int)sizeof(id));
  UART3_Printf("APS RDID:");
  for(i = 0; i < (int)sizeof(id); i++)
  {
    UART3_PrintHexByte(id[i]);
  }
  UART3_Printf("\r\n");

  if(ok && psram_id_is_valid(id, (unsigned int)sizeof(id)))
  {
    UART3_Printf("APS RDID: OK\r\n");
  }
  else
  {
    UART3_Printf("APS RDID: FAIL\r\n");
  }
}

static void run_psram_edma_bench_test(void)
{
  int i;
  uint32_t t0;
  uint32_t t1;
  uint32_t off;
  uint32_t sum;
  uint32_t base = 0x00100000u;
  int bench_ok = 1;
  static uint32_t bench_w_u32[APS6404L_BENCH_CHUNK_BYTES / 4U];
  static uint32_t bench_r_u32[APS6404L_BENCH_CHUNK_BYTES / 4U];
  unsigned char *bench_w = (unsigned char *)bench_w_u32;
  unsigned char *bench_r = (unsigned char *)bench_r_u32;

  aps_perf_init();

  for(i = 0; i < (int)APS6404L_BENCH_CHUNK_BYTES; i++)
  {
    bench_w[i] = (unsigned char)(0xC3u ^ (unsigned char)i);
    bench_r[i] = 0;
  }

  UART3_Printf("APS BENCH: total=");
  UART3_PrintHexU32(APS6404L_BENCH_TOTAL_BYTES);
  UART3_Printf(" chunk=");
  UART3_PrintHexU32(APS6404L_BENCH_CHUNK_BYTES);
  UART3_Printf("\r\n");

  t0 = aps_perf_cycles();
  for(off = 0; off < APS6404L_BENCH_TOTAL_BYTES; off += APS6404L_BENCH_CHUNK_BYTES)
  {
    if(!PSRAM_EDMA_Write(base + off, bench_w, (int)APS6404L_BENCH_CHUNK_BYTES))
    {
      bench_ok = 0;
      break;
    }
  }
  t1 = aps_perf_cycles();
  if(bench_ok)
  {
    aps_print_speed("APS BENCH WR EDMA", APS6404L_BENCH_TOTAL_BYTES, t1 - t0);
  }
  else
  {
    UART3_Printf("APS BENCH WR EDMA: FAIL\r\n");
  }

  sum = 0;
  bench_ok = 1;
  t0 = aps_perf_cycles();
  for(off = 0; off < APS6404L_BENCH_TOTAL_BYTES; off += APS6404L_BENCH_CHUNK_BYTES)
  {
    if(!PSRAM_EDMA_Read(base + off, bench_r, (int)APS6404L_BENCH_CHUNK_BYTES))
    {
      bench_ok = 0;
      break;
    }
    for(i = 0; i < 16; i++)
    {
      sum ^= (uint32_t)bench_r[i];
    }
  }
  t1 = aps_perf_cycles();
  if(bench_ok)
  {
    aps_print_speed("APS BENCH RD EDMA", APS6404L_BENCH_TOTAL_BYTES, t1 - t0);
    UART3_Printf("APS BENCH RD EDMA: sum=");
    UART3_PrintHexU32(sum);
    UART3_Printf("\r\n");
  }
  else
  {
    UART3_Printf("APS BENCH RD EDMA: FAIL\r\n");
  }
}

static void run_psram_edma_rw_test(void)
{
  int i;
  unsigned char edma_pass = 1;
  unsigned char edma_fail_index = 0;
  unsigned int edma_len = (unsigned int)(sizeof(uint32_t) * 256U);
  static uint32_t edma_w_u32[256];
  static uint32_t edma_r_u32[256];
  unsigned char *edma_w = (unsigned char *)edma_w_u32;
  unsigned char *edma_r = (unsigned char *)edma_r_u32;

  for(i = 0; i < (int)edma_len; i++)
  {
    edma_w[i] = (unsigned char)(0x55u ^ (unsigned char)i);
    edma_r[i] = 0;
  }

  UART3_Printf("APS EDMA RW: START len=");
  UART3_PrintHexU32(edma_len);
  UART3_Printf("\r\n");

  if(!PSRAM_EDMA_Write(0x00001000u, edma_w, (int)edma_len))
  {
    UART3_Printf("APS EDMA RW: W FAIL\r\n");
    return;
  }
  if(!PSRAM_EDMA_Read(0x00001000u, edma_r, (int)edma_len))
  {
    UART3_Printf("APS EDMA RW: R FAIL\r\n");
    return;
  }

  for(i = 0; i < (int)edma_len; i++)
  {
    if(edma_w[i] != edma_r[i])
    {
      edma_pass = 0;
      edma_fail_index = (unsigned char)i;
      break;
    }
  }

  if(edma_pass)
  {
    UART3_Printf("APS EDMA RW: OK\r\n");
  }
  else
  {
    UART3_Printf("APS EDMA RW: FAIL idx=");
    UART3_PrintHexByte(edma_fail_index);
    UART3_Printf(" exp=");
    UART3_PrintHexByte(edma_w[edma_fail_index]);
    UART3_Printf(" got=");
    UART3_PrintHexByte(edma_r[edma_fail_index]);
    UART3_Printf("\r\n");
  }
}

static void run_psram_xip_rw_test(void)
{
  int i;
  unsigned char *write_buf;
  unsigned int addr = 0;
  unsigned int fail_addr = 0;
  unsigned int offset;
  unsigned char pass = 1;
  unsigned char fail_index = 0;
  unsigned char fail_exp = 0;
  unsigned char fail_got = 0;
  unsigned int seed;
  unsigned int chunk_len;
  unsigned char used_malloc = 0;
  volatile unsigned char *psram = (volatile unsigned char *)APS6404L_MEM_BASE;
  static unsigned char write_buf_fallback[APS6404L_TEST_CHUNK_SIZE];

  UART3_Printf("APS RW 1MB: START\r\n");

  write_buf = (unsigned char *)malloc(APS6404L_TEST_CHUNK_SIZE);
  if(write_buf == 0)
  {
    write_buf = write_buf_fallback;
    UART3_Printf("APS RW 1MB: MALLOC FAIL, USE STATIC\r\n");
  }
  else
  {
    used_malloc = 1;
  }

  for(offset = 0; offset < APS6404L_TEST_TOTAL_BYTES; offset += APS6404L_TEST_CHUNK_SIZE)
  {
    if((offset % APS6404L_TEST_PROGRESS_STEP) == 0U)
    {
      UART3_Printf("off=");
      UART3_PrintHexU32(offset);
      UART3_Printf("\r\n");
    }

    chunk_len = APS6404L_TEST_CHUNK_SIZE;
    if((APS6404L_TEST_TOTAL_BYTES - offset) < chunk_len)
    {
      chunk_len = APS6404L_TEST_TOTAL_BYTES - offset;
    }

    seed = 0x6D2B79F5u ^ (addr + offset);
    for(i = 0; i < (int)chunk_len; i++)
    {
      seed ^= seed << 13;
      seed ^= seed >> 17;
      seed ^= seed << 5;
      write_buf[i] = (unsigned char)(seed & 0xFFu);
    }

    for(i = 0; i < (int)chunk_len; i++)
    {
      psram[addr + offset + (unsigned int)i] = write_buf[i];
    }

    __DSB();
    __ISB();

    for(i = 0; i < (int)chunk_len; i++)
    {
      unsigned char got = psram[addr + offset + (unsigned int)i];
      if(write_buf[i] != got)
      {
        pass = 0;
        fail_addr = addr + offset;
        fail_index = (unsigned char)i;
        fail_exp = write_buf[i];
        fail_got = got;
        break;
      }
    }

    if(!pass)
    {
      break;
    }
  }

  if(used_malloc)
  {
    free(write_buf);
  }

  if(pass)
  {
    UART3_Printf("APS RW 1MB: OK\r\n");
  }
  else
  {
    UART3_Printf("APS RW 1MB: FAIL addr=");
    UART3_PrintHexU32(fail_addr);
    UART3_Printf(" idx=");
    UART3_PrintHexByte(fail_index);
    UART3_Printf(" exp=");
    UART3_PrintHexByte(fail_exp);
    UART3_Printf(" got=");
    UART3_PrintHexByte(fail_got);
    UART3_Printf("\r\n");
    for(i = 0; i < 16; i++)
    {
      UART3_PrintHexByte(psram[fail_addr + (unsigned int)i]);
    }
    UART3_Printf("\r\n");
  }
}

static void run_psram_tests(void)
{
  PSRAM_Reset();
  UART3_Printf("APS TEST: mode=EDMA+XIP\r\n");
  run_psram_read_id_test();
  run_psram_edma_bench_test();
  run_psram_edma_rw_test();
  run_psram_xip_rw_test();
}

static uint8_t uart3_getc_blocking(void)
{
  while(usart_flag_get(USART3, USART_RDBF_FLAG) == RESET)
  {
  }

  return (uint8_t)usart_data_receive(USART3);
}

static int hex_char_to_nibble(uint8_t ch, uint8_t *nibble)
{
  if(nibble == 0)
  {
    return 0;
  }

  if((ch >= '0') && (ch <= '9'))
  {
    *nibble = (uint8_t)(ch - '0');
    return 1;
  }
  if((ch >= 'A') && (ch <= 'F'))
  {
    *nibble = (uint8_t)(ch - 'A' + 10U);
    return 1;
  }
  if((ch >= 'a') && (ch <= 'f'))
  {
    *nibble = (uint8_t)(ch - 'a' + 10U);
    return 1;
  }

  return 0;
}

static uint16_t uart3_read_hex_u16_blocking(void)
{
  uint16_t value = 0U;
  uint32_t digits = 0U;
  uint8_t nibble;
  uint8_t ch;

  for(;;)
  {
    ch = uart3_getc_blocking();
    if((ch == '\r') || (ch == '\n') || (ch == ' ') || (ch == '\t'))
    {
      continue;
    }
    if(hex_char_to_nibble(ch, &nibble))
    {
      UART3_Printf("%c", ch);
      value = (uint16_t)((value << 4) | nibble);
      digits++;
      if(digits >= 4U)
      {
        break;
      }
    }
  }

  UART3_Printf("\r\n");
  return value;
}

static void lcd_uart_fill_loop(void)
{
  UART3_Printf("LCD: input 4 hex chars (0000-FFFF), fill screen with this value\r\n");

  for(;;)
  {
    uint16_t color = uart3_read_hex_u16_blocking();

    UART3_Printf("LCD: fill color=");
    UART3_PrintHexByte((uint8_t)(color >> 8));
    UART3_PrintHexByte((uint8_t)color);
    UART3_Printf("\r\n");

    if(!lcd_fill_color(color))
    {
      UART3_Printf("LCD: fill FAIL\r\n");
    }
  }
}

static int lcd_fill_color(uint16_t color)
{
  static uint16_t line[TXW430038B0_WIDTH];
  uint32_t y;
  uint32_t i;

  for(i = 0; i < TXW430038B0_WIDTH; i++)
  {
    line[i] = color;
  }

  for(y = 0; y < TXW430038B0_HEIGHT; y++)
  {
    if(!txw430038b0_nv3041a_01_set_window(0U,
                                          (uint16_t)y,
                                          (uint16_t)(TXW430038B0_WIDTH - 1U),
                                          (uint16_t)y))
    {
      return 0;
    }
    if(!txw430038b0_nv3041a_01_write_pixels_rgb565(line, TXW430038B0_WIDTH))
    {
      return 0;
    }
  }

  return 1;
}

static int lcd_draw_test_bars(void)
{
  static const uint16_t test_colors[] = {
    0xF800U, /* red */
    0x07E0U, /* green */
    0x001FU, /* blue */
    0xFFFFU, /* white */
    0x0000U  /* black */
  };
  static uint16_t line[TXW430038B0_WIDTH];
  uint32_t y;
  uint32_t x;

  for(y = 0; y < TXW430038B0_HEIGHT; y++)
  {
    for(x = 0; x < TXW430038B0_WIDTH; x++)
    {
      uint32_t bar = (x * (uint32_t)(sizeof(test_colors) / sizeof(test_colors[0]))) / TXW430038B0_WIDTH;

      if(bar >= (uint32_t)(sizeof(test_colors) / sizeof(test_colors[0])))
      {
        bar = (uint32_t)(sizeof(test_colors) / sizeof(test_colors[0])) - 1U;
      }

      line[x] = test_colors[bar];
    }

    if(!txw430038b0_nv3041a_01_set_window(0U,
                                          (uint16_t)y,
                                          (uint16_t)(TXW430038B0_WIDTH - 1U),
                                          (uint16_t)y))
    {
      return 0;
    }
    if(!txw430038b0_nv3041a_01_write_pixels_rgb565(line, TXW430038B0_WIDTH))
    {
      return 0;
    }
  }

  return 1;
}

static void lcd_run_bmp_test(void)
{
  BitmapBuffer bmp;
  uint32_t t0;
  uint32_t t1;
  uint32_t draw_ms;

  bmp.width = BMP_WIDTH;
  bmp.height = BMP_HEIGHT;
  bmp.format = BMP_FORMAT_RGB565;
  bmp.data = (uint8_t *)(void *)bmp_pixels;

  aps_perf_init();
  t0 = aps_perf_cycles();
  txw430038b0_nv3041a_01_spi_drawBitmap(&bmp, 0, 0);
  t1 = aps_perf_cycles();

  draw_ms = (uint32_t)(((uint64_t)(t1 - t0) * 1000ULL) / (uint64_t)SystemCoreClock);
  UART3_Printf("LCD: bmp draw_ms=%lu\r\n", (unsigned long)draw_ms);
}

/* add user code end 0 */

/**
  * @brief main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  /* add user code begin 1 */

  /* add user code end 1 */

  /* system clock config. */
  wk_system_clock_config();

  /* config periph clock. */
  wk_periph_clock_config();

  /* nvic config. */
  wk_nvic_config();

  /* timebase config for
     void wk_delay_us(uint32_t delay);
     void wk_delay_ms(uint32_t delay); */
  wk_timebase_init();

  /* init gpio function. */
  wk_gpio_config();

  /* init dma1 channel1 */
  wk_dma1_channel1_init();
  /* config dma channel transfer parameter */
  /* user need to modify define values DMAx_CHANNELy_XXX_BASE_ADDR
     and DMAx_CHANNELy_BUFFER_SIZE in at32xxx_wk_config.h */
  wk_dma_channel_config(DMA1_CHANNEL1,
                        (uint32_t)&SPI1->dt,
                        DMA1_CHANNEL1_MEMORY_BASE_ADDR,
                        DMA1_CHANNEL1_BUFFER_SIZE);
  dma_channel_enable(DMA1_CHANNEL1, TRUE);

  /* init edma stream1 */
  wk_edma_stream1_init();
  /* config edma stream transfer parameter */
  /* user need to modify define values EDMA_STREAMx_XXX_BASE_ADDR
     and EDMA_STREAMx_BUFFER_SIZE in at32xxx_wk_config.h */
  wk_edma_stream_config(EDMA_STREAM1,
                        (uint32_t)&QSPI1->dt,
                        EDMA_STREAM1_MEMORY0_BASE_ADDR,
                        EDMA_STREAM1_BUFFER_SIZE);
  /* enable stream */
  edma_stream_enable(EDMA_STREAM1, TRUE);

  /* init edma stream2 */
  wk_edma_stream2_init();
  /* QSPI2 LCD updates configure EDMA_STREAM2 dynamically per transfer. */
  edma_stream_enable(EDMA_STREAM2, FALSE);

  /* init usart1 function. */
  wk_usart1_init();

  /* init usart2 function. */
  wk_usart2_init();

  /* init usart3 function. */
  wk_usart3_init();

  /* init acc function. */
  wk_acc_init();

  /* init usb_otgfs1 function. */
  wk_usb_otgfs1_init();

  /* init spi1 function. */
  wk_spi1_init();

  /* init qspi1 function. */
  wk_qspi1_init();

  /* init qspi2 function. */
  wk_qspi2_init();

  /* init exint function. */
  wk_exint_config();

  /* add user code begin 2 */
  UART3_Printf("\x1b[2J\x1b[H");
  UART3_Printf("start up\r\n");

  /* add user code end 2 */

  while(1)
  {
    /* add user code begin 3 */
    static unsigned char ran = 0;

    if(!ran)
    {
#if 1
      run_psram_tests();
#endif

      {
        if(!txw430038b0_nv3041a_01_init())
        {
          UART3_Printf("LCD: init FAIL\r\n");
#if TXW430038B0_LCD_BUS_MODE == TXW430038B0_LCD_BUS_MODE_QSPI2
          UART3_Printf("LCD: qspi2 bus fail\r\n");
#endif
        }
        else
        {
          UART3_Printf("LCD: %s init OK\r\n", txw430038b0_nv3041a_01_name());
          UART3_Printf("LCD: cfg bus=QSPI2 cmd=111 pixel=114 gram=RAM32 dctl=01 rgb=565\r\n");

#if TXW430038B0_LCD_BUS_MODE == TXW430038B0_LCD_BUS_MODE_QSPI2
          wk_delay_ms(30);
          (void)qspi2_lcd_bus_wait_te_fall(2000000U);
          (void)qspi2_lcd_bus_wait_te_rise(2000000U);
#endif

          if(!lcd_draw_test_bars())
          {
            UART3_Printf("LCD: draw 5 bars FAIL\r\n");
          }
          else
          {
            UART3_Printf("LCD: bars RED GREEN BLUE WHITE BLACK shown\r\n");
          }

          /* lcd_run_bmp_test(); */
          lcd_uart_fill_loop();
        }
      }

      ran = 1;
    }

    wk_delay_ms(1000);

    /* add user code end 3 */
  }
}

  /* add user code begin 4 */

  /* add user code end 4 */
