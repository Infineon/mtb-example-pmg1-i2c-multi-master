/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for PMG1: I2C Multi Master code example
*
* Related Document: See README.md
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
#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "stdio.h"
#include "I2CMasterSlave.h"
#include "uart.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define SW_DEBOUNCE_DELAY           (25u)
#define CY_ASSERT_FAILED            (0u)

/*******************************************************************************
* Global Variable
*******************************************************************************/
/* Flag to detect button press event */
volatile uint8_t interrupt_flag = 0;

/*******************************************************************************
* Function Prototypes
********************************************************************************/
void Button_IntHandler(void);


/******************************************************************************
 * Switch interrupt configuration structure
 *****************************************************************************/
const cy_stc_sysint_t button_interrupt_config =
{
    .intrSrc = CYBSP_USER_BTN_IRQ,             //Source of interrupt signal
    .intrPriority = 3u,
};

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
    char str[70];
    cy_rslt_t result;
    uint32_t status;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize the UART */
    InitUART();

    /* \x1b[2J\x1b[;H - ANSI ESC sequence to clear screen. */
    uartSendString("\x1b[2J\x1b[;H");

    uartSendString(" ****************** "
                    "PMG 1: I2C Multi Master "
                    "****************** \r\n\n");

    /* User button (CYBSP_USER_BTN) interrupt initialization*/
    result = Cy_SysInt_Init(&button_interrupt_config, &Button_IntHandler);
    if (result != CY_SYSINT_SUCCESS)
    {
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
        CY_ASSERT(0);
    }

    sprintf(str," Kit %s address is set to 0x%X\r\n", KIT_BSP,
                            KIT_I2C_SLAVE_ADDRESS);
    uartSendString( str );
    sprintf(str," Please make sure the Kit address "
                "of the other kit is 0x%X\r\n",EXTERNAL_SLAVE_ADDRESS);
    uartSendString( str );
    uartSendString( " If not, then check kit address macro in file I2CMasterSlave.h\r\n\n");

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
                
                uartSendString(" Switch press detected, ");
                            
                /* If button pressed, then process command to send to slave*/
                processI2CCommandToSlave();
            }

            /* Clear the Button Press Event */
            interrupt_flag = 0;
        }

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
    NVIC_ClearPendingIRQ(button_interrupt_config.intrSrc);

    /* Set interrupt flag */
    interrupt_flag = 1u;
}


/* [] END OF FILE */
