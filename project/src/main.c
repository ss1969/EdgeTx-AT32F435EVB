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
#include "wk_edma.h"
#include "wk_gpio.h"
#include "wk_system.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */
#include "aps6404l.h"
#include "spi1_lcd_bus.h"
#include "txw430038b0_nv3041a_01_spi1.h"
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

#if 0
static inline void spi1_lcd_cs_low(void)
{
  gpio_bits_reset(SPI1_CS_GPIO_PORT, SPI1_CS_PIN);
}

static inline void spi1_lcd_cs_high(void)
{
  gpio_bits_set(SPI1_CS_GPIO_PORT, SPI1_CS_PIN);
}

static inline void spi1_lcd_dcx_cmd(void)
{
  gpio_bits_reset(SPI1_DCX_GPIO_PORT, SPI1_DCX_PIN);
}

static inline void spi1_lcd_dcx_data(void)
{
  gpio_bits_set(SPI1_DCX_GPIO_PORT, SPI1_DCX_PIN);
}

static int spi1_wait_flag_set(uint32_t flag, uint32_t timeout)
{
  while(spi_i2s_flag_get(SPI1, flag) == RESET)
  {
    if(timeout-- == 0U)
    {
      return 0;
    }
  }
  return 1;
}

static int spi1_wait_flag_reset(uint32_t flag, uint32_t timeout)
{
  while(spi_i2s_flag_get(SPI1, flag) == SET)
  {
    if(timeout-- == 0U)
    {
      return 0;
    }
  }
  return 1;
}

static int spi1_xfer8(uint8_t tx, uint8_t *rx)
{
  if(!spi1_wait_flag_set(SPI_I2S_TDBE_FLAG, 2000000U))
  {
    return 0;
  }
  spi_i2s_data_transmit(SPI1, tx);
  if(!spi1_wait_flag_set(SPI_I2S_RDBF_FLAG, 2000000U))
  {
    return 0;
  }
  if(rx != 0)
  {
    *rx = (uint8_t)spi_i2s_data_receive(SPI1);
  }
  else
  {
    (void)spi_i2s_data_receive(SPI1);
  }
  return 1;
}

