#include "aps6404l.h"
#include "at32f435_437_edma.h"
#include "at32f435_437_qspi.h"

#define APS6404L_HW_TIMEOUT 2000000U

static uint8_t aps6404l_edma_inited = 0;

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

static void aps_edma_clear_stream1_flags(void)
{
  edma_flag_clear(EDMA_FERR1_FLAG);
  edma_flag_clear(EDMA_DMERR1_FLAG);
  edma_flag_clear(EDMA_DTERR1_FLAG);
  edma_flag_clear(EDMA_HDT1_FLAG);
  edma_flag_clear(EDMA_FDT1_FLAG);
}

static void aps_edma_init_once(void)
{
  if(aps6404l_edma_inited)
  {
    return;
  }

  edmamux_enable(TRUE);
  edmamux_init(EDMAMUX_CHANNEL1, EDMAMUX_DMAREQ_ID_QSPI1);
  edma_stream_enable(EDMA_STREAM1, FALSE);
  aps_edma_clear_stream1_flags();

  aps6404l_edma_inited = 1;
}

static void aps_edma_config_stream1(edma_dir_type direction)
{
  edma_init_type edma_init_struct;

  edma_stream_enable(EDMA_STREAM1, FALSE);
  edma_reset(EDMA_STREAM1);

  edma_default_para_init(&edma_init_struct);
  edma_init_struct.direction = direction;
  edma_init_struct.peripheral_data_width = EDMA_PERIPHERAL_DATA_WIDTH_WORD;
  edma_init_struct.peripheral_inc_enable = FALSE;
  edma_init_struct.memory_data_width = EDMA_MEMORY_DATA_WIDTH_WORD;
  edma_init_struct.memory_inc_enable = TRUE;
  edma_init_struct.fifo_mode_enable = TRUE;
  edma_init_struct.fifo_threshold = EDMA_FIFO_THRESHOLD_HALF;
  edma_init_struct.peripheral_burst_mode = EDMA_PERIPHERAL_SINGLE;
  edma_init_struct.memory_burst_mode = EDMA_MEMORY_SINGLE;
  edma_init_struct.priority = EDMA_PRIORITY_HIGH;
  edma_init_struct.loop_mode_enable = FALSE;
  edma_init_struct.buffer_size = 0;
  edma_init_struct.peripheral_base_addr = (uint32_t)&QSPI1->dt;
  edma_init(EDMA_STREAM1, &edma_init_struct);

  aps_edma_clear_stream1_flags();
}

static int aps_edma_wait_fdt(uint32_t fdt_flag, uint32_t ferr_flag, uint32_t dmerr_flag, uint32_t dterr_flag, uint32_t timeout)
{
  while(edma_flag_get(fdt_flag) == RESET)
  {
    if(edma_flag_get(ferr_flag) != RESET)
    {
      return 0;
    }
    if(edma_flag_get(dmerr_flag) != RESET)
    {
      return 0;
    }
    if(edma_flag_get(dterr_flag) != RESET)
    {
      return 0;
    }
    if(timeout-- == 0)
    {
      return 0;
    }
  }
  return 1;
}

/**
  * @brief  Reset APS6404L via command-port (0x66, 0x99).
  * @note   Requires QSPI1 to be configured and wired to APS6404L.
  * @retval none.
  */
void PSRAM_Reset(void)
{
  qspi_cmd_type cmd;

  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
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

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  cmd.instruction_code = APS6404L_RESET_ENABLE;
  qspi_cmd_operation_kick(QSPI1, &cmd);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return;
  }

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  cmd.instruction_code = APS6404L_RESET;
  qspi_cmd_operation_kick(QSPI1, &cmd);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return;
  }

  aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
}

/**
  * @brief  Read APS6404L JEDEC ID via command-port (0x9F).
  * @note   Device returns 3 dummy bytes before actual ID bytes.
  * @param  pBuffer: output buffer for ID bytes.
  * @param  nLength: number of ID bytes to read.
  * @retval 1 on success, 0 on failure.
  */
