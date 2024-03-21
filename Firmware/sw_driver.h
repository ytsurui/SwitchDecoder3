/*
 * sw_driver.h
 *
 * Created: 2022/09/16 22:19:45
 *  Author: Y.Tsurui
 */ 

#define SW_DIR_CLOSE	1
#define SW_DIR_THROWN	2

void initSWdriver(void);
void clkReceiverSWdriver(void);
void setSwitch(uint8_t dir, uint16_t ms);
void toggleSwitch(uint16_t ms);

void basicACK();
void basicACKoff();
uint8_t readBasicACKflag();
void clearBasicACKflag();
