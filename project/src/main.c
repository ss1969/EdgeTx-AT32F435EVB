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

  /* timebase config. */
  wk_timebase_init();

  /* init gpio function. */
  wk_gpio_config();

  /* init edma stream1 */
  wk_edma_stream1_init();

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
    }

    wk_delay_ms(1000);

    /* add user code end 3 */
  }
}

  /* add user code begin 4 */

  /* add user code end 4 */
