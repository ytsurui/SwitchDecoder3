/*
 * sw_config_dummy.h
 *
 * Created: 2022/10/15 23:06:34
 *  Author: Y.Tsurui
 */ 

void initConfig(void);
//uint8_t read_cv_byte(uint16_t CVnum);
//void write_cv_byte(uint16_t CVnum, uint8_t data);

uint8_t chk_addr(uint8_t addr1_data, uint8_t addr2_data);
//uint8_t write_addr(uint8_t addr1_data, uint8_t addr2_data);

uint16_t getTurnOnTime(void);
uint16_t getTurnOnDelayTime(void);
uint8_t getConfigureBytes(void);

#define CONFIG_BIT_REVERSE			0x01
#define CONFIG_BIT_DISABLEPROGPORT	0x02