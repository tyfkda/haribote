#ifndef __UTIL_H__
#define __UTIL_H__

// Reads real time clock, and returns the result into array t.
// return value = year, 0=month, 1=day, 2=hour, 3=minute, 4=second
int read_rtc(unsigned char t[5]);

#endif
