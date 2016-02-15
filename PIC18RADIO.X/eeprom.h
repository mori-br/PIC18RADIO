/*
    File:   eeprom.h  
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

#ifndef EEPROM_H
#define	EEPROM_H

#include "GenericTypeDefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define MAX_CHANNEL_MEM     6
#define EEPROM_ADDRESS      0x50
#define EEPROM_MAX_ADDR     0xFFFF
#define EEPROM_SIZE         EEPROM_MAX_ADDR-sizeof(RDA_MEMORY) // 512k bits, 64k bytes
    
typedef enum
{
    EEPROM_LOAD_NOT_INIT,
    EEPROM_LOAD_ALL_ERASED,
    EEPROM_LOAD_FOUND
            
} LoadRes_t;

#define RDA_FLAG_MONO        0x01
#define RDA_FLAG_RDS         0x02
#define RDA_FLAG_BASS        0x04

typedef union
{
    struct
    {
        BYTE  valid;    // valid FF, 0 not
        BYTE  valid1;   // always 0
        BYTE  flags;
        BYTE  volume;
        float channel;
        float memory[MAX_CHANNEL_MEM];
    };
    struct
    {    
        BYTE data[32];
    };
    
} RDA_MEMORY;

BYTE EEPROM_load_data(RDA_MEMORY *mem);
void EEPROM_save_data(const RDA_MEMORY *mem);
void EEPROM_write(BYTE devaddr, DWORD addr, BYTE *data, BYTE size);
void EEPROM_read(BYTE devaddr, DWORD addr, BYTE *data, BYTE size);
BYTE EEPROM_load(BYTE devaddr, WORD *addr, RDA_MEMORY *mem);
WORD EEPROM_save(BYTE devaddr, WORD addr, const RDA_MEMORY *mem);

#ifdef	__cplusplus
}
#endif

#endif	/* EEPROM_H */

