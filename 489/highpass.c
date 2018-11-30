// cd c:\Program Files\DataTransfer\application

//part3
#include <msp430.h>
//High-Pass

signed int data;
signed int data1;
double time_double; //4 byte
int time_int;
int count = 0;
signed int ECG; //2 byte
signed int PPG;
double FiltECG; //4 byte
double FiltPPG;signed short filtPPG;
signed short filtECG; //2 byte


double xPrevHP = 0; //Previous input
double yPrevHP = 0; //Previous output

double delta_t = 1.0 / 205.6; //sample interval is 1/current sample rate
double cutoff = 0.9; //Cut-off frequency

double highPassFilter(double input); //declare function


int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; //Stop WDT
    P1DIR |= 0x01;                            // P1.0 output for overflow counter
    if(CALBC1_1MHZ==0xFF)
    {
        while(1);
    }

    //UART
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
    DCOCTL = CALDCO_1MHZ;
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2;

    //baud rate modulation
    UCA0CTL1 |= UCSSEL_2;                     // CLK = SMCLK
    UCA0BR0 = 104;//3                       // 3 = INT(3.41), 32768Hz/9600 = 3.41
    UCA0BR1 = 0;                           // 3.41-int(3.41) = 0.4133 ->1
    UCA0MCTL = UCBRS0;    //           // Modulation UCBRSx = 3
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;
    __bis_SR_register(GIE);                    // Enter LPM0, interrupts enabled

    //check if ECG, PPG data ready
     //  __bis_SR_register(LPM0_bits);       // Enter LPM0
          //LPM -> stop before filtering
          //ISR -> exit LPM after received 4 bytes

while(1){
     // apply filter
    if (count == 4) {
        //start timer
        //TA0CTL = TASSEL_1 + TAIE + MC_2;

        FiltECG = highPassFilter(ECG);
        FiltPPG = highPassFilter(PPG);

        filtECG = FiltECG;
        filtPPG = FiltPPG;
        count++;
    }
    if (count == 5) {
        //send data to PC
           while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
           UCA0TXBUF = filtECG;                    // shift bits to fill the whole package
           while (!(IFG2&UCA0TXIFG));
           UCA0TXBUF = filtECG>>8;
           while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
           UCA0TXBUF = filtPPG;                    // shift bits to fill the whole package
           while (!(IFG2&UCA0TXIFG));
           UCA0TXBUF = filtPPG>>8;

           /*while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
           UCA0TXBUF = ECG;                    // shift bits to fill the whole package
           while (!(IFG2&UCA0TXIFG));
           UCA0TXBUF = ECG>>8;
           while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
           UCA0TXBUF = PPG;                    // shift bits to fill the whole package
           while (!(IFG2&UCA0TXIFG));
           UCA0TXBUF = PPG>>8;*/
         //send computation time

           time_double =  ( (double) TAR  );
           time_int = (int) time_double;

           while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
           UCA0TXBUF = time_int;                    // shift bits to fill the whole package
           while (!(IFG2&UCA0TXIFG));
           UCA0TXBUF = time_int>>8;
           while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
           UCA0TXBUF = time_int>>16;                    // shift bits to fill the whole package
           while (!(IFG2&UCA0TXIFG));
           UCA0TXBUF = time_int>>24;

           count = 0;
    }
  }//end while
}//end main
// Echo back RXed character, confirm TX buffer is ready first
//UART ISR
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    //received data from P
    if (count == 0) {
           data = UCA0RXBUF; //RX = lsb
           ECG = data;
           TA0CTL = TASSEL_2  + ID_2 + MC_2 + TACLR; //start timer AND clear flag to start counting time
    }
    if (count == 1) {
           //ECG = data+UCA0RXBUF*2^8; // RX = msb
        data1 = UCA0RXBUF;
        ECG = (data1 << 8 ) +(data);
    }
    if (count == 2) {
           data = UCA0RXBUF;
           PPG = data;
    }
    if (count == 3) {
        data1 = UCA0RXBUF;
        PPG = (data1 << 8 ) +(data);
           //PPG = data+UCA0RXBUF*2^8;
           //exit LPM after received all data
           //__bic_SR_register_on_exit(CPUOFF);// Clear CPUOFF bit from 0(SR)
    }
           count++;
}



//takes double, filters it using a low pass filter
// and outputs a filtered double

double highPassFilter(double input)
{

    //calculate coefficents

    //perform HPF
    double alpha = cutoff / (cutoff + delta_t);
    double yout = alpha * (yPrevHP + input - xPrevHP);
    //update state variables
    xPrevHP = input;
    yPrevHP = yout;
    return yout;
}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  P1OUT ^= 0x01;                            // Toggle P1.0
}
