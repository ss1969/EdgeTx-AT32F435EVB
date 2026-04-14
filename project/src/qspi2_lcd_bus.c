#include "qspi2_lcd_bus.h"

#define QSPI2_LCD_BUS_TIMEOUT 2000000U

#define QSPI2_LCD_OPCODE_WRITE_CMD   0x02U
#define QSPI2_LCD_OPCODE_READ_CMD    0x03U
#define QSPI2_LCD_OPCODE_WRITE_RAM32 0x32U

static qspi2_lcd_bus_diag_t qspi2_lcd_bus_diag_after_kick;
static qspi2_lcd_bus_diag_t qspi2_lcd_bus_diag_before_exit;

void qspi2_lcd_bus_diag_capture(qspi2_lcd_bus_diag_t *out)
{
  if(out == 0)
  {
    return;
  }

  out->cmd_w0 = QSPI2->cmd_w0;
  out->cmd_w1 = QSPI2->cmd_w1;
  out->cmd_w2 = QSPI2->cmd_w2;
  out->cmd_w3 = QSPI2->cmd_w3;
  out->ctrl = QSPI2->ctrl;
  out->fifosts = QSPI2->fifosts;
  out->ctrl2 = QSPI2->ctrl2;
  out->cmdsts = QSPI2->cmdsts;
  out->rsts = QSPI2->rsts;
  out->fsize = QSPI2->fsize;
  out->xip_cmd_w0 = QSPI2->xip_cmd_w0;
  out->xip_cmd_w1 = QSPI2->xip_cmd_w1;
  out->xip_cmd_w2 = QSPI2->xip_cmd_w2;
  out->xip_cmd_w3 = QSPI2->xip_cmd_w3;
  out->ctrl3 = QSPI2->ctrl3;
}

void qspi2_lcd_bus_diag_last_get(qspi2_lcd_bus_diag_t *after_kick, qspi2_lcd_bus_diag_t *before_exit)
{
  if(after_kick != 0)
  {
    *after_kick = qspi2_lcd_bus_diag_after_kick;
  }
  if(before_exit != 0)
  {
    *before_exit = qspi2_lcd_bus_diag_before_exit;
  }
}

static int qspi2_wait_cmd_done(uint32_t timeout)
{
  while(qspi_flag_get(QSPI2, QSPI_CMDSTS_FLAG) == RESET)
  {
    if(timeout-- == 0U)
    {
      return 0;
    }
  }
  qspi_flag_clear(QSPI2, QSPI_CMDSTS_FLAG);
  return 1;
}

static int qspi2_wait_txfifo_ready(uint32_t timeout)
{
  while(qspi_flag_get(QSPI2, QSPI_TXFIFORDY_FLAG) == RESET)
  {
    if(timeout-- == 0U)
    {
      return 0;
    }
  }
  return 1;
}

static int qspi2_wait_rxfifo_ready(uint32_t timeout)
{
  while(qspi_flag_get(QSPI2, QSPI_RXFIFORDY_FLAG) == RESET)
  {
    if(timeout-- == 0U)
    {
      return 0;
    }
  }
  return 1;
}

static inline int qspi2_te_is_high(void)
{
  return (gpio_input_data_bit_read(QSPI2_TE_GPIO_PORT, QSPI2_TE_PIN) == SET) ? 1 : 0;
}

void qspi2_lcd_bus_reset_pulse(uint32_t low_ms, uint32_t high_ms)
{
  gpio_bits_reset(QSPI2_RESET_GPIO_PORT, QSPI2_RESET_PIN);
  wk_delay_ms(low_ms);
  gpio_bits_set(QSPI2_RESET_GPIO_PORT, QSPI2_RESET_PIN);
  wk_delay_ms(high_ms);
}

int qspi2_lcd_bus_wait_te_rise(uint32_t timeout)
{
  while(qspi2_te_is_high())
  {
    if(timeout-- == 0U)
    {
      return 0;
    }
  }

  while(!qspi2_te_is_high())
  {
    if(timeout-- == 0U)
    {
      return 0;
    }
  }

  return 1;
}

