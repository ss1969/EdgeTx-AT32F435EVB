#include "txw430038b0_nv3041a_01.h"

#include "qspi2_lcd_bus.h"
#include "wk_system.h"

const char *txw430038b0_nv3041a_01_name(void)
{
  return "TXW430038B0";
}

static int dcs_write_0(uint8_t cmd)
{
  return qspi2_lcd_bus_write(cmd, 0, 0);
}

static int dcs_write_1(uint8_t cmd, uint8_t v)
{
  return qspi2_lcd_bus_write(cmd, &v, 1);
}

static int dcs_write_n(uint8_t cmd, const uint8_t *data, uint32_t len)
{
  return qspi2_lcd_bus_write(cmd, data, len);
}

typedef struct
{
  uint8_t cmd;
  const uint8_t *data;
  uint8_t data_bytes;
  uint16_t delay_ms;
} nv3041a01_init_cmd_t;

static const uint8_t nv3041a01_cmd_ff_a5[] = {0xA5};
static const uint8_t nv3041a01_cmd_e7_10[] = {0x10};
static const uint8_t nv3041a01_cmd_35_00[] = {0x00};
static const uint8_t nv3041a01_cmd_36_c0[] = {0xC0};
static const uint8_t nv3041a01_cmd_3a_01[] = {0x01};
static const uint8_t nv3041a01_cmd_40_01[] = {0x01};
static const uint8_t nv3041a01_cmd_41_03[] = {0x03};
static const uint8_t nv3041a01_cmd_44_15[] = {0x15};
static const uint8_t nv3041a01_cmd_45_15[] = {0x15};
static const uint8_t nv3041a01_cmd_7d_03[] = {0x03};
static const uint8_t nv3041a01_cmd_c1_bb[] = {0xBB};
static const uint8_t nv3041a01_cmd_c2_05[] = {0x05};
static const uint8_t nv3041a01_cmd_c3_10[] = {0x10};
static const uint8_t nv3041a01_cmd_c6_3e[] = {0x3E};
static const uint8_t nv3041a01_cmd_c7_25[] = {0x25};
static const uint8_t nv3041a01_cmd_c8_11[] = {0x11};
static const uint8_t nv3041a01_cmd_7a_5f[] = {0x5F};
static const uint8_t nv3041a01_cmd_6f_44[] = {0x44};
static const uint8_t nv3041a01_cmd_78_70[] = {0x70};
static const uint8_t nv3041a01_cmd_c9_00[] = {0x00};
static const uint8_t nv3041a01_cmd_67_21[] = {0x21};
static const uint8_t nv3041a01_cmd_51_0a[] = {0x0A};
static const uint8_t nv3041a01_cmd_52_76[] = {0x76};
static const uint8_t nv3041a01_cmd_53_0a[] = {0x0A};
static const uint8_t nv3041a01_cmd_54_76[] = {0x76};
static const uint8_t nv3041a01_cmd_46_0a[] = {0x0A};
static const uint8_t nv3041a01_cmd_47_2a[] = {0x2A};
static const uint8_t nv3041a01_cmd_48_0a[] = {0x0A};
static const uint8_t nv3041a01_cmd_49_1a[] = {0x1A};
static const uint8_t nv3041a01_cmd_56_43[] = {0x43};
static const uint8_t nv3041a01_cmd_57_42[] = {0x42};
static const uint8_t nv3041a01_cmd_58_3c[] = {0x3C};
static const uint8_t nv3041a01_cmd_59_64[] = {0x64};
static const uint8_t nv3041a01_cmd_5a_41[] = {0x41};
static const uint8_t nv3041a01_cmd_5b_3c[] = {0x3C};
static const uint8_t nv3041a01_cmd_5c_02[] = {0x02};
static const uint8_t nv3041a01_cmd_5d_3c[] = {0x3C};
static const uint8_t nv3041a01_cmd_5e_1f[] = {0x1F};
static const uint8_t nv3041a01_cmd_60_80[] = {0x80};
static const uint8_t nv3041a01_cmd_61_3f[] = {0x3F};
static const uint8_t nv3041a01_cmd_62_21[] = {0x21};
static const uint8_t nv3041a01_cmd_63_07[] = {0x07};
static const uint8_t nv3041a01_cmd_64_e0[] = {0xE0};
static const uint8_t nv3041a01_cmd_65_02[] = {0x02};
static const uint8_t nv3041a01_cmd_ca_20[] = {0x20};
static const uint8_t nv3041a01_cmd_cb_52[] = {0x52};
static const uint8_t nv3041a01_cmd_cc_10[] = {0x10};
static const uint8_t nv3041a01_cmd_cd_42[] = {0x42};
static const uint8_t nv3041a01_cmd_d0_20[] = {0x20};
static const uint8_t nv3041a01_cmd_d1_52[] = {0x52};
static const uint8_t nv3041a01_cmd_d2_10[] = {0x10};
static const uint8_t nv3041a01_cmd_d3_42[] = {0x42};
static const uint8_t nv3041a01_cmd_d4_0a[] = {0x0A};
static const uint8_t nv3041a01_cmd_d5_32[] = {0x32};
static const uint8_t nv3041a01_cmd_f8_03[] = {0x03};
static const uint8_t nv3041a01_cmd_f9_20[] = {0x20};

