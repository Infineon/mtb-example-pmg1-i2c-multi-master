/******************************************************************************
* File Name:  I2CMasterSlave.c
*
* Description:  This file contains all the functions and variables required for
*               proper operation of I2C SCB component in master-slave mode.
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
#include "I2CMasterSlave.h"
#include "stdio.h"
#include "uart.h"

/*******************************************************************************
* Macros
*******************************************************************************/
/* Combine master error statuses in single mask  */
#define MASTER_ERROR_MASK   (CY_SCB_I2C_MASTER_DATA_NAK | CY_SCB_I2C_MASTER_ADDR_NAK   | \
                            CY_SCB_I2C_MASTER_ARB_LOST | CY_SCB_I2C_MASTER_ABORT_START | \
                            CY_SCB_I2C_MASTER_BUS_ERR)

/*******************************************************************************
* Global variables
*******************************************************************************/
/* Buffer to hold the data received fram master */
uint8_t i2cWriteBuffer[COMMAND_PACKET_SIZE] = {COMMAND_HEADER, COMMAND_BSP,
                                                I2C_COMMAND_LED_OFF, COMMAND_FOOTER};

/* Buffer to hold the data that is to be read by the master */
uint8_t i2cReadBuffer[COMMAND_PACKET_SIZE]  = {COMMAND_HEADER, COMMAND_BSP,
                                                I2C_COMMAND_LED_OFF, COMMAND_FOOTER};

/* Structure for master transfer configuration */
cy_stc_scb_i2c_master_xfer_config_t masterTransferCfg =
{
    .slaveAddress = EXTERNAL_SLAVE_ADDRESS,
    .buffer       = NULL,
    .bufferSize   = 0U,         /* Read 1 Byte from Slave */
    .xferPending  = false       /* Generate Stop condition the end of transaction */
};

/** The instance-specific context structure.
 * It is used by the driver for internal configuration and
 * data keeping for the I2C. Do not modify anything in this structure.
 */
cy_stc_scb_i2c_context_t CYBSP_I2C_context;

/* Flag that holds the status of command reception from master */
uint8_t commandFromMasterReceived = false;

/*******************************************************************************
* Function Declaration
*******************************************************************************/
void CYBSP_I2C_Interrupt(void);
static void I2C_SlaveEventHandler(uint32_t events);

/*******************************************************************************
* Function Name: InitI2CMasterSlave
********************************************************************************
* Summary:
*  This function initializes and enables SCB as I2C Master-Slave
*
* Parameters:
*  none
*
* Return:
*   Status of I2C Initializations. Returns I2C_SUCCESS upon successful initialization of
*   I2C SCB . Otherwise the function returns I2C_FAILURE.
*
*******************************************************************************/
uint32_t InitI2CMasterSlave()
{
    cy_en_scb_i2c_status_t initStatus;
    cy_en_sysint_status_t sysStatus;
    cy_stc_sysint_t CYBSP_I2C_SCB_IRQ_cfg =
    {
            /*.intrSrc =*/ CYBSP_I2C_IRQ,
            /*.intrPriority =*/ 3u
    };

    /*Set the Slave adress*/
    CYBSP_I2C_config.slaveAddress = KIT_I2C_SLAVE_ADDRESS;

    /*Initialize and enable the I2C in master-slave mode*/
    initStatus = Cy_SCB_I2C_Init(CYBSP_I2C_HW, &CYBSP_I2C_config, &CYBSP_I2C_context);
    if(initStatus != CY_SCB_I2C_SUCCESS)
    {
        return I2C_FAILURE;
    }

    /* Hook interrupt service routine */
    sysStatus = Cy_SysInt_Init(&CYBSP_I2C_SCB_IRQ_cfg, &CYBSP_I2C_Interrupt);
    if(sysStatus != CY_SYSINT_SUCCESS)
    {
        return I2C_FAILURE;
    }
    NVIC_EnableIRQ((IRQn_Type) CYBSP_I2C_SCB_IRQ_cfg.intrSrc);

    /* Register callback for event notification.*/
    Cy_SCB_I2C_RegisterEventCallback(CYBSP_I2C_HW, I2C_SlaveEventHandler,
                                        &CYBSP_I2C_context);

    /*Enable the I2C in master-slave mode*/
    Cy_SCB_I2C_Enable(CYBSP_I2C_HW, &CYBSP_I2C_context);

    /* Configure the i2c read buffer*/
    Cy_SCB_I2C_SlaveConfigReadBuf(CYBSP_I2C_HW, i2cReadBuffer,
                                    COMMAND_PACKET_SIZE, &CYBSP_I2C_context);

    /* Configure the i2c write buffer*/
    Cy_SCB_I2C_SlaveConfigWriteBuf(CYBSP_I2C_HW, i2cWriteBuffer,
                                    COMMAND_PACKET_SIZE, &CYBSP_I2C_context);

    return I2C_SUCCESS;
}

