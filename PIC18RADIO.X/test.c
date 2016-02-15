#include "hardwareprofile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <i2c.h>
#include "log.h"
#include "TimeDelay.h"
#include "eeprom.h"
#include "test.h"

#ifdef TEST_EEPROM

static WORD addr = 0; // current valid address

BYTE data[128] = {0};
    
/*
 * 512K bits = 64k bytes
 The 512K bits is internally organized as 512 pages of 128-bytes each
 */
void TEST_eeprom_reset(void)
{
    WORD i = 0;
    WORD addr = 0;

    memset(data, 0xFF, 128);
    
LOG((ROMCHAR)"reset\r\n");

    // 512 pages of 128 bytes
    for(; i < 512u; ++i)
    {
        EEPROM_write(EEPROM_ADDRESS, addr, data, 128);
        addr += 128;
        DelayMs(10);
    }
LOG((ROMCHAR)"reset ok\r\n");
}

static void TEST_eeprom_setup(RDA_MEMORY *mem, BYTE x)
{
    mem->valid = 0xFF;

    mem->channel   = 1.0  + x;
    mem->memory[0] = 10.0 + x;
    mem->memory[1] = 20.0 + x;
    mem->memory[2] = 30.0 + x;
    mem->memory[3] = 40.0 + x;
    mem->memory[4] = 50.0 + x;
    mem->memory[5] = 60.0 + x;
}

void TEST_eeprom_test1(void)
{
    RDA_MEMORY mem;    
    static WORD i = 0;
    WORD addr1 = 0;
    WORD j = 0;
    
    memset(mem.data, 0, sizeof(RDA_MEMORY));
    
    if(EEPROM_load(EEPROM_ADDRESS, &addr, &mem) != EEPROM_LOAD_FOUND)
    {
        LOG((ROMCHAR)"Endereco nao valido/nao iniciado. addr 0x%04X\r\n", addr);
        LOG_ARRAY(mem.data, sizeof(RDA_MEMORY));
        addr = EEPROM_MAX_ADDR;
    }
    else
    {
        LOG((ROMCHAR)"Endereco ok. addr 0x%04X\r\n", addr);
        LOG_ARRAY(mem.data, sizeof(RDA_MEMORY));
    }

    LOG((ROMCHAR)"writing...\r\n");
    TEST_eeprom_setup(&mem, i);

    LOG((ROMCHAR)"addr 0x%04X\r\n", addr);
    addr = EEPROM_save(EEPROM_ADDRESS, addr, &mem);
    
    LOG((ROMCHAR)"-------------------------------\r\n");
    LOG((ROMCHAR)"Reading...\r\n");
    
    for(j = 0; j < 10; ++j)
    {
        LOG((ROMCHAR)"addr 0x%04X\r\n", addr1);        
        EEPROM_read(EEPROM_ADDRESS, addr1, mem.data, sizeof(RDA_MEMORY));
        LOG_ARRAY(mem.data, sizeof(RDA_MEMORY));
        addr1 += sizeof(RDA_MEMORY);
    }
    
    i++;
    
    LOG((ROMCHAR)"Test end\r\n");    
}

void TEST_eeprom_test(void)
{
/*    RDA_MEMORY mem;    
    WORD i = 0;
    
addr = 0;

    memset(mem.data, 0, sizeof(RDA_MEMORY));
    
    if(!EEPROM_load(EEPROM_ADDRESS, &addr, &mem))
    {
        LOG((ROMCHAR)"Endereco nao valido/nao iniciado. addr 0x%04X\r\n", addr);
        LOG_ARRAY(mem.data, sizeof(RDA_MEMORY));
    
        mem.valid = 0xFF;
        
        LOG((ROMCHAR)"writing...\r\n");
        for(i = 0; i < 10; ++i)
        {
            mem.channel   = 1.0  + i;
            mem.memory[0] = 10.0 + i;
            mem.memory[1] = 20.0 + i;
            mem.memory[2] = 30.0 + i;
            mem.memory[3] = 40.0 + i;
            mem.memory[4] = 50.0 + i;
            mem.memory[5] = 60.0 + i;

            LOG((ROMCHAR)"addr 0x%04X\r\n", addr);        
            EEPROM_write(EEPROM_ADDRESS, addr, mem.data, sizeof(RDA_MEMORY));
            addr += sizeof(RDA_MEMORY);
        
            DelayMs(10);
        }
    }   
    else
    {
        LOG((ROMCHAR)"Endereco ok. addr 0x%04X\r\n", addr);
        LOG_ARRAY(mem.data, sizeof(RDA_MEMORY));
    }
    
    LOG((ROMCHAR)"-------------------------------\r\n");
    LOG((ROMCHAR)"Reading...\r\n");
    
    for(i = 0; i < 10; ++i)
    {
        LOG((ROMCHAR)"addr 0x%04X\r\n", addr);        
        EEPROM_read(EEPROM_ADDRESS, addr, mem.data, sizeof(RDA_MEMORY));
        LOG_ARRAY(mem.data, sizeof(RDA_MEMORY));
        addr += sizeof(RDA_MEMORY);
    }
    
    LOG((ROMCHAR)"Test end\r\n");
 */ 
}


#endif // TEST_EEPROM