static const uint8_t nv3041a01_cmd_80_00[] = {0x00};
static const uint8_t nv3041a01_cmd_a0_00[] = {0x00};
static const uint8_t nv3041a01_cmd_81_07[] = {0x07};
static const uint8_t nv3041a01_cmd_a1_06[] = {0x06};
static const uint8_t nv3041a01_cmd_82_02[] = {0x02};
static const uint8_t nv3041a01_cmd_a2_01[] = {0x01};
static const uint8_t nv3041a01_cmd_86_11[] = {0x11};
static const uint8_t nv3041a01_cmd_a6_10[] = {0x10};
static const uint8_t nv3041a01_cmd_87_27[] = {0x27};
static const uint8_t nv3041a01_cmd_a7_27[] = {0x27};
static const uint8_t nv3041a01_cmd_83_37[] = {0x37};
static const uint8_t nv3041a01_cmd_a3_37[] = {0x37};
static const uint8_t nv3041a01_cmd_84_35[] = {0x35};
static const uint8_t nv3041a01_cmd_a4_35[] = {0x35};
static const uint8_t nv3041a01_cmd_85_3f[] = {0x3F};
static const uint8_t nv3041a01_cmd_a5_3f[] = {0x3F};
static const uint8_t nv3041a01_cmd_88_0b[] = {0x0B};
static const uint8_t nv3041a01_cmd_a8_0b[] = {0x0B};
static const uint8_t nv3041a01_cmd_89_14[] = {0x14};
static const uint8_t nv3041a01_cmd_a9_14[] = {0x14};
static const uint8_t nv3041a01_cmd_8a_1a[] = {0x1A};
static const uint8_t nv3041a01_cmd_aa_1a[] = {0x1A};
static const uint8_t nv3041a01_cmd_8b_0a[] = {0x0A};
static const uint8_t nv3041a01_cmd_ab_0a[] = {0x0A};
static const uint8_t nv3041a01_cmd_8c_14[] = {0x14};
static const uint8_t nv3041a01_cmd_ac_08[] = {0x08};
static const uint8_t nv3041a01_cmd_8d_17[] = {0x17};
static const uint8_t nv3041a01_cmd_ad_07[] = {0x07};
static const uint8_t nv3041a01_cmd_8e_16[] = {0x16};
static const uint8_t nv3041a01_cmd_ae_06[] = {0x06};
static const uint8_t nv3041a01_cmd_8f_1b[] = {0x1B};
static const uint8_t nv3041a01_cmd_af_07[] = {0x07};
static const uint8_t nv3041a01_cmd_90_04[] = {0x04};
static const uint8_t nv3041a01_cmd_b0_04[] = {0x04};
static const uint8_t nv3041a01_cmd_91_0a[] = {0x0A};
static const uint8_t nv3041a01_cmd_b1_0a[] = {0x0A};
static const uint8_t nv3041a01_cmd_92_16[] = {0x16};
static const uint8_t nv3041a01_cmd_b2_15[] = {0x15};

static const uint8_t nv3041a01_cmd_ff_00[] = {0x00};
static const uint8_t nv3041a01_cmd_11_00[] = {0x00};
static const uint8_t nv3041a01_cmd_29_00[] = {0x00};

