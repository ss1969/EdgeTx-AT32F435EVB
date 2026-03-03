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
#include "wk_gpio.h"
#include "wk_system.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */

/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */

/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */

/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

static void log_write_str(const char* s);
static void log_write_hex_u8(uint8_t v);
static void log_write_hex_u32(uint32_t v);
static void log_write_u32(uint32_t v);

static int qspi1_cmd_no_data(uint8_t instruction);
static int qspi1_cmd_read(uint8_t instruction, uint32_t address, qspi_cmd_adrlen_type address_length, uint8_t* out, uint32_t out_len);
static int qspi1_cmd_write(uint8_t instruction, uint32_t address, qspi_cmd_adrlen_type address_length, const uint8_t* data, uint32_t data_len);

static int qspi1_flash_read_jedec_id(uint8_t id[3]);
static int qspi1_flash_read_sr1(uint8_t* sr1);
static int qspi1_flash_write_enable(void);
static int qspi1_flash_wait_wip_clear(uint32_t timeout_ms);
static int qspi1_flash_erase_4k(uint32_t address, uint32_t timeout_ms);
static int qspi1_flash_program_page(uint32_t address, const uint8_t* data, uint32_t data_len, uint32_t timeout_ms);
static int qspi1_flash_read(uint32_t address, uint8_t* out, uint32_t out_len);
static void qspi1_flash_test_en25qh128a(void);
static void qspi1_dump_regs(const char* tag);
static int qspi1_bb_read_jedec_id(uint8_t id[3]);

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

  wk_delay_ms(50);
  log_write_str("\r\nUART3 debug ready\r\n");
  qspi1_flash_test_en25qh128a();

  /* add user code end 2 */

  while(1)
  {
    /* add user code begin 3 */

    /* add user code end 3 */
  }
}

  /* add user code begin 4 */

static void uart3_write_u8(uint8_t v)
{
  while(usart_flag_get(USART3, USART_TDBE_FLAG) == RESET)
  {
  }
  usart_data_transmit(USART3, v);
}

static void log_write_str(const char* s)
{
  if(!s)
  {
    return;
  }

  while(*s)
  {
    char c = *s++;
    if(c == '\n')
    {
      uart3_write_u8('\r');
    }
    uart3_write_u8((uint8_t)c);
  }
}

static void log_write_hex_u8(uint8_t v)
{
  static const char hex[] = "0123456789ABCDEF";
  uart3_write_u8((uint8_t)hex[(v >> 4) & 0x0F]);
  uart3_write_u8((uint8_t)hex[(v >> 0) & 0x0F]);
}

static void log_write_hex_u32(uint32_t v)
{
  log_write_hex_u8((uint8_t)((v >> 24) & 0xFF));
  log_write_hex_u8((uint8_t)((v >> 16) & 0xFF));
  log_write_hex_u8((uint8_t)((v >> 8) & 0xFF));
  log_write_hex_u8((uint8_t)((v >> 0) & 0xFF));
}

static void log_write_u32(uint32_t v)
{
  char buf[11];
  uint32_t i = 0;

  if(v == 0)
  {
    uart3_write_u8('0');
    return;
  }

  while(v && i < (uint32_t)sizeof(buf))
  {
    buf[i++] = (char)('0' + (v % 10U));
    v /= 10U;
  }

  while(i)
  {
    uart3_write_u8((uint8_t)buf[--i]);
  }
}

static int qspi1_wait_flag(uint32_t flag, flag_status expected, uint32_t timeout_ms)
{
  uint32_t timeout_us = timeout_ms * 1000U;
  while(timeout_us--)
  {
    if(qspi_flag_get(QSPI1, flag) == expected)
    {
      return 0;
    }
    wk_delay_us(1);
  }
  return -1;
}

