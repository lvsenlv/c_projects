/*************************************************************************
	> File Name: decoder.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: October 31st,2017 Tuesday 09:03:41
 ************************************************************************/

#include "decoder.h"

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



/*
 *  @Briefs: Check if the string is UTF-8 format
 *  @Return: TRUE / FALSE
 *  @Note:   None
 */

_BOOL_ UTF8_VerifyStrFormat(const char *ptr)
{
    if(NULL == ptr)
        return FALSE;
        
    int ByteNum = 0;
 
    while('\0' != *ptr)
    {
        if(ByteNum == 0) //Get the occupying space according to the first byte characteristic
        {
            ByteNum = UTF8_GetSymbolByteNum(*ptr);
            if(0 == ByteNum)
                return FALSE;
        }
        else //Occupy multi bytes
        {
            if(0x80 != (*ptr & 0xC0)) //If disobey the rule: 10xxxxxx
                return FALSE;
        }
        
        ByteNum--;
        ptr++;
    }
 
    if(ByteNum > 0)
        return FALSE;
 
    return TRUE;
}

/*
 *  @Briefs: Calculate the number of UTF-8 symbol including the symbol in ASCII
 *  @Return: The number of UTF-8 symbol including the symbol in ASCII
 *  @Note:   None
 */
int UTF8_GetLength(const char *ptr)
{
    if(NULL == ptr)
        return 0;

    int ByteNum = 0;
    int length = 0;
 
    while('\0' != *ptr)
    {
        ByteNum = UTF8_GetSymbolByteNum(*ptr);
        if (0 == ByteNum)
            return 0;
            
        length++;
        ptr += ByteNum;
    }
 
    return length;
}

/*
 *  @Briefs: Calculate the width, symbol in ASCII is 1 width unit and else is 2 width unit
 *  @Return: The width of content that the ptr ponits to
 *  @Note:   None
 */
int UTF8_GetWidth(const char *ptr)
{
    if(NULL == ptr)
        return 0;

    int ByteNum = 0;
    int width = 0;
 
    while('\0' != *ptr)
    {
        ByteNum = UTF8_GetSymbolByteNum(*ptr);
        if (0 == ByteNum)
            return 0;
            
        width += (1 == ByteNum) ? 1 : 2;
        ptr += ByteNum;
    }
 
    return width;
}


/*
 *  @Briefs: Calculate the number of UTF-8 symbol except the symbol in ASCII
 *  @Return: The number of UTF-8 symbol except the symbol in ASCII
 *  @Note:   None
 */
int UTF8_GetSpecialSymbolNum(const char *ptr)
{
    if(NULL == ptr)
        return 0;

    int ByteNum;
    int SymbolNum = 0;
    
    while('\0' != *ptr)
    {
        ByteNum = UTF8_GetSymbolByteNum(*ptr);
        
        if(1 < ByteNum)
        {
            SymbolNum++;
        }
        else if(0 == ByteNum) //Is not the UTF-8 character
            return 0;
            
        ptr += ByteNum;
    }
    
    return SymbolNum;
}


#ifdef __WINDOWS
#include <windows.h>

/*
 *  @Briefs: Convert ANSI format to Unicode format
 *  @Return: The pointer pointing to the wide character string
 *  @Note:   Release memory manually
 */
wchar_t *ANSIToUnicode(const char *ptr)
{
    int length;
    wchar_t *pwRes;
    
    length = MultiByteToWideChar(CP_ACP, 0, ptr, -1, NULL,0);
    pwRes = (wchar_t *)malloc(length * sizeof(wchar_t)); 
    memset(pwRes, 0, length*sizeof(wchar_t)); 
    MultiByteToWideChar(CP_ACP, 0, ptr, -1, (LPWSTR)pwRes, length);
    
    return pwRes; 
}

/*
 *  @Briefs: Convert UTF-8 format to Unicode format
 *  @Return: The pointer pointing to the wide character string
 *  @Note:   Release memory manually
 */
