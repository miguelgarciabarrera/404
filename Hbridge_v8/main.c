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

#define GPIO_BASEADDR     XPAR_PMODDHB1_0_AXI_LITE_GPIO_BASEADDR
#define PWM_BASEADDR      XPAR_PMODDHB1_0_PWM_AXI_BASEADDR
#define MOTOR_FB_BASEADDR XPAR_PMODDHB1_0_MOTOR_FB_AXI_BASEADDR

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
   DHB1_begin(&pmodDHB1, GPIO_BASEADDR, PWM_BASEADDR, CLK_FREQ, PWM_PER);
   /*
   MotorFeedback_init(
      &motorFeedback,
      MOTOR_FB_BASEADDR,
      CLK_FREQ,
      SENSOR_EDGES_PER_REV,
      GEARBOX_RATIO
   );
   */
   DHB1_motorDisable(&pmodDHB1);
}

void DemoRun() {
   DHB1_setMotorSpeeds(&pmodDHB1, 95, 95);
   // first parameter is low ratio out of 100, second parameter is high.

  // MotorFeedback_clearPosCounter(&motorFeedback);

   DHB1_motorDisable(&pmodDHB1); // Disable PWM before changing direction,
   usleep(6);                    // short circuit possible otherwise
   DHB1_setDirs(&pmodDHB1, 0, 1); // Set direction 1 and 1
   drive(240); // Drive until sensor produces 240 positive edges
   usleep(2000);

   DHB1_motorDisable(&pmodDHB1);
   usleep(6);
   DHB1_setDirs(&pmodDHB1, 1, 0); // Set direction 1 and 1
   drive(240);
   usleep(2000);

   DHB1_motorDisable(&pmodDHB1);
   usleep(6);
   DHB1_setDirs(&pmodDHB1, 1, 1); // Set direction RIGHT for flags
   drive(120);
   usleep(2000);

   DHB1_motorDisable(&pmodDHB1);
   usleep(6);
   DHB1_setDirs(&pmodDHB1, 0, 0); // Set direction LEFT for flags
   drive(120);
}

void DemoCleanup() {
   DisableCaches();
}

void drive(int16_t i) {
   DHB1_motorEnable(&pmodDHB1);

   for(int delay=0; delay<2500; delay++){
	   xil_printf("Value of speed: %d \r \n",delay);
	   	   if(delay == 2499){
	   		xil_printf("DONE WITH FIRST CYCLE");
	   		usleep(2000000);
	   	   }
   }

   usleep(2000000);
   DHB1_motorDisable(&pmodDHB1);
   usleep(2000000);
}

void EnableCaches() {
   Xil_DCacheEnable();
}

void DisableCaches() {
   Xil_DCacheDisable();
}
