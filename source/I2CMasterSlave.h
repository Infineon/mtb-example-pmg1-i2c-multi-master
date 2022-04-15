/******************************************************************************
* File Name:  I2CMasterSlave.h
*
* Description:  This file provides constants and parameter values for the I2C
*               Master-Slave peripheral.
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

#ifndef SOURCE_I2CMASTERSLAVE_H_
#define SOURCE_I2CMASTERSLAVE_H_

/*******************************************************************************
 * Include header files
 ******************************************************************************/
#include "cy_pdl.h"
#include "cybsp.h"

/*******************************************************************************
* Macros
*******************************************************************************/

/* KIT ADDRESS macro for defining the address of the kit*/
/* Only one should be uncommented at any one time*/

/* Uncomment PMG1_KIT_1_ADDRESS_ENABLE and comment PMG1_KIT_2_ADDRESS_ENABLE 
 * when programming the first kit*/
#define PMG1_KIT_1_ADDRESS_ENABLE

/* Uncomment PMG1_KIT_2_ADDRESS_ENABLE and comment PMG1_KIT_1_ADDRESS_ENABLE 
 * when programming the second kit*/
//  #define PMG1_KIT_2_ADDRESS_ENABLE

#define LED_ON                      (0UL)
#define LED_OFF                     (1UL)

#define I2C_SUCCESS                 (0UL)
#define I2C_FAILURE                 (1UL)

#define I2C_PACKET_VALID            (0UL)
#define I2C_PACKET_INVALID          (1UL)

#define TRANSFER_CMPLT              (0x00UL)
#define READ_CMPLT                  (TRANSFER_CMPLT)
#define TRANSFER_ERROR              (0xFFUL)
#define INVALID_DATA_ERROR          (0x0FUL)
#define READ_ERROR                  (TRANSFER_ERROR)

#define COMMAND_PACKET_SIZE         (4u)
#define COMMAND_HEADER_INDEX        (0u)
#define COMMAND_DEVICE_INDEX        (1u)
#define COMMAND_LED_INDEX           (2u)
#define COMMAND_FOOTER_INDEX        (3u)
#define COMMAND_HEADER              (0xDC)
#define COMMAND_FOOTER              (0x89)

#define I2C_COMMAND_LED_OFF         (0x76u)
#define I2C_COMMAND_LED_ON          (0xE3u)

#ifdef PMG1_KIT_1_ADDRESS_ENABLE
#define EXTERNAL_SLAVE_ADDRESS      (0x32)
#define KIT_I2C_SLAVE_ADDRESS       (0x24)
#else
#define EXTERNAL_SLAVE_ADDRESS      (0x24)
#define KIT_I2C_SLAVE_ADDRESS       (0x32)
#endif


#if defined(CY_DEVICE_CCG3PA)
    #define COMMAND_BSP             (0x10u)
#elif defined (CY_DEVICE_CCG6)
    #define COMMAND_BSP             (0x11u)
#elif defined (CY_DEVICE_CCG3)
    #define COMMAND_BSP             (0x12u)
#elif defined (CY_DEVICE_PMG1S3)
    #define COMMAND_BSP             (0x13u)
#endif /* defined(CY_DEVICE_CCG3PA) */

#define PMG1_S0_BSP                 (0x10u)
#define PMG1_S1_BSP                 (0x11u)
#define PMG1_S2_BSP                 (0x12u)
#define PMG1_S3_BSP                 (0x13u)

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
uint32_t InitI2CMasterSlave(void);
void handle_error(void);
uint8_t ReadFromI2CSlave(uint8_t slaveAddress,uint8_t* readBuffer, uint8_t readSize);
uint8_t WriteToI2CSlave(uint8_t slaveAddress,uint8_t* writeBuffer, uint8_t writeSize);
uint8_t checkCommandPacketValidity(uint8_t*);
void processI2CCommandFromMaster();
void processI2CCommandToSlave();

#endif /* SOURCE_I2CMASTERSLAVE_H_ */
