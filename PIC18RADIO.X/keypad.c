/*
    File:   keypad.c
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
#include "keypad.h"
#include <stdio.h>
#include <stdlib.h>
#include <p18f2520.h>
#include <timers.h>
#include "hardwareprofile.h"
#include "log.h"


#define BUTTON_DEBOUNCE_PERIOD 		2500

// indexes into ButtonState
#define UP_BUTTON_IDX		0
#define DN_BUTTON_IDX		1
#define LEFT_BUTTON_IDX		2
#define RIGHT_BUTTON_IDX    3
#define OK_BUTTON_IDX       4
#define MENU_BUTTON_IDX     5
#define LAST_BUTTON_IDX     6

#define UP_BUTTON           KEYPAD_BTN6_IO
#define DN_BUTTON           KEYPAD_BTN2_IO
#define LEFT_BUTTON         KEYPAD_BTN3_IO
#define RIGHT_BUTTON        KEYPAD_BTN4_IO
#define OK_BUTTON           KEYPAD_BTN1_IO
#define MENU_BUTTON         KEYPAD_BTN5_IO

static BUTTON_STATE ButtonState[LAST_BUTTON_IDX];

void KeypadInit(void)
{
    BYTE i = 0;
            
    // setup as input
    KEYPAD_BTN1_TRIS = INPUT_PIN;
    KEYPAD_BTN2_TRIS = INPUT_PIN;
    KEYPAD_BTN3_TRIS = INPUT_PIN;
    KEYPAD_BTN4_TRIS = INPUT_PIN;
    KEYPAD_BTN5_TRIS = INPUT_PIN;
    KEYPAD_BTN6_TRIS = INPUT_PIN;

    OpenTimer1(T1_16BIT_RW & T1_PS_1_8 & T1_OSC1EN_OFF && T1_SOURCE_EXT & T1_SYNC_EXT_OFF);
    
    for(i = 0; i < LAST_BUTTON_IDX; ++i) 
        ButtonState[i].State = WAITING_FOR_BUTTON_DOWN;
    
    LOG((ROMCHAR)"keypad init\r\n");    
}

static unsigned char CheckButton(unsigned char ButtonIdx)
{	
	DWORD TimerHighByte;
	DWORD CurrentTime;		

	BYTE ButtonValue;
	BYTE EventBase;
	static DWORD lastCurrentTime = 0;
	
//    if(LEFT_BUTTON == 1)
//        LOG((ROMCHAR)"!! %d, %d, %d, %d, %d\r\n", UP_BUTTON, DN_BUTTON, LEFT_BUTTON, RIGHT_BUTTON, OK_BUTTON);  
    
//LOG((ROMCHAR)"!! %d, %d, %d, %d, %d\r\n", UP_BUTTON, DN_BUTTON, LEFT_BUTTON, RIGHT_BUTTON, OK_BUTTON);  

	// read the IO bit attached to the selected button 
	switch(ButtonIdx)
	{	
		case UP_BUTTON_IDX:
			ButtonValue = UP_BUTTON;
			EventBase = EVENT_UP_BUTTON_BASE;
			break;
		
		case DN_BUTTON_IDX:
			ButtonValue = DN_BUTTON;
			EventBase = EVENT_DN_BUTTON_BASE;
			break;
			
		case LEFT_BUTTON_IDX:
			ButtonValue = LEFT_BUTTON;
			EventBase = EVENT_LEFT_BUTTON_BASE;
			break;
            
		case RIGHT_BUTTON_IDX:
			ButtonValue = RIGHT_BUTTON;
			EventBase = EVENT_RIGHT_BUTTON_BASE;
			break;
            
		case OK_BUTTON_IDX:
			ButtonValue = OK_BUTTON;
			EventBase = EVENT_OK_BUTTON_BASE;
			break;            
            
		case MENU_BUTTON_IDX:
			ButtonValue = MENU_BUTTON;
			EventBase = EVENT_MENU_BUTTON_BASE;
			break;            
	}

	// check if nothing is going on (waiting for button to go down and it's not pressed)
	if ((ButtonState[ButtonIdx].State == WAITING_FOR_BUTTON_DOWN) && (ButtonValue == 0))
    {
		return(EVENT_NOTHING);	// return no new event
    }

	// something is going on, read the time
    TimerHighByte = ReadTimer1();
	CurrentTime   = TimerHighByte;		

//LOG((ROMCHAR)"algo pressionado %d, %d, %d, %ld\r\n", ButtonValue, ButtonState[ButtonIdx].State, ButtonIdx, TimerHighByte);  
    
	// FIX: se o timer rodar preciso corrigir o start senao nao dispara nunca
	if(lastCurrentTime != 0 && lastCurrentTime > CurrentTime)
		ButtonState[ButtonIdx].EventStartTime -= (lastCurrentTime - CurrentTime);
		
	lastCurrentTime = CurrentTime;
	
	// check the state that the button was in last
	switch(ButtonState[ButtonIdx].State)
	{	
		// waiting for button to go down
		case WAITING_FOR_BUTTON_DOWN:
			// check if button is now down
			if (ButtonValue == 1)				
			{	
				// button down, start timer
				ButtonState[ButtonIdx].EventStartTime = TimerHighByte;
				ButtonState[ButtonIdx].State = DEBOUNCE_AFTER_BUTTON_DOWN;
                
				// return button "pressed" event
				return(EventBase + BUTTON_PUSHED); 
			}
			break;
			
		// waiting for timer after button has gone down	
		case DEBOUNCE_AFTER_BUTTON_DOWN:		
			// check if it has been up long enough
			if (CurrentTime >= (ButtonState[ButtonIdx].EventStartTime + BUTTON_DEBOUNCE_PERIOD))
			{	
				// debouncing period over, start auto repeat timer
				ButtonState[ButtonIdx].EventStartTime = TimerHighByte;
				ButtonState[ButtonIdx].State = WAITING_FOR_BUTTON_UP;
			}
			break;
				
		// waiting for button to go back up
		case WAITING_FOR_BUTTON_UP:
			// check if button is now up
			if (ButtonValue == 0)
			{									
				// button up, start debounce timer
				ButtonState[ButtonIdx].EventStartTime = TimerHighByte;
				ButtonState[ButtonIdx].State = DEBOUNCE_AFTER_BUTTON_UP;
			
				// return button "released" event
				return(EventBase + BUTTON_RELEASED); 
			}
			break;
		
		// waiting for timer after button has gone up
		case DEBOUNCE_AFTER_BUTTON_UP:			
			if (CurrentTime >= (ButtonState[ButtonIdx].EventStartTime + BUTTON_DEBOUNCE_PERIOD))
				ButtonState[ButtonIdx].State = WAITING_FOR_BUTTON_DOWN;
          
			break;
	}

	// return no new event
	return(EVENT_NOTHING);
}

BYTE GetButtonsEvents(void)
{	
	BYTE Event = 0;
	
	Event = CheckButton(UP_BUTTON_IDX);
	if (Event != EVENT_NOTHING) 
		return(Event);
	
	Event = CheckButton(DN_BUTTON_IDX);
	if (Event != EVENT_NOTHING) 
		return(Event);

	Event = CheckButton(LEFT_BUTTON_IDX);
	if (Event != EVENT_NOTHING) 
		return(Event);

	Event = CheckButton(RIGHT_BUTTON_IDX);
	if (Event != EVENT_NOTHING) 
		return(Event);

	Event = CheckButton(OK_BUTTON_IDX);
	if (Event != EVENT_NOTHING) 
		return(Event);

    Event = CheckButton(MENU_BUTTON_IDX);
	if (Event != EVENT_NOTHING) 
		return(Event);

	return(EVENT_NOTHING);
}
