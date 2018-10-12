//Listing B.3. C Code for Sending a Single Character to UART 4

BOOL putcU4( int ch)
{
UART_DATA c;
BOOL done = FALSE;
   c.data8bit = (char) ch;
   if(UARTTransmitterIsReady(UART4))
   {
   	UARTSendDataByte(UART4, c.data8bit);
	done = TRUE;
   }
  return done;
} /* End of putU4 */