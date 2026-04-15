#ifndef __EDGETX_LCD_COMPAT_H
#define __EDGETX_LCD_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Local compatibility types used until the driver is wired to the real
   EdgeTX display interfaces. Replace this header with the upstream EdgeTX
   definitions during integration. */
typedef enum {
  BMP_FORMAT_RGB565 = 0,
  BMP_FORMAT_ARGB32,
  BMP_FORMAT_LCD1,
  BMP_FORMAT_LCD4,
  BMP_FORMAT_LCD8
} BitmapFormat;

typedef int16_t coord_t;

typedef struct BitmapBuffer {
  uint16_t width;
  uint16_t height;
  uint16_t format;
  uint8_t *data;
} BitmapBuffer;

typedef uint32_t LcdFlags;

#ifdef __cplusplus
}
#endif

#endif
