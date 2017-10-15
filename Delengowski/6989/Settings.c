nonvolatile unsigned int num_of_bytes;			// Takes on the Value of Received Byte Zero (when byte_count = 0),
												// Stores Total Number of Bytes to be Received
nonvolatile unsigned int byte_count = 0; 		// For Counting Number of Bytes Received through RXBUF
nonvolatile unsigned int index_count = 0;		// For Assigning Received Bytes to messages[i] Array
nonvolatile unsigned int messages[80];		    // For Storing Received Bytes from RXBUF...at most we will receive 80 bytes,
												// Later Used for Transmitting Messages to TXBUF

#define red   TB0CCR3 							// For Controlling RED LED DUTY CYCLE
#define green TB0CCR4 							// For Controlling GREEN LED DUTY CYCLE
#define blue  TB0CCR5 							// For Controlling BLUE LED DUTY CYCLE
#define LED1ON P1OUT |= BIT0					//<comment on use>
#define LED1OFF P1OUT &= ~BIT0					//<comment on use>
#define LED2ON P9OUT |= BIT7					//<comment on use>
#define LED2OFF P9OUT &= ~BIT7					//<comment on use>
#define RECEIVED UCA0RXBUF						// Byte Received Buffer
#define TRANSMIT UCA0TXBUF						// Transmit Byte Buffer

void TimerB0Setup();
void UARTSetup();
void GPIOSetup();

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;   			    // Stop Watchdog
	PM5CTL0 &= ~LOCKLPM5;   				    // Disable the GPIO power-on default high-impedance mode to activate
			  								    // previously configured port settings

  	TimerB0Setup();
  	UARTSetup();
  	GPIOSetup;

	__bis_SR_register(LPM3_bits | GIE);       // Enter LPM3, interrupts enabled
	__no_operation();                         // For debugger
 	 
}

#pragma vector = TIMERB0_VECTOR				  // NOT SURE IF NEEDED, SKELETON LEFT JUST IN CASE
__interrupt void TB0_ISR(void)
{
	switch(TBIV)
	{
		case TB0IV_NONE   :
		{

			break;
		}           
		case TB0IV_TBCCR1 :
		{

			break;
		}           
		case TB0IV_TBCCR2 :
		{

			break;
		}           
		case TB0IV_TBCCR3 :
		{

			break;
		}           
		case TB0IV_TBCCR4 :
		{

			break;
		}           
		case TB0IV_TBCCR5 :
		{

			break;
		}           
		case TB0IV_TBCCR6 :
		{

			break;
		}           
		case TB0IV_TBIFG  :
		{

			break;
		}  
	}         
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	switch(__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG))
	{
	    case USCI_NONE: 
	    {
	    	break;
	    }
	    case USCI_UART_UCRXIFG:
	    {
	        LED1ON;														// Indicate We Are Receiving Bytes
	        if(byte_count == 0)											// Store First Byte Received, Has Number of Bytes to be Received
	        {
	            num_of_bytes = RECEIVED;								
	            byte_count++;
	        }
	        else if((byte_count > 0) && (byte_count <= 3))
	        {
	            switch(byte_count)
	            {
		            case 1:
		            {
		            	red = RECEIVED;									// Store RED LED Byte...should be 2nd byte, byte_count = 1
		                break;
		            }
		            case 2:
		            {
		            	green = RECEIVED;								// Store GREEN LED Byte...should be 3rd byte, byte_count = 2
		                break;
		            }   
		            case 3:
		            {
		            	blue = RECEIVED;								// Store BLUE LED Byte...should be 4th byte, byte_count = 3
		                break;
		            }
		            byte_count++;
	            }
	        }
	        else if((byte_count <= num_of_bytes) && (byte_count > 3))	// Bytes to be Transmitted
	        {
	            if(byte_count == 4)
	            {
	               index_count = 0;
	               messages[index_count] = num_of_bytes -3;				// Number of Bytes to eventually Transmit
	            }
	            index_count++;
	            messages[index_count] = RECEIVED;						// Storing RGB Bytes to be Transmitted
	            byte_count++;
	        }
	        else if(byte_count > num_of_bytes)
	        {
	            index_count = 0;										// Reset Array Index to Start Transmitting
	            byte_count = 0;											// Reset byte_count for Next Cycle of Receiving
	            LED1OFF;												// Indicate We have stopped receiving bytes
	        }
	      	__no_operation();
	      	break;
	    }
	    case USCI_UART_UCTXIFG:
	    {   
	    	LED2ON;														// Indicate Transmission Started
	        if(index_count <= num_of_bytes)
	        {
	            TRANSMIT = messages[index_count];						// Start Transmitting Bytes
	            index_count++;
	        }
	        else if(index_count > num_of_bytes)
	        {
	            LED2OFF;												// Indicate Transmission Ended
	            index_count = 0;										// Reset Index for Next Receive Cycle
	        }
	    	break;
	    }
	    case USCI_UART_UCSTTIFG: 
	    {
	    	break;
	    }
	    case USCI_UART_UCTXCPTIFG: 
	    {
	    	break;
	    }
	}
}

void TimerB0Setup()
{
	TB0CCR0 = 255;							  // Count Up To 
	TB0CTL |= TBSSEL_2 + MC_1 + TBCLR;		  // SMCLK (1MHz) Up Mode Clear Timer
	//TB0CCR3 = ;							  // RED LED DUTY CYCLE...Gets Set In UART ISR
	//TB0CCR4 = ;							  // GREEN LED DUTY CYCLE...Gets Set In UART ISR
	//TB0CCR5 = ; 							  // BLUE LED DUTY CYCLE...Gets Set In UART ISR
	TB0CCTL1 |= OUTMOD_7;					  // Reset/Set mode...QUESTIONABLE?!?
	TB0CTL |= CCIE;							  // Enable TB0 Interrupts
}

void UARTSetup()
{
	// Startup clock system with max DCO setting ~8MHz
  CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
  CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
  CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
  CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
  CSCTL0_H = 0;                             // Lock CS registers

  // Configure USCI_A0 for UART mode
  UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
  UCA0CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK
  // Baud Rate calculation
  // 8000000/(16*9600) = 52.083
  // Fractional portion = 0.083
  // User's Guide Table 21-4: UCBRSx = 0x04
  // UCBRFx = int ( (52.083-52)*16) = 1
  UCA0BR0 = 52;                             // 8000000/16/9600
  UCA0BR1 = 0x00;
  UCA0MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
  UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
  UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}

void GPIOSetup()
{
	// Configure GPIO
  P2SEL0 |= BIT0 | BIT1;                    // USCI_A0 UART operation
  P2SEL1 &= ~(BIT0 | BIT1);

  P2SEL0 |= BIT4;							// Direct TB0CCR3 (RED LED) to P2.4 
  P2SEL1 &= ~BIT4;

  P2SEL0 |= BIT5;							// Direct TB0CCR4 (GREEN LED) to P2.5
  P2SEL1 &= ~BIT5;

  P2SEL0 |= BIT6;							// Direct TB0CCR5 (BLUE LED) to P2.6 
  P2SEL1 &= ~BIT6;
}

//NEED FUNCTION FOR DETERMINING PWM FOR EACH LED. 