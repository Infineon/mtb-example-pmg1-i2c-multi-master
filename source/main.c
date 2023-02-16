/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for PMG1: I2C Multi Master code example
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2021-2023, Cypress Semiconductor Corporation (an Infineon company) or
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
#include "cy_pdl.h"
#include "cybsp.h"
#include "stdio.h"
#include "I2CMasterSlave.h"
#include <inttypes.h>

/*******************************************************************************
* Macros
*******************************************************************************/
#define SW_DEBOUNCE_DELAY           (25u)
#define CY_ASSERT_FAILED            (0u)

/* Debug print macro to enable UART print */
/* (For S0 - Debug print will be always zero as SCB UART is not available) */
#if (!defined(CY_DEVICE_CCG3PA))
#define DEBUG_PRINT         (0u)
#endif

/*******************************************************************************
* Function Prototypes
********************************************************************************/
void Button_IntHandler(void);

/*******************************************************************************
* Global Variable
*******************************************************************************/
/* Flag to detect button press event */
volatile uint8_t interrupt_flag = 0;

/* Switch interrupt configuration structure */
const cy_stc_sysint_t button_interrupt_config =
{
    .intrSrc = CYBSP_USER_BTN_IRQ,             //Source of interrupt signal
    .intrPriority = 3u,
};

#if DEBUG_PRINT
/* Structure for UART Context */
cy_stc_scb_uart_context_t CYBSP_UART_context;

/* Variable used for tracking the print status */
volatile bool ENTER_LOOP = true;

/*******************************************************************************
* Function Name: check_status
********************************************************************************
* Summary:
*  Prints the error message.
*
* Parameters:
*  error_msg - message to print if any error encountered.
*  status - status obtained after evaluation.
*
* Return:
*  void
*
*******************************************************************************/
void check_status(char *message, cy_rslt_t status)
{
    char error_msg[50];

    sprintf(error_msg, "Error Code: 0x%08" PRIX32 "\n", status);

    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n=====================================================\r\n");
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\nFAIL: ");
    Cy_SCB_UART_PutString(CYBSP_UART_HW, message);
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n");
    Cy_SCB_UART_PutString(CYBSP_UART_HW, error_msg);
    Cy_SCB_UART_PutString(CYBSP_UART_HW, "\r\n=====================================================\r\n");
}
#endif

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  System entrance point. This function performs
*  - initial setup of device
*  - initialize external interrupt for Button
*  - check for button press and send I2C Commands to slave device
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    cy_en_sysint_status_t intr_result;
    uint32_t status;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(CY_ASSERT_FAILED);
    }

#if DEBUG_PRINT
     /* Configure and enable the UART peripheral */
     Cy_SCB_UART_Init(CYBSP_UART_HW, &CYBSP_UART_config, &CYBSP_UART_context);
     Cy_SCB_UART_Enable(CYBSP_UART_HW);
     /* Sequence to clear screen */
     Cy_SCB_UART_PutString(CYBSP_UART_HW, "\x1b[2J\x1b[;H");
     /* Print "I2C Multi Master" */
     Cy_SCB_UART_PutString(CYBSP_UART_HW, "**************************");
     Cy_SCB_UART_PutString(CYBSP_UART_HW, "PMG1 MCU: I2C MULTI MASTER");
     Cy_SCB_UART_PutString(CYBSP_UART_HW, "**************************\r\n\n");
#endif

    /* User button (CYBSP_USER_BTN) interrupt initialization*/
    intr_result = Cy_SysInt_Init(&button_interrupt_config, &Button_IntHandler);
    if (intr_result != CY_SYSINT_SUCCESS)
    {
#if DEBUG_PRINT
        check_status("API Cy_SysInt_Init failed with error code", intr_result);
#endif

        CY_ASSERT(CY_ASSERT_FAILED);
    }

    /* Clear any pending interrupt and enable the User Button Interrupt */
    NVIC_ClearPendingIRQ(button_interrupt_config.intrSrc);
    NVIC_EnableIRQ(button_interrupt_config.intrSrc);

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize the I2C master slave component */
    status = InitI2CMasterSlave();
    if(status != I2C_SUCCESS)
    {
#if DEBUG_PRINT
        check_status("API InitI2CMasterSlave failed with error code", status);
#endif

        CY_ASSERT(CY_ASSERT_FAILED);
    }

    for (;;)
    {
        /* Process the I2C Commands received from master*/
        processI2CCommandFromMaster();

        /* Check if button is pressed */
        if(interrupt_flag)
        {
            /* Wait for 25 milliseconds for button de-bounce*/
            Cy_SysLib_Delay(SW_DEBOUNCE_DELAY);

            if(!Cy_GPIO_Read(CYBSP_USER_BTN_PORT, CYBSP_USER_BTN_PIN))
            {
                /* If button pressed, then process command to send to slave*/
                processI2CCommandToSlave();
            }

            /* Clear the Button Press Event */
            interrupt_flag = 0;
        }
#if DEBUG_PRINT
        if (ENTER_LOOP)
        {
            Cy_SCB_UART_PutString(CYBSP_UART_HW, "Entered for loop\r\n");
            ENTER_LOOP = false;
        }
#endif
    }
}


/*******************************************************************************
* Function Name: Button_IntHandler
********************************************************************************
*
* Summary:
*  Handler for falling edge interrupt detected on button press.
*
*******************************************************************************/
void Button_IntHandler(void)
{
    /* Clears the triggered pin interrupt */
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN_PORT, CYBSP_USER_BTN_NUM);
    
    /* Set interrupt flag */
    interrupt_flag = 1u;
}


/* [] END OF FILE */
