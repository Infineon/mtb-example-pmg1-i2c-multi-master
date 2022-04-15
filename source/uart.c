/******************************************************************************
* File Name:  uart.c
*
* Description:  This file contains all the functions and variables required for
*               proper operation of UART SCB component.
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

/*******************************************************************************
 * Include header files
 ******************************************************************************/
#include "uart.h"

/*******************************************************************************
* Global variables
*******************************************************************************/
/** The instance-specific context structure.
 * It is used by the driver for internal configuration and
 * data keeping for the UART. Do not modify anything in this structure.
 */
cy_stc_scb_uart_context_t CYBSP_UART_context;


/*******************************************************************************
* Function Name: InitUART
********************************************************************************
* Summary:
*  This function initializes and enables SCB fro UART
*
* Parameters:
*  none
*
* Return:
*  none
*
*******************************************************************************/
void InitUART()
{
#if !UART_DEBUG_DISABLED
    cy_en_scb_uart_status_t status;

    /* Initialize the UART SCB block */
    status = Cy_SCB_UART_Init(CYBSP_UART_HW, &CYBSP_UART_config, &CYBSP_UART_context);
    if (status != CY_SCB_UART_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable UART     */
    Cy_SCB_UART_Enable(CYBSP_UART_HW);
#endif
}

/*******************************************************************************
* Function Name: uartSendString
********************************************************************************
* Summary:
*  This function is used to send a message over UART SCB
*
* Parameters:
*  message: The message that is to be transmitted over UART
*
* Return:
*  none
*
*******************************************************************************/
void uartSendString(char* message)
{
#if !UART_DEBUG_DISABLED
    Cy_SCB_UART_PutString(CYBSP_UART_HW,message);
#endif
}



