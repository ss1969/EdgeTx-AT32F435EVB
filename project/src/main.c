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
  /* config edma stream transfer parameter */
  /* user need to modify define values EDMA_STREAMx_XXX_BASE_ADDR and EDMA_STREAMx_BUFFER_SIZE in at32xxx_wk_config.h */
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

      edma_stream_enable(EDMA_STREAM1, FALSE);

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

      UART3_Printf("stage=");
      UART3_PrintHexU32(APS6404LHwGetLastStage());
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
          UART3_Printf("stage=");
          UART3_PrintHexU32(APS6404LHwGetLastStage());
          UART3_Printf("\r\n");
        }
        else
        {
          if(!PSRAM_QPI_FastWrite(0, qpi_w, (int)sizeof(qpi_w)))
          {
            UART3_Printf("APS QPI FAST: W FAIL\r\n");
            UART3_Printf("stage=");
            UART3_PrintHexU32(APS6404LHwGetLastStage());
            UART3_Printf("\r\n");
          }
          else if(!PSRAM_QPI_FastRead(0, qpi_r, (int)sizeof(qpi_r)))
          {
            UART3_Printf("APS QPI FAST: R FAIL\r\n");
            UART3_Printf("stage=");
            UART3_PrintHexU32(APS6404LHwGetLastStage());
            UART3_Printf("\r\n");
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
            UART3_Printf("stage=");
            UART3_PrintHexU32(APS6404LHwGetLastStage());
            UART3_Printf("\r\n");
          }
        }
      }

      PSRAM_Reset();

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
        UART3_Printf("stage=");
        UART3_PrintHexU32(APS6404LHwGetLastStage());
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
