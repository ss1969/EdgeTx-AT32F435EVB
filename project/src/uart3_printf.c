#include "uart3_printf.h"

#include <stdarg.h>
#include <stdio.h>

#include "at32f435_437_usart.h"

static void uart3_putc(char ch)
{
  while(usart_flag_get(USART3, USART_TDBE_FLAG) == RESET)
  {
  }
  usart_data_transmit(USART3, (uint16_t)ch);
}

void UART3_Write(const char *str)
{
  if(str == 0)
  {
    return;
  }

  while(*str)
  {
    uart3_putc(*str++);
  }

  while(usart_flag_get(USART3, USART_TDC_FLAG) == RESET)
  {
  }
}

void UART3_Printf(const char *fmt, ...)
{
  char buf[256];
  va_list args;
  int n;

  if(fmt == 0)
  {
    return;
  }

  va_start(args, fmt);
  n = vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if(n <= 0)
  {
    return;
  }

  UART3_Write(buf);
}

void UART3_PrintHexByte(uint8_t value)
{
  static const char hex[] = "0123456789ABCDEF";
  uart3_putc(' ');
  uart3_putc(hex[value >> 4]);
  uart3_putc(hex[value & 0x0F]);
}

void UART3_PrintHexU32(uint32_t value)
{
  UART3_PrintHexByte((uint8_t)(value >> 24));
  UART3_PrintHexByte((uint8_t)(value >> 16));
  UART3_PrintHexByte((uint8_t)(value >> 8));
  UART3_PrintHexByte((uint8_t)(value >> 0));
}

