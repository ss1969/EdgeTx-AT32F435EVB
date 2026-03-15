#include "aps6404l.h"
#include "at32f435_437_qspi.h"

#define APS6404L_HW_TIMEOUT 2000000U

static volatile uint32_t aps6404l_hw_stage = 0;

/**
  * @brief  Get last stage/error code for APS6404L hardware driver.
  * @note   Value is 0 on success. Non-zero values indicate the last failure point.
  * @retval Stage/error code.
  */
uint32_t APS6404LHwGetLastStage(void)
{
  return aps6404l_hw_stage;
}

/**
  * @brief  Wait for command completion flag without clearing it.
  * @param  qspi_x: QSPI peripheral instance.
  * @param  timeout: loop counter timeout.
  * @retval 1 on success, 0 on timeout.
  */
static int qspi_wait_cmd_flag(qspi_type *qspi_x, uint32_t timeout)
{
  while(qspi_flag_get(qspi_x, QSPI_CMDSTS_FLAG) == RESET)
  {
    if(timeout-- == 0)
    {
      return 0;
    }
  }
  return 1;
}

/**
  * @brief  Wait for command completion flag and clear it.
  * @param  qspi_x: QSPI peripheral instance.
  * @param  timeout: loop counter timeout.
  * @retval 1 on success, 0 on timeout.
  */
static int qspi_wait_cmd_done(qspi_type *qspi_x, uint32_t timeout)
{
  while(qspi_flag_get(qspi_x, QSPI_CMDSTS_FLAG) == RESET)
  {
    if(timeout-- == 0)
    {
      return 0;
    }
  }
  qspi_flag_clear(qspi_x, QSPI_CMDSTS_FLAG);
  return 1;
}

/**
  * @brief  Wait for TX FIFO ready flag.
  * @param  qspi_x: QSPI peripheral instance.
  * @param  timeout: loop counter timeout.
  * @retval 1 on success, 0 on timeout.
  */
static int qspi_wait_txfifo_ready(qspi_type *qspi_x, uint32_t timeout)
{
  while(qspi_flag_get(qspi_x, QSPI_TXFIFORDY_FLAG) == RESET)
  {
    if(timeout-- == 0)
    {
      return 0;
    }
  }
  return 1;
}

/**
  * @brief  Switch QSPI1 between command-port mode and XIP (memory-mapped) mode.
  * @note   This function adds timeouts to avoid hard lockups during mode switching.
  * @param  new_state: TRUE enables XIP, FALSE enables command-port mode.
  * @param  timeout: loop counter timeout.
  * @retval 1 on success, 0 on timeout.
  */
static int aps_qspi_xip_enable(confirm_state new_state, uint32_t timeout)
{
  uint32_t dly;

  if(new_state == (confirm_state)(QSPI1->ctrl_bit.xipsel))
  {
    return 1;
  }

  while(QSPI1->fifosts_bit.txfifordy == 0)
  {
    if(timeout-- == 0)
    {
      return 0;
    }
  }

  dly = 64;
  while(dly--)
  {
    __NOP();
  }

  QSPI1->ctrl_bit.xiprcmdf = 1;

  while(QSPI1->ctrl_bit.abort)
  {
    if(timeout-- == 0)
    {
      return 0;
    }
  }

  dly = 64;
  while(dly--)
  {
    __NOP();
  }

  QSPI1->ctrl_bit.xipsel = new_state;

  while(QSPI1->ctrl_bit.abort)
  {
    if(timeout-- == 0)
    {
      return 0;
    }
  }

  if(new_state == TRUE)
  {
    while(QSPI1->xip_cmd_w3_bit.csts)
    {
      if(timeout-- == 0)
      {
        return 0;
      }
    }
  }

  return 1;
}

/**
  * @brief  Reset APS6404L via command-port (0x66, 0x99).
  * @note   Requires QSPI1 to be configured and wired to APS6404L.
  * @retval none.
  */
void APS6404LHwReset(void)
{
  qspi_cmd_type cmd;

  aps6404l_hw_stage = 1;
  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
    /* failed to switch to command-port mode */
    aps6404l_hw_stage = 0xEA;
    return;
  }

  cmd.pe_mode_enable = FALSE;
  cmd.pe_mode_operate_code = 0;
  cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  cmd.address_code = 0;
  cmd.address_length = QSPI_CMD_ADRLEN_0_BYTE;
  cmd.data_counter = 0;
  cmd.second_dummy_cycle_num = 0;
  cmd.operation_mode = QSPI_OPERATE_MODE_111;
  cmd.read_status_config = QSPI_RSTSC_SW_ONCE;
  cmd.read_status_enable = TRUE;
  cmd.write_data_enable = FALSE;

  aps6404l_hw_stage = 2;
  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  cmd.instruction_code = APS6404L_RESET_ENABLE;
  qspi_cmd_operation_kick(QSPI1, &cmd);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    /* reset-enable command did not complete */
    aps6404l_hw_stage = 0xE1;
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return;
  }

  aps6404l_hw_stage = 3;
  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  cmd.instruction_code = APS6404L_RESET;
  qspi_cmd_operation_kick(QSPI1, &cmd);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    /* reset command did not complete */
    aps6404l_hw_stage = 0xE2;
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return;
  }

  aps6404l_hw_stage = 0;
  aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
}

