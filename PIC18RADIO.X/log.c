/*
    File:   log.c
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
#include "log.h"
#include "hardwareprofile.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


void LOG_ARRAY1(const unsigned char *data, unsigned short size)
{
    int i=0;
    for(i = 0; i < size; ++i)
    {
        printf((ROMCHAR)"%02X ", data[i]);
    }
}

void LOG_ARRAY(const unsigned char *data, unsigned short size)
{
	LOG_ARRAY1(data, size);
    printf((ROMCHAR)"\r\n");
}

void LOG_ARRAYW(const unsigned short *data, unsigned short size)
{
    int i = 0;
    for(i = 0; i < size; ++i)
    {
        printf((ROMCHAR)"%04X ", data[i]);
    }
    
    printf((ROMCHAR)"\r\n");
}