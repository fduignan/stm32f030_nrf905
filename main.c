// This is a test program for use with a low energy weather station based
// around the STM32F030 and an NRF905 radio module.  The sensing module is
// a GY-652 (HMC5983 + BMP180) which uses an I2C interface.
// The readings for this sensor will be transmitted periodically over
// the radio link
// The STM32F030 runs at the default speed of 8MHz on its internal oscillator.
// A serial interface is provided for debugging purposes.
// This version (0.1) does not take much heed of power saving - that will come later.
//
//
//
/*  STM32F030          NRF905
 PA1                DR
 PF0                Pwr  
 PF1                CE   
 PA4                CSN
 PA5                SPI SCLK
 PA6                SPI MISO
 PA7                SPI MOSI
 PB1                TXEn
 
 UART Interface
 PA2                UART TX  
 PA3                UART RX  
  
 */
#include "stm32f030xx.h"
#include "serial.h"
#include "spi.h"
#include "nrf905.h"
const uint8_t BaseStationAddr[]={0xde,0xad,0xb0,0x55};
const uint8_t NodeAddr[]={0xfd,0x00,0x00,0x00};
char Msg[32]="Hello World";
uint8_t Buffer[32];
void delay(int dly)
{
  while( dly--);
}
void delay_ms(unsigned ms)
{
    // Software delay that provides approx 1ms delay:  Not power efficient - look for a better solution :)
    while(ms--)
        delay(850);
}
char UserInput[10];
/*************** 
 * ADC Test routines
 **************/  
void initADC()
{

  // Turn on GPIOB
  RCC_AHBENR |= BIT18;
  // Turn on ADC 
  RCC_APB2ENR |= BIT9;
  // Select analog mode for PB1
  GPIOB_MODER |= (BIT2+BIT3);  
  // Begin ADCCalibration
  ADC_CR |= BIT31;
  // Wait for calibration complete:  
  while ((ADC_CR & BIT31));
  // Select Channel 9
  ADC_CHSELR |= BIT9;
  // Enable the reference voltage
  ADC_CCR |= BIT22;	
  // Enable the ADC
  ADC_CR |= BIT0;  
}
int readADC()
{
  // Trigger a conversion
  ADC_CR |=  BIT2;
  // Wait for End of Conversion
  while ( (ADC_CR & BIT2) );
  // return result
  return ADC_DR;
}
void testADC()
{
  eputs("ADC: ");
  printHex(readADC());
  eputs("\r\n");
}
void configPins()
{
  // Power up PORTA
  RCC_AHBENR |= BIT17;	

}	

void TxPacket(uint8_t *Pkt,int len)
{
    
    TXEnHigh();
    delay(2000);
    writeTXPayload(Pkt,len);
    setTXAddress(BaseStationAddr);
    CEHigh();
    delay(2000); // wait for tx to go
    CELow();
   // while( (getStatus()&0x20) == 0); // wait for tx to complete    
    delay_ms(12);
    TXEnLow();  // clear the DR bit
}
void Int2String(unsigned int x,char *Str)
{
    int index=0;
    // Can have up to 4billion so 10 digits
    Str[10]=0; // terminate the string 
    while(index < 10)
    {
        Str[9-index]= x%10+'0';
        x = x / 10;
        index++;
    }
}
int main()
{
  int i=0;
  int Count = 0;
  NRF905 nrf;
  char TestMsg[12];
  configPins();
  initUART(9600);  
  initADC();
  enable_interrupts();
  initSPI();
  initNRF905();
  PwrHigh(); // turn on the radio
  CELow();
  TXEnLow();  
// Set Frequency to 434.2MHz 
  
  setRXPower(0); // normal RX sensitivity
  setTXPower(3); // Maximum TX power (10mW)
  setChannel(0x76); // Channel number
  
  setRange(0);
  setAutoRetran(0);  // Don't bother with auto-retransmit (actually seems to work better without)
  
  
  writeRegister(9,0x5f); // 8 bit CRC, CRC Enable, 16MHz external crystal, not using clock out
  writeRegister(2,0x44);
  writeRegister(3,0x20);
  setTXAddress(BaseStationAddr);
  setRXAddress(BaseStationAddr);
    
  TestMsg[11]=0;  
  while(1) {
        
    Int2String(Count,Msg);
    Count++;
    TxPacket(Msg,0x20);
    delay(1000000);            
    
    readRegisters(&nrf);      
    eputs("\r\nNode ");
    eputs("St: ");
    printByte(nrf.Status);    
    eputs(" 0:");
    printByte(nrf.Register[0]);                
    eputs(" 1:");
    printByte(nrf.Register[1]);
    eputs(" 2:");
    printByte(nrf.Register[2]);
    eputs(" 3:");
    printByte(nrf.Register[3]);
    eputs(" 4:");
    printByte(nrf.Register[4]);
    eputs(" 5:");
    printByte(nrf.Register[5]);
    eputs(" 6:");
    printByte(nrf.Register[6]);
    eputs(" 7:");
    printByte(nrf.Register[7]);
    eputs(" 8:");
    printByte(nrf.Register[8]);
    eputs(" 9:");
    printByte(nrf.Register[9]);
    
  }
  return 0;
}