static const nv3041a01_init_cmd_t nv3041a01_init_cmds[] = {
  {0xFF, nv3041a01_cmd_ff_a5, 1, 0},
  {0xE7, nv3041a01_cmd_e7_10, 1, 0},
  {0x35, nv3041a01_cmd_35_00, 1, 0},
  {0x36, nv3041a01_cmd_36_c0, 1, 0},
  {0x3A, nv3041a01_cmd_3a_01, 1, 0},
  {0x40, nv3041a01_cmd_40_01, 1, 0},
  {0x41, nv3041a01_cmd_41_03, 1, 0},
  {0x44, nv3041a01_cmd_44_15, 1, 0},
  {0x45, nv3041a01_cmd_45_15, 1, 0},
  {0x7D, nv3041a01_cmd_7d_03, 1, 0},
  {0xC1, nv3041a01_cmd_c1_bb, 1, 0},
  {0xC2, nv3041a01_cmd_c2_05, 1, 0},
  {0xC3, nv3041a01_cmd_c3_10, 1, 0},
  {0xC6, nv3041a01_cmd_c6_3e, 1, 0},
  {0xC7, nv3041a01_cmd_c7_25, 1, 0},
  {0xC8, nv3041a01_cmd_c8_11, 1, 0},
  {0x7A, nv3041a01_cmd_7a_5f, 1, 0},
  {0x6F, nv3041a01_cmd_6f_44, 1, 0},
  {0x78, nv3041a01_cmd_78_70, 1, 0},
  {0xC9, nv3041a01_cmd_c9_00, 1, 0},
  {0x67, nv3041a01_cmd_67_21, 1, 0},
  {0x51, nv3041a01_cmd_51_0a, 1, 0},
  {0x52, nv3041a01_cmd_52_76, 1, 0},
  {0x53, nv3041a01_cmd_53_0a, 1, 0},
  {0x54, nv3041a01_cmd_54_76, 1, 0},
  {0x46, nv3041a01_cmd_46_0a, 1, 0},
  {0x47, nv3041a01_cmd_47_2a, 1, 0},
  {0x48, nv3041a01_cmd_48_0a, 1, 0},
  {0x49, nv3041a01_cmd_49_1a, 1, 0},
  {0x56, nv3041a01_cmd_56_43, 1, 0},
  {0x57, nv3041a01_cmd_57_42, 1, 0},
  {0x58, nv3041a01_cmd_58_3c, 1, 0},
  {0x59, nv3041a01_cmd_59_64, 1, 0},
  {0x5A, nv3041a01_cmd_5a_41, 1, 0},
  {0x5B, nv3041a01_cmd_5b_3c, 1, 0},
  {0x5C, nv3041a01_cmd_5c_02, 1, 0},
  {0x5D, nv3041a01_cmd_5d_3c, 1, 0},
  {0x5E, nv3041a01_cmd_5e_1f, 1, 0},
  {0x60, nv3041a01_cmd_60_80, 1, 0},
  {0x61, nv3041a01_cmd_61_3f, 1, 0},
  {0x62, nv3041a01_cmd_62_21, 1, 0},
  {0x63, nv3041a01_cmd_63_07, 1, 0},
  {0x64, nv3041a01_cmd_64_e0, 1, 0},
  {0x65, nv3041a01_cmd_65_02, 1, 0},
  {0xCA, nv3041a01_cmd_ca_20, 1, 0},
  {0xCB, nv3041a01_cmd_cb_52, 1, 0},
  {0xCC, nv3041a01_cmd_cc_10, 1, 0},
  {0xCD, nv3041a01_cmd_cd_42, 1, 0},
  {0xD0, nv3041a01_cmd_d0_20, 1, 0},
  {0xD1, nv3041a01_cmd_d1_52, 1, 0},
  {0xD2, nv3041a01_cmd_d2_10, 1, 0},
  {0xD3, nv3041a01_cmd_d3_42, 1, 0},
  {0xD4, nv3041a01_cmd_d4_0a, 1, 0},
  {0xD5, nv3041a01_cmd_d5_32, 1, 0},

  {0xF8, nv3041a01_cmd_f8_03, 1, 0},
  {0xF9, nv3041a01_cmd_f9_20, 1, 0},

  {0x80, nv3041a01_cmd_80_00, 1, 0},
  {0xA0, nv3041a01_cmd_a0_00, 1, 0},
  {0x81, nv3041a01_cmd_81_07, 1, 0},
  {0xA1, nv3041a01_cmd_a1_06, 1, 0},
  {0x82, nv3041a01_cmd_82_02, 1, 0},
  {0xA2, nv3041a01_cmd_a2_01, 1, 0},
  {0x86, nv3041a01_cmd_86_11, 1, 0},
  {0xA6, nv3041a01_cmd_a6_10, 1, 0},
  {0x87, nv3041a01_cmd_87_27, 1, 0},
  {0xA7, nv3041a01_cmd_a7_27, 1, 0},
  {0x83, nv3041a01_cmd_83_37, 1, 0},
  {0xA3, nv3041a01_cmd_a3_37, 1, 0},
  {0x84, nv3041a01_cmd_84_35, 1, 0},
  {0xA4, nv3041a01_cmd_a4_35, 1, 0},
  {0x85, nv3041a01_cmd_85_3f, 1, 0},
  {0xA5, nv3041a01_cmd_a5_3f, 1, 0},
  {0x88, nv3041a01_cmd_88_0b, 1, 0},
  {0xA8, nv3041a01_cmd_a8_0b, 1, 0},
  {0x89, nv3041a01_cmd_89_14, 1, 0},
  {0xA9, nv3041a01_cmd_a9_14, 1, 0},
  {0x8A, nv3041a01_cmd_8a_1a, 1, 0},
  {0xAA, nv3041a01_cmd_aa_1a, 1, 0},
  {0x8B, nv3041a01_cmd_8b_0a, 1, 0},
  {0xAB, nv3041a01_cmd_ab_0a, 1, 0},
  {0x8C, nv3041a01_cmd_8c_14, 1, 0},
  {0xAC, nv3041a01_cmd_ac_08, 1, 0},
  {0x8D, nv3041a01_cmd_8d_17, 1, 0},
  {0xAD, nv3041a01_cmd_ad_07, 1, 0},
  {0x8E, nv3041a01_cmd_8e_16, 1, 0},
  {0xAE, nv3041a01_cmd_ae_06, 1, 0},
  {0x8F, nv3041a01_cmd_8f_1b, 1, 0},
  {0xAF, nv3041a01_cmd_af_07, 1, 0},
  {0x90, nv3041a01_cmd_90_04, 1, 0},
  {0xB0, nv3041a01_cmd_b0_04, 1, 0},
  {0x91, nv3041a01_cmd_91_0a, 1, 0},
  {0xB1, nv3041a01_cmd_b1_0a, 1, 0},
  {0x92, nv3041a01_cmd_92_16, 1, 0},
  {0xB2, nv3041a01_cmd_b2_15, 1, 0},

  {0xFF, nv3041a01_cmd_ff_00, 1, 0},
  {0x11, nv3041a01_cmd_11_00, 1, 700},
  {0x29, nv3041a01_cmd_29_00, 1, 100},
};

