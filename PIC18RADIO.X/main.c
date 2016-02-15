/*
    File:   main.c
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
//#include <p18cxxx.h>
#include <p18f2520.h>
#include <delays.h>
#include <usart.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <i2c.h>
#include <timers.h>
#include "rda5807.h"
#include "hardwareprofile.h"
#include "LCDBlocking.h"
#include "TimeDelay.h"
#include "keypad.h"
#include "log.h"
#include "eeprom.h"
#include "rds.h"
#include "options.h"
#include "test.h"
#include <math.h>

#pragma config OSC    = HS
#pragma config WDT    = OFF
#pragma config LVP    = OFF
#pragma config MCLRE  = ON
#pragma config PWRT   = ON
#pragma config PBADEN = OFF

#define MAX_VOLUME          0x0C    //0x0F

#define SAVE_TIMEOUT        5000
#define BACK2IDLE_TIMEOUT   1000
#define MENU_TIMEOUT        50000

#define STATE_IDLE          0
#define STATE_VOLUME        1
#define STATE_MEMORY        2
#define STATE_MENU          3
#define STATE_LAST_BASIC    3
#define STATE_LAST          4

static BYTE mem_idx = 0;
static BYTE state = STATE_IDLE;
static LONG state_timeout = 0;

extern volatile RDSINFO rds;
static RDAINFO info;

typedef struct 
{
    RDA_MEMORY mem;
    LONG mem_save_timeout;
    BYTE enabled;
    
} SAVEINFO;

static SAVEINFO save_info = {{0},0,0};

static void update_lcd(void);
static void process_dn(void);
static void process_up(void);
static BYTE setup_memory(void);

void main(void)
{   
    char *c = 0;
    DWORD counter = 0, read = 0;
    BYTE  evt;
    DWORD ellapsed = 0;
    BYTE  btn_ok_pressed = 0;
    
    CMCON   = 0x07; // Comparators Off
    CVRCON  = 0;    // voltage reference=off
    ADCON1  = 0x0F; // Disable ALL adc channels, PDF page 226
    
    LED1_TRIS    = OUTPUT_PIN;
	I2C_SCL_TRIS = OUTPUT_PIN;
	I2C_SDA_TRIS = OUTPUT_PIN;
    
    // Uart
    OpenUSART(USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, 10);

    LOG((ROMCHAR)"\r\n-------------------------------------------------\r\n");
    
    KeypadInit();
    
    LCDInit();
  
    // Setup I2C
    OpenI2C(MASTER, SLEW_OFF);
    //SSPADD = 0x32; //100kHz Baud clock @20MHz
    SSPADD = 0x0B;//400kHz Baud clock @20MHz
    
    LCDErase();
    strcpypgm2ram(c, (ROMCHAR)"    STARTING    ");
    LCDWriteLine(1, c);
    
    memset(save_info.mem.data, 0, sizeof(RDA_MEMORY));

//TEST_eeprom_reset(); 

    if(!EEPROM_load_data(&save_info.mem))
    {
LOG((ROMCHAR)"loading default config\r\n");
        save_info.mem.flags   = RDA_FLAG_RDS|RDA_FLAG_BASS;
        save_info.mem.channel = 101.7;
        save_info.mem.volume  = 1;

//save_info.mem.memory[0] = 97.7;
//save_info.mem.memory[3] = 103.3;
//save_info.mem.memory[4] = 90.5;
    }
    
    LOG((ROMCHAR)"init\r\n");    
    RDA5807M_init();

    RDA5807M_PowerON();
    
    DelayMs(200);

    // Setup radio
    RDA5807M_SetVolume(save_info.mem.volume); 
    RDA5807M_SetFrequency(save_info.mem.channel);
    RDA5807M_BassBoost(save_info.mem.flags & RDA_FLAG_BASS);
    RDA5807M_Mono(save_info.mem.flags & RDA_FLAG_MONO);
    RDA5807M_RDS(save_info.mem.flags & RDA_FLAG_RDS);
    
	while(1)
	{
        LED1 = 0;
        if(counter++ > 3000)
        {
            counter = 0;
            LED1 = 1;
            DelayMs(10);
        }

        evt = GetButtonsEvents();
     	if(evt != EVENT_NOTHING)
      	{
			if(evt == EVENT_UP_BUTTON_PUSHED)
			{
                if(state != STATE_MENU)
                {
//LOG((ROMCHAR)"up %d\r\n", state);                    
                    if(++state >= STATE_LAST_BASIC)
                        state = STATE_IDLE;
//LOG((ROMCHAR)"up %d\r\n", state);
                    
                    if(state == STATE_MEMORY)
                        mem_idx = setup_memory();
                    
                    if(state > STATE_IDLE)
                        state_timeout = BACK2IDLE_TIMEOUT;
                    else 
                    {
                        state_timeout = 0;
                        rds.ps_valid = 1;
                        rds.rt_valid = 1;
                    }
                }
                else
                {
                    state_timeout = MENU_TIMEOUT;
                    MENU_run(MENU_UP);
                }
			}
			else if(evt == EVENT_DN_BUTTON_PUSHED)
			{
                if(state != STATE_MENU)
                {
//LOG((ROMCHAR)"dn %d\r\n", state);                    
                    if(--state >= 0xFF)
                        state = STATE_IDLE;
//LOG((ROMCHAR)"dn %d\r\n", state);
                    if(state > STATE_IDLE)
                        state_timeout = BACK2IDLE_TIMEOUT;
                    else 
                    {
                        state_timeout = 0;
                        rds.ps_valid = 1;
                        rds.rt_valid = 1;
                    }
                }
                else
                {
                    state_timeout = MENU_TIMEOUT;
                    MENU_run(MENU_DOWN);
                }
			}
			else if(evt == EVENT_LEFT_BUTTON_PUSHED)
			{
                //LOG((ROMCHAR)"left\r\n");
                if(state == STATE_MENU)
                {
                    state_timeout = MENU_TIMEOUT;
                    MENU_run(MENU_LEFT);
                }
                else
                    process_dn();
			}   
			else if(evt == EVENT_RIGHT_BUTTON_PUSHED)
			{
                //LOG((ROMCHAR)"right\r\n");
                if(state == STATE_MENU)
                {
                    state_timeout = MENU_TIMEOUT;
                    MENU_run(MENU_RIGHT);
                }
                else                
                    process_up();
			}   
            else if(evt == EVENT_OK_BUTTON_PUSHED)
			{
                btn_ok_pressed = 1;
                ellapsed = 0;
			}   
            else if(evt == EVENT_MENU_BUTTON_PUSHED)
			{
                LOG((ROMCHAR)"menu\r\n");

                if(state == STATE_MENU)
                    state_timeout = 1;  // change to idle
                else
                {
                    MENU_init(&save_info.mem);
                    state = STATE_MENU;
                    state_timeout = MENU_TIMEOUT;
                }
			}
            else if(evt == EVENT_OK_BUTTON_RELEASED)
            {
LOG((ROMCHAR)"OK released\r\n");
                btn_ok_pressed = 0;
                if(ellapsed < 250) 
                {
                    if(state == STATE_MENU)
                    {
                        if(MENU_run(MENU_OK)) 
                        {
                            save_info.enabled = 1;
                            state_timeout = 400;
                        }
                        else
                            state_timeout = MENU_TIMEOUT;
                    }
                    else                
                    {
                        switch(state)
                        {
                            case STATE_IDLE:
                                RDA5807M_Mute();
                                break;

                            case STATE_MEMORY:
                                if(save_info.mem.memory[mem_idx] > 0)
                                {
                                    RDA5807M_SetFrequency(save_info.mem.memory[mem_idx]);
                                    state_timeout = 1;  // change to idle
                                }
                                break;
                        }
                    }
                }
            }
		} 
        
        if(read++ > 1)
        {
            read = 0;
            RDA5807M_Read(&info);
            update_lcd();
        }
        
        if(save_info.mem_save_timeout > 0)
        {
            if(--save_info.mem_save_timeout <= 0)
            {
                save_info.mem_save_timeout = 0;
LOG((ROMCHAR)"SAVE DATA EEPROM\r\n");                      

LOG((ROMCHAR)"chann %d\r\n", (int)save_info.mem.channel);

            EEPROM_save_data(&save_info.mem);
            }
        }
        
        // Dont back to IDLE while pressing memory saving button
        if(!btn_ok_pressed && state_timeout > 0)
        {
            if(--state_timeout <= 0)
            {
LOG((ROMCHAR)"IDLE\r\n");                
                state_timeout = 0;
                rds.ps_valid = 1;
                rds.rt_valid = 1;
                state = STATE_IDLE;
            }
        }
        
        ellapsed++;
        if(btn_ok_pressed && ellapsed > 250)
        {
            save_info.mem.memory[mem_idx] = save_info.mem.channel;
            state_timeout = 1;
        }
	}
}

static void process_up(void)
{
    switch(state)
    {
        case STATE_IDLE:
            save_info.enabled = 1;
            RDA5807M_SeekUp();
            break;
    
        case STATE_VOLUME:
            if(++save_info.mem.volume >= MAX_VOLUME)
                save_info.mem.volume = MAX_VOLUME;
            state_timeout = BACK2IDLE_TIMEOUT;
            RDA5807M_SetVolume(save_info.mem.volume); 
            break;
            
        case STATE_MEMORY:
            if(++mem_idx >= MAX_CHANNEL_MEM)
                mem_idx = MAX_CHANNEL_MEM-1;
            state_timeout = BACK2IDLE_TIMEOUT;
            break;
    }
}

static void process_dn(void)
{
    switch(state)
    {
        case STATE_IDLE:    
            save_info.enabled = 1;
            RDA5807M_SeekDown();
            break;
            
        case STATE_VOLUME:
            if(--save_info.mem.volume >= 0xFF)
                save_info.mem.volume = 0;
            state_timeout = BACK2IDLE_TIMEOUT;
            RDA5807M_SetVolume(save_info.mem.volume); 
            break;
            
        case STATE_MEMORY:
            if(--mem_idx >= 0xFF)
                mem_idx = 0;
            state_timeout = BACK2IDLE_TIMEOUT;
            break;
    }
}

static void update_volume(void)
{
    BYTE i = 0;
    
    strcpypgm2ram(LCDText[1], (ROMCHAR)"VOL ");
    for(; i < save_info.mem.volume; ++i)
        strcatpgm2ram(LCDText[1], (ROMCHAR)"\xFF");
}

static BYTE setup_memory(void)
{
    BYTE i = 0;
    for(; i < MAX_CHANNEL_MEM; ++i)
    {
        if(save_info.mem.memory[i] > 0 && save_info.mem.memory[i] == info.freq)
        {
            return i;
        }
    }
    
    return 0xFF;
}

static void update_lcd(void)
{
    int num, dec;
    char c[17] = {0};
    BYTE memidx = 0;
    char m[1] = {' '};
    char m1[3] = {' ',' ', 0};
    
    static BYTE size = 0;
    static BYTE pos  = 0;
    static WORD ps   = 0;
    
    if(state < STATE_LAST_BASIC)
    {
        // Draw line 1
        
        if(state == STATE_MEMORY)
        {
            // Display current memory station
            strncpypgm2ram(LCDText[0], (ROMCHAR)"       FM ---.- ", 16);
            
            if(save_info.mem.memory[mem_idx] > 0)
            {
                num = save_info.mem.memory[mem_idx];
                dec = (int)floor(((save_info.mem.memory[mem_idx] - num) * 100)/10);
                if(dec % 2 == 0) dec += 1;
                sprintf(c, (ROMCHAR)"       FM %3d.%1d ", num, dec);
                strncpy(LCDText[0], c, 16);
            }
        }
        else
        {
            // Display station currently tunned 
            strncpypgm2ram(LCDText[0], (ROMCHAR)" \xA5\xA5 SCANNING \xA5\xA5 ", 16);
            
            //if(info.fmtrue && info.tuneok)
            //{
                num = info.freq;
                dec = (int)floor(((info.freq - num) * 100)/10);
                if(dec % 2 == 0) dec += 1;

                memidx = setup_memory();
                if(memidx != 0xFF) {
                    itoa(memidx+1, m);
                    m1[0] = 'M';
                    m1[1] = m[0];
                }
                if(info.stereo)      
                    sprintf(c, (ROMCHAR)"%1d %s   FM %3d.%1d\xA5", info.rssi, m1, num, dec);
                else
                    sprintf(c, (ROMCHAR)"%1d %s   FM %3d.%1d ", info.rssi, m1, num, dec);

                strncpy(LCDText[0], c, 16);
                
if(info.fmtrue && info.tuneok)
{
                // Start save timer
                if(save_info.enabled)
                {
                    save_info.mem.channel = info.freq;
                    save_info.mem_save_timeout = SAVE_TIMEOUT;
                    save_info.enabled = 0;
                }
            }
        }
        
        switch(state)
        {
            case STATE_IDLE:
            {
                if(!rds.use_rt && rds.ps_valid)
                {
                    rds.ps_valid = 0;
                    memset(LCDText[1], ' ', 16);
                    strncpy(&LCDText[1][4], rds.ps_copy, 8);
                }
                else
                {
                    if(rds.rt_valid)
                    {
                        rds.rt_valid = 0;
                        memset(LCDText[1], ' ', 16);
                        size = strlen(rds.rt_copy);
                        pos = 0;
                        ps = 0;
                        strncpy(LCDText[1], rds.rt_copy, size > 16 ? 16 : size);
                    }
                    else 
                    {
                        if(rds.use_rt && ps++ > 30)
                        {
                            ps = 0;
                            if(size < 16)
                                strncpy(LCDText[1], rds.rt_copy, size > 16 ? 16 : size);
                            else
                                strncpy(LCDText[1], &rds.rt_copy[pos++], (size - pos) > 16 ? 16 : size);
                        }
                    }
                }
            }
            break;

            case STATE_VOLUME:
                update_volume();
                break;

            case STATE_MEMORY:
            {
                switch(mem_idx)
                {
                    case 0:
                        strncpypgm2ram(LCDText[1], (ROMCHAR)"MEM \x0A51 2 3 4 5 6", 16);
                        break;
                    case 1:
                        strncpypgm2ram(LCDText[1], (ROMCHAR)"MEM 1 \x0A52 3 4 5 6", 16);
                        break;
                    case 2:
                        strncpypgm2ram(LCDText[1], (ROMCHAR)"MEM 1 2 \x0A53 4 5 6", 16);
                        break;
                    case 3:
                        strncpypgm2ram(LCDText[1], (ROMCHAR)"MEM 1 2 3 \x0A54 5 6", 16);
                        break;
                    case 4:
                        strncpypgm2ram(LCDText[1], (ROMCHAR)"MEM 1 2 3 4 \x0A55 6", 16);
                        break;
                    case 5:
                        strncpypgm2ram(LCDText[1], (ROMCHAR)"MEM 1 2 3 4 5 \x0A56", 16);
                        break;
                    default:
                        LOG((ROMCHAR)"ERR %d\r\n", mem_idx);
                        mem_idx = 0;
                        break;
                }
            }        
            break;
        }
    }
    else
    {
        MENU_draw();
    }
    
    LCDUpdate();
}
    