static int qspi1_cmd_no_data(uint8_t instruction)
{
  qspi_cmd_type cmd;
  cmd.pe_mode_enable = FALSE;
  cmd.pe_mode_operate_code = 0;
  cmd.instruction_code = instruction;
  cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  cmd.address_code = 0;
  cmd.address_length = QSPI_CMD_ADRLEN_0_BYTE;
  cmd.data_counter = 0;
  cmd.second_dummy_cycle_num = 0;
  cmd.operation_mode = QSPI_OPERATE_MODE_111;
  cmd.read_status_config = QSPI_RSTSC_HW_AUTO;
  cmd.read_status_enable = FALSE;
  cmd.write_data_enable = TRUE;

  qspi_interrupt_enable(QSPI1, FALSE);
  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);

  if(qspi1_wait_flag(QSPI_CMDSTS_FLAG, SET, 50) != 0)
  {
    return -1;
  }
  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  return 0;
}

static int qspi1_cmd_read(uint8_t instruction, uint32_t address, qspi_cmd_adrlen_type address_length, uint8_t* out, uint32_t out_len)
{
  qspi_cmd_type cmd;
  uint32_t i = 0;

  if(!out && out_len)
  {
    return -1;
  }

  cmd.pe_mode_enable = FALSE;
  cmd.pe_mode_operate_code = 0;
  cmd.instruction_code = instruction;
  cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  cmd.address_code = address;
  cmd.address_length = address_length;
  cmd.data_counter = out_len;
  cmd.second_dummy_cycle_num = 0;
  cmd.operation_mode = QSPI_OPERATE_MODE_111;
  cmd.read_status_config = QSPI_RSTSC_HW_AUTO;
  cmd.read_status_enable = FALSE;
  cmd.write_data_enable = FALSE;

  qspi_interrupt_enable(QSPI1, FALSE);
  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);

  for(i = 0; i < out_len; i++)
  {
    if(qspi1_wait_flag(QSPI_RXFIFORDY_FLAG, SET, 5) != 0)
    {
      if(qspi_flag_get(QSPI1, QSPI_CMDSTS_FLAG) != SET)
      {
        return -1;
      }
    }
    out[i] = qspi_byte_read(QSPI1);
  }

  if(qspi1_wait_flag(QSPI_CMDSTS_FLAG, SET, 50) != 0)
  {
    return -1;
  }
  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  return 0;
}

static int qspi1_cmd_write(uint8_t instruction, uint32_t address, qspi_cmd_adrlen_type address_length, const uint8_t* data, uint32_t data_len)
{
  qspi_cmd_type cmd;
  uint32_t i = 0;

  if(!data && data_len)
  {
    return -1;
  }

  cmd.pe_mode_enable = FALSE;
  cmd.pe_mode_operate_code = 0;
  cmd.instruction_code = instruction;
  cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  cmd.address_code = address;
  cmd.address_length = address_length;
  cmd.data_counter = data_len;
  cmd.second_dummy_cycle_num = 0;
  cmd.operation_mode = QSPI_OPERATE_MODE_111;
  cmd.read_status_config = QSPI_RSTSC_HW_AUTO;
  cmd.read_status_enable = FALSE;
  cmd.write_data_enable = TRUE;

  qspi_interrupt_enable(QSPI1, FALSE);
  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);

  for(i = 0; i < data_len; i++)
  {
    if(qspi1_wait_flag(QSPI_TXFIFORDY_FLAG, SET, 50) != 0)
    {
      return -1;
    }
    qspi_byte_write(QSPI1, data[i]);
  }

  if(qspi1_wait_flag(QSPI_CMDSTS_FLAG, SET, 200) != 0)
  {
    return -1;
  }
  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  return 0;
}

static int qspi1_flash_read_jedec_id(uint8_t id[3])
{
  return qspi1_cmd_read(0x9F, 0, QSPI_CMD_ADRLEN_0_BYTE, id, 3);
}

static int qspi1_flash_read_sr1(uint8_t* sr1)
{
  return qspi1_cmd_read(0x05, 0, QSPI_CMD_ADRLEN_0_BYTE, sr1, 1);
}

