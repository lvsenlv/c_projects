/*************************************************************************
	> File Name: encryption.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 2017年08月03日 星期四 08时38分01秒
 ************************************************************************/

#ifndef __ENCRYPTION_H
#define __ENCRYPTION_H

#include "common.h"
#include <sys/stat.h>
#include <unistd.h>

#define CYT_FILE_NAME_LENGTH                128
#define CYT_PASSWORD_LENGHT                 19
#define CYT_PASSWORD_LENGHT_LIMIT           8
#define ENCRYPT_FILE_SUFFIX_NAME            ".ept"
#define DECRYPT_FILE_SUFFIX_NAME            ".dpt"

#define BUF_SIZE_SMALL                      ((uint32_t)(1024*1024))         //1Mb
#define BUF_SIZE_MEDIUM                     ((uint32_t)(1024*1024*10))      //10Mb
#define BUF_SIZE_LARGE                      ((uint32_t)(1024*1024*100))     //100Mb

extern char g_password[CYT_PASSWORD_LENGHT];

#ifdef __LINUX
static inline int CheckFile(char *pFileName)
{
    if(access(pFileName, F_OK) != 0)
        return -1;
    struct stat FileInfo;
    if(stat(pFileName, &FileInfo) != 0)
        return -1;
    if(S_IFREG & FileInfo.st_mode)
        return 0;
    
    return -1;
}
#elif defined __WINDOWS
static inline int CheckFile(char *pFileName)
{
    if(access(pFileName, F_OK) != 0)
        return -1;
    struct _stati64 FileInfo;
    if(_stati64(pFileName, &FileInfo) != 0)
        return -1;
    if(S_IFREG & FileInfo.st_mode)
        return 0;
    
    return -1;
}
#endif

G_STATUS encrypt(char func);

#endif

