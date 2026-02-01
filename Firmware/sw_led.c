/*
 * sw_led.c
 *
 * Created: 2025/12/30 18:32:46
 *  Author: ytsurui
 */ 

#include <avr/io.h>
#include "sw_config.h"
#include "sw_driver.h"

#define LED_PIN_C 2
#define LED_PIN_T 3

uint8_t led_en;

void initLEDdriver(void)
{
	PORTC.DIRSET = (1 << LED_PIN_C) | (1 << LED_PIN_T);
	PORTC.OUTCLR = (1 << LED_PIN_C) | (1 << LED_PIN_T);
}

void setLED(uint8_t sw_dir)
{
	if (getDirectionLEDenable()) {
		if (sw_dir == SW_DIR_CLOSE) {
			PORTC.OUTSET = (1 << LED_PIN_C);
			PORTC.OUTCLR = (1 << LED_PIN_T);
		} else if (sw_dir == SW_DIR_THROWN) {
			PORTC.OUTSET = (1 << LED_PIN_T);
			PORTC.OUTCLR = (1 << LED_PIN_C);
		}
	}
}