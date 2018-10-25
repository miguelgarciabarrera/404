/******************************************************************************/
/*                                                                            */
/* main.c -- Program to drive 2 dual channel H-bridges (i.e. 4 motors)        */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
/* File Description:                                                          */
/*                                                                            */
/* This program drives 4 motors in the 4 possible directions using tank       */
/* steering.                                                                  */
/******************************************************************************/

#include "PmodDHB1.h"
#include "PWM.h"
#include "sleep.h"
#include "xil_cache.h"
#include "xparameters.h"

// headers for GPIO
#include "xil_io.h"
#include "sleep.h"
#include <xgpio.h>
#include "platform.h"
#include <stdio.h>

// headers for UART
#include "xuartps.h"
#include "xil_printf.h"


/************ Macro Definitions ************/

//GPIO control
#define GPIO_BASEADDR     XPAR_PMODDHB1_0_AXI_LITE_GPIO_BASEADDR

//PMOD JD
#define JD_GPIO_BASEADDR     XPAR_PMODDHB1_0_AXI_LITE_GPIO_BASEADDR
#define JD_PWM_BASEADDR      XPAR_PMODDHB1_0_PWM_AXI_BASEADDR

//PMOD JE
#define JE_GPIO_BASEADDR     XPAR_PMODDHB1_1_AXI_LITE_GPIO_BASEADDR
#define JE_PWM_BASEADDR      XPAR_PMODDHB1_1_PWM_AXI_BASEADDR


#ifdef __MICROBLAZE__	// ifdef and endif pair. define below used if found in xparamters.h
#define CLK_FREQ XPAR_CPU_M_AXI_DP_FREQ_HZ
#else
//#define CLK_FREQ XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ // FCLK0 frequency not found in xparameters.h
#define CLK_FREQ 1000000
#endif

#define PWM_PER 2

////////////////////////////////////////////////////
////////////////////////////////////////////////////
//UART
//#define UART_DEVICE_ID		XPAR_XUARTPS_0_DEVICE_ID

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the device, this constant must be 32 bytes or less since
 * only as much as FIFO size data can be sent or received in polled mode.
 */
#define TEST_BUFFER_SIZE 16		// SEND INTERVALS OF 2 BYTES, mg
XUartPs Uart_PS;		/* Instance of the UART Device */
static u8 RecvBuffer[TEST_BUFFER_SIZE];	/* Buffer for Receiving Data */

/************ Function Prototypes ************/

void Initialize_bridges();
void Run();
void Cleanup();

void EnableCaches();
void DisableCaches();

void drive_forward();
void drive_back();
void drive_left();
void drive_right();
void stop_moving();
void kill_switch();

/************ Global Variables ************/

PmodDHB1 pmodDHB1;
PmodDHB2 pmodDHB2;
XGpio input;

/************ Function Definitions ************/

int main(void) {

    // GPIO getting ready

	int button_data = 0;
	int switch_data = 0;

	xil_printf("\nApplication started.... \r \n");

	XGpio_Initialize(&input, XPAR_AXI_GPIO_0_DEVICE_ID);	//initialize input XGpio variable

	/******************************************************************************/
	/******************************************************************************/

	// init UART
	XUartPs_Config *Config;
	unsigned int ReceivedCount;
	u16 Index;
	u32 LoopCount = 0;
	ReceivedCount = 0;

	/*
	 * Initialize the UART driver so that it's ready to use.
	 * Look up the configuration in the config table, then initialize it.
	*/
		Config = XUartPs_LookupConfig(XPAR_XUARTPS_0_DEVICE_ID); // changed param 10/25
		if (NULL == Config) {
			return XST_FAILURE;
		}

	XUartPs_SetBaudRate(&Uart_PS, 9600);	// changed value
	XUartPs_SetOperMode(&Uart_PS, XUARTPS_OPER_MODE_NORMAL);

	// Init H-bridges
	Initialize_bridges();

   // polling data from switches/push buttons to control motors
   while(1){

  	// read input from channel 1 of axi_gpio_0 (buttons) and pass that data to "button_data" variable
  	button_data = XGpio_DiscreteRead(&input, 1);
  	switch_data = XGpio_DiscreteRead(&input, 2); // read switch input and store data in variable

  	// receive buffer to control motors with XBEE
  	XUartPs_Recv(&Uart_PS, &RecvBuffer[ReceivedCount], (TEST_BUFFER_SIZE - ReceivedCount));

  	if(RecvBuffer[0] == '4'){
  		drive_left();
  	    usleep(6);
  		}

  	else if(button_data == 0b0010){
  	  	drive_forward();
  	  	usleep(6);
  	  	}

  	else if(button_data == 0b0100){
  	  	 drive_back();
  	  	 usleep(6);
  	  	 }

  	else if(button_data == 0b1000){
  	  	 drive_right();
  	  	 usleep(6);
  	  	 }

  	else if(switch_data == 0b1111){
  	  	 kill_switch();
  	  	 usleep(6);
  	  	 }

  	else{
  		stop_moving();
  	}

     }

   Cleanup();
   return 0;
}

void Initialize_bridges() {
   EnableCaches();

   //init the 2 hbridges
   DHB1_begin(&pmodDHB1, JD_GPIO_BASEADDR, JD_PWM_BASEADDR, CLK_FREQ, PWM_PER);
   DHB2_begin(&pmodDHB2, JE_GPIO_BASEADDR, JE_PWM_BASEADDR, CLK_FREQ, PWM_PER);

   // disable the 2 hbridges
   DHB1_motorDisable(&pmodDHB1);
   DHB2_motorDisable(&pmodDHB2);
}


