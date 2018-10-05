/*****************************************************
The purpose of this program is to control the duty cycle of the PWM controller designed in hardware
The PWM window size can be changed in the IP graphic interface
See documentation for reference

Terminal Settings:
   -Baud: 115200
   -Data bits: 8
   -Parity: no
   -Stop bits: 1

4/30/8 Final Iteration
Author: Miguel Garcia-Barrera
ECEN 403 - Battlebot

Run as --> Launch on Hardware (SDB)
****************************************************/

#include "xparameters.h"
#include "xil_io.h"
#include "sleep.h"
#include <xgpio.h>
#include "platform.h"
#include <stdio.h>

#define MY_PWM 0x43C00000 //This value is found in the Address editor tab in Vivado (next to Diagram tab)

int main(){
    int duty_cycle=0;
    int i;

    XGpio input;
    int button_data = 0;
    int switch_data = 0;

    xil_printf("\nApplication started.... \r \n");

    XGpio_Initialize(&input, XPAR_AXI_GPIO_0_DEVICE_ID);	//initialize input XGpio variable

    /*
        * XGpio_SetDataDirection(XGpio *InstancePtr, unsigned Channel, u32 DirectionMask);
        *  "InstancePtr" is a pointer to an XGpio instance to be worked
        *  "unsigned Channel" contains the channel of the GPIO (1 or 2) to operate on
        *  "u32 DirectionMask" is a bitmask specifying which discretes are input and which are output.
        *   Bits set to 0 are output and bits set to 1 are input.
        *   See Vivado AXI_GPIO blocks to identify channel:
        *   Channel 1 = buttons
        *   Channel 2 = switches
        *   0xF means input
        */
     XGpio_SetDataDirection(&input, 1, 0xF);
     XGpio_SetDataDirection(&input, 2, 0xF);

     init_platform();

    while(1){
    	// read input from channel 1 of axi_gpio_0 (buttons) and pass that data to "button_data" variable
    	button_data = XGpio_DiscreteRead(&input, 1);
    	switch_data = XGpio_DiscreteRead(&input, 2); // read switch input and store data in variable

 /*
    	if(switch_data == 0b0000){
        	duty_cycle = 0;
        	Xil_Out32(MY_PWM, duty_cycle);
        	Xil_Out32((MY_PWM+4), duty_cycle);
        	Xil_Out32((MY_PWM+8), duty_cycle);
        	Xil_Out32((MY_PWM+12), duty_cycle);
        	usleep(200000);   // implement interrupt?
        	xil_printf("Value of speed: %d \r \n",duty_cycle);
        }
*/
    	if(switch_data == 0b0001){
        	duty_cycle = 5;
        	Xil_Out32(MY_PWM, duty_cycle);
        	Xil_Out32((MY_PWM+4), duty_cycle);
        	Xil_Out32((MY_PWM+8), duty_cycle);
        	Xil_Out32((MY_PWM+12), duty_cycle);
        	usleep(200000);
        	xil_printf("Speed fixed: %d \r \n",duty_cycle);
        }

        else if(switch_data == 0b0010){

        	duty_cycle = 20;
        	Xil_Out32(MY_PWM, duty_cycle);
            Xil_Out32((MY_PWM+4), duty_cycle);
       	    Xil_Out32((MY_PWM+8), duty_cycle);
        	Xil_Out32((MY_PWM+12), duty_cycle);
        	usleep(200000);
        	xil_printf("Speed fixed: %d \r \n",duty_cycle);
        }

        else if(switch_data == 0b0100){

        	duty_cycle = 50;
        	Xil_Out32(MY_PWM, duty_cycle);
            Xil_Out32((MY_PWM+4), duty_cycle);
       	    Xil_Out32((MY_PWM+8), duty_cycle);
        	Xil_Out32((MY_PWM+12), duty_cycle);
        	usleep(200000);
        	xil_printf("Speed fixed: %d \r \n",duty_cycle);
        }

        else if(switch_data == 0b1000){

        	duty_cycle = 500;
        	Xil_Out32(MY_PWM, duty_cycle);
            Xil_Out32((MY_PWM+4), duty_cycle);
       	    Xil_Out32((MY_PWM+8), duty_cycle);
        	Xil_Out32((MY_PWM+12), duty_cycle);
        	usleep(200000);
        	xil_printf("Speed fixed: %d \r \n",duty_cycle);
        }

        else if(button_data == 0b0001){ // button 0 (R18) pressed
           duty_cycle=duty_cycle+1;
           // increase speed more quickly (controlled with usleep fxn)
      	   Xil_Out32(MY_PWM, duty_cycle);
      	   Xil_Out32((MY_PWM+4), duty_cycle);
      	   Xil_Out32((MY_PWM+8), duty_cycle);
      	   Xil_Out32((MY_PWM+12), duty_cycle);
      	   usleep(200);
      	   xil_printf("Accelerating: %d \r \n",duty_cycle);
      	   	   if(duty_cycle == 1024){
      	   		   xil_printf("MAX speed reached - STOP acceleration \r \n");
      	   		   sleep(3);   // instead of delay, use "getchar" version of pause
      	   	   }
        }

        else if(button_data == 0b0010){ // button 1 (P16) pressed
           duty_cycle++;
           // increase speed very slowly (controlled with usleep fxn)
      	   Xil_Out32(MY_PWM, duty_cycle);
      	   Xil_Out32((MY_PWM+4), duty_cycle);
      	   Xil_Out32((MY_PWM+8), duty_cycle);
      	   Xil_Out32((MY_PWM+12), duty_cycle);
      	   usleep(200000);
      	   xil_printf("Accelerating: %d \r \n",duty_cycle);
      	   	   if(duty_cycle == 1024){
      	   		   xil_printf("MAX speed reached - STOP acceleration \r \n");
      	   		   sleep(3);
      	   	   }
        }

        else if(button_data == 0b0100){ // button 2 (V16) pressed
           duty_cycle--;
           //  decrease speed very slowly
      	   Xil_Out32(MY_PWM, duty_cycle);
      	   Xil_Out32((MY_PWM+4), duty_cycle);
      	   Xil_Out32((MY_PWM+8), duty_cycle);
      	   Xil_Out32((MY_PWM+12), duty_cycle);
      	   usleep(200000);
      	   xil_printf("Slowing down: %d \r \n",duty_cycle);
  	   	   	   if(duty_cycle == 0){
  	   	   		   xil_printf("MIN speed reached - STOP deceleration or go in reverse \r \n");
  	   	   		   sleep(2);
  	   	   	   }
        }

        else if(button_data == 0b1000){ // button 3 (Y16) pressed
           duty_cycle--;
           //  decrease speed more quickly
      	   Xil_Out32(MY_PWM, duty_cycle);
      	   Xil_Out32((MY_PWM+4), duty_cycle);
      	   Xil_Out32((MY_PWM+8), duty_cycle);
      	   Xil_Out32((MY_PWM+12), duty_cycle);
      	   usleep(200);
      	   xil_printf("Slowing down: %d \r \n",duty_cycle);
  	   	   	   if(duty_cycle == 0){
  	   	   		   xil_printf("MIN speed reached - STOP deceleration or go in reverse \r \n");
  	   	   		   sleep(2);
  	   	   	   }
        }

    	// The Kill Switch = all switches HIGH
        else if(switch_data == 0b1111){
           duty_cycle = 0;
      	   Xil_Out32(MY_PWM, duty_cycle);
      	   Xil_Out32((MY_PWM+4), duty_cycle);
      	   Xil_Out32((MY_PWM+8), duty_cycle);
      	   Xil_Out32((MY_PWM+12), duty_cycle);
      	   usleep(200);
      	   xil_printf("Kill Switch Triggered - Shutting down \r \n");
  	   	   sleep(40);
        }

        else {
        	usleep(200000);
        	xil_printf("AUTOPILOT ON - speed : %d \r \n",duty_cycle); // add on? run highest speed
        }


        // frequency = 65k. Then, to fully go over 1 period in a certain time t,
        // multiply time t in seconds times 65k.
        // example, if desired completion of cycle is 5 seconds,
        // (5s)*(65k) = 325k
        for(i=0;i<800000; i++);  // repeat "else" statement i times, thus increasing PWM LEDs brightness

        // control the speed of the increase in brightness via the usleep function, which takes microseconds
        // 10k gives loop duration approx 10seconds (why?)
        //usleep(10000);

}
    cleanup_platform();
    return 0;
}
