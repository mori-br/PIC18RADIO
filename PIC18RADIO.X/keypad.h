/*
    File:   keypad.h
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
#ifndef KEYPAD_H
#define	KEYPAD_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "GenericTypeDefs.h"
    
// types of button events
#define BUTTON_PUSHED 								0x01
#define BUTTON_RELEASED 							0x02
#define BUTTON_REPEAT 								0x03

#define EVENT_NOTHING								0

// values for State in ButtonState
#define WAITING_FOR_BUTTON_DOWN 					0
#define DEBOUNCE_AFTER_BUTTON_DOWN					1
#define WAITING_FOR_BUTTON_UP	 					2
#define WAITING_FOR_BUTTON_UP_AFTER_AUTO_REPEAT		3
#define DEBOUNCE_AFTER_BUTTON_UP					4

typedef struct _BUTTON_STATE
{	
	BYTE  State;
	DWORD EventStartTime;
	
} BUTTON_STATE;

#define EVENT_UP_BUTTON_BASE            0x10
#define EVENT_UP_BUTTON_PUSHED          (EVENT_UP_BUTTON_BASE + BUTTON_PUSHED)
#define EVENT_UP_BUTTON_RELEASED        (EVENT_UP_BUTTON_BASE + BUTTON_RELEASED)

#define EVENT_DN_BUTTON_BASE            0x20
#define EVENT_DN_BUTTON_PUSHED          (EVENT_DN_BUTTON_BASE + BUTTON_PUSHED)
#define EVENT_DN_BUTTON_RELEASED        (EVENT_DN_BUTTON_BASE + BUTTON_RELEASED)

#define EVENT_LEFT_BUTTON_BASE          0x30
#define EVENT_LEFT_BUTTON_PUSHED		(EVENT_LEFT_BUTTON_BASE + BUTTON_PUSHED)
#define EVENT_LEFT_BUTTON_RELEASED      (EVENT_LEFT_BUTTON_BASE + BUTTON_RELEASED)

#define EVENT_RIGHT_BUTTON_BASE         0x40
#define EVENT_RIGHT_BUTTON_PUSHED		(EVENT_RIGHT_BUTTON_BASE + BUTTON_PUSHED)
#define EVENT_RIGHT_BUTTON_RELEASED     (EVENT_RIGHT_BUTTON_BASE + BUTTON_RELEASED)

#define EVENT_OK_BUTTON_BASE            0x50
#define EVENT_OK_BUTTON_PUSHED          (EVENT_OK_BUTTON_BASE + BUTTON_PUSHED)
#define EVENT_OK_BUTTON_RELEASED        (EVENT_OK_BUTTON_BASE + BUTTON_RELEASED)

#define EVENT_MENU_BUTTON_BASE          0x60
#define EVENT_MENU_BUTTON_PUSHED        (EVENT_MENU_BUTTON_BASE + BUTTON_PUSHED)
#define EVENT_MENU_BUTTON_RELEASED      (EVENT_MENU_BUTTON_BASE + BUTTON_RELEASED)

void KeypadInit(void);
BYTE GetButtonsEvents(void);

#ifdef	__cplusplus
}
#endif

#endif	/* KEYPAD_H */

