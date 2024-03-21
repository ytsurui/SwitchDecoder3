/*
 * sw_driver.c
 *
 * Created: 2022/09/16 21:59:43
 *  Author: Y.Tsurui
 */ 

#include <avr/io.h>
#include "sw_driver.h"
#include "sw_config.h"
#include "prog_port.h"

uint16_t switchTimerCounter = 0;
uint16_t delayTimer = 0;

uint8_t waitMode = 0;
uint16_t waitTime = 0;

uint8_t basicACKflag = 0;
uint8_t basicACKcounter = 0;
#define BASIC_ACK_COUNT_MAX	10

#define LAST_SWITCH_STAT_C	0
#define LAST_SWITCH_STAT_T	1

uint8_t lastSwitchStat = LAST_SWITCH_STAT_C;

void initSWdriver(void) {
	PORTA.DIRSET = PIN4_bm | PIN5_bm;
	PORTB.DIRSET = PIN0_bm;
	
	PORTA.OUTCLR = PIN4_bm | PIN5_bm;
	PORTB.OUTCLR = PIN0_bm;
}


void clkReceiverSWdriver(void)
{	
	if (basicACKcounter) {
		basicACKcounter++;
		if (basicACKcounter >= BASIC_ACK_COUNT_MAX) {
			basicACKcounter = 0;
			//basicACKflag = 0;
			PORTA.OUTCLR = PIN4_bm | PIN5_bm;
			PORTB.OUTCLR = PIN0_bm;
		}
		return;
	}
	
	
	if (delayTimer) {
		delayTimer--;
		if (delayTimer == 0) {
			PORTB.OUTSET = PIN0_bm;
		}
		return;
	}
	
	if (switchTimerCounter) {
		switchTimerCounter--;
	}
	
	if (switchTimerCounter == 0) {
		PORTA.OUTCLR = PIN4_bm | PIN5_bm;
		PORTB.OUTCLR = PIN0_bm;
	}
	
	if (waitMode > 0) {
		if (readLockStat() == 0) {
			setSwitch(waitMode, waitTime);
			waitMode = 0;
			waitTime = 0;
		}
		return;
	}
}

void setSwitch(uint8_t dir, uint16_t ms)
{
	if (readLockStat()) {
		waitMode = dir;
		waitTime = ms;
		return;
	}
	
	PORTB.OUTCLR = PIN0_bm;
	
	if (getConfigureBytes() & CONFIG_BIT_REVERSE) {
		PORTA.OUTCLR = PIN4_bm;
		PORTA.OUTCLR = PIN5_bm;
		if (dir == SW_DIR_CLOSE) {
			PORTA.OUTSET = PIN4_bm;
			lastSwitchStat = LAST_SWITCH_STAT_C;
		} else if (dir == SW_DIR_THROWN) {
			PORTA.OUTSET = PIN5_bm;
			lastSwitchStat = LAST_SWITCH_STAT_T;
		}
	} else {
		PORTA.OUTCLR = PIN4_bm;
		PORTA.OUTCLR = PIN5_bm;
		if (dir == SW_DIR_CLOSE) {
			PORTA.OUTSET = PIN5_bm;
			lastSwitchStat = LAST_SWITCH_STAT_C;
		} else if (dir == SW_DIR_THROWN) {
			PORTA.OUTSET = PIN4_bm;
			lastSwitchStat = LAST_SWITCH_STAT_T;
		}
	}
	
	delayTimer = getTurnOnDelayTime();
	if (delayTimer == 0) {
		PORTB.OUTSET = PIN0_bm;
	}
	
	switchTimerCounter = ms;
}

void toggleSwitch(uint16_t ms)
{
	if (lastSwitchStat == LAST_SWITCH_STAT_T) {
		// T to C
		setSwitch(SW_DIR_CLOSE, ms);
	} else {
		// C to T
		setSwitch(SW_DIR_THROWN, ms);
	}
}

void basicACK()
{
	basicACKcounter = 1;
	basicACKflag = 1;
	
	PORTA.OUTCLR = PIN4_bm;
	PORTB.OUTCLR = PIN0_bm;
	PORTA.OUTCLR = PIN5_bm;
	PORTA.OUTSET = PIN4_bm;
	PORTB.OUTSET = PIN0_bm;
}

void basicACKoff()
{
	basicACKcounter = 0;
	PORTA.OUTCLR = PIN4_bm | PIN5_bm;
	PORTB.OUTCLR = PIN0_bm;
}

uint8_t readBasicACKflag()
{
	return basicACKflag;
}

void clearBasicACKflag()
{
	basicACKflag = 0;
}