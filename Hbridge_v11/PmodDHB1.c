/******************************************************************************/
/*                                                                            */
/* PmodDHB1.h -- Driver definitions for the PmodDHB1 and PmodDHB2             */
/*                                                                            */
/******************************************************************************/
/* Author: Miguel Garcia-Barrera                                              */

/************ Include Files ************/

#include "PmodDHB1.h"
#include "PWM.h"
#include "xil_io.h"


/************ Macro Definition ************/

#define DHB1_GPIO_CHANNEL_OFFSET 0x8


/************ Function Definitions ************/

/*
 * void DHB1_begin(PmodDHB1 *InstancePtr, u32 GPIO_Address, u32 PWM_Address,
 *       u32 Clk_Freq, u32 PWM_Per)
 * -----------------------------------------------------------------------------
 * Parameters:
 *    InstancePtr:  Pointer to a PmodDHB1 object to initialize
 *    GPIO_Address: Base address of PmodDHB1 GPIO registers
 *    PWM_Address:  Base address of PmodDHB1 PWM registers
 *    Clk_Freq:     Clock frequency of the DHB1 IP in Hertz (Hz)
 *    PWM_Per:      Period of DHB1 Enable signal in milliseconds (ms)
 *
 * Return:
 *    void
 *
 * Description:
 *    Initialize the PmodDHB1 IP
 */
void DHB1_begin(PmodDHB1 *InstancePtr, u32 GPIO_Address, u32 PWM_Address,
      u32 Clk_Freq, u32 PWM_Per) {
   InstancePtr->GPIO_addr = GPIO_Address;
   InstancePtr->PWM_addr  = PWM_Address;
   InstancePtr->PWM_per   = Clk_Freq * PWM_Per / 1000;

   PWM_Set_Period(PWM_Address, InstancePtr->PWM_per);
   DHB1_motorDisable(InstancePtr);
}

void DHB2_begin(PmodDHB2 *InstancePtr, u32 GPIO_Address, u32 PWM_Address,
      u32 Clk_Freq, u32 PWM_Per) {
   InstancePtr->GPIO_addr = GPIO_Address;
   InstancePtr->PWM_addr  = PWM_Address;
   InstancePtr->PWM_per   = Clk_Freq * PWM_Per / 1000;

   PWM_Set_Period(PWM_Address, InstancePtr->PWM_per);
   DHB2_motorDisable(InstancePtr);
}

void DHB3_begin(PmodDHB3 *InstancePtr, u32 GPIO_Address, u32 PWM_Address,
      u32 Clk_Freq, u32 PWM_Per) {
   InstancePtr->GPIO_addr = GPIO_Address;
   InstancePtr->PWM_addr  = PWM_Address;
   InstancePtr->PWM_per   = Clk_Freq * PWM_Per / 1000;

   PWM_Set_Period(PWM_Address, InstancePtr->PWM_per);
   DHB3_motorDisable(InstancePtr);
}

/*
 * void DHB1_motorEnable(PmodDHB1 *InstancePtr)
 * void DHB1_motorDisable(PmodDHB1 *InstancePtr)
 * -----------------------------------------------------------------------------
 * Parameters:
 *    InstancePtr: Pointer to a PmodDHB1 object to enable/disable motors of
 *
 * Return:
 *    void
 *
 * Description:
 *    Enable or disable the motors of the PmodDHB1
 */
void DHB1_motorEnable(PmodDHB1 *InstancePtr) {
   PWM_Enable(InstancePtr->PWM_addr);
}

void DHB2_motorEnable(PmodDHB2 *InstancePtr) {
   PWM_Enable(InstancePtr->PWM_addr);
}

void DHB3_motorEnable(PmodDHB3 *InstancePtr) {
   PWM_Enable(InstancePtr->PWM_addr);
}
//////////////////////////////
//////////////////////////////
void DHB1_motorDisable(PmodDHB1 *InstancePtr) {
   PWM_Disable(InstancePtr->PWM_addr);
}

void DHB2_motorDisable(PmodDHB2 *InstancePtr) {
   PWM_Disable(InstancePtr->PWM_addr);
}

void DHB3_motorDisable(PmodDHB3 *InstancePtr) {
   PWM_Disable(InstancePtr->PWM_addr);
}

/*
 * void DHB1_setDirs(PmodDHB1 *InstancePtr, u8 dir1, u8 dir2)
 * void DHB1_setDir1(PmodDHB1 *InstancePtr, u8 dir1)
 * void DHB1_setDir2(PmodDHB1 *InstancePtr, u8 dir2)
 * -----------------------------------------------------------------------------
 * Parameters:
 *    InstancePtr: Pointer to a PmodDHB1 object to write motor directions to
 *    dir1:        8-bit value for motor1 direction, LSB determines direction
 *    dir2:        8-bit value for motor2 direction, LSB determines direction
 *
 * Return:
 *    void
 *
 * Description:
 *    Set direction pins for
 *     - Both motors
 *     - Motor1 only
 *     - Motor2 only
 *    The LSB of dir1 and dir2 determine the value of the DIR1 and DIR2 pins
 */
void DHB1_setDirs(PmodDHB1 *InstancePtr, u8 dir1, u8 dir2) {
   DHB1_setDir1(InstancePtr, dir1);
   DHB1_setDir2(InstancePtr, dir2);
}

void DHB2_setDirs(PmodDHB2 *InstancePtr, u8 dir1, u8 dir2) {
   DHB2_setDir1(InstancePtr, dir1);
   DHB2_setDir2(InstancePtr, dir2);
}