void Cleanup() {
   DisableCaches();
}

void drive_forward() { // case to move all 4 motors forward
   DHB1_setMotorSpeeds(&pmodDHB1, 99, 99);		// JD = motor 1 top, motor 2 bottom
   DHB2_setMotorSpeeds(&pmodDHB2, 99, 99);		// JE = motor 3 top, motor 4 bottom

   DHB1_motorDisable(&pmodDHB1); // Disable PWM before changing direction
   DHB2_motorDisable(&pmodDHB2); // short circuit possible otherwise
   usleep(6);

   DHB1_setDirs(&pmodDHB1, 0, 0); // Set DIR A and B motors forward (left side)
   DHB2_setDirs(&pmodDHB2, 1, 1); // Set DIR C and D motors forward (right side)

   usleep(6);
   DHB1_motorEnable(&pmodDHB1);	// Motors A and B
   DHB2_motorEnable(&pmodDHB2);	// Motors C and D

   xil_printf("\nRunning 4 motors forward\n");
   usleep(6);
   DHB1_motorDisable(&pmodDHB1);
   DHB2_motorDisable(&pmodDHB2);
}

void drive_back() { // case to move all 4 motors backward
   DHB1_setMotorSpeeds(&pmodDHB1, 99, 99);		// JD = motor 1 top, motor 2 bottom
   DHB2_setMotorSpeeds(&pmodDHB2, 99, 99);		// JE = motor 3 top, motor 4 bottom

   DHB1_motorDisable(&pmodDHB1); // Disable PWM before changing direction
   DHB2_motorDisable(&pmodDHB2); // short circuit possible otherwise
   usleep(6);
   DHB1_setDirs(&pmodDHB1, 1, 1); // Set DIR A and B motors backwards (left side)
   DHB2_setDirs(&pmodDHB2, 0, 0); // Set DIR C and D motors backwards (right side)
   usleep(6);

   DHB1_motorEnable(&pmodDHB1);	// Motors A and B
   DHB2_motorEnable(&pmodDHB2);	// Motors C and D

   xil_printf("\nRunning 4 motors backward\n");
   usleep(6);
   DHB1_motorDisable(&pmodDHB1);
   DHB2_motorDisable(&pmodDHB2);
}

/*move C and D motors at max speed forward to turn left,
 * AND also move A and B at MAX speed but in opposite direction backward
 * to help with torque and turn better	*/
void drive_left() {
   DHB1_setMotorSpeeds(&pmodDHB1, 99, 99);		// JD = motor 1 top, motor 2 bottom
   DHB2_setMotorSpeeds(&pmodDHB2, 99, 99);		// JE = motor 3 top, motor 4 bottom

   DHB1_motorDisable(&pmodDHB1); // Disable PWM before changing direction
   DHB2_motorDisable(&pmodDHB2); // short circuit possible otherwise
   usleep(6);
   DHB1_setDirs(&pmodDHB1, 1, 1); // Set DIR A and B motors back (left side)
   DHB2_setDirs(&pmodDHB2, 1, 1); // Set DIR C and D motors forward (right side)
   usleep(6);

   DHB1_motorEnable(&pmodDHB1);	// Motors A and B
   DHB2_motorEnable(&pmodDHB2);	// Motors C and D

   xil_printf("\nRunning 4 motors left\n");
   usleep(6);
   DHB1_motorDisable(&pmodDHB1);
   DHB2_motorDisable(&pmodDHB2);
}

/*move A and B motors forward at max speed to turn right,
 * AND still move C and D backwards at max speed to help with torque
 * and turn better	*/
void drive_right() {
   DHB1_setMotorSpeeds(&pmodDHB1, 99, 99);		// JD = motor 1 top, motor 2 bottom
   DHB2_setMotorSpeeds(&pmodDHB2, 99, 99);		// JE = motor 3 top, motor 4 bottom

   DHB1_motorDisable(&pmodDHB1); // Disable PWM before changing direction
   DHB2_motorDisable(&pmodDHB2); // short circuit possible otherwise
   usleep(6);

   DHB1_setDirs(&pmodDHB1, 0, 0); // Set DIR A and B motors forward (left side)
   DHB2_setDirs(&pmodDHB2, 0, 0); // Set DIR C and D motors back (right side)

   usleep(6);

   DHB1_motorEnable(&pmodDHB1);	// Motors A and B
   DHB2_motorEnable(&pmodDHB2);	// Motors C and D

   xil_printf("\nRunning 4 motors right\n");
   usleep(6);
   DHB1_motorDisable(&pmodDHB1);
   DHB2_motorDisable(&pmodDHB2);
}

void stop_moving(){
	usleep(6);
	xil_printf("\nMOTORS ON\n");
	DHB1_motorDisable(&pmodDHB1);
	DHB2_motorDisable(&pmodDHB2);
}

void kill_switch(){
	usleep(6);
	DHB1_motorDisable(&pmodDHB1);
	DHB2_motorDisable(&pmodDHB2);
	xil_printf("\nALL MOTORS DISABLED\n");
	sleep(45);	// virtual kill switch ~ kill motors for 45 seconds only, then run stop_moving
}

void EnableCaches() {
   Xil_DCacheEnable();
}

void DisableCaches() {
   Xil_DCacheDisable();
}
