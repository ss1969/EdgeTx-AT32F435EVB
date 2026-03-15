#ifndef __UART3_PRINTF_H
#define __UART3_PRINTF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void UART3_Write(const char *str);
void UART3_Printf(const char *fmt, ...);
void UART3_PrintHexByte(uint8_t value);
void UART3_PrintHexU32(uint32_t value);

#ifdef __cplusplus
}
#endif

#endif