/**
  * @brief  Read APS6404L JEDEC ID via command-port (0x9F).
  * @note   Device returns 3 dummy bytes before actual ID bytes.
  * @param  pBuffer: output buffer for ID bytes.
  * @param  nLength: number of ID bytes to read.
  * @retval 1 on success, 0 on failure.
  */
int APS6404LHwReadId(unsigned char *pBuffer, int nLength)
{
  int i;
  qspi_cmd_type cmd;
  uint8_t value;
  volatile uint32_t delay;
  uint32_t total_len;

  if(nLength <= 0)
  {
    return 0;
  }

  total_len = (uint32_t)nLength + 3U;

  aps6404l_hw_stage = 10;
  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
    /* failed to switch to command-port mode */
    aps6404l_hw_stage = 0xEA;
    return 0;
  }

  cmd.pe_mode_enable = FALSE;
  cmd.pe_mode_operate_code = 0;
  cmd.instruction_code = APS6404L_READ_ID;
  cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  cmd.address_code = 0;
  cmd.address_length = QSPI_CMD_ADRLEN_0_BYTE;
  cmd.data_counter = total_len;
  cmd.second_dummy_cycle_num = 0;
  cmd.operation_mode = QSPI_OPERATE_MODE_111;
  cmd.read_status_config = QSPI_RSTSC_HW_AUTO;
  cmd.read_status_enable = FALSE;
  cmd.write_data_enable = FALSE;

  aps6404l_hw_stage = 11;
  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);

  aps6404l_hw_stage = 12;
  if(!qspi_wait_cmd_flag(QSPI1, APS6404L_HW_TIMEOUT))
  {
    /* command did not complete */
    aps6404l_hw_stage = 0xE3;
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  for(i = 0; i < (int)total_len; i++)
  {
    delay = 16;
    while(delay--)
    {
      __NOP();
    }
    value = qspi_byte_read(QSPI1);
    if((unsigned int)i >= 3U)
    {
      unsigned int out_index = (unsigned int)i - 3U;
      if(out_index < (unsigned int)nLength)
      {
        pBuffer[out_index] = (unsigned char)value;
      }
    }
  }

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);

  aps6404l_hw_stage = 0;
  aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
  return 1;
}

/**
  * @brief  Write bytes to APS6404L via command-port (0x02 + 24-bit address).
  * @note   For best performance and robustness, prefer memory-mapped access at APS6404L_MEM_BASE.
  * @param  address: 24-bit address.
  * @param  pBuffer: source buffer.
  * @param  nLength: number of bytes to write.
  * @retval 1 on success, 0 on failure.
  */
int APS6404LHwWriteBytes(unsigned int address, const unsigned char *pBuffer, int nLength)
{
  int i;
  qspi_cmd_type cmd;

  if(nLength <= 0)
  {
    return 0;
  }

  aps6404l_hw_stage = 20;
  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
    /* failed to switch to command-port mode */
    aps6404l_hw_stage = 0xEA;
    return 0;
  }

  cmd.pe_mode_enable = FALSE;
  cmd.pe_mode_operate_code = 0;
  cmd.instruction_code = APS6404L_WRITE;
  cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  cmd.address_code = address;
  cmd.address_length = QSPI_CMD_ADRLEN_3_BYTE;
  cmd.data_counter = (uint32_t)nLength;
  cmd.second_dummy_cycle_num = 0;
  cmd.operation_mode = QSPI_OPERATE_MODE_111;
  cmd.read_status_config = QSPI_RSTSC_HW_AUTO;
  cmd.read_status_enable = FALSE;
  cmd.write_data_enable = TRUE;

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);

  for(i = 0; i < nLength; i++)
  {
    if(!qspi_wait_txfifo_ready(QSPI1, APS6404L_HW_TIMEOUT))
    {
      /* TX FIFO did not become ready */
      aps6404l_hw_stage = 0xE6;
      aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
      return 0;
    }
    qspi_byte_write(QSPI1, (uint8_t)pBuffer[i]);
  }

  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    /* command did not complete */
    aps6404l_hw_stage = 0xE7;
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  aps6404l_hw_stage = 0;
  aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
  return 1;
}

/**
  * @brief  Read bytes from APS6404L via command-port (0x03 + 24-bit address).
  * @note   For best performance and robustness, prefer memory-mapped access at APS6404L_MEM_BASE.
  * @param  address: 24-bit address.
  * @param  pBuffer: destination buffer.
  * @param  nLength: number of bytes to read.
  * @retval 1 on success, 0 on failure.
  */