int qspi2_lcd_bus_wait_te_fall(uint32_t timeout)
{
  while(!qspi2_te_is_high())
  {
    if(timeout-- == 0U)
    {
      return 0;
    }
  }

  while(qspi2_te_is_high())
  {
    if(timeout-- == 0U)
    {
      return 0;
    }
  }

  return 1;
}

int qspi2_lcd_bus_write(uint8_t cmd, const uint8_t *data, uint32_t len)
{
  qspi_cmd_type qcmd;
  uint32_t i;
  uint32_t addr;
  uint32_t actual_len;
  uint8_t pad;

  /* NV3041A Quad-SPI uses a 24-bit address field formatted as {0x00, CMD, 0x00}. */
  addr = ((uint32_t)cmd) << 8;

  qcmd.pe_mode_enable = FALSE;
  qcmd.pe_mode_operate_code = 0;
  qcmd.instruction_code = QSPI2_LCD_OPCODE_WRITE_CMD;
  qcmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  qcmd.address_code = addr;
  qcmd.address_length = QSPI_CMD_ADRLEN_3_BYTE;
  /* Some NV3041A command-only writes do not complete cleanly with a zero
     data-count on this controller, so send one pad byte while keeping the
     same command framing. */
  actual_len = (len == 0U) ? 1U : len;
  pad = 0x00U;

  qcmd.data_counter = actual_len;
  qcmd.second_dummy_cycle_num = 0;
  qcmd.operation_mode = QSPI_OPERATE_MODE_111;
  qcmd.read_status_config = QSPI_RSTSC_SW_ONCE;
  qcmd.read_status_enable = FALSE;
  qcmd.write_data_enable = TRUE;

  qspi_flag_clear(QSPI2, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI2, &qcmd);

  for(i = 0; i < actual_len; i++)
  {
    if(!qspi2_wait_txfifo_ready(QSPI2_LCD_BUS_TIMEOUT))
    {
      return 0;
    }
    qspi_byte_write(QSPI2, (len != 0U) ? data[i] : pad);
  }

  if(!qspi2_wait_cmd_done(QSPI2_LCD_BUS_TIMEOUT))
  {
    return 0;
  }

  return 1;
}

int qspi2_lcd_bus_read(uint8_t cmd, uint8_t *data, uint32_t len, uint8_t dummy_cycles)
{
  qspi_cmd_type qcmd;
  uint32_t i;
  uint32_t addr;

  if((data == 0) && (len != 0U))
  {
    return 0;
  }

  /* NV3041A Quad-SPI uses a 24-bit address field formatted as {0x00, CMD, 0x00}. */
  addr = ((uint32_t)cmd) << 8;

  qcmd.pe_mode_enable = FALSE;
  qcmd.pe_mode_operate_code = 0;
  qcmd.instruction_code = QSPI2_LCD_OPCODE_READ_CMD;
  qcmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  qcmd.address_code = addr;
  qcmd.address_length = QSPI_CMD_ADRLEN_3_BYTE;
  qcmd.data_counter = len;
  qcmd.second_dummy_cycle_num = dummy_cycles;
  qcmd.operation_mode = QSPI_OPERATE_MODE_111;
  qcmd.read_status_config = QSPI_RSTSC_SW_ONCE;
  qcmd.read_status_enable = FALSE;
  qcmd.write_data_enable = FALSE;

  qspi_flag_clear(QSPI2, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI2, &qcmd);

  for(i = 0; i < len; i++)
  {
    if(!qspi2_wait_rxfifo_ready(QSPI2_LCD_BUS_TIMEOUT))
    {
      return 0;
    }
    data[i] = qspi_byte_read(QSPI2);
  }

  if(!qspi2_wait_cmd_done(QSPI2_LCD_BUS_TIMEOUT))
  {
    return 0;
  }

  return 1;
}

