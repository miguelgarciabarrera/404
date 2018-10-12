//Listing B.2. C code to allow the “printf” output to be redirected to UART 4

void _mon_putc(char c)
{
   while(!UARTTransmitterIsReady(UART4));
   UARTSendDataByte(UART4, c);
} /* End of _mon_putc */