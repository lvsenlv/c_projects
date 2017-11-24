/*************************************************************************
	> File Name: decoder.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: November 3rd,2017 Friday 10:09:31
 ************************************************************************/

#ifndef __DECODER_H
#define __DECODER_H

#include "common.h"

/*----------------------------------------------------------------------------
Unicode symbol range  | UTF-8 encoded mode
    hexadecimal       |     binary
0000 0000 - 0000 007F | 0xxxxxxx
0000 0080 - 0000 07FF | 110xxxxx 10xxxxxx
0000 0800 - 0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
0001 0000 - 001F FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
0020 0000 - 03FF FFFF | 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
0400 0000 - 7FFF FFFF | 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
------------------------------------------------------------------------------*/

/*---------------------------------------------------------------
                              UTF-8
-----------------------------------------------------------------
         Occupying space        |    First byte characteristic
                                | 
            1 byte              |   0x00 <=   first byte   < 0x80
            2 bytes             |   0xC0 <=   first byte   < 0xE0
            3 bytes             |   0xE0 <=   first byte   < 0xF0
            4 bytes             |   0xF0 <=   first byte   < 0xF8
            5 bytes             |   0xF8 <=   first byte   < 0xFC
            6 bytes             |   0xFC <=   first byte   < 0xFF
-----------------------------------------------------------------*/

_BOOL_ UTF8_VerifyStrFormat(const char *ptr);
int UTF8_GetLength(const char *ptr);
int UTF8_GetWidth(const char *ptr);
int UTF8_GetSpecialSymbolNum(const char *ptr);

#ifdef __WINDOWS
wchar_t *ANSIToUnicode(const char *ptr);
wchar_t *UTF8ToUnicode(const char* ptr);
char *UnicodeToANSI(const wchar_t *ptr);
char *UnicodeToUTF8(const wchar_t *ptr);
char *ANSIToUTF8(const char *ptr, int *pLength);
char *UTF8ToANSI(const char *ptr, int *pLength);
#endif



//Static inline functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*
 *  @Briefs: Check if the character is UTF-8 format
 *  @Return: TRUE / FALSE
 *  @Note:   None
 */
static inline _BOOL_ UTF8_VerifyCharFormat(const uint8_t ch)
{
    return (((0x80 <= ch) && (0xC0 > ch)) || 0xFE < ch) ? FALSE : TRUE;
}

/*
 *  @Briefs: Calculate how many bytes does an UTF-8 symbol occupy
 *  @Return: The number of bytes of UTF-8 character
 *  @Note:   Return value 0 means that it is not the UTF-8 format
 */
static inline int UTF8_GetSymbolByteNum(const uint8_t ch)
{
    return (0xF0 <= ch) ?
                (0xF8 <= ch) ? (   (0xFC <= ch) ? ( (0xFE < ch) ? 0 : 6 ) : 5   ) : 4
            :
                (0xE0 <= ch) ? 3 : (   (0xC0 <= ch) ? 2 : ( (0x80 <= ch) ? 0 : 1 )   )
            ;
}

/*
 *  @Briefs: Check if the character is in ASCII table
 *  @Return: TRUE / FALSE
 *  @Note:   None
 */
static inline _BOOL_ IsStandardSymbol(const uint8_t ch)
{
    return ((ch & 0x80) ? FALSE : TRUE);
}

#endif