int PSRAM_ReadID(unsigned char *pBuffer, int nLength)
{
  int i;
  qspi_cmd_type cmd;
  uint8_t value;
  volatile uint32_t delay;
  uint32_t total_len;
  uint32_t wait;

  if(nLength <= 0)
  {
    return 0;
  }

  total_len = (uint32_t)nLength + 3U;

  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
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
  cmd.read_status_config = QSPI_RSTSC_SW_ONCE;
  cmd.read_status_enable = TRUE;
  cmd.write_data_enable = FALSE;

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);

  if(!qspi_wait_cmd_flag(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  for(i = 0; i < (int)total_len; i++)
  {
    wait = 256;
    while(qspi_flag_get(QSPI1, QSPI_RXFIFORDY_FLAG) == RESET)
    {
      if(wait-- == 0)
      {
        break;
      }
    }
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
int PSRAM_QPI_Write(unsigned int address, const unsigned char *pBuffer, int nLength)
{
  int i;
  qspi_cmd_type cmd;

  if(nLength <= 0)
  {
    return 0;
  }

  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
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
      aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
      return 0;
    }
    qspi_byte_write(QSPI1, (uint8_t)pBuffer[i]);
  }

  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

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
int PSRAM_QPI_Read(unsigned int address, unsigned char *pBuffer, int nLength)
{
  int i;
  qspi_cmd_type cmd;
  unsigned int timeout;

  if(nLength <= 0)
  {
    return 0;
  }

  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
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

  for(i = 0; i < nLength; i++)
  {
    timeout = 200000U;
    while(qspi_flag_get(QSPI1, QSPI_RXFIFORDY_FLAG) == RESET)
    {
      if(timeout-- == 0U)
      {
        aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
        return 0;
      }
    }
    pBuffer[i] = (unsigned char)qspi_byte_read(QSPI1);
  }

  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  if(!aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT))
  {
    return 0;
  }

  return 1;
}

int PSRAM_QPI_FastRead(unsigned int address, unsigned char *pBuffer, int nLength)
{
  int i;
  qspi_cmd_type cmd;
  unsigned int timeout;

  if(nLength <= 0)
  {
    return 0;
  }

  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
    return 0;
  }

  cmd.pe_mode_enable = FALSE;
  cmd.pe_mode_operate_code = 0;
  cmd.instruction_code = APS6404L_QUAD_READ;
  cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  cmd.address_code = address;
  cmd.address_length = QSPI_CMD_ADRLEN_3_BYTE;
  cmd.data_counter = (uint32_t)nLength;
  cmd.second_dummy_cycle_num = APS6404L_FAST_READ_DUMMY_CYCLES;
  cmd.operation_mode = QSPI_OPERATE_MODE_444;
  cmd.read_status_config = QSPI_RSTSC_HW_AUTO;
  cmd.read_status_enable = FALSE;
  cmd.write_data_enable = FALSE;

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);

  for(i = 0; i < nLength; i++)
  {
    timeout = 200000U;
    while(qspi_flag_get(QSPI1, QSPI_RXFIFORDY_FLAG) == RESET)
    {
      if(timeout-- == 0U)
      {
        aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
        return 0;
      }
    }
    pBuffer[i] = (unsigned char)qspi_byte_read(QSPI1);
  }

  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  if(!aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT))
  {
    return 0;
  }

  return 1;
}

int PSRAM_QPI_FastWrite(unsigned int address, const unsigned char *pBuffer, int nLength)
{
  int i;
  qspi_cmd_type cmd;

  if(nLength <= 0)
  {
    return 0;
  }

  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
    return 0;
  }

  cmd.pe_mode_enable = FALSE;
  cmd.pe_mode_operate_code = 0;
  cmd.instruction_code = APS6404L_QUAD_WRITE;
  cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  cmd.address_code = address;
  cmd.address_length = QSPI_CMD_ADRLEN_3_BYTE;
  cmd.data_counter = (uint32_t)nLength;
  cmd.second_dummy_cycle_num = 0;
  cmd.operation_mode = QSPI_OPERATE_MODE_444;
  cmd.read_status_config = QSPI_RSTSC_HW_AUTO;
  cmd.read_status_enable = FALSE;
  cmd.write_data_enable = TRUE;

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);

  for(i = 0; i < nLength; i++)
  {
    if(!qspi_wait_txfifo_ready(QSPI1, APS6404L_HW_TIMEOUT))
    {
      aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
      return 0;
    }
    qspi_byte_write(QSPI1, (uint8_t)pBuffer[i]);
  }

  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
  return 1;
}

