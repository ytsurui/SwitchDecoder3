/*
 * dcc_poller_tiny406.c
 *
 * Created: 2022/09/16 18:35:27
 *  Author: Y.Tsurui
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>


uint8_t portTestFlag = 0;

uint8_t oldInputPortStat = 0;		

volatile uint16_t dccTimerRightStart = 0;
volatile uint16_t dccTimerLeftStart = 0;
volatile uint16_t dccTimerCounter = 0;
//uint16_t dccTimerCounterLeft = 0;

uint8_t railcomCutoutFlag = 0;
//uint16_t railcomCutoutCounter = 0;

uint8_t _preambleCount = 0;
uint8_t _dccPulseReceiveBit;

uint8_t _dccPulseBitCount;
uint8_t _dccPacketCount;
uint8_t _dccPacketError;

#define DCC_PACKET_LENGTH	32
uint8_t _dccPacketData[DCC_PACKET_LENGTH];

uint8_t dccRecvPacketCache[DCC_PACKET_LENGTH];
uint8_t dccRecvPacketCacheLength;
uint8_t dccRecvPacketCacheEnableFlag;

/* BaseClock = 20MHz/2 = 10MHz */
#define DCC_PULSE_LENGTH_ONE_MIN	220		//44us (counter 1cycle = 200ns @ 10MHz)
#define DCC_PULSE_LENGTH_ONE_MAX	375		//75us
#define DCC_PULSE_LENGTH_ZERO_MIN	425		//85us
#define DCC_PULSE_LENGTH_ZERO_MAX	50000	//10000us
//#define DCC_PULSE_LENGTH_ZERO_MAX	20000	//20000us

//void sendPacketToUart(void);
//void packetTestRecv(void);
//void sendUartToASCII(uint8_t data);
//uint8_t convToASCII(uint8_t data);

//void sendUartDecimal(uint16_t decimal);
//void sendUartDecimalStub(uint8_t data);

/*
uint16_t analogCheckCounterRight = 0;
uint16_t analogCheckCounterLeft = 0;
#define ANALOG_COUNTER_MAX				1000
#define ANALOG_COUNTER_THRESHOLD		600
#define ANALOG_COUNTER_DECREASE_VALUE	5
*/

uint16_t dccTimeoutCounter = 0;
#define DCCTIMEOUT_COUNTER_MAX	800

void DCCpollerRightReset(void);
void DCCpollerLeftReset(void);

//uint8_t railcomPollerFlag = 0;
//uint16_t railcomPollerCounter = 0;

//uint16_t railcomCutoutChannel1Timming = 0;

/* DCC Pin Checker Funcs/Variables */
uint8_t oldPortStatFlag = 0;
uint8_t portChecker(void);
void portReader(uint8_t chkFlag);

//uint8_t railcomCounter2;


ISR(PORTB_PORT_vect)
{
	PORTB.INTFLAGS |= PIN4_bm;
	portReader(portChecker());
}


uint8_t portChecker(void) {
	uint8_t chFlag = 0;
	
	if ((oldPortStatFlag & PIN4_bm) != (PORTB.IN & PIN4_bm)) {
		chFlag |= 0x01;
		if (PORTB.IN & PIN4_bm) {
			oldPortStatFlag |= PIN4_bm;
		} else {
			oldPortStatFlag &= ~PIN4_bm;
		}
	}
	
	return (chFlag);
}

void portReader(uint8_t chkFlag) {
	
	if (chkFlag & 0x01) {

		if (oldPortStatFlag & PIN4_bm) {	// RAIL+
		//if (PORTA.IN & PIN2_bm) {		// RAIL+
			if ((oldInputPortStat & 0x01) == 0) {
				oldInputPortStat |= 0x01;
			
				if (dccTimerRightStart < TCB0.CNT) {
					dccTimerCounter = TCB0.CNT - dccTimerRightStart;
				} else {
					dccTimerCounter = 0xD000 - dccTimerRightStart + TCB0.CNT;
				}
			
			}
			railcomCutoutFlag = 0;
			//railcomPollerFlag = 0;
		} else {
			// Start Right Count
			if (oldInputPortStat & 0x01) {
				oldInputPortStat &= ~0x01;
				dccTimerRightStart = TCB0.CNT;
			}
		
		}
	
	}
	
}



void DCCpollerClkReceiver(void) {
	if (dccTimeoutCounter < DCCTIMEOUT_COUNTER_MAX) dccTimeoutCounter++;
}

