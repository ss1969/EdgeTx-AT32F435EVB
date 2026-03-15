/* add user code begin Header */
/**
  **************************************************************************
  * @file     aps6404l.h
  * @brief    APS6404L QSPI RAM driver header file
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

/* define to prevent recursive inclusion -----------------------------------*/
#ifndef __APS6404L_H
#define __APS6404L_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes -----------------------------------------------------------------------*/
#include <stdint.h>

/* private includes -------------------------------------------------------------*/
/* add user code begin private includes */

/* add user code end private includes */

/* exported types -------------------------------------------------------------*/
/* add user code begin exported types */

/* add user code end exported types */

/* exported constants --------------------------------------------------------*/
/* add user code begin exported constants */

/* APS6404L commands */
#define APS6404L_WRITE                    0x02
#define APS6404L_READ                     0x03
#define APS6404L_FAST_READ                0x0B
#define APS6404L_QUAD_READ                0xEB
#define APS6404L_QUAD_WRITE               0x38
#define APS6404L_ENTER_QUAD_MODE          0x35
#define APS6404L_EXIT_QUAD_MODE           0xF5
#define APS6404L_WRAP_BOUNDARY_TOGGLE     0xC0
#define APS6404L_READ_ID                  0x9F
#define APS6404L_RESET_ENABLE             0x66
#define APS6404L_RESET                    0x99

#define APS6404L_MEM_BASE                 ((uint32_t)0x90000000)

/* add user code end exported constants */

/* exported macro ------------------------------------------------------------*/
/* add user code begin exported macro */

/* add user code end exported macro */

/* exported functions ------------------------------------------------------- */
  void APS6404LHwReset(void);
  int APS6404LHwReadId(unsigned char *pBuffer, int nLength);
  int APS6404LHwWriteBytes(unsigned int address, const unsigned char *pBuffer, int nLength);
  int APS6404LHwReadBytes(unsigned int address, unsigned char *pBuffer, int nLength);
  int APS6404LHwEnterQuadMode(void);
  int APS6404LHwExitQuadMode(void);
  int APS6404LHwWrapBoundaryToggle(void);
  uint32_t APS6404LHwGetLastStage(void);

/* add user code begin exported functions */

/* add user code end exported functions */

#ifdef __cplusplus
}
#endif

#endif
