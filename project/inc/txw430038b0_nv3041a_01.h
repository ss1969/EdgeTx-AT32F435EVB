#ifndef __TXW430038B0_NV3041A_01_H
#define __TXW430038B0_NV3041A_01_H

#ifdef __cplusplus
extern "C" {
#endif

#include "at32f435_437_wk_config.h"

#define TXW430038B0_WIDTH 480U
#define TXW430038B0_HEIGHT 272U

const char *txw430038b0_nv3041a_01_name(void);
int txw430038b0_nv3041a_01_init(void);
int txw430038b0_nv3041a_01_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
int txw430038b0_nv3041a_01_write_pixels_rgb565(const uint16_t *pixels, uint32_t pixel_count);

#ifdef __cplusplus
}
#endif

#endif
