/*
 * ST7032i.c
 *
 * Created: 2018/02/17 5:29:19
 *  Author: yu2924
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <alloca.h>
#include "USI_TWI_Master.h"
#include "ST7032i.h"

#define I2C_WRITE	0
#define I2C_READ	1

// 
// command sequence:
// 1. I2C address + R/W bit
// 2. control byte
//   D7:CO  CO=1:continue, CO=0:final
//   D6:RS  RS=0:command, RS=1:display data
// 3. data byte
// 

ST7032API void ST7032Initialize(uint8_t addr, uint8_t bon, uint8_t contrast)
{
	// cf. Sitronix ST7032i datasheet, ST7032_v1.4.pdf, P29, Initializing by Instruction
	_delay_ms(40);
	ST7032WriteCommand(addr, ST7032CmdFunction | ST7032DataLength8 | ST7032LineNumber2);
	ST7032WriteCommand(addr, ST7032CmdFunction | ST7032DataLength8 | ST7032LineNumber2 | ST7032ExtensionInst); // begin extension IS=1
	ST7032WriteCommand(addr, ST7032CmdIntOscFreq | ST7032OscFreqBias5th | 0b100); // BS | Fn(adjust internal OSC frequency for FR frequency)
	// about contrast and power-icon-control:
	// akizuki recomends:
	//   contrast=35(0b100011) for 5V
	//   BON=1
	// rumors: http://miha.jugem.cc/?eid=172
	//   5V: { 0x73(0b01110011), 0x56(0b01010110) } i.e. { ST7032CmdContrast|0b0011, ST7032CmdPowerIconCtrl|ST7032BoosterOn|0b10 }
	//   3V: { 0x73(0b01110011), 0x52(0b01010010) } i.e. { ST7032CmdContrast|0b0011, ST7032CmdPowerIconCtrl|0b10 }
	ST7032WriteCommand(addr, ST7032CmdContrast | (contrast & 0b1111)); // 0b0111xxxx
	ST7032WriteCommand(addr, ST7032CmdPowerIconCtrl | ST7032IconDisplayOff | (bon?ST7032BoosterOn:ST7032BoosterOff) | ((contrast>>4)&0b11)); // 0b010100xx
	ST7032WriteCommand(addr, ST7032CmdFollowerCtrl | ST7032FollowerOn | 0b100); // Fon(follower circuit on/off) | Rab(follower amplified ratio)
	_delay_ms(200);
	ST7032WriteCommand(addr, ST7032CmdFunction | ST7032DataLength8 | ST7032LineNumber2 | ST7032NormalInst); // end extension IS=0
	ST7032WriteCommand(addr, ST7032CmdClearDisplay);
	ST7032WriteCommand(addr, ST7032CmdDisplay | ST7032DisplayOn | ST7032CursorOff | ST7032BlinkOff);
}

ST7032API void ST7032WriteCommand(uint8_t addr, uint8_t v)
{
	uint8_t buf[] = { (uint8_t)(addr | I2C_WRITE), ST7032CoFinal | ST7032RSCommand, v };
	USI_TWI_Start_Read_Write(buf, sizeof(buf));
	USI_TWI_Master_Stop();
	_delay_ms(1);
}

ST7032API void ST7032WriteData(uint8_t addr, const uint8_t* p, uint8_t c)
{
	uint8_t* buf = (uint8_t*)alloca(c * 2 + 1);
	buf[0] = addr | I2C_WRITE;
	for(int i = 0; i < c; i ++)
	{
		buf[i * 2 + 1] = ((i < (c-1)) ? ST7032CoContinue : ST7032CoFinal) | ST7032RSData;
		buf[i * 2 + 2] = p[i];
	}
	USI_TWI_Start_Read_Write(buf, c * 2 + 1);
	USI_TWI_Master_Stop();
	_delay_ms(1);
}

ST7032API void ST7032WriteString(uint8_t addr, const char* p)
{
	uint8_t c = 0; while(p[c]) c ++;
	ST7032WriteData(addr, (const uint8_t*)p, c);
}
