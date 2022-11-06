/*
 * main.c
 *
 * Created: 2022/10/15 22:57:24
 * Author : Y.Tsurui
 */ 

#include <avr/io.h>

#include "cpuclk.h"
#include "cpu_wdt.h"
#include "rtc.h"

#include "dcc_poller.h"
#include "dcc_packet_exec.h"
#include "prog_port.h"

#include "sw_config_dummy.h"
#include "sw_driver.h"


int main(void)
{
	uint8_t dccRecvPacketCache[48];
	uint8_t dccRecvPacketLength;
	
	uint16_t progTestCount = 0;
	
	setCLK();
	startWDT();
	initRTC();
	
	//initConfig();
	initProgPort();
	
	initDCCpoller();
	initSWdriver();
	
    /* Replace with your application code */
    while (1) 
    {
		taskProgMode();
		
		
		clearWDT();
		dccPacketShifter(&dccRecvPacketLength, dccRecvPacketCache);
		
		if (dccRecvPacketLength) {
			dccPacketRouter(dccRecvPacketLength, dccRecvPacketCache);
			dccRecvPacketLength = 0;
		}
		
		if (chkRTCflag()) {
			clkReceiverSWdriver();
			clkReceiverProgPort();
			dccRouterClockReceiver();
			
			if (readProgMode()) {
				progTestCount++;
				if (progTestCount == 500) {
					setSwitch(SW_DIR_CLOSE, getTurnOnTime());
				} else if (progTestCount == 1500) {
					setSwitch(SW_DIR_THROWN, getTurnOnTime());
				} else if (progTestCount >= 2000) {
					progTestCount = 0;
				}
			}
		}
    }
}

