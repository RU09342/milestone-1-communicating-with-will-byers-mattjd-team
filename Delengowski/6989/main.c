#include <msp430.h>

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;                 // Stop Watchdog

  // Configure GPIO
<<<<<<< HEAD
  P3SEL0 |= BIT4 | BIT5;                    // USCI_A1 UART operation
=======
  P3SEL0 |= BIT4 | BIT5;                    // USCI_A0 UART operation
>>>>>>> 463627c01a29e2151ef010548eeddbda3d5102ad
  P3SEL1 &= ~(BIT4 | BIT5);

  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;

  // Startup clock system with max DCO setting ~8MHz
  CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
  CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
  CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
  CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
  CSCTL0_H = 0;                             // Lock CS registers

<<<<<<< HEAD
  // Configure USCI_A1 for UART mode
=======
  // Configure USCI_A0 for UART mode
>>>>>>> 463627c01a29e2151ef010548eeddbda3d5102ad
  UCA1CTLW0 = UCSWRST;                      // Put eUSCI in reset
  UCA1CTLW0 |= UCSSEL__SMCLK;               // CLK = SMCLK
  // Baud Rate calculation
  // 8000000/(16*9600) = 52.083
  // Fractional portion = 0.083
  // User's Guide Table 21-4: UCBRSx = 0x04
  // UCBRFx = int ( (52.083-52)*16) = 1
  UCA1BR0 = 52;                             // 8000000/16/9600
  UCA1BR1 = 0x00;
  UCA1MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
  UCA1CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
<<<<<<< HEAD
  UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt
=======
  UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
>>>>>>> 463627c01a29e2151ef010548eeddbda3d5102ad

  __bis_SR_register(LPM3_bits | GIE);       // Enter LPM3, interrupts enabled
  __no_operation();                         // For debugger
}

#pragma vector = USCI_A1_VECTOR
<<<<<<< HEAD
__interrupt void USCI_A1_ISR(void)
=======
__interrupt void USCI_A0_ISR(void)
>>>>>>> 463627c01a29e2151ef010548eeddbda3d5102ad
{
  switch(__even_in_range(UCA1IV, USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE:
        {
            break;
        }
    case USCI_UART_UCRXIFG:
        {
            while(!(UCA1IFG & UCTXIFG));
            UCA1TXBUF = UCA1RXBUF;
            __no_operation();
            break;
        }
    case USCI_UART_UCTXIFG:
        {
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
