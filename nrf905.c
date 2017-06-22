#include "stm32f030xx.h"
#include "spi.h"
#include "nrf905.h"
void delay(unsigned len);
void delay_ms(unsigned ms)
{
    // Software delay that provides approx 1ms delay:  Not power efficient - look for a better solution :)
    while(ms--)
        delay(850);
}

void initNRF905()
{
    RCC_AHBENR |= BIT18+BIT17+BIT22; 	// enable ports A,B and F
    // CE and CSN configuration 
/*     
 STM32F030          NRF905
 PA1                DR
 PA2                Pwr  ---> PF0
 PA3                CE   ---> PF1
 PA4                CSN
 PA5                SPI SCLK
 PA6                SPI MISO
 PA7                SPI MOSI
 PA9                UART TX  ---> PA2
 PA10               UART RX  ---> PA3
 
 ??                 CD
 PB1                TXEn
                    I2C SDA   PA10
                    I2C SCL   PA9 
*/
    
    GPIOF_MODER &= ~BIT1;	// Make Port F, bit 0 behave as a
    GPIOF_MODER |= BIT0;	// general purpose output
    
    GPIOF_MODER &= ~BIT3;	// Make Port F, bit 1 behave as a
    GPIOF_MODER |= BIT2;	// general purpose output

    GPIOA_MODER &= ~BIT9;  // Make Port A, bit 4 behave as a
    GPIOA_MODER |= BIT8;	// general purpose output

    GPIOB_MODER &= ~BIT3;   // Make PB1 an output
    GPIOB_MODER |= BIT2;
    
    
    CSNHigh(); 		// Make CSN output initially high
    CELow();	    // Make CE output initially low    
    TXEnLow();      // Listen mode
    PwrHigh();      // Turn on the radio
    delay_ms(3);
}
int DataReady()
{
    //FIX THIS
    
}
void CELow()
{
    GPIOF_ODR &= ~BIT1;
    delay(100);
}
void CEHigh()
{
    GPIOF_ODR |= BIT1;
    delay(100);
}
void CSNLow()
{
    GPIOA_ODR &= ~BIT4;
    delay(100);
}
void CSNHigh()
{
    GPIOA_ODR |= BIT4;
    delay(100);
}
void TXEnHigh()
{
    GPIOB_ODR |= BIT1;
    delay_ms(2);
}
void TXEnLow()
{
    GPIOB_ODR &= ~BIT1;
    delay_ms(2);
}
void PwrHigh()
{
    GPIOF_ODR |= BIT0;
    delay_ms(3);
}
void PwrLow()
{
    GPIOF_ODR &= ~BIT0;
    delay_ms(3);
}
void writeRegister(int RegNum, uint8_t contents)
{
    CSNLow();
    transferSPI(RegNum);
    transferSPI(contents);
    CSNHigh();
}
uint8_t readRegister(int RegNum)
{
    uint8_t ReturnValue;
    CSNLow();
    transferSPI(0b00010000+RegNum);    
    ReturnValue = transferSPI(0);
    CSNHigh();
    return ReturnValue;
}
uint8_t getStatus()
{
    uint8_t ReturnValue;
    CSNLow(); 
    ReturnValue = transferSPI(0b00010000);
    CSNHigh();
    return ReturnValue;
}
void readRegisters(NRF905 *nrf)
{
    int Index;
    CSNLow(); 
    nrf->Status = transferSPI(0b00010000);
    for (Index = 0; Index < 10; Index++)
        nrf->Register[Index]=transferSPI(0x00);
    CSNHigh(); 
}
void setRXAddress(const uint8_t *Address)
{
    CSNLow();
    transferSPI(0b00000101);
    transferSPI(Address[3]);
    transferSPI(Address[2]);
    transferSPI(Address[1]);
    transferSPI(Address[0]);
    CSNHigh();
}
void setTXAddress(const uint8_t *Address)
{
    CSNLow();
    transferSPI(0b00100010);
    transferSPI(Address[3]);
    transferSPI(Address[2]);
    transferSPI(Address[1]);
    transferSPI(Address[0]);
    CSNHigh();
}
void setChannel(int Channel)
{
    uint8_t CurrentValue;
    Channel &= 0x1ff;
    writeRegister(0,Channel & 0xff);
    CurrentValue = readRegister(1);
    CurrentValue &= ~BIT0;
    CurrentValue |= Channel >> 8;
    writeRegister(1,CurrentValue);
}
void setRange(int Range)
{
    // Range 0 : 433MHz range
    // Range 1 : 868MHz range
    uint8_t CurrentValue = readRegister(1);
    CurrentValue = CurrentValue & ~BIT1;
    CurrentValue |= (Range & 1) << 1;
    writeRegister(1,CurrentValue);
}
void setAutoRetran(int Auto)
{
    // If Auto = 1 enable auto-retransmit
    uint8_t CurrentValue = readRegister(1);
    CurrentValue = CurrentValue & ~(BIT5);
    CurrentValue |= ((Auto & 1) << 5);
    writeRegister(1,CurrentValue);
}
void setTXPower(int pwr)
{    
    // pwr 0: -10dBm
    // pwr 1: -2dBm
    // pwr 2: +6dBm
    // pwr 3: +10dBm    
    
    uint8_t CurrentValue = readRegister(1);
    CurrentValue = CurrentValue & ~(BIT3+BIT2);
    CurrentValue |= ((pwr & 3) << 2);
    writeRegister(1,CurrentValue);
}
void setRXPower(int pwr)
{

    // pwr 0: Normal power in receive mode 
    // pwr 1: Low power in receive mode (less sensitive)
    uint8_t CurrentValue = readRegister(1);
    CurrentValue = CurrentValue & ~(1 << 4);
    CurrentValue |= ((pwr & 1) << 4);
    writeRegister(1,CurrentValue);
}


int writeTXPayload(uint8_t *Payload, unsigned length)
{
    int Index;
    if (length > 32)
        return -1;
    
// Write TX Payload width
    writeRegister(4,length);
// Write the data
    CSNLow(); 
    transferSPI(0x20);
    for (Index = 0; Index < length; Index++)        
        transferSPI(Payload[Index]);
    CSNHigh(); 
    return 0;
}
int readTXPayload(uint8_t *Payload, unsigned length)
{
    int Index;
    if (length > 32)
        return -1;
// Read the data
    CSNLow(); 
    transferSPI(0x21);
    for (Index = 0; Index < length; Index++)        
        Payload[Index]=transferSPI(0);
    CSNHigh(); 
    return 0;
}
int readRXPayload(uint8_t *Payload, unsigned length)
{
    int Index;
    
    if (length > 32)
        return -1;    
// Read the data
    CSNLow();     
    transferSPI(0x24);
    for (Index = 0; Index < length; Index++)        
        Payload[Index]=transferSPI(0);
    CSNHigh(); 
    return 0;
}
int readTXAddress(uint8_t *addr)
{
    CSNLow();     
    transferSPI(0x23);
    addr[0]=transferSPI(0);
    addr[1]=transferSPI(0);
    addr[2]=transferSPI(0);
    addr[3]=transferSPI(0);
}
