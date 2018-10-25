/******************************************************************************
*
* Copyright (C) 2010 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file     xuartps_polled_example.c
*
* This file contains an example using the XUartPs driver in polled mode.
*
* This function sends data and expects to receive the data thru the device
* using the local loopback mode.
*
* @note
* If the device does not work properly, the example may hang.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00a  drg/jz 01/13/10 First Release
* 1.03a  sg     07/16/12 Modified the device ID to use the first Device Id
*			Removed the printf at the start of the main
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xuartps.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define UART_DEVICE_ID              XPAR_XUARTPS_0_DEVICE_ID

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the device, this constant must be 32 bytes or less since
 * only as much as FIFO size data can be sent or received in polled mode.
 */
#define TEST_BUFFER_SIZE 64		// SEND INTERVALS OF 2 BYTES, mg

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int UartPsPolledExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XUartPs Uart_PS;		/* Instance of the UART Device */

/*
 * The following buffers are used in this example to send and receive data
 * with the UART.
 */
static u8 SendBuffer[TEST_BUFFER_SIZE];	/* Buffer for Transmitting Data - change to char to debug */
static u8 RecvBuffer[TEST_BUFFER_SIZE];	/* Buffer for Receiving Data */

// declare variables for TX part
int ConsoleTXBufferLen = 0;
int SendComplete = 1;
int ConsoleTXBufferIndex = 0;


/*****************************************************************************/
/**
*
* Main function to call the Uart Polled mode example.
*
* @param	None
*
* @return	XST_SUCCESS if succesful, otherwise XST_FAILURE
*
* @note		None
*
******************************************************************************/

int main(void)
{
	int Status;


	/*
	 * Run the Uart_PS polled example , specify the the Device ID that is
	 * generated in xparameters.h
	 */
	Status = UartPsPolledExample(UART_DEVICE_ID);


	/*
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 */

	if (Status != XST_SUCCESS) {
		xil_printf("UART Polled Mode Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran UART Polled Mode Example Test\r\n");
	return XST_SUCCESS;
}


	/*
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 */


/**
*
* This function does a minimal test on the XUartPs device in polled mode.
*
* This function sends data and expects to receive the data thru the UART
* using the local loopback mode.
*
*
* @param	DeviceId is the unique device id from hardware build.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note
* This function polls the UART, it may hang if the hardware is not
* working correctly.
*
****************************************************************************/
int UartPsPolledExample(u16 DeviceId)
{
	int Status;
	XUartPs_Config *Config;
	unsigned int SentCount;
	unsigned int ReceivedCount;
	u16 Index;
	u32 LoopCount = 0;


	/*
		 * --------------------------------------------------------------------------
		 * --------------------------------------------------------------------------
		 * --------------------------------------------------------------------------
		 * --------------------------------------------------------------------------
		 */

	// GET SYSTEM READY
	/*
	 * Initialize the UART driver so that it's ready to use.
	 * Look up the configuration in the config table, then initialize it.
	 */
	Config = XUartPs_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XUartPs_CfgInitialize(&Uart_PS, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Check hardware build. */
	Status = XUartPs_SelfTest(&Uart_PS);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 */

	//--------------------------------------------------------------------------
	// Set the operation mode
	//--------------------------------------------------------------------------
	XUartPs_SetBaudRate(&Uart_PS, 9600);	// changed value


	/* Example used local loopback mode.
	 * CHANGED TO NORMAL MODE , mg
	 *  */
	XUartPs_SetOperMode(&Uart_PS, XUARTPS_OPER_MODE_NORMAL);

	/*
	 * Initialize the send buffer bytes with a pattern and zero out
	 * the receive buffer.
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		//SendBuffer[Index] = '0' + Index;  // this is getting : and ; for #s over 9
		//SendBuffer[Index] = Index;
		SendBuffer[Index] = Index;
		RecvBuffer[Index] = 0;
	}


	/*
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 */

	//while(1){
	/* Block sending the buffer. */
	ConsoleTXBufferLen = XUartPs_Send(&Uart_PS, SendBuffer, TEST_BUFFER_SIZE);  // get length of send buffer
	if (ConsoleTXBufferLen != TEST_BUFFER_SIZE) {
		return XST_FAILURE;
	}

	//Frankenstein version of send_string_buffer function from Dr. Lusher
	ConsoleTXBufferIndex = 0;
	SendComplete = 0;

	while (ConsoleTXBufferIndex <  ConsoleTXBufferLen)
	//while (1)
		{
			XUartPs_Send(&Uart_PS, (u8 *)&SendBuffer[ConsoleTXBufferIndex++], 1);
			//ConsoleTXBufferIndex++;
		}

	/*
	 * Wait while the UART is sending the data so that we are guaranteed
	 * to get the data the 1st time we call receive, otherwise this function
	 * may enter receive before the data has arrived
	 */
	while (XUartPs_IsSending(&Uart_PS)) {	// check if device is still sending data, if so delay by incrementing loopcount+
		LoopCount++;
	}


	/*
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 */

	/* Block receiving the buffer. */
	ReceivedCount = 0;
	//while (ReceivedCount < TEST_BUFFER_SIZE) {  // CHANGED DURING TESTING 8/24
	while (1) {
		XUartPs_Recv(&Uart_PS, &RecvBuffer[ReceivedCount], (TEST_BUFFER_SIZE - ReceivedCount));
	}

	//Frankenstein version of receive data functions from Dr. Lusher
	//while (XUartPs_IsReceiveData(Uart_PS.Config.BaseAddress))
		//{
		// DOUBLE CHECK THIS --- 10/15 mg
			u8 buffer[32];
			XUartPs_Recv(&Uart_PS, buffer, 1);
	//	}

	/*
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 * --------------------------------------------------------------------------
	 */

	// should not need this function below only as a check

	/*
	 * Check the receive buffer against the send buffer and verify the
	 * data was correctly received
	 */

	/*
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		if (SendBuffer[Index] != RecvBuffer[Index]) {
			return XST_FAILURE;
		}
	}
	*/

	/* Restore to normal mode. */
	//XUartPs_SetOperMode(&Uart_PS, XUARTPS_OPER_MODE_NORMAL);

	return XST_SUCCESS;
}
