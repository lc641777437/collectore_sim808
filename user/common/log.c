/*
 * log.c
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "uart.h"
#include "log.h"

/*
 * The hex log is in the following format:
 *
 *     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F      0123456789ABCDEF
 * 01  aa 55 01 00 00 00 25 00 38 36 35 30 36 37 30 32     .U....%.86506702
 * 02  30 34 39 30 31 36 38 30 00 00 00 00 00 00 00 00     04901680........
 * 03  00 00 00 00 00 00 00 00 00 00 00 00                 ............
 *
 */
void log_hex(const char* data, int length)
{
    int i = 0, j = 0;

    print("    ");
    for (i  = 0; i < 16; i++)
    {
        print("%X  ", i);
    }
    print("    ");
    for (i = 0; i < 16; i++)
    {
        print("%X", i);
    }

    print("\r\n");

    for (i = 0; i < length; i += 16)
    {
        print("%02d  ", i / 16 + 1);
        for (j = i; j < i + 16 && j < length; j++)
        {
            print("%02x ", data[j] & 0xff);
        }
        if (j == length && length % 16)
        {
            for (j = 0; j < (16 - length % 16); j++)
            {
                print("   ");
            }
        }
        print("    ");
        for (j = i; j < i + 16 && j < length; j++)
        {
            if (data[j] < 32 || data[j] >= 127)
            {
                print(".");
            }
            else
            {
                print("%c", data[j] & 0xff);
            }
        }

        print("\r\n");
    }
}
