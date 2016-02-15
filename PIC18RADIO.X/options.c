/*
    File:   options.c
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
#include <p18f2520.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rda5807.h"
#include "LCDBlocking.h"
#include "TimeDelay.h"
#include "log.h"
#include "options.h"
#include "eeprom.h"
#include <math.h>

#define NUM_LINES   2
#define MAX_ITEMS   4

static BYTE cur_item = 0;
static BYTE old_cur_item = 0;
static RDA_MEMORY *mem_data = NULL;

void action_stereo(void);
static void action_bass(void);
static void action_rds(void);
static void action_about(void);


static MENUITEM menu[MAX_ITEMS] = {
    { 0, "", action_stereo },
    { 1, "", action_bass   },
    { 2, "", action_rds    },
    { 3, "", action_about  },
};

static void MENU_update(void)
{
    static const far rom char *values[] = {
        (ROMCHAR)"OFF", 
        (ROMCHAR)"ON",
    };
    
    static const far rom char *formats[] = {
        (ROMCHAR)" STEREO     ", 
        (ROMCHAR)" BASS BOOST ", 
        (ROMCHAR)" RDS        ", 
        (ROMCHAR)" ABOUT          " };
        
    memset(menu[0].title, ' ', 16);
    memset(menu[1].title, ' ', 16);
    memset(menu[2].title, ' ', 16);
   
    strcpypgm2ram(menu[0].title, formats[0]);
    strcpypgm2ram(menu[1].title, formats[1]);
    strcpypgm2ram(menu[2].title, formats[2]);
    strcpypgm2ram(menu[3].title, formats[3]);
    
    strcatpgm2ram(menu[0].title, values[!(mem_data->flags & RDA_FLAG_MONO)]);
    strcatpgm2ram(menu[1].title, values[(mem_data->flags & RDA_FLAG_BASS) == RDA_FLAG_BASS]);
    strcatpgm2ram(menu[2].title, values[(mem_data->flags & RDA_FLAG_RDS) == RDA_FLAG_RDS]);
}

void action_stereo(void)
{
LOG((ROMCHAR)"STEREO\r\n");
    if((mem_data->flags & RDA_FLAG_MONO) == RDA_FLAG_MONO)
        mem_data->flags &= ~RDA_FLAG_MONO;
    else
        mem_data->flags |= RDA_FLAG_MONO;

    RDA5807M_Mono(mem_data->flags & RDA_FLAG_MONO);
    MENU_update();
}

static void action_bass(void)
{
LOG((ROMCHAR)"BASS\r\n");    
    if((mem_data->flags & RDA_FLAG_BASS) == RDA_FLAG_BASS)
        mem_data->flags &= ~RDA_FLAG_BASS;
    else
        mem_data->flags |= RDA_FLAG_BASS;

    RDA5807M_BassBoost(mem_data->flags & RDA_FLAG_BASS);
    MENU_update();
}

static void action_rds(void)
{
LOG((ROMCHAR)"RDS\r\n");    
    if((mem_data->flags & RDA_FLAG_RDS) == RDA_FLAG_RDS)
        mem_data->flags &= ~RDA_FLAG_RDS;
    else
        mem_data->flags |= RDA_FLAG_RDS;

    RDA5807M_RDS(mem_data->flags & RDA_FLAG_RDS);
    MENU_update();
}

static void action_about(void)
{
    
}

void MENU_init(RDA_MEMORY *data)
{
    old_cur_item = 0;
    mem_data = data;
    MENU_update();
}

BYTE MENU_run(MenuKey_t key)
{
    switch(key)
    {
        case MENU_UP: 
            cur_item--; 
            break;
            
        case MENU_DOWN: 
            cur_item++; 
            break;
            
        case MENU_LEFT: 
            break;
            
        case MENU_RIGHT: 
            break;
            
        case MENU_OK: 
            if(menu[cur_item].Action != NULL)
            {
                menu[cur_item].Action();
                return 1;
            }
            break;
    }
    
    if(cur_item >= 0xFF)
        cur_item = 0;
    
    if(cur_item >= MAX_ITEMS)
        cur_item = MAX_ITEMS - 1;
    
    return 0;
}

void MENU_draw(void)
{
    BYTE i = 0;
    BYTE top = 0;
    
    if(cur_item > 0 && cur_item == old_cur_item)
        return;
    
    memset(LCDText[0], ' ', 16);
    memset(LCDText[1], ' ', 16);

    top = cur_item;
    
    if(cur_item > old_cur_item)
        top = cur_item - 1;
    
    for(i = 0; i < NUM_LINES; ++i)
    {
        strncpy(LCDText[i], menu[top + i].title, 16);
        if(menu[top + i].id == cur_item)
            LCDText[i][0] = '>';
    }

    old_cur_item = cur_item;    
}
