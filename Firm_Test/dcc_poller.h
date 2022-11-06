/*
 * dcc_poller_tiny202.h
 *
 * Created: 2021/01/25 9:35:35
 *  Author: Y.Tsurui
 */ 

void DCCpollerClkReceiver(void);
uint8_t readAnalogStat(void);

void initDCCpoller(void);
void dccPacketShifter(uint8_t* recvPacketLength, uint8_t* recvPacket);
