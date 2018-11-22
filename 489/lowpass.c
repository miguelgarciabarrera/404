// low pass filter code
// read noisy ECG and PPG data from PC using UART and MATLAB exe
// apply filter
// send results back to MATLABexe using UART
// also calculate time processing for each sample
// cd c:\Program Files\DataTransfer\application
#include <msp430.h>

int ECG_filtered = 0;
int PPG_filtered = 0;
int ECG_noisy = 0;
int PPG_noisy = 0;
int byte_number = 0;

// lines below from code given
//History of outputs for low pass filter for each channel
#define ORDER 6
double xValues[ORDER+1] = {0};
double yValues[ORDER+1] = {0};


static double aCoeff[] = {1,-4.65261339186391,9.14112947874648,-9.68872072337630,5.83446402896588,-1.89059959898871,0.257315928152397};
static double bCoeff[] = {1.52456505599146e-05,9.14739033594878e-05,0.000228684758398720,0.000304913011198293,0.000228684758398720,9.14739033594878e-05,1.52456505599146e-05};

double lowPassFilter();

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; //Stop WDT

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

    // clock selection and more UART configuration
    UCA0CTL1 |= UCSSEL_1;                     // ACLK = 32768Hz
    UCA0BR0 = 3;                              // baud rate 9600 - from example code
    UCA0BR1 = 0;                              // baud rate 9600
    //UCA0MCTL = UCBRS2 + UCBRS0;               // Modulation UCBRSx = 5 - activate this instead of below?
    UCA0MCTL = UCBRS1 + UCBRS0;                 // Modulation UCBRSx = 3
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt

    // filter data using function provided

    // convert data to integer


    // send data using UCA0TXBUF
    __delay_cycles(1000);
    while (!UCA0TXIFG);                // USCI_A0 TX buffer ready?
    UCA0TXBUF = ECG_filtered>>8;           // shift bits to fill the whole package

    __delay_cycles(1000); // must have delay, otherwise double while loop needed

    while (!UCA0TXIFG);
    UCA0TXBUF = ECG_filtered;

    __delay_cycles(1000);      // lower if signal is squary

    // send PPG now 2 bytes
    while (!UCA0TXIFG);
    UCA0TXBUF = PPG_filtered>>8;

    __delay_cycles(1000);

    while (!UCA0TXIFG);
    UCA0TXBUF = PPG_filtered;

    __delay_cycles(1000);

    // also need to send time
    __bis_SR_register(GIE);
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
   // receive data with UCA0RXBUF
    byte_number++;

    switch(byte_number)
    {
    case 1:
        //receive byte 1 of ECG
        ECG_noisy = UCA0RXBUF>>8;
        break;

    case 2:
        //receive byte 2 of ECG
        ECG_noisy = UCA0RXBUF;
        break;

    case 3:
        //receive byte 1 of PPG
        PPG_noisy = UCA0RXBUF>>8;
        break;

    case 4:
        // receive byte 2 of PPG
        PPG_noisy = UCA0RXBUF;
        byte_number = 0; // reset byte number to start case 1 again
        break;
    }
    //__delay_cycles(1000);
}


//takes double, filters it using a low pass filter
// and outputs a filtered double
double lowPassFilter(double input)
{
    int i;
    int n;
    for(i = 0; i<ORDER; i++)
    {
        xValues[i] = xValues[i+1];
        yValues[i] = yValues[i+1];
    }

    xValues[ORDER] = input;
    yValues[ORDER] = xValues[ORDER] * bCoeff[0];

    for (n = 2; n <= ORDER+1; n++)
    {
        yValues[ORDER] = yValues[ORDER] + xValues[ORDER - n + 1] * bCoeff[n - 1] - yValues[ORDER - n + 1] * aCoeff[n - 1];
    }

    return yValues[ORDER];
}



