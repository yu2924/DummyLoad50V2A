/*
 * tiny85-withadc.cpp
 *
 * Created: 2019/08/12 0:39:17
 * Author : yu2924
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
// to use float with printf, requires linker options: -Wl,-u,vfprintf -lprintf_flt -lm
#include <stdio.h>
#include "USI_TWI_Master.h"
#include "MCP342x.h"
#include "ST7032i.h"

inline uint8_t getbit(volatile uint8_t& port, uint8_t bit) {return (port >> bit) & 0x01;}
inline void setbit(volatile uint8_t& port, uint8_t bit, uint8_t v) {port = (port & (uint8_t)~(1 << bit)) | (v << bit);}
	
inline int roundd2ip(double v) { return (int)(v + 0.5); }
inline int16_t absi16(int16_t v) { return (0 <= v) ? v : -v; }

struct AverageBuffer8ui
{
	uint16_t totalsum;
	uint8_t buffer[4];
	uint8_t pos;
	uint8_t push(uint8_t vnew)
	{
		uint8_t vlast = buffer[pos & 0x03];
		totalsum -= vlast;
		buffer[pos & 0x03] = vnew;
		totalsum += vnew;
		pos ++;
		return (uint8_t)(totalsum >> 2);
	}
};

struct AverageBuffer14i
{
	int16_t totalsum;
	int16_t buffer[4];
	uint8_t pos;
	int16_t push(int16_t vnew)
	{
		int16_t vlast = buffer[pos & 0x03];
		totalsum -= vlast;
		buffer[pos & 0x03] = vnew;
		totalsum += vnew;
		pos ++;
		return totalsum >> 2;
	}
};

#define LED_PORT PORTB
#define LED_BIT PB4

static const uint8_t CG_CDEG[] =
{
	0b00001000,
	0b00010110,
	0b00001001,
	0b00001000,
	0b00001000,
	0b00001001,
	0b00000110,
	0b00000000,
};

int main(void)
{
	// clock prescaler
	CLKPR = (1<<CLKPCE); // enable change
	CLKPR = (0b0000<<CLKPS0); // divisor=1 (8MHz)
	// PortB
	DDRB = (1<<DDB4); // LED output: PB4
	PORTB = (1<<PB5)|(1<<PB1); // pull-up enabled: unused pins PB1, PB5
	// ADC
	ADMUX = (0<<REFS0)|(1<<ADLAR)|(0<<REFS2)|(0b0011<<MUX0); // vref=Vcc, alignment=left-justified, mux=ADC3(PB3)
	ADCSRA = (1<<ADEN)|(0<<ADATE)|(0<<ADIF)|(0<<ADIE)|(0b110<<ADPS0); // ADC=enabled, auto-trigger=disabled, interrupt=disabled, prescaler=1/64 (125kHz)
	ADCSRB = (0<<BIN)|(0<<IPR)|(0b000<<ADTS0); // bipolar=no, polarity-reversal=no, triggersource=freerunning
	DIDR0 = (1<<ADC3D); // digital-input-disabled-ADC3(PB3)=yes
	// timer1
	TCCR1 = (1<<CTC1)|(0b1111<<CS10); // enable CTC, divisor=16384 (488.3Hz)
	GTCCR = 0;
	OCR1A = 24; // tick = 20.35Hz (fire interrupt)
	OCR1C = 24; // tick = 20.35Hz (reset counter)
	TIMSK = (1<<OCIE1A); // enable interrupt
	PLLCSR = 0; // no PLL
	// i2c
	USI_TWI_Master_Initialise();
	MCP342x mcp3426 = {};
	ST7032i st7032i = {};
	mcp3426.initialize(MCP342X_I2CADDR);
	st7032i.initialize(ST7032_I2CADDR, 0, 35); // AQM1602XA or AQM1602Y, 5V
	st7032i.writeCommand(ST7032CmdSetDDRAMAddr | ST7032DDRAMOffsetL1);
	st7032i.writeString("Dummy Load 50V2A");
	st7032i.writeCommand(ST7032CmdSetDDRAMAddr | ST7032DDRAMOffsetL2);
	st7032i.writeString("by yu2924");
	st7032i.writeCommand(ST7032CmdSetCGRAMAddr | 8); // for char_code='\x1', char_height=8
	st7032i.writeData(CG_CDEG, sizeof(CG_CDEG));
	for(int i = 0; i < 2; i ++)
	{
		_delay_ms(500);
		setbit(LED_PORT, LED_BIT, 1);
		_delay_ms(500);
		setbit(LED_PORT, LED_BIT, 0);
	}
	st7032i.writeCommand(ST7032CmdClearDisplay);
	// run
	// T:
	//   MCP9700A: Vmeas=0.5+T/100 => Tmeas=(Vmeas-0.5)*100
	//   ADC: Vmeas/5=vcode/255 => Vmeas=5*vcode/255=vcode/51, vcode=255*Vmeas/5=Vmeas*51
	//   Tmeas=(vcode/51-0.5)*100
	//   vcode=(0.5+Tmeas/100)*51
	// V:
	//   Vfs=0.25V
	//   ADC: Vmeas/50=vcode/8191
	//   Vmeas=50*vcode/8191=vcode/163.82
	//   vcode=8191*Vmeas/50=Vmeas*163.82
	//
	// I:
	//   Vfs=0.15V
	//   ADC: Imeas/1.2=vcode/(8191*0.15/0.25)=vcode/4914.6
	//   Imeas=1.2*vcode/4914.6=vcode/4095.5
	//   vcode=4914.6*Imeas/1.2=Imeas*4095.5
	static const int Tht = 60;
	static const double Av = 1.0 / 163.82;
	static const double Ai = 1000.0 / 2800.0; // 4095.5;
	AverageBuffer8ui averaget = {};
	AverageBuffer14i averagev = {};
	AverageBuffer14i averagei = {};
	char str[16];
	bool flipch = false;
	ADCSRA |= (1<<ADSC); // trigger internal ADC
	mcp3426.StartConversion(MCP342X_CH1 | MCP342X_ONESHOT | MCP342X_SIZE_14 | MCP342X_PGA_X8);
	sei();
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_mode();
	while(1)
	{
		uint8_t vt = averaget.push(ADCH);
		int tmeas = roundd2ip(((double)vt / 51.0 - 0.5) * 100.0);
		sprintf(str, "T:%d\x1 ", tmeas);
		st7032i.writeCommand(ST7032CmdSetDDRAMAddr | (ST7032DDRAMOffsetL1 + 10));
		st7032i.writeString(str);
		setbit(LED_PORT, LED_BIT, (Tht <= tmeas) ? 1 : 0);
		ADCSRA |= (1<<ADSC); // trigger internal ADC
		if(!flipch)
		{
			int16_t v; uint8_t cfg; mcp3426.GetResult14(&v, &cfg);
			v = averagev.push(absi16(v));
			sprintf(str, "V:%4.1fV ", (double)v * Av);
			// sprintf(str, "V:%4.1fV %d ", (double)v * Av, v);
			st7032i.writeCommand(ST7032CmdSetDDRAMAddr | ST7032DDRAMOffsetL1);
			st7032i.writeString(str);
			mcp3426.StartConversion(MCP342X_CH2 | MCP342X_ONESHOT | MCP342X_SIZE_14 | MCP342X_PGA_X8);
		}
		else
		{
			int16_t v; uint8_t cfg; mcp3426.GetResult14(&v, &cfg);
			v = averagei.push(absi16(v));
			sprintf(str, "I:%2dmA ", roundd2ip((double)v * Ai));
			// sprintf(str, "I:%2dmA %d ", roundd2ip((double)v * Ai), v);
			st7032i.writeCommand(ST7032CmdSetDDRAMAddr | ST7032DDRAMOffsetL2);
			st7032i.writeString(str);
			mcp3426.StartConversion(MCP342X_CH1 | MCP342X_ONESHOT | MCP342X_SIZE_14 | MCP342X_PGA_X8);
		}
		flipch = !flipch;
		sleep_mode();
	}
	return 0;
}

ISR(TIM1_COMPA_vect)
{
}
