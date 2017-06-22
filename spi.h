// spi.h for stm32f030xx
#include <stdint.h>
void initSPI();
uint8_t transferSPI(uint8_t data);
void CELow();
void CEHigh();
void CSNLow();
void CSNHigh();
