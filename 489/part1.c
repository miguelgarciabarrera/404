//10/17/18
// p1
#include <msp430.h>


int heartrate = 60;     // this is the third byte to be sent
int start = 0;
double heartRateDouble;
int threshold = 700;
int oldVoltage;
int currentVoltage;


int main(void)
{


    WDTCTL = WDTPW + WDTHOLD; //Stop WDT

    // Length of sample array.


    if(CALBC1_1MHZ==0xFF)
    {
        while(1);
    }

    //setup UART
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
    DCOCTL = CALDCO_1MHZ;
    P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2;



    // clock selection
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 8;                              // 1MHz 115200
    UCA0BR1 = 0;                              // 1MHz 115200
    //UCA0CTL1 |= UCSSEL_1;                        // ACLK
    //UCA0BR0 = 3;                              // 32768/9600 = 3.41 ~ 3
    //UCA0BR1 = 0;

    //UCA0MCTL = UCBRS2 + UCBRS0;               // Modulation UCBRSx = 5
    UCA0MCTL = UCBRS1 + UCBRS0;                 // Modulation UCBRSx = 3
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrup

    // setup ADC
    ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE; // ADC10ON, interrupt enabled ,
    ADC10CTL1 = INCH_3;                       // input from 1.3
    ADC10AE0 |= BIT3;

//    // setup timer below
      TA0CTL = TASSEL_1  + MC_2;    // ACLK + divide by 4 + count up to TACCR0
      //TA0CCTL0 = CCIE;    //enable interrupt

    __bis_SR_register(GIE);
}
#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
    __delay_cycles(1000);
    while (!UCA0TXIFG);                // USCI_A0 TX buffer ready?
    UCA0TXBUF = ADC10MEM>>8;                    // shift bits to fill the whole package

    __delay_cycles(1000); // must have delay, otherwise double while loop needed

    while (!UCA0TXIFG);
    UCA0TXBUF = ADC10MEM;

    __delay_cycles(1000);      // lower if signal is squary

    oldVoltage = currentVoltage;
    currentVoltage = ADC10MEM;
    if(currentVoltage > threshold && !start) {
        start = 1;
    }
    if(currentVoltage < oldVoltage && start ) {
        heartRateDouble = 60 * 32768.0 / ( (double) TAR  );
        heartrate = (int) heartRateDouble;
        start = 0;
        TAR = 0;
    }

    while(!UCA0TXIFG);
    UCA0TXBUF = heartrate;

    __delay_cycles(1000);

    ADC10CTL0 &= ~ADC10IFG;
    ADC10CTL0 |= ENC + ADC10SC;

}
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{          // USCI_A0 TX buffer ready?
    if(UCA0RXBUF == 'a') {
        ADC10CTL0 |= ENC + ADC10SC;
    }
}

// timer interrupt
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
    // reset counter
}




