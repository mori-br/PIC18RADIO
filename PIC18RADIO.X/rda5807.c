/*
    File:   rda5807.c
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
#include "rda5807.h"
#include <stdio.h>
#include <stdlib.h>
#include <i2c.h>
#include "log.h"
#include <timers.h>
#include <string.h>
#include "hardwareprofile.h"
#include "rds.h"


static RDA5807M_REG radio_reg;

void RDA5807M_init(void)
{
	radio_reg.reg[R2].reg_addr = 0x02;
	radio_reg.reg[R2].value = R2_DHIZ|R2_DMUTE|R2_ENABLE|R2_RDS_EN;

	radio_reg.reg[R3].reg_addr = 0x03;
	radio_reg.reg[R3].value = 0;

	radio_reg.reg[R4].reg_addr = 0x04;
	radio_reg.reg[R4].value = 0;

	radio_reg.reg[R5].reg_addr = 0x05;
	radio_reg.reg[R5].value = R5_INT_MODE|0x800|0x0F;

	radio_reg.reg[R6].reg_addr = 0x06;
	radio_reg.reg[R6].value = 0;

	radio_reg.reg[R7].reg_addr = 0x07;
	radio_reg.reg[R7].value = 0x4000;
    
    RDA5807M_write();
    
    RDS_reset();
}

void RDA5807M_write_reg(unsigned char regidx)
{
    IdleI2C();
    StartI2C();
    IdleI2C();
    WriteI2C( 0x11 << 1 );
    IdleI2C();
    WriteI2C( radio_reg.reg[regidx].reg_addr );
    IdleI2C();
    WriteI2C( radio_reg.reg[regidx].hi );
    IdleI2C();
    WriteI2C( radio_reg.reg[regidx].lo );
    IdleI2C();
    StopI2C();      
}

// adress
// hi
// lo
void RDA5807M_write(void)
{
    BYTE i = 0;

    IdleI2C();
    StartI2C();
    IdleI2C();
    WriteI2C( 0x10 << 1 );
    IdleI2C();
  
	for(i = 0; i < 6; ++i)
	{
        WriteI2C( radio_reg.reg[i].hi );
        IdleI2C();
        WriteI2C( radio_reg.reg[i].lo );
        IdleI2C();
	}

    StopI2C();
}

// value = 0 -> 64 (log)
// return 0 -> 5
static BYTE proc_rssi(BYTE value)
{
    if(value > 0 && value <= 2)
        return 1;
    if(value > 2 && value <= 4)
        return 2;
    if(value > 4 && value <= 8)
        return 3;
    if(value > 8 && value <= 32)
        return 4;
    if(value > 32 && value <= 64)
        return 5;
    
    return 0;
}

void RDA5807M_Read(RDAINFO *info)
{
	BYTE i = 0;
    BYTE x = 0;
    BYTE data[12] = {0};
	WORD buffer[6] = {0};
    
	DWORD freq = 0;

	BYTE rdsready = 0;
	//BYTE tuneok = 0;
	BYTE rds_sync =0;
	BYTE blke = 0;

	BYTE ABCD_E = 0;
	BYTE BLERA = 0;
    BYTE BLERB = 0;
    
    IdleI2C();
    StartI2C();
    IdleI2C();

    WriteI2C( (0x10 << 1) | 0x01 );
    IdleI2C();

    for(i = 0; i < 12; ++i)
    {
        data[i] = ReadI2C();
        if(i < 11) {
            AckI2C();
        } else {
            NotAckI2C();
        }
    }

    StopI2C();

	for(i = 0; i < 12; ++i)
	{
		buffer[x] = data[i];
        i++;
		buffer[x] <<= 8;
		buffer[x] |= data[i];
		x++;
	}

	freq          = (((buffer[0] & 0x03FF) * 100) + 87000);
    info->freq    = (float)(((float)freq) / 1000);
	
    // 0 - 3F (RSSI scale is logarithmic)
    info->rssi    = proc_rssi(buffer[1] >> 9);
    
    // Stereo Indicator. 0 = Mono; 1 = Stereo
	info->stereo  = (buffer[0] >> 10) & 0x01;
    // 1=ready, 0=not ready
	info->fmready = buffer[1] & 0x0008;
    // 1 = the current channel is a station
    // 0 = the current channel is not a station
	info->fmtrue  = (buffer[1] >> 8) &0x01;
    // Seek/Tune Complete.
    // 0 = Not complete 1 = Complete
    // The seek/tune complete flag is set when the seek or tune operation completes.
	info->tuneok  = (buffer[0] >> 14) & 0x01;
    // RDS ready 0 = No RDS/RBDS group ready(default) 1 = New RDS/RBDS group ready    
	rdsready = (buffer[0] >> 15) & 0x01;
    // Seek Fail.
    // 0 = Seek successful; 1 = Seek failure
    // The seek fail flag is set when the seek operation fails to find a channel 
    // with an RSSI level greater than SEEKTH[5:0].    
    info->tunefail = (buffer[0] >> 13) & 0x01;
    // RDS Synchronization 
    // 0 = RDS decoder not synchronized(default) 
    // 1 = RDS decoder synchronized Available only in RDS Verbose mode    
	rds_sync = (buffer[0] >> 12) & 0x01;
    // When RDS enable:
    // 1 = Block E has been found
    // 0 = no Block E has been found
	blke = (buffer[0] >> 11) & 0x01;
    // 1= the block id of register 0cH,0dH,0eH,0fH is E
    // 0= the block id of register 0cH, 0dH, 0eH,0fH is A, B, C, D
	ABCD_E = (buffer[1] >> 4) & 0x01;
    // Block Errors Level of RDS_DATA_0, and is always read as Errors Level of 
    // RDS BLOCK A (in RDS mode) or BLOCK E (in RBDS mode when ABCD_E flag is 1)
    // 00= 0 errors requiring correction
    // 01= 1~2 errors requiring correction
    // 10= 3~5 errors requiring correction
    // 11= 6+ errors or error in checkword, correction not possible.
    // Available only in RDS Verbose mode    
    BLERA = (buffer[1] >> 2) & 0x03;
    // Block Errors Level of RDS_DATA_1, and is always read as Errors Level of 
    // RDS BLOCK B (in RDS mode ) or E (in RBDS mode when ABCD_E flag is 1).
    // 00= 0 errors requiring correction
    // 01= 1~2 errors requiring correction
    // 10= 3~5 errors requiring correction
    // 11= 6+ errors or error in checkword, correction not possible.
    // Available only in RDS Verbose mode            
	BLERB = buffer[1] & 0x03;

//LOG((ROMCHAR)"%04X, %04X, %02X, %u\r\n", buffer[1], buffer[1] >> 9, info->rssi, info->rssi);    
/*	
	LOG((ROMCHAR)"RDSS %d, BLK_E %d ABCD_E %d BLERA %d BLERB %d\r\n", rds_sync, blke, ABCD_E, BLERA, BLERB);
*/

    if(BLERA > 0 || BLERB > 0) 
    {
//    LOG((ROMCHAR)"BLERA %d BLERB %d\r\n", BLERA, BLERB);
        return;
    }
    
	if(rdsready)
		RDS_parse(&buffer[2]);
}

