/*
    File:   rda5807.h
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
#ifndef RDA5807_H
#define	RDA5807_H

#include "GenericTypeDefs.h"

#ifdef	__cplusplus
extern "C" {
#endif
    
#define R2                  0
#define R3                  1
#define R4                  2
#define R5                  3
#define R6                  4
#define R7                  5    
    
#define R2_DHIZ             0x8000
#define R2_DMUTE            0x4000
#define R2_MONO             0x2000
#define R2_BASS             0x1000
#define R2_SEEKUP           0x0200
#define R2_SEEK             0x0100
#define R2_SKMODE           0x0080
#define R2_RDS_EN           0x0008
#define R2_CTRL_NEW         0x0004
#define R2_SOFT_RESET       0x0002
#define R2_ENABLE           0x0001    

#define R3_TUNE             0x0010

#define R4_DE               0x0800
#define R4_SOFTMUTE_EN      0x0200
#define R4_AFCD             0x0100

#define R5_INT_MODE         0x8000

#define R7_65M_50M_MODE     0x0200
#define R7_SOFTBLEND_EN     0x0002
#define R7_FREQ_MODE        0x0001

#define R2_CH_SPACE_100K    0
#define R2_CH_SPACE_200K    1
#define R2_CH_SPACE_50K     2
#define R2_CH_SPACE_25K     3
    
#define R3_BAND_SEL_87_108  0   //(US/Europe)
#define R3_BAND_SEL_76_91   1   //(Japan)
#define R3_BAND_SEL_76_108  2   //(world wide)
#define R3_BAND_SEL_OTHER   3   // 65 ?76 MHz ?East Europe? or 50-65MHz depending on R7_65M_50M_MODE
        
typedef union
{
    struct
    {
        unsigned char reg_addr;        
        unsigned short value;
    };
    
    struct
    {
        unsigned char reg_addr;                
        unsigned char lo;
        unsigned char hi;
    };
    
    struct
    {
        unsigned char data[3];
    };
    
} REG;

typedef union
{
	struct
	{
		REG reg[6];
	};

	struct
	{
		unsigned char data[18];
	};

} RDA5807M_REG;


typedef struct 
{
    float freq;
    BYTE  stereo;
    BYTE  rssi;
    BYTE  tuneok;
    BYTE  fmtrue;
    BYTE  fmready;
    BYTE  tunefail;
    
} RDAINFO;


void RDA5807M_init(void);
void RDA5807M_write_reg(unsigned char regidx);
void RDA5807M_Read(RDAINFO *info);
void RDA5807M_write(void);
void RDA5807M_PowerON(void);
void RDA5807M_PowerOFF(void);
void RDA5807M_SetFrequency(float freq);
void RDA5807M_SetVolume(unsigned char volume);
void RDA5807M_SeekUp(void);
void RDA5807M_SeekDown(void);
void RDA5807M_SoftBlend(void);
void RDA5807M_BassBoost(BYTE on);
void RDA5807M_Mono(BYTE on);
void RDA5807M_Mute(void);
void RDA5807M_SoftMute(void);
void RDA5807M_RDS(BYTE on);

#ifdef	__cplusplus
}
#endif

    