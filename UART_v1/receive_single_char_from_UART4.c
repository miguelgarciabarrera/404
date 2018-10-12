//Listing B.4. C Code for Receiving a Single Character from UART 4

BOOL getcU4( char *ch)
{
UART_DATA c;
BOOL done = FALSE;
   if(UARTReceivedDataIsAvailable(UART4))	/* wait for new char to arrive */
   {
       c = UARTGetData(UART4);	/* read the char from receive buffer */
       *ch = (c.data8bit);
       done = TRUE;		/* Return new data available flag */
   }
   return done;           /* Return new data not available flag */
}/* End of getU4 */