void RDA5807M_PowerON(void)
{
	radio_reg.reg[R3].value |= R3_TUNE;//|R2_CH_SPACE_100K|0x10;
	radio_reg.reg[R2].value |= R2_ENABLE|R2_CTRL_NEW;//|0x800;
    radio_reg.reg[R7].value = 0;
	RDA5807M_write();
}

void RDA5807M_PowerOFF(void)
{
	radio_reg.reg[R2].value &= ~R2_ENABLE;
	RDA5807M_write();
}

void RDA5807M_SetFrequency(float freq)
{
    // keep only the SPACE[1:0] e o BAND[1:0]
	WORD regChannel = radio_reg.reg[R3].value & 0x0F;
    WORD newChannel = (WORD) ((freq * 10) - 870);
    
    LOG((const far rom char*)"CH %d\r\n", newChannel);
    
	regChannel |= R3_TUNE;
	regChannel |= newChannel << 6;

	radio_reg.reg[R3].value = regChannel;
    RDA5807M_write_reg(R3);
    RDS_reset();
}

void RDA5807M_SetVolume(unsigned char volume)
{
	radio_reg.reg[R5].value &= ~0x0F; // Clean up volume
	radio_reg.reg[R5].value |= (volume & 0x0F);
	RDA5807M_write_reg(R5);
}

