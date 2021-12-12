#ifndef __GUI_PORTING_H__
#define __GUI_PORTING_H__

#include <stdint.h>

#define LOGIN_SUCCESS 0x00
#define LOGIN_FAILED  0x01

void     get_time_str(char *buffer, uint8_t show_mark);
uint8_t  login(const char *username, const char *password);
float    getCurrentFlow();      // in ml/s
float    getCurrentUsage();     // in L
int      getCurrentUsingTime(); // in sec
void     logout();
uint16_t get_qr_code(uint8_t **pArray); // return size

#endif // __GUI_PORTING_H__