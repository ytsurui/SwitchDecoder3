/*
 * sw_config_dummy.c
 *
 * Created: 2022/10/15 23:06:16
 *  Author: ytsurui
 */ 

#include <avr/io.h>
#include "sw_config_dummy.h"

uint8_t chk_addr(uint8_t addr1_data, uint8_t addr2_data)
{
	addr2_data &= 0xFE;
	
	if ((addr1_data == 0x81) && (addr2_data == 0xF8)) {
		return (0xFF);
	} else {
		return (0x00);
	}
}

uint16_t getTurnOnTime(void)
{
	return 200;
}

uint16_t getTurnOnDelayTime(void)
{
	return 0;
}

uint8_t getConfigureBytes(void)
{
	return 0;
}