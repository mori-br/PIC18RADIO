/*
    File:   rds.c
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
/*
http://www.interactive-radio-system.com/docs/EN50067_RDS_Standard.pdf
*/
#include <stdio.h>
#include <stdlib.h>
#include <timers.h>
#include <string.h>
#include "rds.h"
#include "log.h"
#include "hardwareprofile.h"

volatile RDSINFO rds;

static BYTE parse_text(WORD *blocks);
static BYTE parse_basic(WORD *blocks);

#ifdef __PARSE_TIME__
static BYTE parse_time(WORD *blocks);
#endif

static char rt_data[65];
static char ps_data[9];

void RDS_reset(void)
{
LOG((ROMCHAR)"rds reset\r\n");

    memset(rt_data, 0, 65);
    memset(rds.rt_copy, 0, 65);
    memset(ps_data, 0, 9);
    memset(rds.ps_copy, 0, 9);
    rds.ps_valid = 1;
    rds.rt_valid = 1;
    rds.use_rt = 0;
}

//
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |A|A|A|A|V|T|P|P|P|P|P| | | | | |
// |3|2|1|0| | |4|3|2|1|0| | | | | |
// +---------+
//  group id
//             +---------+
//                 pty
//
void RDS_parse(WORD *blocks)
{
    BYTE group = 0;
    
    if (blocks[0] == 0)
        return;

    group = (blocks[1] >> 11) & 0x1F;
    
    switch (group) 
    {
        case 0:
        case 1:
            //Basic Tuning and Switching Information only 0A/0B
            if(parse_basic(blocks)){
                //LOG((ROMCHAR)">> %s\r\n", rds.ps_copy);
                rds.ps_valid = 1;
            }
            break;
            
        case 4:
        case 5:
            //Radio Text only 2A/2B
            if(parse_text(blocks)) {
                //LOG((ROMCHAR)"[%s]\r\n", rds.rt_copy);
                rds.rt_valid = 1;
                rds.use_rt = 1;
            }
            break;
            
        case 8:
            // Clock Time and Date only 4A
            //parse_time(blocks);
            break;
            
        default:
            break;
  }
}

static BYTE parse_basic(WORD *blocks)
{
    BYTE segment, pos, ret = 0;
    static BYTE size = 0;
    char c = 0;
    
    segment = (blocks[1] & 3);

    pos = segment * 2;
    
    if(pos == 0)
    {
        if(size > 0) 
        {
            size = 0;
            ps_data[8] = '\0';        
        
            if (strcmp(rds.ps_copy, ps_data) != 0)
            {
                strncpy(rds.ps_copy, ps_data, 8);
                memset(ps_data, 0, 8);            
                ret = 1;
            }
        }
    }
    
    c = (blocks[3] >> 8) & 0xFF;
    if(c >= 0x20 && c <= 0x7E)    
        ps_data[pos] = c;
    else
        ps_data[pos] = ' ';
    
    c = (blocks[3] & 0xFF);
    if(c >= 0x20 && c <= 0x7E)    
        ps_data[pos+1] = c;
    else
        ps_data[pos+1] = ' ';
    
    size = pos;
    
    return ret;
}

static BYTE parse_char(int pos, char c)
{
    switch (c) 
    {
        case 0x0A:
            rt_data[pos] = ' ';
            break;

        case 0x0D:
            rt_data[pos] = '\0';
            return 1;

        default:
            if(c >= 0x20 && c <= 0x7E)
                rt_data[pos] = c;
            else
                rt_data[pos] = ' ';
            break;
    }
    
    return 0;
}
static BYTE is_blank(char *buffer, WORD size)
{
    WORD i = 0;
    for(i=0;i<size;++i)
    {
        if(buffer[i] != ' ' && buffer[i] != 0)
            return 0;
    }
    
    return 1;
}

static void rtrim(char *buffer, WORD size)
{
    WORD i = 0;
    for(i = size-1; i >= 0; i--)
    {
        if(buffer[i] != ' ' && buffer[i] != NULL)
        {
//LOG((ROMCHAR)"{%02X, %c, %d }\r\n", buffer[i], buffer[i], size);
            return;
        }
        buffer[i] = 0;
    }
}

static BYTE parse_text(WORD *blocks)
{
    BYTE text_ab_flag = 0; 
    BYTE text_seg, ver, pos;
    BYTE ret = 0;
    BYTE text[4];
    BYTE i=0, count=0;
    static BYTE size = 0;
    static BYTE text_ab_flag_old;
    
    text_ab_flag = (blocks[1] >> 4) & 1;
    text_seg = (blocks[1] & 0xF);
    ver = (blocks[1] >> 11) & 1;
    
    if (text_ab_flag != text_ab_flag_old) 
    {
        text_ab_flag_old = text_ab_flag;
        memset(rt_data, 0, 65);
//        memset(rds.rt_copy, 0, 65);
        size = 0;
    }
    
    if (ver == 0) 
    {
        pos = text_seg * 4;
        count = 4;
        
        text[0] = ((blocks[2] >> 8) & 0xFF);
        text[1] = (blocks[2] & 0xFF);
        text[2] = ((blocks[3] >> 8) & 0xFF);
        text[3] = (blocks[3] & 0xFF);
    }
    else
    {
        pos = text_seg * 2;
        count = 2;
        
        text[0] = ((blocks[3] >> 8) & 0xFF);
        text[1] = (blocks[3] & 0xFF);
    }
    
    if(pos == 0)
    {
        if(size == 64)
        {
            rt_data[size] = '\0';        
            if(!is_blank(rt_data, size))
            {
                rtrim(rt_data, size);
                if (strcmp(rds.rt_copy, rt_data) != 0)
                {
                    strcpy(rds.rt_copy, rt_data);
                    ret = 1;
                }
            }
        }
        memset(rt_data, 0, 64);
        size = 0;
    }    

    for(i = 0; i < count; ++i)        
        parse_char(pos++, text[i]);

    size = pos;
    return ret;    
}

#ifdef __PARSE_TIME__
// group 4A
static BYTE parse_time(WORD *blocks)
{
    long int seconds = 0;
    int daycode = ((blocks[1] << 15) & 0x18000) |((blocks[2] >> 1) & 0x07FFF);
    int hour    = ((blocks[2] << 4) & 0x10) | ((blocks[3] >> 12) & 0x0F);
    int minute  = ((blocks[3] >> 6) & 0x3F);
    
    int offset_sig = (blocks[3] >> 5) & 1;
    int offset_abs = blocks[3] & 0x1F;

LOG((ROMCHAR)"{%d:%d}\r\n", hour, minute);
    
    if (daycode < 55927) {
        /* invalid date, before 2012-01-01 */
        return 0;
    }
    if ((hour >= 24) || (minute >= 60)) {
        /* invalid time */
        return 0;
    }
    if (offset_abs > 24) {
        /* invalid local time offset */
        return 0;
    }

//LOG((ROMCHAR)"{%d:%d}\r\n", hour, minute);    
    
    /* convert modified julian day + time to UTC */
    seconds = (daycode - 40587) * 86400;
    seconds += hour * 3600;
    seconds += minute * 60;
    seconds += ((offset_sig == 0) ? offset_abs : -offset_abs) * 1800;
    rds->ct_data = seconds;

    return 1;
}
#endif
