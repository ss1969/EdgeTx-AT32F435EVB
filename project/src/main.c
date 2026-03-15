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
/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */
#define APS6404L_TEST_MAX_SIZE            (8U * 1024U * 1024U)
#define APS6404L_TEST_STEP                (64U * 1024U)
#define APS6404L_TEST_BLOCK_SIZE          256U
#define APS6404L_TEST_PROGRESS_STEP       (1024U * 1024U)

/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */

/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */
void UART3_SendString(char *str);

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */


/**
  * @brief  Send a string via UART3
  * @param  str: pointer to the null-terminated string
  * @retval none
  */
void UART3_SendString(char *str)
{
    while (*str)
    {
        /* Wait until transmit data buffer is empty */
        while (usart_flag_get(USART3, USART_TDBE_FLAG) == RESET)
        {
        }

        /* Send data */
        usart_data_transmit(USART3, (uint16_t)(*str));
        str++;
    }

    /* Wait until transmit complete */
    while (usart_flag_get(USART3, USART_TDC_FLAG) == RESET)
    {
    }
}


static void UART3_SendHexByte(unsigned char value)
{
  char hex_buf[4];
  hex_buf[0] = ' ';
  hex_buf[1] = "0123456789ABCDEF"[value >> 4];
  hex_buf[2] = "0123456789ABCDEF"[value & 0x0F];
  hex_buf[3] = '\0';
  UART3_SendString(hex_buf);
}

static void UART3_SendHexU32(uint32_t value)
{
  UART3_SendHexByte((unsigned char)(value >> 24));
  UART3_SendHexByte((unsigned char)(value >> 16));
  UART3_SendHexByte((unsigned char)(value >> 8));
  UART3_SendHexByte((unsigned char)(value >> 0));
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

  UART3_SendString("\x1b[2J\x1b[H");
  UART3_SendString("start up\r\n");

  /* add user code end 2 */

  while(1)
  {
    /* add user code begin 3 */
    static unsigned char ran = 0;

    if(!ran)
    {
      int ok;
      unsigned char write_buf[APS6404L_TEST_BLOCK_SIZE];
      unsigned char read_buf[APS6404L_TEST_BLOCK_SIZE];
      volatile unsigned char *psram = (volatile unsigned char *)APS6404L_MEM_BASE;
      unsigned int addr;
      unsigned int fail_addr = 0;
      unsigned int last_block;
      unsigned char pass = 1;
      unsigned char fail_index = 0;
      unsigned char fail_exp = 0;
      unsigned char fail_got = 0;
      unsigned int seed;

      edma_stream_enable(EDMA_STREAM1, FALSE);

      for(i = 0; i < (int)sizeof(id); i++)
      {
        id[i] = 0;
      }

      APS6404LHwReset();
      ok = APS6404LHwReadId(id, (int)sizeof(id));

      UART3_SendString("APS RDID:");
      for(i = 0; i < (int)sizeof(id); i++)
      {
        UART3_SendHexByte(id[i]);
      }
      UART3_SendString("\r\n");

      UART3_SendString("stage=");
      UART3_SendHexU32(APS6404LHwGetLastStage());
      UART3_SendString("\r\n");

      if(!ok)
      {
        UART3_SendString("APS RDID: FAIL\r\n");
      }

      UART3_SendString("APS RW BIG: START\r\n");

      if(APS6404L_TEST_MAX_SIZE < (unsigned int)sizeof(write_buf))
      {
        last_block = 0;
      }
      else
      {
        last_block = APS6404L_TEST_MAX_SIZE - (unsigned int)sizeof(write_buf);
      }

      for(addr = 0; addr <= last_block; addr += APS6404L_TEST_STEP)
      {
        if((addr % APS6404L_TEST_PROGRESS_STEP) == 0U)
        {
          UART3_SendString("addr=");
          UART3_SendHexU32(addr);
          UART3_SendString("\r\n");
        }

        seed = 0x6D2B79F5u ^ addr;
        for(i = 0; i < (int)sizeof(write_buf); i++)
        {
          seed ^= seed << 13;
          seed ^= seed >> 17;
          seed ^= seed << 5;
          write_buf[i] = (unsigned char)(seed & 0xFFu);
          read_buf[i] = 0;
        }

        for(i = 0; i < (int)sizeof(write_buf); i++)
        {
          psram[addr + (unsigned int)i] = write_buf[i];
        }

        __DSB();
        __ISB();

        for(i = 0; i < (int)sizeof(read_buf); i++)
        {
          read_buf[i] = psram[addr + (unsigned int)i];
        }

        for(i = 0; i < (int)sizeof(write_buf); i++)
        {
          if(write_buf[i] != read_buf[i])
          {
            pass = 0;
            fail_addr = addr;
            fail_index = (unsigned char)i;
            fail_exp = write_buf[i];
            fail_got = read_buf[i];
            break;
          }
        }

        if(!pass)
        {
          break;
        }
      }

      if(pass)
      {
        UART3_SendString("APS RW BIG: OK\r\n");
      }
      else
      {
        UART3_SendString("APS RW BIG: FAIL addr=");
        UART3_SendHexU32(fail_addr);
        UART3_SendString(" idx=");
        UART3_SendHexByte(fail_index);
        UART3_SendString(" exp=");
        UART3_SendHexByte(fail_exp);
        UART3_SendString(" got=");
        UART3_SendHexByte(fail_got);
        UART3_SendString(" read0-15:");
        for(i = 0; i < 16; i++)
        {
          UART3_SendHexByte(read_buf[i]);
        }
        UART3_SendString("\r\n");
        UART3_SendString("stage=");
        UART3_SendHexU32(APS6404LHwGetLastStage());
        UART3_SendString("\r\n");
      }

      ran = 1;
    }

    wk_delay_ms(1000);

    /* add user code end 3 */
  }
}

  /* add user code begin 4 */

  /* add user code end 4 */