int qspi2_lcd_bus_read_03h(uint8_t addr, uint8_t *data, uint32_t len, uint8_t dummy_cycles)
{
  qspi_cmd_type qcmd;
  uint32_t i;
  uint32_t addr24;

  qspi2_lcd_bus_diag_capture(&qspi2_lcd_bus_diag_after_kick);
  qspi2_lcd_bus_diag_capture(&qspi2_lcd_bus_diag_before_exit);
  if((data == 0) && (len != 0U))
  {
    return 0;
  }

  addr24 = ((uint32_t)addr) << 8;

  qcmd.pe_mode_enable = FALSE;
  qcmd.pe_mode_operate_code = 0;
  qcmd.instruction_code = QSPI2_LCD_OPCODE_READ_CMD;
  qcmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  qcmd.address_code = addr24;
  qcmd.address_length = QSPI_CMD_ADRLEN_3_BYTE;
  qcmd.data_counter = len;
  qcmd.second_dummy_cycle_num = dummy_cycles;
  qcmd.operation_mode = QSPI_OPERATE_MODE_111;
  qcmd.read_status_config = QSPI_RSTSC_HW_AUTO;
  qcmd.read_status_enable = FALSE;
  qcmd.write_data_enable = FALSE;

  qspi_flag_clear(QSPI2, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI2, &qcmd);
  qspi2_lcd_bus_diag_capture(&qspi2_lcd_bus_diag_after_kick);

  for(i = 0; i < len; i++)
  {
    if(!qspi2_wait_rxfifo_ready(QSPI2_LCD_BUS_TIMEOUT))
    {
      qspi2_lcd_bus_diag_capture(&qspi2_lcd_bus_diag_before_exit);
      return 0;
    }
    data[i] = qspi_byte_read(QSPI2);
  }

  if(!qspi2_wait_cmd_done(QSPI2_LCD_BUS_TIMEOUT))
  {
    qspi2_lcd_bus_diag_capture(&qspi2_lcd_bus_diag_before_exit);
    return 0;
  }

  qspi2_lcd_bus_diag_capture(&qspi2_lcd_bus_diag_before_exit);
  return 1;
}

int qspi2_lcd_bus_write_pixels_rgb565(const uint16_t *pixels, uint32_t pixel_count)
{
  qspi_cmd_type qcmd;
  uint32_t i;
  uint32_t addr;

  if((pixels == 0) && (pixel_count != 0U))
  {
    return 0;
  }

  /* NV3041A RAM write: use 32h with command/address on IO0 and pixel data on 4 lanes. */
  addr = ((uint32_t)0x2CU) << 8;

  qcmd.pe_mode_enable = FALSE;
  qcmd.pe_mode_operate_code = 0;
  qcmd.instruction_code = QSPI2_LCD_OPCODE_WRITE_RAM32;
  qcmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  qcmd.address_code = addr;
  qcmd.address_length = QSPI_CMD_ADRLEN_3_BYTE;
  qcmd.data_counter = pixel_count * 2U;
  qcmd.second_dummy_cycle_num = 0;
  qcmd.operation_mode = QSPI_OPERATE_MODE_114;
  qcmd.read_status_config = QSPI_RSTSC_SW_ONCE;
  qcmd.read_status_enable = FALSE;
  qcmd.write_data_enable = (pixel_count != 0U) ? TRUE : FALSE;

  qspi_flag_clear(QSPI2, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI2, &qcmd);

  for(i = 0; i < pixel_count; i++)
  {
    uint16_t px = pixels[i];
    uint8_t hi = (uint8_t)(px >> 8);
    uint8_t lo = (uint8_t)(px & 0xFFU);

    if(!qspi2_wait_txfifo_ready(QSPI2_LCD_BUS_TIMEOUT))
    {
      return 0;
    }
    qspi_byte_write(QSPI2, hi);

    if(!qspi2_wait_txfifo_ready(QSPI2_LCD_BUS_TIMEOUT))
    {
      return 0;
    }
    qspi_byte_write(QSPI2, lo);
  }

  if(!qspi2_wait_cmd_done(QSPI2_LCD_BUS_TIMEOUT))
  {
    return 0;
  }

  return 1;
}