int txw430038b0_nv3041a_01_init(void)
{
  uint32_t i;

  qspi2_lcd_bus_reset_pulse(120, 120);

  for(i = 0; i < (uint32_t)(sizeof(nv3041a01_init_cmds) / sizeof(nv3041a01_init_cmds[0])); i++)
  {
    const nv3041a01_init_cmd_t *c = &nv3041a01_init_cmds[i];
    if(!dcs_write_n(c->cmd, c->data, c->data_bytes))
    {
      return 0;
    }
    if(c->delay_ms != 0U)
    {
      wk_delay_ms(c->delay_ms);
    }
  }

  return txw430038b0_nv3041a_01_set_window(0, 0, (uint16_t)(TXW430038B0_WIDTH - 1U), (uint16_t)(TXW430038B0_HEIGHT - 1U));
}

int txw430038b0_nv3041a_01_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  uint8_t col[4];
  uint8_t row[4];

  if((x1 < x0) || (y1 < y0))
  {
    return 0;
  }
  if((x1 >= TXW430038B0_WIDTH) || (y1 >= TXW430038B0_HEIGHT))
  {
    return 0;
  }

  col[0] = (uint8_t)(x0 >> 8);
  col[1] = (uint8_t)(x0 & 0xFFU);
  col[2] = (uint8_t)(x1 >> 8);
  col[3] = (uint8_t)(x1 & 0xFFU);

  row[0] = (uint8_t)(y0 >> 8);
  row[1] = (uint8_t)(y0 & 0xFFU);
  row[2] = (uint8_t)(y1 >> 8);
  row[3] = (uint8_t)(y1 & 0xFFU);

  if(!dcs_write_n(0x2A, col, sizeof(col)))
  {
    return 0;
  }
  if(!dcs_write_n(0x2B, row, sizeof(row)))
  {
    return 0;
  }
  return 1;
}

int txw430038b0_nv3041a_01_write_pixels_rgb565(const uint16_t *pixels, uint32_t pixel_count)
{
  return qspi2_lcd_bus_write_pixels_rgb565(0x2C, pixels, pixel_count);
}