int PSRAM_EDMA_Write(unsigned int address, const unsigned char *pBuffer, int nLength)
{
  qspi_cmd_type qpi_cmd;
  qspi_cmd_type cmd;
  qspi_cmd_type exit_cmd;
  qspi_cmd_type exit_cmd2;
  uint32_t word_count;
  uint32_t word_bytes;
  uint32_t remain_bytes;

  if(nLength <= 0)
  {
    return 0;
  }

  if(((uint32_t)pBuffer & 3U) != 0U)
  {
    return PSRAM_QPI_Write(address, pBuffer, nLength);
  }

  if((uint32_t)nLength < 32U)
  {
    return PSRAM_QPI_Write(address, pBuffer, nLength);
  }

  word_count = (uint32_t)nLength >> 2;
  if(word_count < 8U)
  {
    return PSRAM_QPI_Write(address, pBuffer, nLength);
  }

  if(word_count > 0xFFFFU)
  {
    word_count = 0xFFFFU;
  }

  word_bytes = word_count << 2;
  remain_bytes = (uint32_t)nLength - word_bytes;

  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
    return 0;
  }

  aps_edma_init_once();
  aps_edma_config_stream1(EDMA_DIR_MEMORY_TO_PERIPHERAL);

  qspi_dma_enable(QSPI1, FALSE);

  qpi_cmd.pe_mode_enable = FALSE;
  qpi_cmd.pe_mode_operate_code = 0;
  qpi_cmd.instruction_code = APS6404L_ENTER_QUAD_MODE;
  qpi_cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  qpi_cmd.address_code = 0;
  qpi_cmd.address_length = QSPI_CMD_ADRLEN_0_BYTE;
  qpi_cmd.data_counter = 0;
  qpi_cmd.second_dummy_cycle_num = 0;
  qpi_cmd.operation_mode = QSPI_OPERATE_MODE_111;
  qpi_cmd.read_status_config = QSPI_RSTSC_SW_ONCE;
  qpi_cmd.read_status_enable = TRUE;
  qpi_cmd.write_data_enable = FALSE;

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &qpi_cmd);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  cmd.pe_mode_enable = FALSE;
  cmd.pe_mode_operate_code = 0;
  cmd.instruction_code = APS6404L_QUAD_WRITE;
  cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  cmd.address_code = address;
  cmd.address_length = QSPI_CMD_ADRLEN_3_BYTE;
  cmd.data_counter = word_bytes;
  cmd.second_dummy_cycle_num = 0;
  cmd.operation_mode = QSPI_OPERATE_MODE_444;
  cmd.read_status_config = QSPI_RSTSC_HW_AUTO;
  cmd.read_status_enable = FALSE;
  cmd.write_data_enable = TRUE;

  EDMA_STREAM1->dtcnt = (uint16_t)word_count;
  EDMA_STREAM1->paddr = (uint32_t)&QSPI1->dt;
  EDMA_STREAM1->m0addr = (uint32_t)pBuffer;

  qspi_dma_enable(QSPI1, TRUE);
  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);

  edma_stream_enable(EDMA_STREAM1, TRUE);
  if(!aps_edma_wait_fdt(EDMA_FDT1_FLAG, EDMA_FERR1_FLAG, EDMA_DMERR1_FLAG, EDMA_DTERR1_FLAG, APS6404L_HW_TIMEOUT))
  {
    edma_stream_enable(EDMA_STREAM1, FALSE);
    qspi_dma_enable(QSPI1, FALSE);
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  edma_stream_enable(EDMA_STREAM1, FALSE);
  aps_edma_clear_stream1_flags();

  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    qspi_dma_enable(QSPI1, FALSE);
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  qspi_dma_enable(QSPI1, FALSE);

  exit_cmd.pe_mode_enable = FALSE;
  exit_cmd.pe_mode_operate_code = 0;
  exit_cmd.instruction_code = APS6404L_EXIT_QUAD_MODE;
  exit_cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  exit_cmd.address_code = 0;
  exit_cmd.address_length = QSPI_CMD_ADRLEN_0_BYTE;
  exit_cmd.data_counter = 0;
  exit_cmd.second_dummy_cycle_num = 0;
  exit_cmd.operation_mode = QSPI_OPERATE_MODE_444;
  exit_cmd.read_status_config = QSPI_RSTSC_SW_ONCE;
  exit_cmd.read_status_enable = TRUE;
  exit_cmd.write_data_enable = FALSE;

  exit_cmd2 = exit_cmd;
  exit_cmd2.operation_mode = QSPI_OPERATE_MODE_111;

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &exit_cmd);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &exit_cmd2);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);

  if(remain_bytes != 0U)
  {
    return PSRAM_QPI_Write(address + word_bytes, pBuffer + word_bytes, (int)remain_bytes);
  }

  return 1;
}

