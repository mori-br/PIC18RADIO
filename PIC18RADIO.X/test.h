/* 
 * File:   test.h
 * Author: mori
 *
 * Created on February 12, 2016, 5:28 PM
 */

#ifndef TEST_H
#define	TEST_H

#include "GenericTypeDefs.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define TEST_EEPROM
    
#ifdef TEST_EEPROM
    
void TEST_eeprom_reset(void);    
    
void TEST_eeprom_test1(void);
void TEST_eeprom_test(void);

#endif // TEST_EEPROM
   


#ifdef	__cplusplus
}
#endif

#endif	/* TEST_H */

