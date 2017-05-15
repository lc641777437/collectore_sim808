/*
 * version.h
 *
 *  Created on: 2016/7/25
 *      Author: lc
 */

#ifndef USER__VERSION_H_
#define USER__VERSION_H_

#define VERSION_MAJOR   1
#define VERSION_MINOR   0
#define VERSION_MICRO   0

#define VERSION_INT(a, b, c)    (a << 16 | b << 8 | c)
#define VERSION_DOT(a, b, c)    a##.##b##.##c
#define VERSION(a, b, c)        VERSION_DOT(a, b, c)

#define STRINGIFY(s)         TOSTRING(s)
#define TOSTRING(s) #s

#define VERSION_STR STRINGIFY(VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO))

#define VERSION_NUM VERSION_INT(VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO)

#endif /* USER_ELECTROMBILE_VERSION_H_ */
