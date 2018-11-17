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

//headers for GPS
#include "PmodGPS.h"
#include "xil_cache.h"


/************ Macro Definitions ************/

//GPIO control
#define GPIO_BASEADDR     XPAR_PMODDHB1_0_AXI_LITE_GPIO_BASEADDR

//PMOD JD
#define JD_GPIO_BASEADDR     XPAR_PMODDHB1_0_AXI_LITE_GPIO_BASEADDR
#define JD_PWM_BASEADDR      XPAR_PMODDHB1_0_PWM_AXI_BASEADDR

//PMOD JE
#define JE_GPIO_BASEADDR     XPAR_PMODDHB1_1_AXI_LITE_GPIO_BASEADDR
#define JE_PWM_BASEADDR      XPAR_PMODDHB1_1_PWM_AXI_BASEADDR


//PMOD JC FOR WEAPON
#define JC_GPIO_BASEADDR	 XPAR_PMODDHB1_2_AXI_LITE_GPIO_BASEADDR
#define JC_PWM_BASEADDR		 XPAR_PMODDHB1_2_PWM_AXI_BASEADDR

#ifdef __MICROBLAZE__	// ifdef and endif pair. define below used if found in xparamters.h
#define CLK_FREQ XPAR_CPU_M_AXI_DP_FREQ_HZ
#else
//#define CLK_FREQ XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ // FCLK0 frequency not found in xparameters.h
#define CLK_FREQ 1000000
#endif

#define PWM_PER 2


// GPS
#ifdef XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ
#define PERIPHERAL_CLK 100000000 // FCLK0 frequency is not in xparameters.h for some reason
#else
#define PERIPHERAL_CLK XPAR_CPU_M_AXI_DP_FREQ_HZ
#endif

#ifdef XPAR_XINTC_NUM_INSTANCES
#include "xintc.h"
#define INTC         XIntc
#define INTC_HANDLER XIntc_InterruptHandler
#else
#ifdef XPAR_SCUGIC_0_DEVICE_ID
#include "xscugic.h"
#define INTC         XScuGic
#define INTC_HANDLER XScuGic_InterruptHandler
#endif
#endif

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
static u8 RecvBuffer[TEST_BUFFER_SIZE];	/* Buffer for Receiving Data */
static u8 SendBuffer[TEST_BUFFER_SIZE];	/* Buffer for Transmitting Data - change to char to debug */
int SendComplete = 1;
int ConsoleTXBufferIndex = 0;

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
void weapon_motor_drive();

//
void gpsInitialize();
void gpsRun();
void gps_data();

/************ Global Variables ************/

PmodDHB1 pmodDHB1;
PmodDHB2 pmodDHB2;
PmodDHB3 pmodDHB3;
XGpio input;
XUartPs Uart_PS;		/* Instance of the UART Device */

PmodGPS GPS;
INTC intc;


/************ Function Definitions ************/
int switch_data = 0;
int gps_index = 0;