/*******************************************************************************
* Function Name: CYBSP_I2C_Interrupt
****************************************************************************//**
*
* Summary:
*   Invokes the Cy_SCB_I2C_Interrupt() PDL driver function.
*
*******************************************************************************/
void CYBSP_I2C_Interrupt(void)
{
    Cy_SCB_I2C_Interrupt(CYBSP_I2C_HW, &CYBSP_I2C_context);
}


/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
*  Function to handle errors
*
* Parameters:
*  none
*
* Return:
*  none
*
*******************************************************************************/
void handle_error(void)
{
    /* Disable all interrupts. */
    __disable_irq();

    /* Infinite loop. */
    while(1u) {}
}


/*******************************************************************************
* Function Name: pmg1I2CDeviceCheck
********************************************************************************
* Summary:
*  Function to return the PMG1 device string based on the device argument received. 
*
* Parameters:
*  device       The device number macro
*
* Return:
*  bspString    BSP string associated with the device number
*
*******************************************************************************/
void pmg1I2CDeviceCheck(uint8_t device,char* bspString)
{
    switch(device)
    {
    case PMG1_S0_BSP:
        strcpy(bspString,"PMG1-S0");
        break;
    case PMG1_S1_BSP:
        strcpy(bspString,"PMG1-S1");
        break;
    case PMG1_S2_BSP:
        strcpy(bspString,"PMG1-S2");
        break;
    case PMG1_S3_BSP:
        strcpy(bspString,"PMG1-S3");
        break;
    default:
        strcpy(bspString,"Unknown BSP");
        break;
    }
}

/*******************************************************************************
* Function Name: processI2CCommandFromMaster
********************************************************************************
* Summary:
*  Function to process the data received from master.
*   - Check if command has been received from Master
*   - Check validity of the command packet received from master
*   - Set the state of the LED based on received command.
*
* Parameters:
*  none
*
* Return:
*  none
*
*******************************************************************************/
void processI2CCommandFromMaster()
{
    char bspString[12];
    char str[70];

    /* Check if command was received from Master */
    if(commandFromMasterReceived)
    {
        commandFromMasterReceived = false;

        /* Verify if the received command packet is valid*/
        if(checkCommandPacketValidity(i2cWriteBuffer) == I2C_PACKET_INVALID)
        {
            uartSendString(" Invalid command packet received from master.\r\n\n");
            return;
        }

        /* Populate the device BSP string based on the packet received from master*/
        pmg1I2CDeviceCheck(i2cWriteBuffer[COMMAND_DEVICE_INDEX], bspString);

        /* Set the state of the LED based on the received command */
        switch(i2cWriteBuffer[COMMAND_LED_INDEX])
        {
            case I2C_COMMAND_LED_ON:
                sprintf(str," LED ON command received from master (%s) \r\n\n", bspString);
                uartSendString(str);
                /* Turn LED on*/
                Cy_GPIO_Write(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN, LED_ON);
            break;

            case I2C_COMMAND_LED_OFF:
                sprintf(str," LED OFF command received from master (%s) \r\n\n", bspString);
                uartSendString(str);
                /* Turn LED off*/
                Cy_GPIO_Write(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN, LED_OFF);
            break;

            default:
            break;
        }
        
        /* Update the i2c read buffer with the LED command received from master */
        i2cReadBuffer[COMMAND_LED_INDEX] = i2cWriteBuffer[COMMAND_LED_INDEX];
    }
}


