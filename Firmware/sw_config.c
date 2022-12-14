/*
 * sw_config.c
 *
 * Created: 2022/09/16 23:22:58
 *  Author: Y.Tsurui
 */ 

#include <avr/io.h>
#include <avr/eeprom.h>

uint8_t __attribute__((section(".eeprom"))) val[] =
{
	0xFF,	// Configure Area
	0x81,	// Addr1 (Low)
	0xF8,	// Addr2 (High)
	0xC8,	// Turn-On Time Low-Byte
	0x00,	// Turn-On Time High-Byte
	0x00,	// Turn-On Delay Time Low-Byte
	0x00,	// Turn-On Delay Time High-Byte
	0x00,	// Configure Bits
};

uint8_t addrInfo[2];
uint8_t turnOnTime[2];
uint8_t turnOnDelayTime[2];
uint8_t configureBits;

#define FIRMWARE_VERSION_CODE	1
#define MANUFACTURE_ID	13

uint8_t addr_1byte_write_cache;
uint8_t addr_2byte_write_cache;
uint8_t lastStat_cache;

uint8_t read_cv_raw(uint8_t CVnum)
{
	return (eeprom_read_byte(val + CVnum));
}

void write_eeprom(uint8_t CVnum, uint8_t data)
{
	eeprom_busy_wait();
	eeprom_update_byte(val + CVnum, data);
}

void initCV(void)
{
	write_eeprom(1, 0x81);
	write_eeprom(2, 0xF8);
	write_eeprom(3, 0xC8);
	write_eeprom(4, 0x00);
	write_eeprom(5, 0x00);
	write_eeprom(6, 0x00);
	write_eeprom(7, 0x00);
	
	// Write Init Flag
	write_eeprom(0, 0);
	
	//initConfig();
}

void initConfig(void)
{
	uint8_t cfgInfo;
	
	eeprom_busy_wait();
	
	cfgInfo = read_cv_raw(0);
	
	if (cfgInfo == 0xFF) {
		initCV();
		eeprom_busy_wait();
	}
	
	addrInfo[0] = read_cv_raw(1);
	addrInfo[1] = read_cv_raw(2);
	turnOnTime[0] = read_cv_raw(3);
	turnOnTime[1] = read_cv_raw(4);
	turnOnDelayTime[0] = read_cv_raw(5);
	turnOnDelayTime[1] = read_cv_raw(6);
	configureBits = read_cv_raw(7);
	
}

uint8_t read_cv_byte(uint16_t CVnum)
{
	switch (CVnum) {
		case 7:
			return FIRMWARE_VERSION_CODE;
		case 8:
			return MANUFACTURE_ID;
		case 1:
			return addrInfo[0];
		case 9:
			return addrInfo[1];
		case 34:
			return turnOnTime[0];
		case 35:
			return turnOnTime[1];
		case 36:
			return turnOnDelayTime[0];
		case 37:
			return turnOnDelayTime[1];
		case 33:
			return configureBits;
	}
	
	return 0xFF;
}

void write_cv_byte(uint16_t CVnum, uint8_t data)
{
	uint8_t eepromAddr = 0;
	
	switch (CVnum) {
		case 7:
			// Version ID, read-only
			return;
		case 8:
			// Manufacture ID, Reset
			if ((data == MANUFACTURE_ID) || (data == 8)) {
				write_eeprom(0, 0xFF);
				initCV();
			}
			return;
		case 1:
			// Decoder Address LSB
			addrInfo[0] = data;
			eepromAddr = 1;
			break;
		case 9:
			// Decoder Address MSB
			addrInfo[1] = data;
			eepromAddr = 2;
			break;
		case 34:
			// Turn-On Time LSB
			turnOnTime[0] = data;
			eepromAddr = 3;
			break;
		case 35:
			// Turn-On Time MSB
			turnOnTime[1] = data;
			eepromAddr = 4;
			break;
		case 36:
			// Turn-On Delay Time LSB
			turnOnDelayTime[0] = data;
			eepromAddr = 5;
			break;
		case 37:
			// Turn-On Delay Time MSB
			turnOnDelayTime[1] = data;
			eepromAddr = 6;
			break;
		case 33:
			// Decoder Configure Bits
			configureBits = data;
			eepromAddr = 7;
			break;
	}
	
	if (eepromAddr > 0) {
		eeprom_busy_wait();
		write_eeprom(eepromAddr, data);
		eeprom_busy_wait();
	}
	
	return;
}


uint8_t chk_addr(uint8_t addr1_data, uint8_t addr2_data)
{
	addr2_data &= 0xFE;
	
	if ((addr1_data == addrInfo[0]) && (addr2_data == addrInfo[1])) {
		return (0xFF);
	} else {
		return (0x00);
	}
}

uint8_t write_addr(uint8_t addr1_data, uint8_t addr2_data)
{
	uint8_t stat;
	
	stat = addr2_data & 0x01;
	addr2_data &= 0xFE;
		
	if ((addr1_data == addr_1byte_write_cache) && (addr2_data == addr_2byte_write_cache)) {
		if (stat != lastStat_cache) {
			eeprom_busy_wait();
			//eeprom_write_byte(&addr_1byte_eemem, addr1_data);
			//eeprom_write_byte(&addr_2byte_eemem, addr2_data);
		
			write_eeprom(1, addr1_data);
			write_eeprom(2, addr2_data);
		
			addrInfo[0] = addr1_data;
			addrInfo[1] = addr2_data;
			eeprom_busy_wait();
			return (0xFF);
		} else {
			return 0x00;
		}
	} else {
		addr_1byte_write_cache = addr1_data;
		addr_2byte_write_cache = addr2_data;
		lastStat_cache = stat;		
		return (0x00);
	}
}

uint16_t getTurnOnTime(void)
{
	return (uint16_t)(turnOnTime[1] << 8) + turnOnTime[0];
}

uint16_t getTurnOnDelayTime(void)
{
	return (uint16_t)(turnOnDelayTime[1] << 8) + turnOnDelayTime[0];
}

uint8_t getConfigureBytes(void)
{
	return configureBits;
}