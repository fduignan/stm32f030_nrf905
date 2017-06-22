// spi.h for stm32f030
// Author: Frank Duignan.  
// Updates posted on http://ioprog.com and http://eleceng.dit.ie/frank
// Revisions
// 29/5/2017: Changed CS and CSN pins such that they stay away from SWD
#include "stm32f030xx.h"
#include <stdint.h>
#include "spi.h"
void initSPI()
{
	int drain_count,drain;
	
	RCC_APB2ENR |= BIT12;		// turn on SPI1 	
	
	
	// GPIOA bits 5,6,7 are used for SPI1 (Alternative functions 0)
	RCC_AHBENR |= BIT18+BIT17; 	// enable ports A and B
    GPIOA_MODER &= ~(BIT14+BIT12+BIT10); // select Alternative function
    GPIOA_MODER |= (BIT15+BIT13+BIT11);  // for bits 5,6,7
    GPIOA_AFRL &= 0x000fffff;		     // select Alt. Function 0
	
	// Now configure the SPI interface
	drain = SPI1_SR;				     // dummy read of SR to clear MODF
	// enable SSM, set SSI, enable SPI, PCLK/16, LSB First Master	
	SPI1_CR1 = BIT9+BIT8+BIT6+BIT2;
	SPI1_CR2 = BIT10+BIT9+BIT8; 	     // configure for 8 bit operation
    
      
    for (drain_count = 0; drain_count < 32; drain_count++)
		drain = transferSPI(0x00);
}

uint8_t transferSPI(uint8_t data)
{
    unsigned Timeout = 1000000;
    int ReturnValue;
	
    while (((SPI1_SR & BIT7)!=0)&&(Timeout--));
    SPI1_DR8 = data;
    Timeout = 1000000;
    while (((SPI1_SR & BIT7)!=0)&&(Timeout--));        
	ReturnValue = SPI1_DR8;
	
    return ReturnValue;
}

