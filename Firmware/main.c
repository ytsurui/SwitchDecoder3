/*
 * main.c
 *
 * Created: 2022/09/16 18:25:00
 * Author : Y.Tsurui
 */ 

#include <avr/io.h>

#include "cpuclk.h"
#include "cpu_wdt.h"
#include "rtc.h"

#include "dcc_poller.h"
#include "dcc_packet_exec.h"
#include "prog_port.h"

#include "sw_config.h"
#include "sw_driver.h"
#include "sw_led.h"

int main(void)
{
	uint8_t dccRecvPacketCache[48];
	uint8_t dccRecvPacketLength;
	
	setCLK();
	startWDT();
	initRTC();
	
	initProgPort();
	initDCCpoller();
	initSWdriver();
	initLEDdriver();
	
	initConfig();
	
	while (loadCVevent()) {
		clearWDT();
		dccPacketShifter(&dccRecvPacketLength, dccRecvPacketCache);
	}
	
	
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
		}
    }
}

