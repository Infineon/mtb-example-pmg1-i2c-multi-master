/******************************************************************************
* File Name:  uart.h
*
* Description:  This file provides constants and parameter values for UART.
*
* Related Document: See Readme.md
*
*
*******************************************************************************
* Copyright 2021-2022, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

#ifndef SOURCE_UART_H_
#define SOURCE_UART_H_

/*******************************************************************************
 * Include header files
 ******************************************************************************/
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "string.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/*
 * Detect the PMG1 Kit based on the CCG device used and  enable or disable
 * UART debugging based on the same.
 * NOTE : UART Debugging is disabled for PMG1-S0 Kit because the I2C and
 * UART Pins require usage of the same SCB1 block. Hence SCB1 is used for I2C
 * on the PMG1-S0 kit.
 */
#if defined(CY_DEVICE_CCG3PA)
#define UART_DEBUG_DISABLED         (true)       
#define KIT_BSP                     "PMG1-S0"
#elif defined (CY_DEVICE_CCG6)
#define UART_DEBUG_DISABLED         (false)
#define KIT_BSP                     "PMG1-S1"
#elif defined (CY_DEVICE_CCG3)
#define UART_DEBUG_DISABLED         (false)
#define KIT_BSP                     "PMG1-S2"
#elif defined (CY_DEVICE_PMG1S3)
#define UART_DEBUG_DISABLED         (false)
#define KIT_BSP                     "PMG1-S3"
#endif 

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void InitUART();
void uartSendString(char* message);


#endif /* SOURCE_UART_H_ */
