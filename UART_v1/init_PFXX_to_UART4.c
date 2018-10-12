// Listing B.1. C Function that Initializes PF12 and PF13 to UART 4

void uart_init(unsigned int baud, int parity)
{
// The next two statements map the PPS IO pins to the UART 4 Tx and Rx
   RPF12R = 0x02;  // Mapping U4TX to RPF12;
   U4RXR = 0x09;   // Mapping U4RX to RPF13

   UARTConfigure(UART4, UART_ENABLE_PINS_TX_RX_ONLY );
   UARTSetDataRate(UART4, GetPeripheralClock(), baud);	// Set UART data rate
// Note the need to specify the UART number twice in the following statement
   UARTEnable(UART4, UART_ENABLE_FLAGS(UART_ENABLE | UART4 | UART_RX | UART_TX));  
   switch(parity)
   {
       case NO_PARITY:
           UARTSetLineControl(UART4, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE |\
                              UART_STOP_BITS_1);
           break;
       case ODD_PARITY:
           UARTSetLineControl(UART4, UART_DATA_SIZE_8_BITS | UART_PARITY_ODD |\  
                              UART_STOP_BITS_1);
           break;
       case EVEN_PARITY:
           UARTSetLineControl(UART4, UART_DATA_SIZE_8_BITS | UART_PARITY_EVEN |\
                              UART_STOP_BITS_1);
           break;
   }
   printf("\n\rUART Serial Port 4 ready\n\n\r");