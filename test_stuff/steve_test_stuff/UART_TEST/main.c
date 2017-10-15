/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2014, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************
//  MSP430FR69xx Demo - eUSCI_A0 UART echo at 9600 baud using BRCLK = 8MHz
//
//  Description: This demo echoes back characters received via a PC serial port.
//  SMCLK/ DCO is used as a clock source and the device is put in LPM3
//  The auto-clock enable feature is used by the eUSCI and SMCLK is turned off
//  when the UART is idle and turned on when a receive edge is detected.
//  Note that level shifter hardware is needed to shift between RS232 and MSP
//  voltage levels.
//
//  The example code shows proper initialization of registers
//  and interrupts to receive and transmit data.
//  To test code in LPM3, disconnect the debugger.
//
//  ACLK = VLO, MCLK =  DCO = SMCLK = 8MHz
//
//                MSP430FR6989
//             -----------------
//       RST -|     P2.0/UCA1TXD|----> PC (echo)
//            |                 |
//            |                 |
//            |     P2.1/UCA1RXD|<---- PC
//            |                 |
//
//   William Goh
//   Texas Instruments Inc.
//   April 2014
//   Built with IAR Embedded Workbench V5.60 & Code Composer Studio V6.0
//******************************************************************************
#include <msp430.h>
volatile unsigned int j = 1;
volatile unsigned int num_of_bytes;
volatile unsigned int red,green,blue;
volatile unsigned int byte_count=0;
volatile unsigned int messages[80];
volatile unsigned int i = 0;
int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;                 // Stop Watchdog

// Configure GPIO
  P3SEL0 |= BIT4 | BIT5;                    // USCI_A0 UART operation
  P3SEL1 &= ~(BIT4 | BIT5);

  P2SEL0 |= BIT4;                           // Direct TB0CCR3 (RED LED) to P2.4
  P2SEL1 &= ~BIT4;
  P2DIR |= BIT4;

  P2SEL0 |= BIT5;                           // Direct TB0CCR4 (GREEN LED) to P2.5
  P2SEL1 &= ~BIT5;
  P2DIR |= BIT5;

  P2SEL0 |= BIT6;                           // Direct TB0CCR5 (BLUE LED) to P2.6
  P2SEL1 &= ~BIT6;
  P2DIR |= BIT6;
//RX/TX LED SETUP
  P1DIR |= BIT0; //RX LED SETUP
  P1OUT &= ~BIT0;
  P9DIR |= BIT7; //TX LED SETUP
  P9OUT &= ~BIT7;

  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;

//UART SETUP:
  // Startup clock system with max DCO setting ~8MHz
      CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
      CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
      CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
      CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
      CSCTL0_H = 0;                             // Lock CS registers

  // Configure USCI_A0 for UART mode
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
      UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

//PWM TIMER SETUP:
  TB0CCR0 = 256 - 1;                            // Count Up To
  TB0CTL |= TBSSEL_2 + MC_1 + TBCLR;        // SMCLK (1MHz) Up Mode Clear Timer
  TB0CCR3 = 0;                              // RED LED DUTY CYCLE...Changed Set In UART RX ISR
  TB0CCR4 = 0;                              // GREEN LED DUTY CYCLE...Changed Set In UART RX ISR
  TB0CCR5 = 0;                              // BLUE LED DUTY CYCLE...Changeed Set In UART RX ISR
  TB0CCTL3 |= OUTMOD_3;                    // Reset/Set Mode
  TB0CCTL4 |= OUTMOD_3;                    // Reset/Set Mode
  TB0CCTL5 |= OUTMOD_3;                    // Reset/Set Mode

  __bis_SR_register(LPM3_bits | GIE);       // Enter LPM3, interrupts enabled
  __no_operation();                         // For debugger

}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
  switch(__even_in_range(UCA1IV, USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
        P1OUT |= BIT0;
        if(byte_count == 0)
        {

            num_of_bytes = UCA1RXBUF;
            byte_count++;
        }
        else if((byte_count > 0) && (byte_count <= 3))
        {
            switch(byte_count)
            {
            case 1:
                TB0CCR3 = UCA1RXBUF;
                break;
            case 2:
                TB0CCR4 = UCA1RXBUF;
                break;
            case 3:
                TB0CCR5 = UCA1RXBUF;
                break;
            }
            byte_count++;
        }
        else if((byte_count <= num_of_bytes) && (byte_count > 3))
        {
            if(byte_count == 4)
            {
               i = 0;
               messages[i] = num_of_bytes -3;
            }
            i++;
            messages[i] = UCA1RXBUF;
            byte_count++;
            if(byte_count == num_of_bytes)
                    {
                        i = 0;
                        byte_count = 0;
                        UCA1IE &= ~UCRXIE;                         // Disable USCI_A1 RX interrupt
                        UCA1IE |= UCTXIE;                         // Enable USCI_A1 TX interrupt
                        UCA1TXBUF = messages[i];
                        P1OUT &= ~BIT0;
                    }
        }

      __no_operation();
      break;
    case USCI_UART_UCTXIFG:
        P9OUT |= BIT7;
        if(i < messages[0]-1)
        {
            i++;
            UCA1TXBUF = messages[i];

        }
        else if(i >= messages[0]-1)
        {
            P9OUT &= ~BIT7;
            i = 0;
            num_of_bytes = 0;
            UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt
            UCA1IE &= ~UCTXIE;                          // DISABLE USCI_A1 TX interrupt
        }
    break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
  }
}