wchar_t *UTF8ToUnicode(const char* ptr)
{
    int length;
    wchar_t * pwRes;
    
    length = MultiByteToWideChar(CP_UTF8, 0, ptr, -1, NULL, 0); 
    pwRes = (wchar_t *)malloc(length * sizeof(wchar_t)); 
    memset(pwRes,0, length*sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, ptr, -1, (LPWSTR)pwRes, length);
    
    return pwRes;
}

/*
 *  @Briefs: Convert Unicode format to ANSI format
 *  @Return: The pointer pointing to the string
 *  @Note:   Release memory manually
 */
char *UnicodeToANSI(const wchar_t *ptr)
{
    int length;
    char *pRes;
    
    length = WideCharToMultiByte(CP_ACP, 0, ptr, -1, NULL, 0, NULL, NULL);
    pRes =(char *)malloc(length * sizeof(char));
    memset(pRes, 0, length*sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, ptr, -1, pRes, length, NULL, NULL);
    
    return pRes;
}

/*
 *  @Briefs: Convert Unicode format to UTF-8 format
 *  @Return: The pointer pointing to the string
 *  @Note:   Release memory manually
 */
char *UnicodeToUTF8(const wchar_t *ptr)
{
    int length;
    char *pRes;
    
    length = WideCharToMultiByte(CP_UTF8, 0, ptr, -1, NULL, 0, NULL, NULL);
    pRes =(char *)malloc(length * sizeof(char));
    memset(pRes, 0, length*sizeof(char));
    WideCharToMultiByte(CP_UTF8, 0, ptr, -1, pRes, length, NULL, NULL);
    
    return pRes;
}

/*
 *  @Briefs: Convert ANSI format to UTF-8 format
 *  @Return: The pointer pointing to the string
 *  @Note:   1. Release memory manually
 *           2. If pLength isn't NULL, pLength is equal to the length of ptr(counting in '\0')
 */
char *ANSIToUTF8(const char *ptr, int *pLength)
{
    if(NULL == ptr)
        return NULL;
    
    int length;
    wchar_t *pwRes;
    char *pRes;
    
    length = MultiByteToWideChar(CP_ACP, 0, ptr, -1, NULL, 0);
    pwRes = (wchar_t *)malloc(length*sizeof(wchar_t));
    if(NULL == pwRes)
        return NULL;
    
    memset(pwRes, 0, length*sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, ptr, -1, (LPWSTR)pwRes, length);
    
    length = WideCharToMultiByte( CP_UTF8, 0, pwRes, -1, NULL, 0, NULL, NULL);
    pRes = (char *)malloc(length*sizeof(char));
    if(NULL == pRes)
    {
        free(pwRes);
        return NULL;
    }

    if(NULL != pLength)
    {
        *pLength = length;
    }
    memset(pRes, 0, length*sizeof(char));
    WideCharToMultiByte(CP_UTF8, 0, pwRes, -1, pRes, length, NULL, NULL);
    
    free(pwRes);
    return pRes;
}

/*
 *  @Briefs: Convert UTF-8 format to ANSI format
 *  @Return: The pointer pointing to the string
 *  @Note:   1. Release memory manually
 *           2. If pLength isn't NULL, pLength is equal to the length of ptr(counting in '\0')
 */
char *UTF8ToANSI(const char *ptr, int *pLength)
{
    if(NULL == ptr)
        return NULL;

    int length;
    wchar_t *pwRes;
    char *pRes;
    
    length = MultiByteToWideChar(CP_UTF8, 0, ptr, -1, NULL, 0);
    pwRes = (wchar_t *)malloc(length*sizeof(wchar_t));
    if(NULL == pwRes)
        return NULL;
        
    memset(pwRes, 0, length*sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, ptr, -1, (LPWSTR)pwRes, length);
    
    length = WideCharToMultiByte(CP_ACP, 0, pwRes, -1, NULL, 0, NULL, NULL);
    pRes = (char *)malloc(length*sizeof(char));
    if(NULL == pRes)
    {
        free(pwRes);
        return NULL;
    }
    
    if(NULL != pLength)
    {
        *pLength = length;
    }
    memset(pRes, 0, length*sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, pwRes, -1, pRes, length, NULL, NULL);
    
    free(pwRes);
    return pRes;
}

#endif
