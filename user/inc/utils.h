/*
 * utils.h
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */

#ifndef USER_INC_UTILS_H_
#define USER_INC_UTILS_H_

const unsigned char* string_trimLeft(const unsigned char* string);
void string_trimRight(unsigned char* string);
char *string_bypass(const char *str1, const char *str2);

void ascii2unicode(unsigned short* out, const unsigned char* in);
void unicode2ascii(unsigned char* out, const unsigned short* in);

#endif /* USER_INC_UTILS_H_ */
