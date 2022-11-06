/*
 * dcc_prog_mode.c
 *
 * Created: 2022/09/17 1:23:00
 *  Author: Y.Tsurui
 */ 

#include <avr/io.h>

#include "sw_config.h"
#include "sw_driver.h"

uint8_t packetProgCache[8];


// Paged Mode Programming Variables
uint8_t page_r = 0;
uint8_t page_verify_r, page_verify_r_flag = 0;
uint8_t service_page_reg_flag;



void resetPageFlag(void) {
	service_page_reg_flag = 0;
}

void dccProgPageMode(uint8_t packetLength, uint8_t packetData[])
{
	uint16_t cvAddr;
	
	if ((packetData[0] == packetProgCache[0]) && (packetData[1] == packetProgCache[1]) && (packetData[2] == packetProgCache[2])) {
		
		if ((packetProgCache[0] & 0x04) == 0) {
			cvAddr = ((page_r - 1) * 4 + (packetProgCache[0] & 0x03) + 1);
			
			if (packetProgCache[0] & 0x08) {
				// Write CV
				write_cv_byte(cvAddr, packetProgCache[1]);
				service_page_reg_flag |= 0x04;
				basicACK();
			} else {
				// Veryfy CV
				if (packetProgCache[1] == read_cv_byte(cvAddr)) {
					service_page_reg_flag |= 0x02;
					basicACK();
				}
			}
			return;
		}
		
		switch (packetProgCache[0] & 0x07) {
			case 0x05:
				// Paged Mode, Write Page Register
				if (~service_page_reg_flag & 0x01) {
					page_r = packetProgCache[1];
					service_page_reg_flag |= 0x01;
					basicACK();
				}
				return;
			case 0x04:
				// Phys Mode, CV29 (Basic Configuration)
				cvAddr = 29;
				break;
			case 0x06:
				// Phys Mode, CV7 (Version Number)
				cvAddr = 7;
				break;
			case 0x07:
				// Phys Mode, CV8 (Manufacturer ID Number)
				cvAddr = 8;
				break;
			default:
				return;
		}
		
		// Phys Mode
		if (packetProgCache[0] & 0x08) {
			// Write
			write_cv_byte(cvAddr, packetProgCache[1]);
			if (~service_page_reg_flag & 0x04) {
				service_page_reg_flag |= 0x04;
				basicACK();
			}
		} else {
			// Verify
			if (packetProgCache[1] == read_cv_byte(cvAddr)) {
				if (~service_page_reg_flag & 0x02) {
					service_page_reg_flag |= 0x02;
					basicACK();
				}
			}
		}
	
	} else {
		packetProgCache[0] = packetData[0];
		packetProgCache[1] = packetData[1];
		packetProgCache[2] = packetData[2];
		clearBasicACKflag();
	}
	
}

void dccProgDirectMode(uint8_t packetLength, uint8_t packetData[])
{
	uint16_t cvAddr;
	uint8_t BitMask, BitPos, cvCache;
	
	
	if ((packetData[0] == packetProgCache[0]) && (packetData[1] == packetProgCache[1]) && (packetData[2] == packetProgCache[2]) && (packetData[3] == packetProgCache[3])) {
		if (readBasicACKflag()) return;
		
		cvAddr = (packetProgCache[0] & 0x03) + packetProgCache[1] + 1;
		
		switch (packetProgCache[0] & 0x0C) {
			case 0x08:
				// Bit Manipulation
				//basicACK();
				if ((packetProgCache[2] & 0xE0) == 0xE0) {
					BitPos = packetProgCache[2] & 0x07;
					BitMask = 0x01 << BitPos;
					cvCache = read_cv_byte(cvAddr);
					if (packetProgCache[2] & 0x10) {
						// Write Bit
						if (packetProgCache[2] & 0x08) {
							// Write Bit is '1'
							cvCache |= BitMask;
							write_cv_byte(cvAddr, cvCache);
							basicACK();
						} else {
							// Write Bit is '0'
							cvCache &= ~BitMask;
							write_cv_byte(cvAddr, cvCache);
							basicACK();
						}
					} else {
						// Read Bit
						if (packetProgCache[2] & 0x08) {
							// Bit is '1'
							if (cvCache & BitMask) {
								basicACK();
							}
						} else {
							// Bit is '0'
							if (~cvCache & BitMask) {
								basicACK();
							}
						}
					}
				}
				break;
			case 0x04:
				// Verify Byte
				if (packetProgCache[2] == read_cv_byte(cvAddr)) {
					basicACK();
				}
				break;
			case 0x0C:
				// Write Byte
				write_cv_byte(cvAddr, packetProgCache[2]);
				basicACK();
				break;
			case 0x00:
				// Reserved
				break;
		}
	
	} else {
		packetProgCache[0] = packetData[0];
		packetProgCache[1] = packetData[1];
		packetProgCache[2] = packetData[2];
		packetProgCache[3] = packetData[3];
		clearBasicACKflag();
		basicACKoff();
	}
	
	
}