static int qspi1_flash_write_enable(void)
{
  uint8_t sr1 = 0;

  if(qspi1_cmd_no_data(0x06) != 0)
  {
    return -1;
  }

  if(qspi1_flash_read_sr1(&sr1) != 0)
  {
    return -1;
  }

  if((sr1 & 0x02) == 0)
  {
    return -1;
  }

  return 0;
}

static int qspi1_flash_wait_wip_clear(uint32_t timeout_ms)
{
  uint32_t waited_ms = 0;
  uint8_t sr1 = 0x01;

  while(waited_ms < timeout_ms)
  {
    if(qspi1_flash_read_sr1(&sr1) != 0)
    {
      return -1;
    }
    if((sr1 & 0x01) == 0)
    {
      return 0;
    }
    wk_delay_ms(1);
    waited_ms++;
  }

  return -1;
}

static int qspi1_flash_erase_4k(uint32_t address, uint32_t timeout_ms)
{
  uint32_t waited_ms = 0;
  uint8_t sr1 = 0;

  if(qspi1_flash_read_sr1(&sr1) == 0)
  {
    log_write_str("SR1(before)=0x");
    log_write_hex_u8(sr1);
    log_write_str("\r\n");
  }

  if(qspi1_cmd_no_data(0x06) != 0)
  {
    log_write_str("WREN cmd: FAIL\r\n");
    return -1;
  }

  if(qspi1_flash_read_sr1(&sr1) != 0)
  {
    log_write_str("RDSR after WREN: FAIL\r\n");
    return -1;
  }

  log_write_str("SR1(after WREN)=0x");
  log_write_hex_u8(sr1);
  log_write_str("\r\n");

  if((sr1 & 0x02) == 0)
  {
    log_write_str("WEL not set\r\n");
    return -1;
  }

  {
    qspi_cmd_type cmd;
    cmd.pe_mode_enable = FALSE;
    cmd.pe_mode_operate_code = 0;
    cmd.instruction_code = 0x20;
    cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
    cmd.address_code = address;
    cmd.address_length = QSPI_CMD_ADRLEN_3_BYTE;
    cmd.data_counter = 0;
    cmd.second_dummy_cycle_num = 0;
    cmd.operation_mode = QSPI_OPERATE_MODE_111;
    cmd.read_status_config = QSPI_RSTSC_HW_AUTO;
    cmd.read_status_enable = FALSE;
    cmd.write_data_enable = TRUE;

    qspi_interrupt_enable(QSPI1, FALSE);
    qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
    qspi_cmd_operation_kick(QSPI1, &cmd);
    if(qspi1_wait_flag(QSPI_CMDSTS_FLAG, SET, 1000) != 0)
    {
      log_write_str("ERASE cmdsts: TIMEOUT\r\n");
      qspi1_dump_regs("QSPI1 ERASE CMDSTS TIMEOUT");
      return -1;
    }
    qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  }

  if(qspi1_flash_read_sr1(&sr1) == 0)
  {
    log_write_str("SR1(after ERASE cmd)=0x");
    log_write_hex_u8(sr1);
    log_write_str("\r\n");
  }

  sr1 = 0x01;
  while(waited_ms < timeout_ms)
  {
    if(qspi1_flash_read_sr1(&sr1) != 0)
    {
      log_write_str("RDSR during erase: FAIL\r\n");
      return -1;
    }
    if((sr1 & 0x01) == 0)
    {
      return 0;
    }
    wk_delay_ms(1);
    waited_ms++;
  }

  log_write_str("Erase WIP timeout SR1=0x");
  log_write_hex_u8(sr1);
  log_write_str("\r\n");
  return -1;
}

static int qspi1_flash_program_page(uint32_t address, const uint8_t* data, uint32_t data_len, uint32_t timeout_ms)
{
  if(data_len == 0 || data_len > 256)
  {
    return -1;
  }

  if(qspi1_flash_write_enable() != 0)
  {
    return -1;
  }

  if(qspi1_cmd_write(0x02, address, QSPI_CMD_ADRLEN_3_BYTE, data, data_len) != 0)
  {
    return -1;
  }

  return qspi1_flash_wait_wip_clear(timeout_ms);
}