static int spi1_lcd_read_reg(uint8_t cmd, uint8_t *out, uint32_t out_len, uint32_t dummy_bytes)
{
  uint32_t i;
  uint8_t tmp;

  spi_half_duplex_direction_set(SPI1, SPI_HALF_DUPLEX_DIRECTION_TX);
  spi1_lcd_cs_low();
  spi1_lcd_dcx_cmd();
  if(!spi1_xfer8(cmd, 0))
  {
    spi1_lcd_cs_high();
    return 0;
  }

  spi1_lcd_dcx_data();
  spi_half_duplex_direction_set(SPI1, SPI_HALF_DUPLEX_DIRECTION_RX);

  for(i = 0; i < dummy_bytes; i++)
  {
    if(!spi1_xfer8(0x00, &tmp))
    {
      spi_half_duplex_direction_set(SPI1, SPI_HALF_DUPLEX_DIRECTION_TX);
      spi1_lcd_cs_high();
      return 0;
    }
  }

  for(i = 0; i < out_len; i++)
  {
    if(!spi1_xfer8(0x00, &out[i]))
    {
      spi_half_duplex_direction_set(SPI1, SPI_HALF_DUPLEX_DIRECTION_TX);
      spi1_lcd_cs_high();
      return 0;
    }
  }

  if(!spi1_wait_flag_reset(SPI_I2S_BF_FLAG, 2000000U))
  {
    spi_half_duplex_direction_set(SPI1, SPI_HALF_DUPLEX_DIRECTION_TX);
    spi1_lcd_cs_high();
    return 0;
  }

  spi_half_duplex_direction_set(SPI1, SPI_HALF_DUPLEX_DIRECTION_TX);
  spi1_lcd_cs_high();
  return 1;
}
#endif
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
  unsigned char id[8];
  int i;

  UART3_Printf("\x1b[2J\x1b[H");
  UART3_Printf("start up\r\n");

  /* add user code end 2 */

  while(1)
  {
    /* add user code begin 3 */
    static unsigned char ran = 0;

    if(!ran)
    {
#if 0
      int ok;
      unsigned char *write_buf;
      volatile unsigned char *psram = (volatile unsigned char *)APS6404L_MEM_BASE;
      unsigned int addr;
      unsigned int fail_addr = 0;
      unsigned int offset;
      unsigned char pass = 1;
      unsigned char fail_index = 0;
      unsigned char fail_exp = 0;
      unsigned char fail_got = 0;
      unsigned int seed;
      unsigned int chunk_len;
      unsigned char used_malloc = 0;
      static unsigned char write_buf_fallback[APS6404L_TEST_CHUNK_SIZE];

      for(i = 0; i < (int)sizeof(id); i++)
      {
        id[i] = 0;
      }

      PSRAM_Reset();
      ok = PSRAM_ReadID(id, (int)sizeof(id));

      UART3_Printf("APS RDID:");
      for(i = 0; i < (int)sizeof(id); i++)
      {
        UART3_PrintHexByte(id[i]);
      }
      UART3_Printf("\r\n");

      if(ok)
      {
        unsigned char all_ff = 1;
        for(i = 0; i < (int)sizeof(id); i++)
        {
          if(id[i] != 0xFF)
          {
            all_ff = 0;
            break;
          }
        }
        if(all_ff)
        {
          ok = 0;
        }
      }

      if(!ok)
      {
        UART3_Printf("APS RDID: FAIL\r\n");
      }

      {
        unsigned char qpi_w[32];
        unsigned char qpi_r[32];
        unsigned char qpi_pass = 1;
        unsigned char qpi_fail_index = 0;

        for(i = 0; i < (int)sizeof(qpi_w); i++)
        {
          qpi_w[i] = (unsigned char)(0xA0 + i);
          qpi_r[i] = 0;
        }

        UART3_Printf("APS QPI FAST: START\r\n");
        if(!PSRAM_EnterQuadMode())
        {
          UART3_Printf("APS QPI FAST: ENTER FAIL\r\n");
        }
        else
        {
          if(!PSRAM_QPI_FastWrite(0, qpi_w, (int)sizeof(qpi_w)))
          {
            UART3_Printf("APS QPI FAST: W FAIL\r\n");
          }
          else if(!PSRAM_QPI_FastRead(0, qpi_r, (int)sizeof(qpi_r)))
          {
            UART3_Printf("APS QPI FAST: R FAIL\r\n");
          }
          else
          {
            for(i = 0; i < (int)sizeof(qpi_w); i++)
            {
              if(qpi_w[i] != qpi_r[i])
              {
                qpi_pass = 0;
                qpi_fail_index = (unsigned char)i;
                break;
              }
            }

            if(qpi_pass)
            {
              UART3_Printf("APS QPI FAST: OK\r\n");
            }
            else
            {
              UART3_Printf("APS QPI FAST: FAIL idx=");
              UART3_PrintHexByte(qpi_fail_index);
              UART3_Printf(" exp=");
              UART3_PrintHexByte(qpi_w[qpi_fail_index]);
              UART3_Printf(" got=");
              UART3_PrintHexByte(qpi_r[qpi_fail_index]);
              UART3_Printf("\r\n");
            }
          }

          if(!PSRAM_ExitQuadMode())
          {
            UART3_Printf("APS QPI FAST: EXIT FAIL\r\n");
          }
        }
      }

      PSRAM_Reset();

      {
        static uint32_t bench_w_u32[APS6404L_BENCH_CHUNK_BYTES / 4U];
        static uint32_t bench_r_u32[APS6404L_BENCH_CHUNK_BYTES / 4U];
        unsigned char *bench_w = (unsigned char *)bench_w_u32;
        unsigned char *bench_r = (unsigned char *)bench_r_u32;
        uint32_t t0;
        uint32_t t1;
        uint32_t off;
        uint32_t sum;
        uint32_t base = 0x00100000u;
        int bench_ok = 1;

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
          if(!PSRAM_QPI_Write(base + off, bench_w, (int)APS6404L_BENCH_CHUNK_BYTES))
          {
            bench_ok = 0;
            break;
          }
        }
        t1 = aps_perf_cycles();
        if(bench_ok)
        {
          aps_print_speed("APS BENCH WR 111", APS6404L_BENCH_TOTAL_BYTES, t1 - t0);
        }
        else
        {
          UART3_Printf("APS BENCH WR 111: FAIL\r\n");
        }

        sum = 0;
        bench_ok = 1;
        t0 = aps_perf_cycles();
        for(off = 0; off < APS6404L_BENCH_TOTAL_BYTES; off += APS6404L_BENCH_CHUNK_BYTES)
        {
          if(!PSRAM_QPI_Read(base + off, bench_r, (int)APS6404L_BENCH_CHUNK_BYTES))
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
          aps_print_speed("APS BENCH RD 111", APS6404L_BENCH_TOTAL_BYTES, t1 - t0);
          UART3_Printf("APS BENCH RD 111: sum=");
          UART3_PrintHexU32(sum);
          UART3_Printf("\r\n");
        }
        else
        {
          UART3_Printf("APS BENCH RD 111: FAIL\r\n");
        }

        if(PSRAM_EnterQuadMode())
        {
          bench_ok = 1;
          t0 = aps_perf_cycles();
          for(off = 0; off < APS6404L_BENCH_TOTAL_BYTES; off += APS6404L_BENCH_CHUNK_BYTES)
          {
            if(!PSRAM_QPI_FastWrite(base + off, bench_w, (int)APS6404L_BENCH_CHUNK_BYTES))
            {
              bench_ok = 0;
              break;
            }
          }
          t1 = aps_perf_cycles();
          if(bench_ok)
          {
            aps_print_speed("APS BENCH WR 444", APS6404L_BENCH_TOTAL_BYTES, t1 - t0);
          }
          else
          {
          UART3_Printf("APS BENCH WR 444: FAIL\r\n");
          }

          sum = 0;
          bench_ok = 1;
          t0 = aps_perf_cycles();
          for(off = 0; off < APS6404L_BENCH_TOTAL_BYTES; off += APS6404L_BENCH_CHUNK_BYTES)
          {
            if(!PSRAM_QPI_FastRead(base + off, bench_r, (int)APS6404L_BENCH_CHUNK_BYTES))
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
            aps_print_speed("APS BENCH RD 444", APS6404L_BENCH_TOTAL_BYTES, t1 - t0);
            UART3_Printf("APS BENCH RD 444: sum=");
            UART3_PrintHexU32(sum);
            UART3_Printf("\r\n");
          }
          else
          {
          UART3_Printf("APS BENCH RD 444: FAIL\r\n");
          }

          if(!PSRAM_ExitQuadMode())
          {
            UART3_Printf("APS BENCH: EXIT QPI FAIL\r\n");
          }
        }
        else
        {
          UART3_Printf("APS BENCH: ENTER QPI FAIL\r\n");
        }

        bench_ok = 1;
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

      {
        static uint32_t edma_w_u32[256];
        static uint32_t edma_r_u32[256];
        unsigned char edma_pass = 1;
        unsigned char edma_fail_index = 0;
        unsigned int edma_len = (unsigned int)sizeof(edma_w_u32);
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
        }
        else if(!PSRAM_EDMA_Read(0x00001000u, edma_r, (int)edma_len))
        {
          UART3_Printf("APS EDMA RW: R FAIL\r\n");
        }
        else
        {
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
      }

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

      addr = 0;
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

      ran = 1;
#endif

      {
        spi_lcd_bus_reset_pulse(20, 20);

        UART3_Printf("LCD(SPI1): %s init\r\n", txw430038b0_nv3041a_01_spi1_name());
        if(!txw430038b0_nv3041a_01_spi_init())
        {
          UART3_Printf("LCD(SPI1): init FAIL\r\n");
        }
        else
        {
          static uint16_t line[TXW430038B0_WIDTH];
          uint32_t x;
          uint32_t y;

          UART3_Printf("LCD(SPI1): INVON 0x21\r\n");
          spi_lcd_bus_write_cmd(0x21);

          for(x = 0; x < TXW430038B0_WIDTH; x++)
          {
            line[x] = 0xFFFFU;
          }

          for(y = 0; y < TXW430038B0_HEIGHT; y++)
          {
            if(!txw430038b0_nv3041a_01_spi_set_window(0, (uint16_t)y, (uint16_t)(TXW430038B0_WIDTH - 1U), (uint16_t)y))
            {
              UART3_Printf("LCD(SPI1): set_window FAIL y=");
              UART3_PrintHexU32(y);
              UART3_Printf("\r\n");
              break;
            }
            if(!txw430038b0_nv3041a_01_spi_write_pixels_rgb565(line, TXW430038B0_WIDTH))
            {
              UART3_Printf("LCD(SPI1): write FAIL y=");
              UART3_PrintHexU32(y);
              UART3_Printf("\r\n");
              break;
            }
          }

          wk_delay_ms(1000);

          for(y = 0; y < TXW430038B0_HEIGHT; y++)
          {
            for(x = 0; x < TXW430038B0_WIDTH; x++)
            {
              uint32_t bar = (x * 8U) / TXW430038B0_WIDTH;
              uint16_t color;

              switch(bar)
              {
                case 0U: color = 0xF800U; break;
                case 1U: color = 0x07E0U; break;
                case 2U: color = 0x001FU; break;
                case 3U: color = 0x07FFU; break;
                case 4U: color = 0xF81FU; break;
                case 5U: color = 0xFFE0U; break;
                case 6U: color = 0xFFFFU; break;
                default: color = 0x0000U; break;
              }

              line[x] = color;
            }

            if(!txw430038b0_nv3041a_01_spi_set_window(0, (uint16_t)y, (uint16_t)(TXW430038B0_WIDTH - 1U), (uint16_t)y))
            {
              UART3_Printf("LCD(SPI1): set_window FAIL y=");
              UART3_PrintHexU32(y);
              UART3_Printf("\r\n");
              break;
            }
            if(!txw430038b0_nv3041a_01_spi_write_pixels_rgb565(line, TXW430038B0_WIDTH))
            {
              UART3_Printf("LCD(SPI1): write FAIL y=");
              UART3_PrintHexU32(y);
              UART3_Printf("\r\n");
              break;
            }
          }

          UART3_Printf("LCD(SPI1): test done\r\n");
        }
      }

#if 0
      UART3_Printf("LCD: %s init\r\n", txw430038b0_nv3041a_01_name());
      if(!txw430038b0_nv3041a_01_init())
      {
        qspi_operate_mode_type modes[3];
        int mi;

        UART3_Printf("LCD: init FAIL opmode=");
        UART3_PrintHexU32((uint32_t)qspi2_lcd_bus_opmode_get());
        UART3_Printf(" bus_err=");
        UART3_PrintHexU32((uint32_t)qspi2_lcd_bus_last_error_get());
        UART3_Printf(" ctrl=");
        UART3_PrintHexU32(QSPI2->ctrl);
        UART3_Printf(" fifosts=");
        UART3_PrintHexU32(QSPI2->fifosts);
        UART3_Printf(" cmdsts=");
        UART3_PrintHexU32(QSPI2->cmdsts);
        UART3_Printf("\r\n");

        modes[0] = QSPI_OPERATE_MODE_114;
        modes[1] = QSPI_OPERATE_MODE_144;
        modes[2] = QSPI_OPERATE_MODE_444;

        for(mi = 0; mi < 3; mi++)
        {
          qspi2_lcd_bus_opmode_set(modes[mi]);
          UART3_Printf("LCD: retry opmode=");
          UART3_PrintHexU32((uint32_t)modes[mi]);
          UART3_Printf("\r\n");
          if(txw430038b0_nv3041a_01_init())
          {
            UART3_Printf("LCD: init OK opmode=");
            UART3_PrintHexU32((uint32_t)modes[mi]);
            UART3_Printf("\r\n");
            break;
          }
        }
      }
      else
      {
        static uint16_t line[TXW430038B0_WIDTH];
        uint32_t x;
        uint32_t y;

        qspi2_lcd_bus_wait_te_rise(2000000U);

        for(y = 0; y < TXW430038B0_HEIGHT; y++)
        {
          for(x = 0; x < TXW430038B0_WIDTH; x++)
          {
            uint32_t bar = (x * 8U) / TXW430038B0_WIDTH;
            uint16_t color;

            switch(bar)
            {
              case 0U: color = 0xF800U; break;
              case 1U: color = 0x07E0U; break;
              case 2U: color = 0x001FU; break;
              case 3U: color = 0x07FFU; break;
              case 4U: color = 0xF81FU; break;
              case 5U: color = 0xFFE0U; break;
              case 6U: color = 0xFFFFU; break;
              default: color = 0x0000U; break;
            }

            line[x] = color;
          }

          if(!txw430038b0_nv3041a_01_set_window(0, (uint16_t)y, (uint16_t)(TXW430038B0_WIDTH - 1U), (uint16_t)y))
          {
            UART3_Printf("LCD: set_window FAIL y=");
            UART3_PrintHexU32(y);
            UART3_Printf("\r\n");
            break;
          }
          if(!txw430038b0_nv3041a_01_write_pixels_rgb565(line, TXW430038B0_WIDTH))
          {
            UART3_Printf("LCD: write FAIL y=");
            UART3_PrintHexU32(y);
            UART3_Printf("\r\n");
            break;
          }
        }

        UART3_Printf("LCD: test done\r\n");
      }
#endif

      ran = 1;
    }

    wk_delay_ms(1000);

    /* add user code end 3 */
  }
}

  /* add user code begin 4 */

  /* add user code end 4 */
