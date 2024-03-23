/*
 * sw_config.h
 *
 * Created: 2022/09/17 0:09:12
 *  Author: Y.Tsurui
 */ 

void initConfig(void);

uint8_t loadCVevent(void);

uint8_t read_cv_byte(uint16_t CVnum);
void write_cv_byte(uint16_t CVnum, uint8_t data);

uint8_t chk_addr(uint8_t addr1_data, uint8_t addr2_data);
uint8_t write_addr(uint8_t addr1_data, uint8_t addr2_data);

uint16_t getTurnOnTime(void);
uint16_t getTurnOnDelayTime(void);
uint8_t getConfigureBytes(void);

uint16_t getLockTime(void);
uint16_t getAutoMoveTime(void);

#define CONFIG_BIT_REVERSE			0x01
#define CONFIG_BIT_DISABLEPROGPORT	0x02

#define CONFIG_SENSORMODE_MASK	0x1C
#define CONFIG_SENSORMODE_LOCK_MASK	0x10

#define CONFIG_SENSORMODE_DISABLE 0

#define CONFIG_SENSORMODE_NONLOCK_AUTO_C		4
#define CONFIG_SENSORMODE_NONLOCK_AUTO_T		8
#define CONFIG_SENSORMODE_NONLOCK_AUTOCHANGE	12

#define CONFIG_SENSORMODE_LOCK_ONLY			16
#define CONFIG_SENSORMODE_LOCK_AUTO_C		20
#define CONFIG_SENSORMODE_LOCK_AUTO_T		24
#define CONFIG_SENSORMODE_LOCK_AUTOCHANGE	28
