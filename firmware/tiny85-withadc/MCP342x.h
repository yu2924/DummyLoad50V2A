/*
 * MCP342x.h
 *
 * Created: 2019/08/05 3:29:52
 *  Author: yu2924
 */ 

#if !defined include_MCP342x_h
#define include_MCP342x_h

#include <stdint.h>

#if defined(__cplusplus)
#define MCP342XAPI extern "C"
#else
#define MCP342XAPI
#endif

// device_code=1101 cf. MCP3426-3427-3428.pdf "5.3.1 I2C DEVICE ADDRESSING"
#define MCP342X_I2CADDR		0b11010000

// configuration register bits

// Ready Bit
#define MCP342X_RDY			0b10000000

// Channel Selection Bits
#define MCP342X_CH1			0b00000000
#define MCP342X_CH2			0b00100000
#define MCP342X_CH3			0b01000000
#define MCP342X_CH4			0b01100000

// Conversion Mode Bit
#define MCP342X_CONTINUOUS	0b00010000
#define MCP342X_ONESHOT		0b00000000

// Sample Rate Selection Bits
#define MCP342X_SIZE_12		0b00000000
#define MCP342X_SIZE_14		0b00000100
#define MCP342X_SIZE_16		0b00001000
#define MCP342X_SIZE_18		0b00001100
#define MCP342X_SIZE_MASK	0b00001100

// PGA Gain Selection Bits
#define MCP342X_PGA_X1		0b00000000
#define MCP342X_PGA_X2		0b00000001
#define MCP342X_PGA_X4		0b00000010
#define MCP342X_PGA_X8		0b00000011
#define MCP342X_PGA_MASK	0b00000011

MCP342XAPI uint8_t MCP342XStartConversion(uint8_t addr, uint8_t cfg);
MCP342XAPI uint8_t MCP342XGetResult12(uint8_t addr, int16_t* pv, uint8_t* pcfg);
MCP342XAPI uint8_t MCP342XGetResult14(uint8_t addr, int16_t* pv, uint8_t* pcfg);
MCP342XAPI uint8_t MCP342XGetResult16(uint8_t addr, int16_t* pv, uint8_t* pcfg);
MCP342XAPI uint8_t MCP342XGetResult18(uint8_t addr, int32_t* pv, uint8_t* pcfg);

#define MCP342X_FINISHED(cfg) (((cfg)&MCP342X_RDY) == 0)

typedef struct MCP342x
{
	uint8_t i2caddress;
#if defined __cplusplus
	void initialize(uint8_t addr) { i2caddress = addr; }
	uint8_t StartConversion(uint8_t cfg) { return MCP342XStartConversion(i2caddress, cfg); }
	uint8_t GetResult12(int16_t* pv, uint8_t* pcfg) { return MCP342XGetResult12(i2caddress, pv, pcfg); }
	uint8_t GetResult14(int16_t* pv, uint8_t* pcfg) { return MCP342XGetResult14(i2caddress, pv, pcfg); }
	uint8_t GetResult16(int16_t* pv, uint8_t* pcfg) { return MCP342XGetResult16(i2caddress, pv, pcfg); }
	uint8_t GetResult18(int32_t* pv, uint8_t* pcfg) { return MCP342XGetResult18(i2caddress, pv, pcfg); }
#endif // __cplusplus
} MCP342x;


#endif // include_MCP342x_h