void DHB3_setDirs(PmodDHB3 *InstancePtr, u8 dir1, u8 dir2) {
   DHB3_setDir1(InstancePtr, dir1);
   DHB3_setDir2(InstancePtr, dir2);
}

//////////////////////////////
//////////////////////////////
void DHB1_setDir1(PmodDHB1 *InstancePtr, u8 dir1) {
   Xil_Out8(InstancePtr->GPIO_addr, dir1);
}

void DHB1_setDir2(PmodDHB1 *InstancePtr, u8 dir2) {
   Xil_Out8(InstancePtr->GPIO_addr + DHB1_GPIO_CHANNEL_OFFSET, dir2);
}

/*
 */////////////////////////////////////////////////// */
void DHB2_setDir1(PmodDHB2 *InstancePtr, u8 dir1) {
   Xil_Out8(InstancePtr->GPIO_addr, dir1);
}

void DHB2_setDir2(PmodDHB2 *InstancePtr, u8 dir2) {
   Xil_Out8(InstancePtr->GPIO_addr + DHB1_GPIO_CHANNEL_OFFSET, dir2);
}


/////

void DHB3_setDir1(PmodDHB3 *InstancePtr, u8 dir1) {
   Xil_Out8(InstancePtr->GPIO_addr, dir1);
}

void DHB3_setDir2(PmodDHB3 *InstancePtr, u8 dir2) {
   Xil_Out8(InstancePtr->GPIO_addr + DHB1_GPIO_CHANNEL_OFFSET, dir2);
}


/*
 * void DHB1_setMotorSpeeds(PmodDHB1 *InstancePtr, u8 m1, u8 m2)
 * void DHB1_setMotor1Speed(PmodDHB1 *InstancePtr, u8 m1)
 * void DHB1_setMotor2Speed(PmodDHB1 *InstancePtr, u8 m2)
 * -----------------------------------------------------------------------------
 * Parameters:
 *    InstancePtr: Pointer to a PmodDHB1 object to set motor speeds for
 *    m1:          8-bit number from 0 to 100 for motor1 speed
 *    m2:          8-bit number from 0 to 100 for motor2 speed
 *
 * Return:
 *    void
 *
 * Description:
 *    Set the duty cycle for one or both motors' enable signals. 0 turns off
 *    the motor(s), 100 for max speed
 */
void DHB1_setMotorSpeeds(PmodDHB1 *InstancePtr, u8 m1, u8 m2) {
   DHB1_setMotor1Speed(InstancePtr, m1);
   DHB1_setMotor2Speed(InstancePtr, m2);
}

void DHB2_setMotorSpeeds(PmodDHB2 *InstancePtr, u8 m1, u8 m2) {
   DHB2_setMotor1Speed(InstancePtr, m1);
   DHB2_setMotor2Speed(InstancePtr, m2);
}

void DHB3_setMotorSpeeds(PmodDHB3 *InstancePtr, u8 m1, u8 m2) {
   DHB3_setMotor1Speed(InstancePtr, m1);
   DHB3_setMotor2Speed(InstancePtr, m2);
}

///////////////////////////
///////////////////////////

void DHB1_setMotor1Speed(PmodDHB1 *InstancePtr, u8 m1) {
   double duty_cycle = m1 / 100.0;
   u32 PWM_addr = InstancePtr->PWM_addr;
   u32 PWM_per  = InstancePtr->PWM_per;
   PWM_Set_Duty(PWM_addr, (u32) (duty_cycle * PWM_per), 0);
}

void DHB1_setMotor2Speed(PmodDHB1 *InstancePtr, u8 m2) {
   double duty_cycle = m2 / 100.0;
   u32 PWM_addr = InstancePtr->PWM_addr;
   u32 PWM_per  = InstancePtr->PWM_per;
   PWM_Set_Duty(PWM_addr, (u32) (duty_cycle * PWM_per), 1);
}



void DHB2_setMotor1Speed(PmodDHB2 *InstancePtr, u8 m1) {
   double duty_cycle = m1 / 100.0;
   u32 PWM_addr = InstancePtr->PWM_addr;
   u32 PWM_per  = InstancePtr->PWM_per;
   PWM_Set_Duty(PWM_addr, (u32) (duty_cycle * PWM_per), 0);
}

void DHB2_setMotor2Speed(PmodDHB2 *InstancePtr, u8 m2) {
   double duty_cycle = m2 / 100.0;
   u32 PWM_addr = InstancePtr->PWM_addr;
   u32 PWM_per  = InstancePtr->PWM_per;
   PWM_Set_Duty(PWM_addr, (u32) (duty_cycle * PWM_per), 1);
}



void DHB3_setMotor1Speed(PmodDHB3 *InstancePtr, u8 m1) {
   double duty_cycle = m1 / 100.0;
   u32 PWM_addr = InstancePtr->PWM_addr;
   u32 PWM_per  = InstancePtr->PWM_per;
   PWM_Set_Duty(PWM_addr, (u32) (duty_cycle * PWM_per), 0);
}

void DHB3_setMotor2Speed(PmodDHB3 *InstancePtr, u8 m2) {
   double duty_cycle = m2 / 100.0;
   u32 PWM_addr = InstancePtr->PWM_addr;
   u32 PWM_per  = InstancePtr->PWM_per;
   PWM_Set_Duty(PWM_addr, (u32) (duty_cycle * PWM_per), 1);
}

