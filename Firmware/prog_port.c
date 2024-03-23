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
uint16_t switchAutoEventCount;

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
	if (switchAutoEventCount > 1) switchAutoEventCount--;
}

void taskProgMode(void)
{
	if ((getConfigureBytes() & CONFIG_SENSORMODE_MASK) != 0) {
		if (PORTA.IN & PIN2_bm) {
			if (getConfigureBytes() & CONFIG_SENSORMODE_LOCK_MASK) {
				lockPortCount = getLockTime();
			}
			switchAutoEventCount = getAutoMoveTime();
			if ((switchAutoEventCount < lockPortCount) && (switchAutoEventCount > 0)) {
				switchAutoEventCount = lockPortCount;
			}
			
		}
		
		if (lockPortCount == 1) {
			lockPortCount = 0;
		}
		
		if (switchAutoEventCount == 1) {
			// automove Event
			switchAutoEventCount = 0;
			if ((getConfigureBytes() & CONFIG_SENSORMODE_MASK) == CONFIG_SENSORMODE_LOCK_AUTO_C) {
				setSwitch(SW_DIR_CLOSE, getTurnOnTime());
			} else if ((getConfigureBytes() & CONFIG_SENSORMODE_MASK) == CONFIG_SENSORMODE_NONLOCK_AUTO_C) {
				setSwitch(SW_DIR_CLOSE, getTurnOnTime());
			} else if ((getConfigureBytes() & CONFIG_SENSORMODE_MASK) == CONFIG_SENSORMODE_LOCK_AUTO_T) {
				setSwitch(SW_DIR_THROWN, getTurnOnTime());
			} else if ((getConfigureBytes() & CONFIG_SENSORMODE_MASK) == CONFIG_SENSORMODE_NONLOCK_AUTO_T) {
				setSwitch(SW_DIR_THROWN, getTurnOnTime());
			} else if ((getConfigureBytes() & CONFIG_SENSORMODE_MASK) == CONFIG_SENSORMODE_LOCK_AUTOCHANGE) {
				toggleSwitch(getTurnOnTime());
			} else if ((getConfigureBytes() & CONFIG_SENSORMODE_MASK) == CONFIG_SENSORMODE_NONLOCK_AUTOCHANGE) {
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
	if ((getConfigureBytes() & CONFIG_SENSORMODE_MASK) == 0) {
		// Lock Mode‚ª–³Œø‚Ìê‡A‚¢‚©‚È‚éê‡‚Å‚àLockStat=Disable‚ð•Ô‚·
		return 0;
	}
	
	if (lockPortCount > 0) {
		return 1;
	}
	return 0;
}