uint8_t readAnalogStat(void)
{
	if (dccTimeoutCounter < DCCTIMEOUT_COUNTER_MAX) return (0);	// Digital Operation
	return (1);	// Analog Operation
}


void initDCCpoller(void)
{
	// Input: PA1 / PA2 (ATtiny202 Pin4/Pin5)
	//PORTA.DIRCLR = PIN1_bm | PIN2_bm;
	
	// Input: PB4 (ATtiny406-M Pin10)
	PORTB.DIRCLR = PIN4_bm;
	
	PORTB.PIN4CTRL = PORT_ISC_BOTHEDGES_gc;

	// Max 6.55msec (1 / 5MHz * 32767, 1cycle=200ns)
	//TCB0.INTCTRL = TCB_CAPT_bm;
	//TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc | TCB_ENABLE_bm | TCB_SYNCUPD_bm;
#ifdef AVR2
	TCB0.CTRLA = TCB_CLKSEL_DIV2_gc | TCB_ENABLE_bm;
#else
	TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc | TCB_ENABLE_bm;
#endif
	TCB0.CCMP = 0xCFFF;
	
	//sei();
}


//void dccPacketShifter(void)
void dccPacketShifter(uint8_t* recvPacketLength, uint8_t* recvPacket)
{
	//uint16_t newCounter;
	
	uint16_t dccTimerTemp;
	uint8_t bit;

	// デフォルトは「受信なし」
	(*recvPacketLength) = 0;

	// ISRと共有の16bitカウンタを原子的に取り出す
	cli();
	if (dccTimerCounter == 0) {
		sei();
		return;
	}
	dccTimerTemp = dccTimerCounter;
	dccTimerCounter = 0;
	sei();

	// パルス長から 0/1 を判定（それ以外は同期を壊すのでリセット）
	if ((dccTimerTemp > DCC_PULSE_LENGTH_ONE_MIN) && (dccTimerTemp < DCC_PULSE_LENGTH_ONE_MAX)) {
		bit = 1;
	} else if ((dccTimerTemp > DCC_PULSE_LENGTH_ZERO_MIN) && (dccTimerTemp < DCC_PULSE_LENGTH_ZERO_MAX)) {
		bit = 0;
	} else {
		bit = 2; // invalid
	}

	// ---- DCC受信 状態機械 ----
	// 同期前(プリアンブル探索中)は一切バッファへ書き込まない。
	// 8bit受信後は必ず区切りビット(0)/終端ビット(1)を検証する。
	static uint8_t inPacket = 0;
	static uint8_t preambleOnes = 0;

	static uint8_t bitPos = 0;      // 0..7: data bits, 8: delimiter/end bit
	static uint8_t byteIndex = 0;
	static uint8_t curByte = 0;
	static uint8_t xorSum = 0;
	static uint8_t buf[DCC_PACKET_LENGTH];

	if (bit == 2) {
		inPacket = 0;
		preambleOnes = 0;
		bitPos = 0;
		byteIndex = 0;
		curByte = 0;
		xorSum = 0;
		return;
	}

	if (!inPacket) {
		// プリアンブル: '1' が10個以上、その後の '0' がスタートビット
		if (bit) {
			if (preambleOnes < 32) preambleOnes++;
		} else {
			if (preambleOnes >= 10) {
				inPacket = 1;
				bitPos = 0;
				byteIndex = 0;
				curByte = 0;
				xorSum = 0;
			}
			preambleOnes = 0;
		}
		return;
	}

	// データビット 8個
	if (bitPos < 8) {
		curByte = (uint8_t)((curByte << 1) | bit);
		bitPos++;
		return;
	}

	// 区切り(0) or 終端(1) ビット
	if (byteIndex < DCC_PACKET_LENGTH) {
		buf[byteIndex] = curByte;
		xorSum ^= curByte;
		byteIndex++;

		if (bit == 0) {
			// 次バイトへ
			curByte = 0;
			bitPos = 0;
			return;
		}

		// bit == 1: パケット終端
		if ((xorSum == 0) && (byteIndex >= 3)) {
			// デジタル受信が続いている扱いにする（アナログ誤判定を減らす）
			dccTimeoutCounter = 0;
			for ((*recvPacketLength) = 0; (*recvPacketLength) < byteIndex; (*recvPacketLength)++) {
				recvPacket[(*recvPacketLength)] = buf[(*recvPacketLength)];
			}
		}
	}

	inPacket = 0;
	preambleOnes = 0;
	bitPos = 0;
	byteIndex = 0;
	curByte = 0;
	xorSum = 0;
	return;
}