int PSRAM_EDMA_Read(unsigned int address, unsigned char *pBuffer, int nLength)
{
  qspi_cmd_type qpi_cmd;
  qspi_cmd_type cmd;
  qspi_cmd_type exit_cmd;
  qspi_cmd_type exit_cmd2;
  uint32_t word_count;
  uint32_t word_bytes;
  uint32_t remain_bytes;

  if(nLength <= 0)
  {
    return 0;
  }

  if(((uint32_t)pBuffer & 3U) != 0U)
  {
    return PSRAM_QPI_Read(address, pBuffer, nLength);
  }

  if((uint32_t)nLength < 32U)
  {
    return PSRAM_QPI_Read(address, pBuffer, nLength);
  }

  word_count = (uint32_t)nLength >> 2;
  if(word_count < 8U)
  {
    return PSRAM_QPI_Read(address, pBuffer, nLength);
  }

  if(word_count > 0xFFFFU)
  {
    word_count = 0xFFFFU;
  }

  word_bytes = word_count << 2;
  remain_bytes = (uint32_t)nLength - word_bytes;

  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
    return 0;
  }

  aps_edma_init_once();
  aps_edma_config_stream1(EDMA_DIR_PERIPHERAL_TO_MEMORY);

  qspi_dma_enable(QSPI1, FALSE);

  qpi_cmd.pe_mode_enable = FALSE;
  qpi_cmd.pe_mode_operate_code = 0;
  qpi_cmd.instruction_code = APS6404L_ENTER_QUAD_MODE;
  qpi_cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  qpi_cmd.address_code = 0;
  qpi_cmd.address_length = QSPI_CMD_ADRLEN_0_BYTE;
  qpi_cmd.data_counter = 0;
  qpi_cmd.second_dummy_cycle_num = 0;
  qpi_cmd.operation_mode = QSPI_OPERATE_MODE_111;
  qpi_cmd.read_status_config = QSPI_RSTSC_SW_ONCE;
  qpi_cmd.read_status_enable = TRUE;
  qpi_cmd.write_data_enable = FALSE;

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &qpi_cmd);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  cmd.pe_mode_enable = FALSE;
  cmd.pe_mode_operate_code = 0;
  cmd.instruction_code = APS6404L_QUAD_READ;
  cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  cmd.address_code = address;
  cmd.address_length = QSPI_CMD_ADRLEN_3_BYTE;
  cmd.data_counter = word_bytes;
  cmd.second_dummy_cycle_num = APS6404L_FAST_READ_DUMMY_CYCLES;
  cmd.operation_mode = QSPI_OPERATE_MODE_444;
  cmd.read_status_config = QSPI_RSTSC_HW_AUTO;
  cmd.read_status_enable = FALSE;
  cmd.write_data_enable = FALSE;

  EDMA_STREAM1->dtcnt = (uint16_t)word_count;
  EDMA_STREAM1->paddr = (uint32_t)&QSPI1->dt;
  EDMA_STREAM1->m0addr = (uint32_t)pBuffer;

  qspi_dma_enable(QSPI1, TRUE);
  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);

  edma_stream_enable(EDMA_STREAM1, TRUE);
  if(!aps_edma_wait_fdt(EDMA_FDT1_FLAG, EDMA_FERR1_FLAG, EDMA_DMERR1_FLAG, EDMA_DTERR1_FLAG, APS6404L_HW_TIMEOUT))
  {
    edma_stream_enable(EDMA_STREAM1, FALSE);
    qspi_dma_enable(QSPI1, FALSE);
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  edma_stream_enable(EDMA_STREAM1, FALSE);
  aps_edma_clear_stream1_flags();

  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    qspi_dma_enable(QSPI1, FALSE);
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  qspi_dma_enable(QSPI1, FALSE);

  exit_cmd.pe_mode_enable = FALSE;
  exit_cmd.pe_mode_operate_code = 0;
  exit_cmd.instruction_code = APS6404L_EXIT_QUAD_MODE;
  exit_cmd.instruction_length = QSPI_CMD_INSLEN_1_BYTE;
  exit_cmd.address_code = 0;
  exit_cmd.address_length = QSPI_CMD_ADRLEN_0_BYTE;
  exit_cmd.data_counter = 0;
  exit_cmd.second_dummy_cycle_num = 0;
  exit_cmd.operation_mode = QSPI_OPERATE_MODE_444;
  exit_cmd.read_status_config = QSPI_RSTSC_SW_ONCE;
  exit_cmd.read_status_enable = TRUE;
  exit_cmd.write_data_enable = FALSE;

  exit_cmd2 = exit_cmd;
  exit_cmd2.operation_mode = QSPI_OPERATE_MODE_111;

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &exit_cmd);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &exit_cmd2);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);

  if(remain_bytes != 0U)
  {
    return PSRAM_QPI_Read(address + word_bytes, pBuffer + word_bytes, (int)remain_bytes);
  }

  return 1;
}

