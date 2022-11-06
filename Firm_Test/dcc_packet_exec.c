/*
 * dcc_packet_exec.c
 *
 * Created: 2022/09/16 19:59:45
 *  Author: Y.Tsurui
 */ 

#include <avr/io.h>

#include "sw_config_dummy.h"
#include "sw_driver.h"

#include "prog_port.h"

//#include "dcc_prog_mode.h"

uint8_t serviceModeFlag = 0;
uint8_t cmdReceivedCache = 0;

void dccNormalOperation(uint8_t packetLength, uint8_t packetData[]);

void dccPacketRouter(uint8_t packetLength, uint8_t packetData[])
{
	// 2-Byte Packet (None)
	if (packetLength == 2) return;
	
	if (serviceModeFlag && ((packetData[0] & 0x70) == 0x70)) {
		// DCC Service Mode
		serviceModeFlag = 30;
		
		switch (packetLength) {
			case 3:
				if ((packetData[0] == 0) && (packetData[1] == 0) && (packetData[2] == 0)) {
					// Reset
					serviceModeFlag = 30;
					return;
				} else if ((packetData[0] == 0xFF) && (packetData[1] == 0) && (packetData[2] == 0xFF)) {
					// Idle Packet
					return;
				} else {
					// Paged Mode, Phys Mode, Address Only Mode
					//dccProgPageMode(packetLength, packetData);
				}
				break;
			case 4:
				// Direct-Mode
				//dccProgDirectMode(packetLength, packetData);
				break;
		}
	} else {
		dccNormalOperation(packetLength, packetData);
	}
	
	
}

void dccNormalOperation(uint8_t packetLength, uint8_t packetData[])
{
	//uint8_t resp;
	
	if (packetLength == 2) {
		cmdReceivedCache = 0;
		return;
	}
	
	if (packetLength == 3) {
		if ((packetData[0] == 0) && (packetData[1] == 0) && (packetData[2] == 0)) {
			// Reset
			serviceModeFlag = 30;
			//resetPageFlag();
			return;
		} else if ((packetData[0] == 0xFF) && (packetData[1] == 0x00) && (packetData[2] == 0xFF)) {
			// Idle, Exit Service-Mode
			serviceModeFlag = 0;
			return;
		}
		
		if ((packetData[0] > 0x7F) && (packetData[0] < 0xC0) && (packetData[1] & 0x80)) {
			// Accessory Packet
			/*
			if (readProgMode()) {
				if (cmdReceivedCache != packetData[1]) {
					resp = write_addr(packetData[0], packetData[1]);
					if (resp) clearProgMode();
					
					cmdReceivedCache = packetData[1];
				}
			}
			*/
			
			if (chk_addr(packetData[0], packetData[1]) && (cmdReceivedCache != packetData[1])) {
				if (packetData[1] & 0x01) {
					// closed
					setSwitch(SW_DIR_CLOSE, getTurnOnTime());
				} else {
					// thrown
					setSwitch(SW_DIR_THROWN, getTurnOnTime());
				}
				
				cmdReceivedCache = packetData[1];
				
				if (cmdReceivedCache == 0) cmdReceivedCache = 0xFF;
			} else {
				cmdReceivedCache = 0;
			}
			
		} else {
			cmdReceivedCache = 0;
		}
		return;
	}
	
	cmdReceivedCache = 0;
	return;
	
	
}


void dccRouterClockReceiver(void)
{
	if (serviceModeFlag) serviceModeFlag--;
}