/*******************************************************************************
* Function Name: processI2CCommandToSlave
********************************************************************************
* Summary:
*  Function to prepare the command data to slave
*   - Read the current state of slave LED
*   - Check validity of the command packet received from slave
*   - Set the new state to set the slave LED to based on read state.
*
* Parameters:
*  none
*
* Return:
*  none
*
*******************************************************************************/
void processI2CCommandToSlave()
{
    uint32_t status;
    char str[70], bspString[12];
    static uint8_t slaveReadBuffer[COMMAND_PACKET_SIZE];
    static uint8_t slaveWriteBuffer[COMMAND_PACKET_SIZE];

    /* Send read command to external slave */
    status = ReadFromI2CSlave(EXTERNAL_SLAVE_ADDRESS,
                            slaveReadBuffer,COMMAND_PACKET_SIZE);
                            
    if(status != TRANSFER_CMPLT)
    {
        uartSendString( "\r\n I2C Read Error \r\n\n");
    }
    else
    {
        /* Verify if the status packet read from external slave is valid*/
        if(checkCommandPacketValidity(slaveReadBuffer) == I2C_PACKET_INVALID)
        {
            uartSendString(
                        "\r\n Invalid command packet read from slave.\r\n\n");
            return;
        }
        
        /* Populate the device BSP string based on the packet read from slave*/
        pmg1I2CDeviceCheck(slaveReadBuffer[COMMAND_DEVICE_INDEX], bspString);

        /* Select command to invert the state of slave LED */
        switch(slaveReadBuffer[COMMAND_LED_INDEX])
        {
            case I2C_COMMAND_LED_ON:
                sprintf(str," Sending command to turn OFF slave (%s) LED \r\n\n",bspString);
                uartSendString(str);
                /* If slave LED is on, then set command to turn it off */
                slaveWriteBuffer[COMMAND_LED_INDEX] = I2C_COMMAND_LED_OFF;
            break;

            case I2C_COMMAND_LED_OFF:
                sprintf(str," Sending command to turn ON slave (%s) LED \r\n\n", bspString);
                uartSendString(str);
                /* If slave LED is off, then set command to turn it on */
                slaveWriteBuffer[COMMAND_LED_INDEX] = I2C_COMMAND_LED_ON;
            break;

            default:
                slaveWriteBuffer[COMMAND_LED_INDEX] = I2C_COMMAND_LED_OFF;
                break;
        }

        /*Populate the header and footer of command packet to send to slave.*/
        slaveWriteBuffer[COMMAND_HEADER_INDEX] = COMMAND_HEADER;
        slaveWriteBuffer[COMMAND_FOOTER_INDEX] = COMMAND_FOOTER;
        slaveWriteBuffer[COMMAND_DEVICE_INDEX] = COMMAND_BSP;

        /* Write command to slave */
        WriteToI2CSlave(EXTERNAL_SLAVE_ADDRESS,slaveWriteBuffer,COMMAND_PACKET_SIZE);
    }
}


/*******************************************************************************
* Function Name: I2C_SlaveEventHandler
********************************************************************************
* Summary:
*  Callback handler to handle I2C slave events when there is a read or
*   write from the master.
*
* Parameters:
*  events I2C events that triggered this event handler
*
* Return:
*  none
*
*******************************************************************************/
void I2C_SlaveEventHandler(uint32_t events)
{
    /* Slave sent data to master */
    if (0UL != (events & CY_SCB_I2C_SLAVE_RD_CMPLT_EVENT))
    {
        if (0UL == (events & CY_SCB_I2C_SLAVE_ERR_EVENT))
        {
            /* Read complete without errors: update buffer content */
        }
        
        /* Setup read buffer for the next read transaction */
        Cy_SCB_I2C_SlaveConfigReadBuf(CYBSP_I2C_HW, i2cReadBuffer,
                                        COMMAND_PACKET_SIZE, &CYBSP_I2C_context);
    }

    /* Slave received data from master */
    if (0UL != (events & CY_SCB_I2C_SLAVE_WR_CMPLT_EVENT))
    {
        if (0UL == (events & CY_SCB_I2C_SLAVE_ERR_EVENT))
        {
            /* Write complete without errors: process received data */

            /* Set the flag to indicate reception of data in i2cWriteBuffer */
            commandFromMasterReceived = true;
        }

        /* Setup buffer for the next write transaction */
        Cy_SCB_I2C_SlaveConfigWriteBuf(CYBSP_I2C_HW, i2cWriteBuffer,
                                        COMMAND_PACKET_SIZE, &CYBSP_I2C_context);
    }
    /* Ignore all other events */
}


/*******************************************************************************
* Function Name: checkCommandPacketValidity
********************************************************************************
* Summary:
*  Function to check if the I2C command packet in commandPacket is valide or not.
*
* Parameters:
*  commandPacket: Packed whose validity is to be checked
*
* Return:
*   Status of packet validity. Returns I2C_PACKET_VALID upon successful validation
*   of packet . Otherwise the function returns I2C_PACKET_INVALID.
*
*******************************************************************************/
uint8_t checkCommandPacketValidity(uint8_t* commandPacket)
{
    uint8_t status = I2C_PACKET_INVALID;

    /* Verify the packet header and footer */
    if( (commandPacket[COMMAND_HEADER_INDEX] == COMMAND_HEADER)  &&
            (commandPacket[COMMAND_FOOTER_INDEX] == COMMAND_FOOTER) )
    {
        status = I2C_PACKET_VALID;
    }

    return status;
}



