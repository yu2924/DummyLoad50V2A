/*
 * ST7032i.h
 *
 * Created: 2018/02/17 5:29:05
 *  Author: yu2924
 */ 

#ifndef include_ST7032_h
#define include_ST7032_h

#include <stdint.h>

#if defined(__cplusplus)
#define ST7032API extern "C"
#else
#define ST7032API
#endif

#define ST7032_I2CADDR			0b01111100

#define ST7032CoContinue		0b10000000
#define ST7032CoFinal			0b00000000
#define ST7032RSData			0b01000000
#define ST7032RSCommand			0b00000000

// commands
#define ST7032CmdClearDisplay	0b00000001
#define ST7032CmdReturnHome		0b00000010
#define ST7032CmdEntryMode		0b00000100
#define ST7032CmdDisplay		0b00001000
#define ST7032CmdShift			0b00010000
#define ST7032CmdFunction		0b00100000
#define ST7032CmdSetCGRAMAddr	0b01000000
#define ST7032CmdSetDDRAMAddr	0b10000000
#define ST7032CmdIntOscFreq		0b00010000 // extension IS=1
#define ST7032CmdPowerIconCtrl	0b01010000 // extension IS=1
#define ST7032CmdFollowerCtrl	0b01100000 // extension IS=1
#define ST7032CmdContrast		0b01110000 // extension IS=1

// flags for ST7032CmdEntryMode
#define ST7032EntryModePerform	0b01
#define ST7032EntryModeInc		0b10
#define ST7032EntryModeDec		0b00

// flags for ST7032CmdDisplay
#define ST7032DisplayOn			0b100
#define ST7032DisplayOff		0b000
#define ST7032CursorOn			0b010
#define ST7032CursorOff			0b000
#define ST7032BlinkOn			0b001
#define ST7032BlinkOff			0b000

// flags for ST7032CmdShift
#define ST7032ShiftScreen		0b1000
#define ST7032ShiftCursor		0b0000
#define ST7032ShiftRight		0b0100
#define ST7032ShiftLeft			0b0000

// flags for ST7032CmdFunction
#define ST7032DataLength8		0b10000
#define ST7032DataLength4		0b00000
#define ST7032LineNumber2		0b01000
#define ST7032LineNumber1		0b00000
#define ST7032DoubleHeight		0b00100
#define ST7032ExtensionInst		0b00001
#define ST7032NormalInst		0b00000

// flags for ST7032CmdSetDDRAMAddr
#define ST7032DDRAMAddrMask		0b1111111
#define ST7032DDRAMOffsetL1		0x00
#define ST7032DDRAMOffsetL2		0x40

// flags for ST7032CmdIntOscFreq
#define ST7032OscFreqBias4th	0b1000
#define ST7032OscFreqBias5th	0b0000
#define ST7032OscFreqMask		0b0111

// flags for ST7032CmdPowerIconCtrl
#define ST7032IconDisplayOn		0b1000
#define ST7032IconDisplayOff	0b0000
#define ST7032BoosterOn			0b0100
#define ST7032BoosterOff		0b0000
#define ST7032IconContrastMask	0b0011

// flags for ST7032CmdFollowerCtrl
#define ST7032FollowerOn		0b1000
#define ST7032FollowerOff		0b0000
#define ST7032FollowerMask		0b0111

// flags for ST7032CmdContrast
#define ST7032ContrastMask		0b1111

ST7032API void ST7032Initialize(uint8_t addr, uint8_t bon, uint8_t contrast); // bon=0or1, contrast=0~63 (bon=1 for 5V)
ST7032API void ST7032WriteCommand(uint8_t addr, uint8_t v);
ST7032API void ST7032WriteData(uint8_t addr, const uint8_t* p, uint8_t c);
ST7032API void ST7032WriteString(uint8_t addr, const char* p);

typedef struct ST7032i
{
	uint8_t i2caddress;
#if defined __cplusplus
	void initialize(uint8_t addr, uint8_t bon, uint8_t contrast) { i2caddress = addr; ST7032Initialize(i2caddress, bon, contrast); }
	void writeCommand(uint8_t v) { ST7032WriteCommand(i2caddress, v); }
	void writeData(const uint8_t* p, uint8_t c) { ST7032WriteData(i2caddress, p, c); }
	void writeString(const char* p) { ST7032WriteString(i2caddress, p); }
#endif
} ST7032i;

#endif // include_ST7032_h
