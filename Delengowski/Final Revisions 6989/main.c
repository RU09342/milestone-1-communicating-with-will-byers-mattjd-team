//MILESTONE 1
//Authors: The Polish Brotherhood: Delengowski & Szymczak


#include <msp430.h>

volatile unsigned int num_of_bytes;          // Takes on the Value of Received Byte Zero (when byte_count = 0),
                                             // Stores Total Number of Bytes to be Received
volatile unsigned int byte_count = 0;        // For Counting Number of Bytes Received through RXBUF
volatile unsigned int index_count = 0;       // For Assigning Received Bytes to messages[i] Array
volatile unsigned int messages[80];          // For Storing Received Bytes from RXBUF...at most we will receive 80 bytes,
                                             // Later Used for Transmitting Messages to TXBUF

#define red   TB0CCR3                           // For Controlling RED LED DUTY CYCLE
#define green TB0CCR4                           // For Controlling GREEN LED DUTY CYCLE
#define blue  TB0CCR5                           // For Controlling BLUE LED DUTY CYCLE
#define LED1ON P1OUT |= BIT0                    // Indicates Start of Receive Cylce
#define LED1OFF P1OUT &= ~BIT0                  // Indicates End of Receive Cycle
#define LED2ON P9OUT |= BIT7                    // Indicates Start of Transmission Cycle
#define LED2OFF P9OUT &= ~BIT7                  // Indicates End of Transmission Cycle
#define RECEIVED UCA1RXBUF                      // Byte Received Buffer
#define TRANSMIT UCA1TXBUF                      // Transmit Byte Buffer
#define EN_REC_INTERRUPT UCA1IE |= UCRXIE       // Enable UCA1 RX Interrupt
#define DIS_REC_INTERRUPT UCA1IE &= ~UCRXIE     // Disable UCA1 RX Interrupt
#define EN_TRAN_INTERRUPT UCA1IE |= UCTXIE      // Enable UCA1 TX Interrupt
#define DIS_TRAN_INTERRUPT UCA1IE &= ~UCTXIE    // Disable UCA1 TX Interrupt

void TimerB0Setup();
void UARTSetup();
void GPIOSetup();
void LEDSetup();

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                   // Stop Watchdog
    PM5CTL0 &= ~LOCKLPM5;                       // Disable the GPIO power-on default high-impedance mode to activate,
                                                // previously configured port settings

    TimerB0Setup();
    UARTSetup();
    GPIOSetup();
    LEDSetup();

    __bis_SR_register(LPM3_bits | GIE);       // Enter LPM3, interrupts enabled
    __no_operation();                         // For debugger

}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    switch(__even_in_range(UCA1IV, USCI_UART_UCTXCPTIFG))
    {
        case USCI_NONE:
        {
            break;
        }
        case USCI_UART_UCRXIFG:
        {
            LED1ON;                                                     // Indicate We Are Receiving Bytes
            if(byte_count == 0)                                         // Store First Byte Received, Has Number of Bytes to be Received
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
                        red = RECEIVED;                                 // Store RED LED Byte...should be 2nd byte, byte_count = 1
                        break;
                    }
                    case 2:
                    {
                        green = RECEIVED;                               // Store GREEN LED Byte...should be 3rd byte, byte_count = 2
                        break;
                    }
                    case 3:
                    {
                        blue = RECEIVED;                                // Store BLUE LED Byte...should be 4th byte, byte_count = 3
                        break;
                    }
                }
                byte_count++;
            }
            else if((byte_count <= num_of_bytes) && (byte_count > 3))   // Bytes to be Transmitted
            {
                if(byte_count == 4)
                {
                   index_count = 0;
                   messages[index_count] = num_of_bytes - 3;            // Number of Bytes to eventually Transmit
                }
                index_count++;
                messages[index_count] = RECEIVED;                       // Storing RGB Bytes to be Transmitted
                byte_count++;
                if(byte_count == num_of_bytes)
                {
                    index_count = 0;                                        // Reset Array Index to Start Transmitting
                    byte_count = 0;                                         // Reset byte_count for Next Cycle of Receiving
                    DIS_REC_INTERRUPT;                                      // Disable USCI_A1 RX Interrupt
                    EN_TRAN_INTERRUPT;                                      // Enable USCI_A1 TX Interrupt
                    TRANSMIT = messages[index_count];                       // Send First Transmission (Indicates Number of Bytes Being Sent)
                    LED1OFF;                                                // Indicate We have stopped receiving bytes
                }
            }
            __no_operation();
            break;
        }
        case USCI_UART_UCTXIFG:
        {
            LED2ON;                                                     // Indicate Transmission Started
            if(index_count < (messages[0] - 1))
            {
                index_count++;
                TRANSMIT = messages[index_count];                       // Start Transmitting Remaining Bytes
            }
            else if(index_count  == (messages[0] - 1))
            {
                LED2OFF;                                                // Indicate Transmission Ended
                index_count = 0;                                        // Reset Index for Next Receive Cycle
                num_of_bytes = 0;                                       // Reset For Next Receive Cycle
                EN_REC_INTERRUPT;                                       // Enable USCI_A1 RX Interrupt
                DIS_TRAN_INTERRUPT;                                     // Disable USCI_A1 TX Interrupt
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
    TB0CCR0 = 256 - 1;                        // Count Up To
    TB0CTL |= TBSSEL_2 + MC_1 + TBCLR;       // SMCLK (1MHz) Up Mode Clear Timer
    TB0CCR3 = 0;                             // RED LED DUTY CYCLE...Changed Set In UART RX ISR
    TB0CCR4 = 0;                             // GREEN LED DUTY CYCLE...Changed Set In UART RX ISR
    TB0CCR5 = 0;                             // BLUE LED DUTY CYCLE...Changeed Set In UART RX ISR
    TB0CCTL3 |= OUTMOD_3;                    // Set/Reset Mode
    TB0CCTL4 |= OUTMOD_3;                    // Set/Reset Mode
    TB0CCTL5 |= OUTMOD_3;                    // Set/Reset Mode
}

void UARTSetup()
{
    // Startup clock system with max DCO setting ~8MHz
  CSCTL0_H = CSKEY >> 8;                    // Unlock clock registers
  CSCTL1 = DCOFSEL_3 | DCORSEL;             // Set DCO to 8MHz
  CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
  CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
  CSCTL0_H = 0;                             // Lock CS registers

  // Configure USCI_A1 for UART mode
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
  UCA1IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt
}

void GPIOSetup()
{
    // Configure GPIO
  P3SEL0 |= BIT4 | BIT5;                    // USCI_A1 UART operation
  P3SEL1 &= ~(BIT4 | BIT5);

  P2SEL0 |= BIT4;                           // Direct TB0CCR3 (RED LED) to P2.4
  P2SEL1 &= ~BIT4;

  P2SEL0 |= BIT5;                           // Direct TB0CCR4 (GREEN LED) to P2.5
  P2SEL1 &= ~BIT5;

  P2SEL0 |= BIT6;                           // Direct TB0CCR5 (BLUE LED) to P2.6
  P2SEL1 &= ~BIT6;

  P2DIR |= (BIT4 + BIT5 + BIT6);             // Set P2.4,P2.5,P2.6 to Output
}

void LEDSetup()
{
    P1DIR |= BIT0;                          // Set P1.0 LED to Output
    P9DIR |= BIT7;                          // Set P9.7 LED to Output

    P1OUT &= ~BIT0;                         // Disable P1.0 LED
    P9OUT &= ~BIT7;                         // Disable P9.7 LED
}
