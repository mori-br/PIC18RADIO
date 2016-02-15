/*
    File:   options.h
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

#ifndef MENU_H
#define	MENU_H

#include "hardwareprofile.h"
#include "eeprom.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum
{
    MENU_UP,
    MENU_DOWN,
    MENU_LEFT,
    MENU_RIGHT,
    MENU_OK
} MenuKey_t;
    
    
typedef struct 
{
    BYTE id;
    //far rom char *title;
    char title[16];
    void (*Action)(void);
    
} MENUITEM;

void MENU_init(RDA_MEMORY *data);
BYTE MENU_run(MenuKey_t key);
void MENU_draw(void);

#ifdef	__cplusplus
}
#endif

#endif	/* MENU_H */

