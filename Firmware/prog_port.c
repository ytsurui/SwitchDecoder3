/*
 * prog_port.c
 *
 * Created: 2022/09/17 0:55:16
 *  Author: Y.Tsurui
 */ 

#include <avr/io.h>

#include "sw_config.h"

uint16_t progPortCount;
uint8_t progFlag;

void initProgPort(void)
{
	PORTA.DIRCLR = PIN2_bm;
	progPortCount = 200;
	progFlag = 0;
}

void clkReceiverProgPort(void)
{
	if (progPortCount) progPortCount--;
}

void taskProgMode(void)
{
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