/**
  * @brief  Enter QPI/quad mode (0x35).
  * @retval 1 on success, 0 on failure.
  */
int PSRAM_EnterQuadMode(void)
{
  qspi_cmd_type cmd;

  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
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
  cmd.read_status_config = QSPI_RSTSC_SW_ONCE;
  cmd.read_status_enable = TRUE;
  cmd.write_data_enable = FALSE;

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
  return 1;
}

/**
  * @brief  Exit QPI/quad mode (0xF5).
  * @retval 1 on success, 0 on failure.
  */
int PSRAM_ExitQuadMode(void)
{
  qspi_cmd_type cmd;
  qspi_cmd_type cmd2;

  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
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
  cmd.operation_mode = QSPI_OPERATE_MODE_444;
  cmd.read_status_config = QSPI_RSTSC_SW_ONCE;
  cmd.read_status_enable = TRUE;
  cmd.write_data_enable = FALSE;

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  cmd2 = cmd;
  cmd2.operation_mode = QSPI_OPERATE_MODE_111;
  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd2);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
  return 1;
}

/**
  * @brief  Toggle wrap boundary mode (0xC0).
  * @retval 1 on success, 0 on failure.
  */
int PSRAM_ToggleWrapBoundary(void)
{
  qspi_cmd_type cmd;

  if(!aps_qspi_xip_enable(FALSE, APS6404L_HW_TIMEOUT))
  {
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
  cmd.read_status_config = QSPI_RSTSC_SW_ONCE;
  cmd.read_status_enable = TRUE;
  cmd.write_data_enable = FALSE;

  qspi_flag_clear(QSPI1, QSPI_CMDSTS_FLAG);
  qspi_cmd_operation_kick(QSPI1, &cmd);
  if(!qspi_wait_cmd_done(QSPI1, APS6404L_HW_TIMEOUT))
  {
    aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
    return 0;
  }

  aps_qspi_xip_enable(TRUE, APS6404L_HW_TIMEOUT);
  return 1;
}