int APS6404LHwReadBytes(unsigned int address, unsigned char *pBuffer, int nLength)
{
  int i;
  qspi_cmd_type cmd;
  volatile unsigned int delay;
  unsigned int timeout;

  if(nLength <= 0)
  {
    return 0;
  }

  aps6404l_hw_stage = 30;
  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
    /* failed to switch to command-port mode */
    aps6404l_hw_stage = 0xEA;
    return 0;
  }

  cmd.pe_mode_enable = FALSE;
  cmd.pe_mode_operate_code = 0;
  cmd.instruction_code = APS6404L_READ;
  cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  cmd.address_code = address;
  cmd.address_length = QSPI_CMD_ADRLEN_3_BYTE;
  cmd.data_counter = (uint32_t)nLength;
  cmd.second_dummy_cycle_num = 0;
  cmd.operation_mode = QSPI_OPERATE_MODE_111;
  cmd.read_status_config = QSPI_RSTSC_HW_AUTO;
  cmd.read_status_enable = FALSE;
  cmd.write_data_enable = FALSE;

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);

  aps6404l_hw_stage = 32;
  timeout = APS6404L_HW_TIMEOUT;
  while(qspi_flag_get(QSPI1, QSPI_CMDSTS_FLAG) == RESET)
  {
    if(timeout-- == 0)
    {
      /* command did not complete */
      aps6404l_hw_stage = 0xE8;
      aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
      return 0;
    }
  }

  for(i = 0; i < nLength; i++)
  {
    delay = 16;
    while(delay--)
    {
      __NOP();
    }
    pBuffer[i] = (unsigned char)qspi_byte_read(QSPI1);
  }

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);

  if(!aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT))
  {
    /* failed to switch back to XIP mode */
    aps6404l_hw_stage = 0xEA;
    return 0;
  }

  aps6404l_hw_stage = 0;
  return 1;
}

/**
  * @brief  Enter QPI/quad mode (0x35).
  * @retval 1 on success, 0 on failure.
  */
int APS6404LHwEnterQuadMode(void)
{
  qspi_cmd_type cmd;

  aps6404l_hw_stage = 40;
  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
    aps6404l_hw_stage = 0xEA;
    return 0;
  }

  cmd.pe_mode_enable = FALSE;
  cmd.pe_mode_operate_code = 0;
  cmd.instruction_code = APS6404L_ENTER_QUAD_MODE;
  cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  cmd.address_code = 0;
  cmd.address_length = QSPI_CMD_ADRLEN_0_BYTE;
  cmd.data_counter = 0;
  cmd.second_dummy_cycle_num = 0;
  cmd.operation_mode = QSPI_OPERATE_MODE_111;
  cmd.read_status_config = QSPI_RSTSC_HW_AUTO;
  cmd.read_status_enable = FALSE;
  cmd.write_data_enable = FALSE;

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps6404l_hw_stage = 0xE9;
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  aps6404l_hw_stage = 0;
  aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
  return 1;
}

/**
  * @brief  Exit QPI/quad mode (0xF5).
  * @retval 1 on success, 0 on failure.
  */
int APS6404LHwExitQuadMode(void)
{
  qspi_cmd_type cmd;

  aps6404l_hw_stage = 41;
  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
    aps6404l_hw_stage = 0xEA;
    return 0;
  }

  cmd.pe_mode_enable = FALSE;
  cmd.pe_mode_operate_code = 0;
  cmd.instruction_code = APS6404L_EXIT_QUAD_MODE;
  cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  cmd.address_code = 0;
  cmd.address_length = QSPI_CMD_ADRLEN_0_BYTE;
  cmd.data_counter = 0;
  cmd.second_dummy_cycle_num = 0;
  cmd.operation_mode = QSPI_OPERATE_MODE_111;
  cmd.read_status_config = QSPI_RSTSC_HW_AUTO;
  cmd.read_status_enable = FALSE;
  cmd.write_data_enable = FALSE;

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps6404l_hw_stage = 0xE9;
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  aps6404l_hw_stage = 0;
  aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
  return 1;
}

/**
  * @brief  Toggle wrap boundary mode (0xC0).
  * @retval 1 on success, 0 on failure.
  */
int APS6404LHwWrapBoundaryToggle(void)
{
  qspi_cmd_type cmd;

  aps6404l_hw_stage = 42;
  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
    aps6404l_hw_stage = 0xEA;
    return 0;
  }

  cmd.pe_mode_enable = FALSE;
  cmd.pe_mode_operate_code = 0;
  cmd.instruction_code = APS6404L_WRAP_BOUNDARY_TOGGLE;
  cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  cmd.address_code = 0;
  cmd.address_length = QSPI_CMD_ADRLEN_0_BYTE;
  cmd.data_counter = 0;
  cmd.second_dummy_cycle_num = 0;
  cmd.operation_mode = QSPI_OPERATE_MODE_111;
  cmd.read_status_config = QSPI_RSTSC_HW_AUTO;
  cmd.read_status_enable = FALSE;
  cmd.write_data_enable = FALSE;

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps6404l_hw_stage = 0xE9;
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  aps6404l_hw_stage = 0;
  aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
  return 1;
}
