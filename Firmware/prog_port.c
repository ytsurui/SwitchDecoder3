/*
 * prog_port.c
 *
 * Created: 2022/09/17 0:55:16
 *  Author: Y.Tsurui
 */ 

#include <avr/io.h>

#include "sw_config.h"
#include "sw_driver.h"

uint16_t progPortCount;
uint8_t progFlag;

uint16_t lockPortCount;

void initProgPort(void)
{
	PORTA.DIRCLR = PIN2_bm;
	progPortCount = 200;
	progFlag = 0;
}

void clkReceiverProgPort(void)
{
	if (progPortCount) progPortCount--;
	if (lockPortCount > 1) lockPortCount--;
}

void taskProgMode(void)
{
	if ((getConfigureBytes() & CONFIG_LOCKMODE_MASK) != 0) {
		if (PORTA.IN & PIN2_bm) {
			lockPortCount = getLockTime();
		}
		
		if (lockPortCount == 1) {
			// Lock release Event
			lockPortCount = 0;
			if ((getConfigureBytes() & CONFIG_LOCKMODE_MASK) == CONFIG_LOCKMODE_AUTO_C) {
				setSwitch(SW_DIR_CLOSE, getTurnOnTime());
			} else if ((getConfigureBytes() & CONFIG_LOCKMODE_MASK) == CONFIG_LOCKMODE_AUTO_T) {
				setSwitch(SW_DIR_THROWN, getTurnOnTime());
			} else if ((getConfigureBytes() & CONFIG_LOCKMODE_MASK) == CONFIG_LOCKMODE_AUTOCHANGE) {
				toggleSwitch(getTurnOnTime());
			}
		}
		return;
	}
	if (progPortCount == 0) return;
	if (getConfigureBytes() & CONFIG_BIT_DISABLEPROGPORT) return;
	
	if (PORTA.IN & PIN2_bm) {
		progFlag = 1;
	}
}

uint8_t readProgMode(void)
{
	return (progFlag);
}

void clearProgMode(void)
{
	progFlag = 0;
}

uint8_t readLockStat(void)
{
	if ((getConfigureBytes() & CONFIG_LOCKMODE_MASK) == 0) {
		// Lock Mode‚ª–³Œø‚Ìê‡A‚¢‚©‚È‚éê‡‚Å‚àLockStat=Disable‚ð•Ô‚·
		return 0;
	}
	
	if (lockPortCount > 0) {
		return 1;
	}
	return 0;
}