static int qspi1_flash_read(uint32_t address, uint8_t* out, uint32_t out_len)
{
  return qspi1_cmd_read(0x03, address, QSPI_CMD_ADRLEN_3_BYTE, out, out_len);
}

static void qspi1_bb_clk_pulse(void)
{
  gpio_bits_set(GPIOF, GPIO_PINS_10);
  wk_delay_us(1);
  gpio_bits_reset(GPIOF, GPIO_PINS_10);
  wk_delay_us(1);
}

static uint8_t qspi1_bb_xfer_u8(uint8_t tx)
{
  uint8_t rx = 0;
  uint32_t i = 0;

  for(i = 0; i < 8; i++)
  {
    if((tx & 0x80) != 0)
    {
      gpio_bits_set(GPIOF, GPIO_PINS_8);
    }
    else
    {
      gpio_bits_reset(GPIOF, GPIO_PINS_8);
    }

    tx <<= 1;

    gpio_bits_set(GPIOF, GPIO_PINS_10);
    wk_delay_us(1);
    rx <<= 1;
    if(gpio_input_data_bit_read(GPIOF, GPIO_PINS_9) == SET)
    {
      rx |= 1;
    }
    gpio_bits_reset(GPIOF, GPIO_PINS_10);
    wk_delay_us(1);
  }

  return rx;
}

static int qspi1_bb_read_jedec_id(uint8_t id[3])
{
  gpio_init_type gpio_init_struct;
  gpio_default_para_init(&gpio_init_struct);

  qspi_xip_enable(QSPI1, FALSE);

  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_UP;

  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_10 | GPIO_PINS_8;
  gpio_init(GPIOF, &gpio_init_struct);

  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_6;
  gpio_init(GPIOG, &gpio_init_struct);

  gpio_init_struct.gpio_mode = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_pins = GPIO_PINS_9;
  gpio_init(GPIOF, &gpio_init_struct);

  gpio_bits_set(GPIOG, GPIO_PINS_6);
  gpio_bits_reset(GPIOF, GPIO_PINS_10);
  gpio_bits_set(GPIOF, GPIO_PINS_8);
  wk_delay_us(2);

  gpio_bits_reset(GPIOG, GPIO_PINS_6);
  wk_delay_us(2);

  (void)qspi1_bb_xfer_u8(0x9F);
  id[0] = qspi1_bb_xfer_u8(0xFF);
  id[1] = qspi1_bb_xfer_u8(0xFF);
  id[2] = qspi1_bb_xfer_u8(0xFF);

  gpio_bits_set(GPIOG, GPIO_PINS_6);
  wk_delay_us(2);

  wk_qspi1_init();
  return 0;
}

static void qspi1_dump_regs(const char* tag)
{
  if(tag)
  {
    log_write_str(tag);
    log_write_str("\r\n");
  }

  log_write_str("CRM_AHBEN3=0x");
  log_write_hex_u32(CRM->ahben3);
  log_write_str("\r\n");

  log_write_str("QSPI1 ctrl=0x");
  log_write_hex_u32(QSPI1->ctrl);
  log_write_str(" ctrl2=0x");
  log_write_hex_u32(QSPI1->ctrl2);
  log_write_str(" ctrl3=0x");
  log_write_hex_u32(QSPI1->ctrl3);
  log_write_str("\r\n");

  log_write_str("QSPI1 fifosts=0x");
  log_write_hex_u32(QSPI1->fifosts);
  log_write_str(" cmdsts=0x");
  log_write_hex_u32(QSPI1->cmdsts);
  log_write_str(" rsts=0x");
  log_write_hex_u32(QSPI1->rsts);
  log_write_str(" fsize=0x");
  log_write_hex_u32(QSPI1->fsize);
  log_write_str("\r\n");

  log_write_str("QSPI1 cmd_w0=0x");
  log_write_hex_u32(QSPI1->cmd_w0);
  log_write_str(" cmd_w1=0x");
  log_write_hex_u32(QSPI1->cmd_w1);
  log_write_str(" cmd_w2=0x");
  log_write_hex_u32(QSPI1->cmd_w2);
  log_write_str(" cmd_w3=0x");
  log_write_hex_u32(QSPI1->cmd_w3);
  log_write_str("\r\n");
}

