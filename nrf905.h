// Interface library for a connection between an STM32F030 and an NRF905 radio module

typedef struct {
    uint8_t Status;
    uint8_t Register[10];
} NRF905;

void TXEnHigh();
void TXEnLow();
void CELow();
void CEHigh();
void CSNLow();
void CSNHigh();
void PwrHigh();
void PwrLow();
void initNRF905();
void writeRegister(int RegNum, uint8_t contents);
uint8_t readRegister(int RegNum);
uint8_t getStatus();
void readRegisters(NRF905 *nrf);
void setRXAddress(const uint8_t *Address);
void setTXAddress(const uint8_t *Address);
void setChannel(int Channel);
void setRange(int Range);
void setAutoRetran(int Auto);
void setTXPower(int pwr);
void setRXPower(int pwr);
int writeTXPayload(uint8_t *Payload, unsigned length);
int readTXPayload(uint8_t *Payload, unsigned length);
int readRXPayload(uint8_t *Payload, unsigned length);
int readTXAddress(uint8_t *addr);
int DataReady();
