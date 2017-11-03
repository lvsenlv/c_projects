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

_bool_ UTF8_VerifyStr(const char *ptr)
{
    if(NULL == ptr)
        return false;
        
    int ByteNum = 0;
 
    while('\0' != *ptr)
    {
        if(ByteNum == 0) //Get the occupying space according to the first byte characteristic
        {
            ByteNum = UTF8_GetByteNum(*ptr);
            if(0 == ByteNum)
                return false;
        }
        else //Occupy multi bytes
        {
            if(0x80 != (*ptr & 0xC0)) //If disobey the rule: 10xxxxxx
                return false;
        }
        
        ByteNum--;
        ptr++;
    }
 
    if(ByteNum > 0)
        return false;
 
    return true;
}

int UTF8_GetLength(const char *ptr)
{
    if(NULL == ptr)
        return 0;

    int lenght = 0;
    int ByteNum = 0;
 
    while('\0' != *ptr)
    {
        ByteNum = UTF8_GetByteNum(*ptr);
        if (0 == ByteNum)
            return 0;
            
        lenght++;
        ptr += ByteNum;
    }
 
    return lenght;
}

#ifdef __WINDOWS
#include <windows.h>

wchar_t *ANSIToUnicode(const char *ptr)
{
    int lenght;
    wchar_t *pwRes;
    
    lenght = MultiByteToWideChar(CP_ACP, 0, ptr, -1, NULL,0);
    pwRes = (wchar_t *)malloc(lenght * sizeof(wchar_t)); 
    memset(pwRes, 0, lenght*sizeof(wchar_t)); 
    MultiByteToWideChar(CP_ACP, 0, ptr, -1, (LPWSTR)pwRes, lenght);
    
    return pwRes; 
}

wchar_t *UTF8ToUnicode(const char* ptr)
{
    int lenght;
    wchar_t * pwRes;
    
    lenght = MultiByteToWideChar(CP_UTF8, 0, ptr, -1, NULL, 0); 
    pwRes = (wchar_t *)malloc(lenght * sizeof(wchar_t)); 
    memset(pwRes,0, lenght*sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, ptr, -1, (LPWSTR)pwRes, lenght);
    
    return pwRes;
}

char *UnicodeToANSI(const wchar_t *ptr)
{
    int lenght;
    char *pRes;
    
    lenght = WideCharToMultiByte(CP_ACP, 0, ptr, -1, NULL, 0, NULL, NULL);
    pRes =(char *)malloc(lenght * sizeof(char));
    memset(pRes, 0, lenght*sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, ptr, -1, pRes, lenght, NULL, NULL);
    
    return pRes;
}

char *UnicodeToUTF8(const wchar_t *ptr)
{
    int lenght;
    char *pRes;
    
    lenght = WideCharToMultiByte(CP_UTF8, 0, ptr, -1, NULL, 0, NULL, NULL);
    pRes =(char *)malloc(lenght * sizeof(char));
    memset(pRes, 0, lenght*sizeof(char));
    WideCharToMultiByte(CP_UTF8, 0, ptr, -1, pRes, lenght, NULL, NULL);
    
    return pRes;
}

char *ANSIToUTF8(const char *ptr)
{
    int lenght;
    wchar_t *pwRes;
    char *pRes;
    
    lenght = MultiByteToWideChar(CP_ACP, 0, ptr,-1, NULL, 0);
    pwRes = (wchar_t *)malloc(lenght*sizeof(wchar_t));
    memset(pwRes, 0, lenght*sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, ptr, -1, (LPWSTR)pwRes, lenght);
    
    lenght = WideCharToMultiByte( CP_UTF8, 0, pwRes, -1, NULL, 0, NULL, NULL);
    pRes = (char *)malloc(lenght*sizeof(char));
    memset(pRes, 0, lenght*sizeof(char));
    WideCharToMultiByte(CP_UTF8, 0, pwRes, -1, pRes, lenght, NULL, NULL);
    
    free(pwRes);
    return pRes;
}

char *UTF8ToANSI(const char *ptr)
{
    int lenght;
    wchar_t *pwRes;
    char *pRes;
    
    lenght = MultiByteToWideChar(CP_UTF8, 0, ptr,-1, NULL, 0);
    pwRes = (wchar_t *)malloc(lenght*sizeof(wchar_t));
    memset(pwRes, 0, lenght*sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, ptr, -1, (LPWSTR)pwRes, lenght);
    
    lenght = WideCharToMultiByte(CP_ACP, 0, pwRes, -1, NULL, 0, NULL, NULL);
    pRes = (char *)malloc(lenght*sizeof(char));
    memset(pRes, 0, lenght*sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, pwRes, -1, pRes, lenght, NULL, NULL);
    
    free(pwRes);
    return pRes;
}

#endif