void RDA5807M_SeekUp(void)
{
	radio_reg.reg[R2].value |= (R2_SEEKUP|R2_SEEK);
	RDA5807M_write_reg(R2);
	radio_reg.reg[R2].value &= ~R2_SEEK;
    RDS_reset();
}

void RDA5807M_SeekDown(void)
{
	radio_reg.reg[R2].value &= ~R2_SEEKUP;
	radio_reg.reg[R2].value |= R2_SEEK;
	RDA5807M_write_reg(R2);
	radio_reg.reg[R2].value &= ~R2_SEEK;
    RDS_reset();
}

void RDA5807M_BassBoost(BYTE on)
{
    if(on)
    {
        if ((radio_reg.reg[R2].value & R2_BASS) == 0)
        {
        	radio_reg.reg[R2].value |= R2_BASS;
            RDA5807M_write_reg(R2);
        }
    }
    else
    {
        radio_reg.reg[R2].value &= ~R2_BASS;
        RDA5807M_write_reg(R2);
    }
    
/*    if ((radio_reg.reg[R2].value & R2_BASS) == 0)
    	radio_reg.reg[R2].value |= R2_BASS;
    else
    	radio_reg.reg[R2].value &= ~R2_BASS;
    
    RDA5807M_write_reg(R2);*/
}

void RDA5807M_Mono(BYTE on)
{
    if(on)
    {
        if ((radio_reg.reg[R2].value & R2_MONO) == 0)
        {
            radio_reg.reg[R2].value |= R2_MONO;
            //RDA5807M_write_reg(R2);
            RDA5807M_write();
        }
    }
    else
    {
        radio_reg.reg[R2].value &= ~R2_MONO;
        //RDA5807M_write_reg(R2);
        RDA5807M_write();
    }
/*    if ((radio_reg.reg[R2].value & R2_MONO) == 0)
    	radio_reg.reg[R2].value |= R2_MONO;
    else
    	radio_reg.reg[R2].value &= ~R2_MONO;
    RDA5807M_write_reg(R2);*/
}

void RDA5807M_Mute(void)
{
    if ((radio_reg.reg[R2].value & R2_DMUTE) == 0)
    	radio_reg.reg[R2].value |= R2_DMUTE;
    else
    	radio_reg.reg[R2].value &= ~R2_DMUTE;
    RDA5807M_write_reg(R2);
}

void RDA5807M_SoftMute(void)
{
    if ((radio_reg.reg[R4].value & R4_SOFTMUTE_EN) == 0)
    	radio_reg.reg[R4].value |= R4_SOFTMUTE_EN;
    else
    	radio_reg.reg[R4].value &= ~R4_SOFTMUTE_EN;
    RDA5807M_write_reg(R4);
}

void RDA5807M_SoftBlend(void)
{
    if ((radio_reg.reg[R7].value & R7_SOFTBLEND_EN) == 0)
    	radio_reg.reg[R7].value |= R7_SOFTBLEND_EN;
    else
    	radio_reg.reg[R7].value &= ~R7_SOFTBLEND_EN;
    RDA5807M_write_reg(R7);
}

void RDA5807M_RDS(BYTE on)
{
    if(on)
    {
        if ((radio_reg.reg[R2].value & R2_RDS_EN) == 0)
        {
            radio_reg.reg[R2].value |= R2_RDS_EN;
            RDA5807M_write_reg(R2);
        }
    }
    else
    {
    	radio_reg.reg[R2].value &= ~R2_RDS_EN;
        RDA5807M_write_reg(R2);
    }
    
/*    if ((radio_reg.reg[R2].value & R2_RDS_EN) == 0)
    	radio_reg.reg[R2].value |= R2_RDS_EN;
    else
    	radio_reg.reg[R2].value &= ~R2_RDS_EN;
    RDA5807M_write_reg(R2);    */
}