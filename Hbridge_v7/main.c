/******************************************************************************/
/*                                                                            */
/* main.c -- Example program using the PmodDHB1 IP                            */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
/* File Description:                                                          */
/*                                                                            */
/* This demo drives 2 motors in the 4 possible directions. When mounted on a  */
/* 2-wheel chassis, the motors will be driven such that the robot goes        */
/* forward, goes backward, turns left, and turns right.                       */
/*                                                                            */


#include "MotorFeedback.h"
#include "PmodDHB1.h"
#include "PWM.h"
#include "sleep.h"
#include "xil_cache.h"
#include "xparameters.h"


/************ Macro Definitions ************/

//PMOD JD
#define JD_GPIO_BASEADDR     XPAR_PMODDHB1_0_AXI_LITE_GPIO_BASEADDR
#define JD_PWM_BASEADDR      XPAR_PMODDHB1_0_PWM_AXI_BASEADDR
//#define JD_MOTOR_FB_BASEADDR XPAR_PMODDHB1_0_MOTOR_FB_AXI_BASEADDR

//PMOD JE
#define JE_GPIO_BASEADDR     XPAR_PMODDHB1_1_AXI_LITE_GPIO_BASEADDR
#define JE_PWM_BASEADDR      XPAR_PMODDHB1_1_PWM_AXI_BASEADDR


#ifdef __MICROBLAZE__	// ifdef and endif pair. define below used if found in xparamters.h
#define CLK_FREQ XPAR_CPU_M_AXI_DP_FREQ_HZ
#else
//#define CLK_FREQ XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ // FCLK0 frequency not found in xparameters.h
#define CLK_FREQ 1000000
#endif

#define PWM_PER              2
//#define SENSOR_EDGES_PER_REV 4
//#define GEARBOX_RATIO        48


/************ Function Prototypes ************/

void DemoInitialize();

void DemoRun();

void DemoCleanup();

//void drive(int16_t sensor_edges);

void EnableCaches();

void DisableCaches();


/************ Global Variables ************/

PmodDHB1 pmodDHB1;
PmodDHB2 pmodDHB2;
//MotorFeedback motorFeedback;


/************ Function Definitions ************/

int main(void) {
   DemoInitialize();

   while(1){
   DemoRun();
   }

   DemoCleanup();
   return 0;
}

void DemoInitialize() {
   EnableCaches();

   //init 2 motors
   DHB1_begin(&pmodDHB1, JD_GPIO_BASEADDR, JD_PWM_BASEADDR, CLK_FREQ, PWM_PER);
   DHB2_begin(&pmodDHB2, JE_GPIO_BASEADDR, JE_PWM_BASEADDR, CLK_FREQ, PWM_PER);

   /*
   MotorFeedback_init(
      &motorFeedback,
      MOTOR_FB_BASEADDR,
      CLK_FREQ,
      SENSOR_EDGES_PER_REV,
      GEARBOX_RATIO
   );
   */

   // disable 2 motors
   DHB1_motorDisable(&pmodDHB1);
   DHB2_motorDisable(&pmodDHB2);
}

void DemoRun() {
   // first parameter is low, second parameter is high.
   DHB1_setMotorSpeeds(&pmodDHB1, 95, 95);		// JD = motor 1 top, motor 2 bottom
   DHB2_setMotorSpeeds(&pmodDHB2, 95, 95);		// JE = motor 3 top, motor 4 bottom


  // MotorFeedback_clearPosCounter(&motorFeedback);

   DHB1_motorDisable(&pmodDHB1); // Disable PWM before changing direction
   DHB2_motorDisable(&pmodDHB2);
   usleep(6);                    // short circuit possible otherwise
   DHB1_setDirs(&pmodDHB1, 0, 1); // Set direction forward
   DHB2_setDirs(&pmodDHB2, 0, 1); // Set direction forward
   drive(); // Drive until sensor produces 240 positive edges

   usleep(2000);
   DHB1_motorDisable(&pmodDHB1);
   DHB2_motorDisable(&pmodDHB2);
   usleep(6);
   DHB1_setDirs(&pmodDHB1, 1, 0); // Set direction backward
   DHB2_setDirs(&pmodDHB2, 1, 0); // Set direction backward
   drive();

   usleep(2000);
   DHB1_motorDisable(&pmodDHB1);
   DHB2_motorDisable(&pmodDHB2);
   usleep(6);
   DHB1_setDirs(&pmodDHB1, 1, 1); // Set direction left
   DHB2_setDirs(&pmodDHB2, 1, 1); // Set direction left
   drive();

   usleep(2000);
   DHB1_motorDisable(&pmodDHB1);
   DHB2_motorDisable(&pmodDHB2);
   usleep(6);
   DHB1_setDirs(&pmodDHB1, 0, 0); // Set direction right
   DHB2_setDirs(&pmodDHB2, 0, 0); // Set direction right
   drive();
}

void DemoCleanup() {
   DisableCaches();
}

void drive() {
   DHB1_motorEnable(&pmodDHB1);
   DHB2_motorEnable(&pmodDHB2);

   for(int delay=0; delay<2500; delay++){
	   xil_printf("Value of speed: %d \r \n",delay);
	   	   if(delay == 2499){
	   		xil_printf("DONE WITH FIRST CYCLE");
	   		usleep(2000000);
	   	   }
   }

   usleep(2000000);
   DHB1_motorDisable(&pmodDHB1);
   DHB2_motorDisable(&pmodDHB2);
   usleep(2000000);
}

void EnableCaches() {
   Xil_DCacheEnable();
}

void DisableCaches() {
   Xil_DCacheDisable();
}