/*******************************************************************************
* Function Name: ReadFromI2CSlave
********************************************************************************
* Summary:
* This function reads the readSize number of bytes of data from the slave to
* the readBuffer.
*
* Parameters:
*  slaveAddress: Address of slave from which to read data from.
*  readBuffer: The read data from slave will be stored in this readBuffer
*  readSize: No of bytes that needs to be read from the slave
*
* Return:
*   Status of I2C read. Returns TRANSFER_CMPLT upon successful completion of read
*   from the slave . Otherwise the function returns TRANSFER_ERROR.
*
*******************************************************************************/
uint8_t ReadFromI2CSlave(uint8_t slaveAddress,uint8_t* readBuffer, uint8_t readSize)
{
    uint8_t status = TRANSFER_ERROR;
    cy_en_scb_i2c_status_t errorStatus;
    uint32_t masterStatus;

    /* Timeout 1 sec (one unit is us) */
    uint32_t timeout = 1000000UL;

    /* Setup transfer specific parameters */
    masterTransferCfg.slaveAddress  = slaveAddress;
    masterTransferCfg.buffer        = readBuffer;
    masterTransferCfg.bufferSize    = readSize;

    /* Generate Stop condition at the end of transaction */
    masterTransferCfg.xferPending   = false;

    /* Initiate read transaction */
    errorStatus = Cy_SCB_I2C_MasterRead(CYBSP_I2C_HW, &masterTransferCfg,
                                            &CYBSP_I2C_context);
    if(errorStatus == CY_SCB_I2C_SUCCESS)
    {
        /* Wait until master complete read transfer or time out has occurred */
        do
        {
            masterStatus = Cy_SCB_I2C_MasterGetStatus(CYBSP_I2C_HW, &CYBSP_I2C_context);
            Cy_SysLib_DelayUs(CY_SCB_WAIT_1_UNIT);
            timeout--;

        } while ((0UL != (masterStatus & CY_SCB_I2C_MASTER_BUSY)) && (timeout > 0));

        if (timeout <= 0)
        {
            /* Timeout recovery */
            Cy_SCB_I2C_Disable(CYBSP_I2C_HW, &CYBSP_I2C_context);
            Cy_SCB_I2C_Enable(CYBSP_I2C_HW, &CYBSP_I2C_context);
        }
        else
        {
            /* Check transfer status */
            if (0u == (MASTER_ERROR_MASK & masterStatus))
            {
                status = TRANSFER_CMPLT;
            }
        }

    }

    return (status);
}


/*******************************************************************************
* Function Name: WriteToI2CSlave
********************************************************************************
* Summary:
* This function writes the writeSize number of bytes of data passed in writeBuffer
* to the slave.
*
* Parameters:
*  slaveAddress: Address of slave to which data is to be sent
*  writeBuffer: Data that needs to written to the slave.
*  writeSize: No of bytes that needs to be written to the slave
*
* Return:
*   Status of I2C write. Returns TRANSFER_CMPLT upon successful completion of write
*   to the slave . Otherwise the function returns TRANSFER_ERROR.
*
*******************************************************************************/
uint8_t WriteToI2CSlave(uint8_t slaveAddress,uint8_t* writeBuffer, uint8_t writeSize)
{
    uint8_t status = TRANSFER_ERROR;
    cy_en_scb_i2c_status_t errorStatus;
    uint32_t masterStatus;

    /* Timeout 1 sec (one unit is us) */
    uint32_t timeout = 1000000UL;

    /* Configure write transaction */
    masterTransferCfg.slaveAddress  = slaveAddress;
    masterTransferCfg.buffer        = writeBuffer;
    masterTransferCfg.bufferSize    = writeSize + 2;

    /* Generate Stop condition at the end of transaction */
    masterTransferCfg.xferPending   = false;


    /* Initiate write transaction */
    errorStatus = Cy_SCB_I2C_MasterWrite(CYBSP_I2C_HW, &masterTransferCfg,
                                            &CYBSP_I2C_context);
    if(errorStatus == CY_SCB_I2C_SUCCESS)
    {
        /* Wait until master complete read transfer or time out has occured */
        do
        {
            masterStatus  = Cy_SCB_I2C_MasterGetStatus(CYBSP_I2C_HW, &CYBSP_I2C_context);
            Cy_SysLib_DelayUs(CY_SCB_WAIT_1_UNIT);
            timeout--;

        } while ((0UL != (masterStatus & CY_SCB_I2C_MASTER_BUSY)) && (timeout > 0));

        if (timeout <= 0)
        {
            /* Timeout recovery */
            Cy_SCB_I2C_Disable(CYBSP_I2C_HW, &CYBSP_I2C_context);
            Cy_SCB_I2C_Enable(CYBSP_I2C_HW,&CYBSP_I2C_context);
        }
        else
        {
            if (
                (0u == (MASTER_ERROR_MASK & masterStatus)) &&
                ((writeSize+2) == Cy_SCB_I2C_MasterGetTransferCount(CYBSP_I2C_HW,
                                                            &CYBSP_I2C_context))
                )
            {
                status = TRANSFER_CMPLT;
            }
        }
    }

    return (status);
}