int main(void) {

    // GPIO getting ready
	int button_data = 0;


	xil_printf("\nApplication started.... \r \n");

	XGpio_Initialize(&input, XPAR_AXI_GPIO_0_DEVICE_ID);	//initialize input XGpio variable

	/******************************************************************************/
	/******************************************************************************/

	// init UART
	XUartPs_Config *Config;
	unsigned int ReceivedCount;
	ReceivedCount = 0;

	/*
	 * Initialize the UART driver so that it's ready to use.
	 * Look up the configuration in the config table, then initialize it.
	*/
		Config = XUartPs_LookupConfig(XPAR_XUARTPS_0_DEVICE_ID); // changed param 10/25
		if (NULL == Config) {
			return XST_FAILURE;
		}

		XUartPs_CfgInitialize(&Uart_PS, Config, Config->BaseAddress);

	XUartPs_SetBaudRate(&Uart_PS, 9600);	// changed value
	XUartPs_SetOperMode(&Uart_PS, XUARTPS_OPER_MODE_NORMAL);

	// Init H-bridges
	Initialize_bridges();

	// turn on weapon motor
	weapon_motor_drive();

   // polling data from switches/push buttons to control motors
   while(1){

  	// read input from channel 1 of axi_gpio_0 (buttons) and pass that data to "button_data" variable
  	button_data = XGpio_DiscreteRead(&input, 1);
  	switch_data = XGpio_DiscreteRead(&input, 2); // read switch input and store data in variable

  	// receive buffer to control motors with XBEE
  	XUartPs_Recv(&Uart_PS, &RecvBuffer[ReceivedCount], (TEST_BUFFER_SIZE - ReceivedCount));

  	if(RecvBuffer[0] == '9'){
  		drive_left();
  	    usleep(6);
  		}

  	else if(RecvBuffer[0] == '1'){
  	  	drive_forward();
  	  	usleep(6);
  	  	}

  	else if(RecvBuffer[0] == '2'){
  	  	 drive_back();
  	  	 usleep(6);
  	  	 }

  	else if(RecvBuffer[0] == '3'){
  	  	 drive_right();
  	  	 usleep(6);
  	  	 }

  	else if(switch_data == 0b1111){
  	  	 kill_switch();
  	  	 usleep(6);
  	  	 }

  	else if(button_data == 0b0001){
  	  	 gps_data();
  	  	 usleep(6);
  	   	 }

  	else if(switch_data == 0b1001){
  	      SendBuffer[0] = 7;
  	      xil_printf("Sending to XBEE: %d \n", SendBuffer[0]);
  	      XUartPs_Send(&Uart_PS, (u8 *)&SendBuffer[0], 1);
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

   //init the 3 hbridges
   DHB1_begin(&pmodDHB1, JD_GPIO_BASEADDR, JD_PWM_BASEADDR, CLK_FREQ, PWM_PER);
   DHB2_begin(&pmodDHB2, JE_GPIO_BASEADDR, JE_PWM_BASEADDR, CLK_FREQ, PWM_PER);
   DHB3_begin(&pmodDHB3, JE_GPIO_BASEADDR, JE_PWM_BASEADDR, CLK_FREQ, PWM_PER);

   // disable the 3 hbridges
   DHB1_motorDisable(&pmodDHB1);
   DHB2_motorDisable(&pmodDHB2);
   DHB3_motorDisable(&pmodDHB3);
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


/*
 * function to drive weapon motor
 */
void weapon_motor_drive() {
   DHB3_setMotorSpeeds(&pmodDHB3, 99, 99);		// JC set pwm speed

   DHB3_motorDisable(&pmodDHB3); // Disable PWM before changing direction
   usleep(6);

   DHB3_setDirs(&pmodDHB3, 0, 0); // Set DIR of both pins forward

   usleep(6);

   DHB3_motorEnable(&pmodDHB3);	// Motors A and B

   xil_printf("\nRunning 4 motors right\n");
   usleep(6);
   //DHB3_motorDisable(&pmodDHB3);  // always ON if commented out
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
	DHB3_motorDisable(&pmodDHB3);
	xil_printf("\nALL MOTORS DISABLED\n");
	sleep(45);	// virtual kill switch ~ kill motors for 45 seconds only, then run stop_moving
}

void EnableCaches() {
   Xil_DCacheEnable();
}

void DisableCaches() {
   Xil_DCacheDisable();
}

void gpsInitialize() {
	   GPS_begin(&GPS, XPAR_PMODGPS_0_AXI_LITE_GPIO_BASEADDR, XPAR_PMODGPS_0_AXI_LITE_UART_BASEADDR, PERIPHERAL_CLK);
	   EnableCaches();
	#ifdef __MICROBLAZE__
	   // Set up interrupts for a MicroBlaze system
	   SetupInterruptSystem(&GPS, XPAR_INTC_0_DEVICE_ID,
			 XPAR_INTC_0_PMODGPS_0_VEC_ID);
	#else
	   // Set up interrupts for a Zynq system
	   SetupInterruptSystem(&GPS, XPAR_PS7_SCUGIC_0_DEVICE_ID, XPAR_FABRIC_PMODGPS_0_GPS_UART_INTERRUPT_INTR);
	#endif

	   GPS_setUpdateRate(&GPS, 1000);
	}


void gpsRun() {
   while (switch_data == 0b0000) {
      if (GPS.ping) {
         GPS_formatSentence(&GPS);
         if (GPS_isFixed(&GPS)) {
            xil_printf("Latitude: %s\n\r", GPS_getLatitude(&GPS));
            xil_printf("Longitude: %s\n\r", GPS_getLongitude(&GPS));
            xil_printf("Altitude: %s\n\r", GPS_getAltitudeString(&GPS));
            xil_printf("Number of Satellites: %d\n\n\r", GPS_getNumSats(&GPS));
            //int x = GPS_getLatitude(&GPS);
         } else {
            xil_printf("Number of Satellites: %d\n\r", GPS_getNumSats(&GPS));
         }

         // send TX
         //SendBuffer[0] = GPS_getLatitude(&GPS);
        // SendBuffer[1] = GPS_getNumSats(&GPS);
         gps_index = strlen(GPS_getLongitude(&GPS));

         SendBuffer[2] = GPS_getLongitude(&GPS);

         XUartPs_Send(&Uart_PS, (u8 *)&SendBuffer[2], 1);
         //XUartPs_Send(&Uart_PS, (u8 *)&SendBuffer[1], 1);

         GPS.ping = FALSE;
      }
   }
   DisableCaches();
}

void gps_data(){
	   gpsInitialize();
	   gpsRun();
}


int SetupInterruptSystem(PmodGPS *InstancePtr, u32 interruptDeviceID,
      u32 interruptID) {
   int Result;
   u16 Options;

#ifdef XPAR_XINTC_NUM_INSTANCES
   INTC *IntcInstancePtr = &intc;
   /*
    * Initialize the interrupt controller driver so that it's ready to use.
    * specify the device ID that was generated in xparameters.h
    */
   Result = XIntc_Initialize(IntcInstancePtr, interruptDeviceID);
   if (Result != XST_SUCCESS) {
      return Result;
   }

   /* Hook up interrupt service routine */
   XIntc_Connect(IntcInstancePtr, interruptID,
         (Xil_ExceptionHandler) XUartNs550_InterruptHandler,
         &InstancePtr->GPSUart);

   /* Enable the interrupt vector at the interrupt controller */

   XIntc_Enable(IntcInstancePtr, interruptID);

   /*
    * Start the interrupt controller such that interrupts are recognized
    * and handled by the processor
    */
   Result = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
   if (Result != XST_SUCCESS) {
      return Result;
   }
   XUartNs550_SetHandler(&InstancePtr->GPSUart, (void*)GPS_intHandler,
         InstancePtr);

   /*
    * Initialize the exception table and register the interrupt
    * controller handler with the exception table
    */
   Xil_ExceptionInit();

   Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
         (Xil_ExceptionHandler)INTC_HANDLER, IntcInstancePtr);

   /* Enable non-critical exceptions */
   Xil_ExceptionEnable();
   Options = XUN_OPTION_DATA_INTR |
   XUN_OPTION_FIFOS_ENABLE;
   XUartNs550_SetOptions(&InstancePtr->GPSUart, Options);

#endif
#ifdef XPAR_SCUGIC_0_DEVICE_ID
   INTC *IntcInstancePtr = &intc;
   XScuGic_Config *IntcConfig;

   /*
    * Initialize the interrupt controller driver so that it is ready to
    * use.
    */
   IntcConfig = XScuGic_LookupConfig(interruptDeviceID);
   if (NULL == IntcConfig) {
      return XST_FAILURE;
   }

   Result = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
         IntcConfig->CpuBaseAddress);
   if (Result != XST_SUCCESS) {
      return XST_FAILURE;
   }

   XScuGic_SetPriorityTriggerType(IntcInstancePtr, interruptID, 0xA0, 0x3);

   /*
    * Connect the interrupt handler that will be called when an
    * interrupt occurs for the device.
    */
   Result = XScuGic_Connect(IntcInstancePtr, interruptID,
         (Xil_ExceptionHandler) XUartNs550_InterruptHandler,
         &InstancePtr->GPSUart);
   if (Result != XST_SUCCESS) {
      return Result;
   }

   /*
    * Enable the interrupt for the GPIO device.
    */
   XScuGic_Enable(IntcInstancePtr, interruptID);

   XUartNs550_SetHandler(&InstancePtr->GPSUart, (void*) GPS_intHandler,
         InstancePtr);

   /*
    * Initialize the exception table and register the interrupt
    * controller handler with the exception table
    */
   Xil_ExceptionInit();

   Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
         (Xil_ExceptionHandler) INTC_HANDLER, IntcInstancePtr);

   /* Enable non-critical exceptions */
   Xil_ExceptionEnable()
   ;

   Options = XUN_OPTION_DATA_INTR | XUN_OPTION_FIFOS_ENABLE;
   XUartNs550_SetOptions(&InstancePtr->GPSUart, Options);
#endif

   return XST_SUCCESS;
}