static void qspi1_flash_test_en25qh128a(void)
{
  uint8_t id[3] = {0};
  uint8_t bbid[3] = {0};
  uint8_t tx[256];
  uint8_t rx[256];
  uint32_t i = 0;
  const uint32_t test_addr = 0x00FF0000;

  log_write_str("QSPI1 EN25QH128A test\r\n");
  log_write_str("TEST_ADDR=0x");
  log_write_hex_u32(test_addr);
  log_write_str("\r\n");

  qspi1_dump_regs("QSPI1 before RDID");

  if(qspi1_bb_read_jedec_id(bbid) == 0)
  {
    log_write_str("BB_RDID: ");
    log_write_hex_u8(bbid[0]);
    log_write_str(" ");
    log_write_hex_u8(bbid[1]);
    log_write_str(" ");
    log_write_hex_u8(bbid[2]);
    log_write_str("\r\n");
  }

  if(qspi1_flash_read_jedec_id(id) != 0)
  {
    log_write_str("RDID: FAIL\r\n");
    qspi1_dump_regs("QSPI1 RDID FAIL");
    return;
  }

  log_write_str("RDID: ");
  log_write_hex_u8(id[0]);
  log_write_str(" ");
  log_write_hex_u8(id[1]);
  log_write_str(" ");
  log_write_hex_u8(id[2]);
  log_write_str("\r\n");

  if(!(id[0] == 0x1C && id[1] == 0x70 && id[2] == 0x18))
  {
    log_write_str("RDID mismatch\r\n");
    return;
  }

  log_write_str("Erase 4K...\r\n");
  if(qspi1_flash_erase_4k(test_addr, 5000) != 0)
  {
    log_write_str("Erase: FAIL\r\n");
    return;
  }
  log_write_str("Erase: OK\r\n");

  log_write_str("Erase verify (all FF)...\r\n");
  if(qspi1_flash_read(test_addr, rx, (uint32_t)sizeof(rx)) != 0)
  {
    log_write_str("Erase verify: READ FAIL\r\n");
    return;
  }
  for(i = 0; i < (uint32_t)sizeof(rx); i++)
  {
    if(rx[i] != 0xFF)
    {
      log_write_str("Erase verify: FAIL offset=");
      log_write_u32(i);
      log_write_str(" val=");
      log_write_hex_u8(rx[i]);
      log_write_str("\r\n");
      return;
    }
  }
  log_write_str("Erase verify: OK\r\n");

  for(i = 0; i < (uint32_t)sizeof(tx); i++)
  {
    tx[i] = (uint8_t)((i * 13U + 7U) & 0xFF);
    rx[i] = 0;
  }

  log_write_str("Program page...\r\n");
  if(qspi1_flash_program_page(test_addr, tx, (uint32_t)sizeof(tx), 2000) != 0)
  {
    log_write_str("Program: FAIL\r\n");
    return;
  }
  log_write_str("Program: OK\r\n");

  log_write_str("Read back...\r\n");
  if(qspi1_flash_read(test_addr, rx, (uint32_t)sizeof(rx)) != 0)
  {
    log_write_str("Read: FAIL\r\n");
    return;
  }

  for(i = 0; i < (uint32_t)sizeof(tx); i++)
  {
    if(rx[i] != tx[i])
    {
      log_write_str("Verify: FAIL offset=");
      log_write_u32(i);
      log_write_str(" exp=");
      log_write_hex_u8(tx[i]);
      log_write_str(" act=");
      log_write_hex_u8(rx[i]);
      log_write_str("\r\n");
      return;
    }
  }

  log_write_str("Verify: OK\r\n");
}

  /* add user code end 4 */
