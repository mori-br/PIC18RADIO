/*
    File:   rds.h
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
#ifndef RDS_H
#define	RDS_H

#include "GenericTypeDefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct
{
    BYTE use_rt;
    BYTE ps_valid;
    BYTE rt_valid;
    char ps_copy[9];
    char rt_copy[65];
    long int ct_data;
    
} RDSINFO;
    
void RDS_reset(void);
void RDS_parse(WORD *blocks);


#ifdef	__cplusplus
}
#endif

#endif	/* RDS_H */

