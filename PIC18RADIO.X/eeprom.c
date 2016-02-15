/*
    File:   eeprom.c
    Copyright 2016 Marcos Mori de Siqueira <mori.br@gmail.com>

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "eeprom.h"
#include "hardwareprofile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <i2c.h>
#include "log.h"
#include "TimeDelay.h"

static WORD addr = 0; // current valid address

static void EEPROM_read_seq(BYTE devaddr, BYTE *data, BYTE size);

void EEPROM_write(BYTE devaddr, DWORD addr, BYTE *data, BYTE size)
{
    BYTE i = 0;

    IdleI2C();
    StartI2C();
    IdleI2C();
    // Dev address
    WriteI2C( devaddr << 1 );
    IdleI2C();
    // Memory address
    WriteI2C( addr >> 8 );
    IdleI2C();
    WriteI2C( addr & 0xFF );
    IdleI2C();
    // data
	for(; i < size; ++i)
	{
        WriteI2C( data[i] );
        IdleI2C();
	}

    StopI2C();
    
    DelayMs(5);
}

static void EEPROM_read_seq(BYTE devaddr, BYTE *data, BYTE size)
{
    BYTE i = 0;
    
    StartI2C();
    IdleI2C();
    WriteI2C( (devaddr << 1) | 0x01 );
    IdleI2C();
    
    for(; i < size; ++i)
    {
        data[i] = ReadI2C();
        if(i < size-1) {
            AckI2C();
        } else {
            NotAckI2C();
        }
    }

    StopI2C();
}

void EEPROM_read(BYTE devaddr, DWORD addr, BYTE *data, BYTE size)
{
    // Dummy read to setup address
    IdleI2C();
    StartI2C();
    IdleI2C();
    // Dev address
    WriteI2C( devaddr << 1 );
    IdleI2C();
    // Memory address
    WriteI2C( addr >> 8 );
    IdleI2C();
    WriteI2C( addr & 0xFF );
    IdleI2C();
    
    EEPROM_read_seq(devaddr, data, size);
}

BYTE EEPROM_load(BYTE devaddr, WORD *addr, RDA_MEMORY *mem)
{
    *addr = 0;
    
    while(*addr < EEPROM_SIZE)
    {
        EEPROM_read(devaddr, *addr, mem->data, sizeof(RDA_MEMORY));
        
        //LOG_ARRAY(mem->data, sizeof(RDA_MEMORY));
        
        if(mem->valid == 0xFF)
        {
            if(mem->valid1 == 0xFF) 
            {
                // Not used address
                memset(mem->data, 0, sizeof(RDA_MEMORY));
                return EEPROM_LOAD_NOT_INIT;                
            }
            else if(mem->valid1 == 0)
            {
                // found a valid position
                return EEPROM_LOAD_FOUND;                
            }
        }

        *addr += sizeof(RDA_MEMORY);
    }    

    // All apositions are marked erased
    
    return EEPROM_LOAD_ALL_ERASED;
}

WORD EEPROM_save(BYTE devaddr, WORD addr, const RDA_MEMORY *mem)
{
    BYTE b[1] = {0};
    WORD new_addr = 0;
    
    // Mark old one as deleted
    if(addr >= 0 && addr < EEPROM_MAX_ADDR)
        EEPROM_write(devaddr, addr, b, 1);
    
    // Calc new address
    if(addr < EEPROM_MAX_ADDR)
        new_addr = addr + sizeof(RDA_MEMORY);
    
    mem->valid = 0xFF;
    
    // write new registry
    EEPROM_write(devaddr, new_addr, mem->data, sizeof(RDA_MEMORY));
    
    return new_addr;
}

BYTE EEPROM_load_data(RDA_MEMORY *mem)
{
    memset(mem, 0, sizeof(RDA_MEMORY));    
    if(EEPROM_load(EEPROM_ADDRESS, &addr, mem) != EEPROM_LOAD_FOUND)
    {
//LOG((ROMCHAR)"INVALID/NOT INIT. addr 0x%04X\r\n", addr);
        addr = EEPROM_MAX_ADDR;
        return 0;
    }
    
//LOG((ROMCHAR)"VALID addr 0x%04X\r\n", addr);
//LOG_ARRAY(mem->data, sizeof(RDA_MEMORY));
    
    return 1;
}

void EEPROM_save_data(const RDA_MEMORY *mem)
{
//WORD addr1 = 0;
//RDA_MEMORY mem1;   
//int j = 0;

    addr = EEPROM_save(EEPROM_ADDRESS, addr, mem);
    
//LOG((ROMCHAR)"-------------------------------\r\n");
//LOG((ROMCHAR)"addr 0x%04X\r\n", addr);

//LOG((ROMCHAR)"Reading...\r\n");
/*    
for(j = 0; j < 10; ++j)
{
    LOG((ROMCHAR)"addr 0x%04X\r\n", addr1);
    EEPROM_read(EEPROM_ADDRESS, addr1, mem1.data, sizeof(RDA_MEMORY));
    LOG_ARRAY(mem1.data, sizeof(RDA_MEMORY));
    addr1 += sizeof(RDA_MEMORY);
}    
*/
}
