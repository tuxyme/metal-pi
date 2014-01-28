
#ifndef _STRING_H_
#define _STRING_H_

typedef unsigned int size_t;

void *memset(void *ptr, int set, size_t size);
void *memcpy(void *dest, const void *src, size_t count);
char *itoa(int val);
char *utoa(unsigned int val);
char *btoa(unsigned int val);
char *htoa(unsigned int val);  // 0xFFFFFFFF
char *htoa4(unsigned int val); // 0xFFFF

void lower(char *str);
void upper(char *str);

#endif // _STRING_H_
