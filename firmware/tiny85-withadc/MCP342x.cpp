/*
 * MCP342x.c
 *
 * Created: 2019/08/05 3:30:15
 *  Author: yu2924
 */ 

#include <avr/io.h>
#include "USI_TWI_Master.h"
#include "MCP342x.h"

#define I2C_WRITE	0
#define I2C_READ	1

// assumes the AVR-GCC is a little endian

MCP342XAPI uint8_t MCP342XStartConversion(uint8_t addr, uint8_t cfg)
{
	// note: to initiate conversion, set the RDY bit too
	uint8_t buf[] = { (uint8_t)(addr | I2C_WRITE), (uint8_t)(cfg | MCP342X_RDY) };
	if(!USI_TWI_Start_Read_Write(buf, sizeof(buf))) return FALSE;
	if(!USI_TWI_Master_Stop()) return FALSE;
	return TRUE;
}

MCP342XAPI uint8_t MCP342XGetResult12(uint8_t addr, int16_t* pv, uint8_t* pcfg)
{
	uint8_t buf[] = { (uint8_t)(addr | I2C_READ), 0, 0, 0 };
	if(!USI_TWI_Start_Read_Write(buf, sizeof(buf))) return FALSE;
	if(!USI_TWI_Master_Stop()) return FALSE;
	uint8_t* pc = (uint8_t*)pv;
	pc[1] = buf[1];
	pc[0] = buf[2];
	pc[1] = (pc[1] & 0b00000111) | (pc[1] & 0b10000000);
	*pcfg = buf[3];
	return TRUE;
}

MCP342XAPI uint8_t MCP342XGetResult14(uint8_t addr, int16_t* pv, uint8_t* pcfg)
{
	uint8_t buf[] = { (uint8_t)(addr | I2C_READ), 0, 0, 0 };
	if(!USI_TWI_Start_Read_Write(buf, sizeof(buf))) return FALSE;
	if(!USI_TWI_Master_Stop()) return FALSE;
	uint8_t* pc = (uint8_t*)pv;
	pc[1] = buf[1];
	pc[0] = buf[2];
	pc[1] = (pc[1] & 0b00011111) | (pc[1] & 0b10000000);
	*pv &= 0b1001111111111111;
	*pcfg = buf[3];
	return TRUE;
}

MCP342XAPI uint8_t MCP342XGetResult16(uint8_t addr, int16_t* pv, uint8_t* pcfg)
{
	uint8_t buf[] = { (uint8_t)(addr | I2C_READ), 0, 0, 0 };
	if(!USI_TWI_Start_Read_Write(buf, sizeof(buf))) return FALSE;
	if(!USI_TWI_Master_Stop()) return FALSE;
	uint8_t* pc = (uint8_t*)pv;
	pc[1] = buf[1];
	pc[0] = buf[2];
	*pcfg = buf[3];
	return TRUE;
}

MCP342XAPI uint8_t MCP342XGetResult18(uint8_t addr, int32_t* pv, uint8_t* pcfg)
{
	uint8_t buf[] = { (uint8_t)(addr | I2C_READ), 0, 0, 0, 0 };
	if(!USI_TWI_Start_Read_Write(buf, sizeof(buf))) return FALSE;
	if(!USI_TWI_Master_Stop()) return FALSE;
	uint8_t* pc = (uint8_t*)pv;
	pc[2] = buf[1];
	pc[1] = buf[2];
	pc[0] = buf[3];
	pc[3] = (pc[2] & 0b00000010) << 6;
	pc[2] &= 0b00000001;
	*pcfg = buf[4];
	return TRUE;
}
