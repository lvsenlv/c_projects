/*************************************************************************
	> File Name: encryption.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 3rd,2017 Thursday 08:38:01
 ************************************************************************/

#ifndef __ENCRYPTION_H
#define __ENCRYPTION_H

#include "common.h"
#include "control.h"
#include <pthread.h>

#define ENCRYPT_FILE_SUFFIX_NAME                ".ept" //Must be .ept in this project
/*
    Modify encrypt file suffix name must modify following point:
    1. Function: 
                    ScanEncryptFile()
                    IsEncryptFormat()
                    CheckDecryptResult()
                    CheckDecryptResult()
*/

#ifdef __LINUX
#define DIR_DELIMITER                           '/'
#elif defined __WINDOWS
#define DIR_DELIMITER                           '\\'
#endif

#ifndef PTHREAD_NUM_MAX
#define PTHREAD_NUM_MAX                         2
#endif

#define REFRESH_INTERVAL                        1 //Unit: ms

typedef enum
{
    PROCESS_STATUS_BUSY = 0,
    PROCESS_STATUS_SUCCESS,

    //error details should be writed into log file, i.e use DISP_LOG
    PROCESS_STATUS_ERR,
    PROCESS_STATUS_FATAL_ERR,

    //error details should be writed into g_ErrBuf, i.e use DISP_ERR
    PROCESS_STATUS_ELSE_ERR,
}PROCESS_STATUS;

typedef enum 
{
    REFRESH_FLAG_FALSE = 0,
    REFRESH_FLAG_TRUE,
    
    REFRESH_FLAG_SUCCESS,
    REFRESH_FLAG_FAIL,
}REFRESH_FLAG;

typedef struct FileListStruct
{
    char *pFileName;
    int FileNameLength; //Count in '\0'
    int64_t FileSize;
    struct FileListStruct *pNext;
}FileList_t;

extern char g_password[CTL_PASSWORD_LENGHT_MAX];
extern pthread_mutex_t g_LogLock;

G_STATUS CTL_EncryptDecrypt(CTL_MENU func);



//Static inline functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#ifdef __WINDOWS
/*
 *  @Briefs: Convert '/' to '\\'
 *  @Return: None
 *  @Note:   pFileName can't be NULL
 */
static inline void ConvertNameFormat(char *pFileName)
{
    while(*pFileName != '\0')
    {
        if('/' == *pFileName)
            *pFileName = 92;    //'\' ASCII is 92 
        pFileName++;
    }
}
#endif

/*
 *  @Briefs: Check the member of pFileList
 *  @Return: STAT_ERR / STAT_OK
 *  @Note:   None
 */
static inline G_STATUS CheckFileListArg(__IO FileList_t *pFileList)
{
#ifdef __DEBUG
    if(NULL == pFileList)
        return STAT_ERR;
#endif

    if((NULL == pFileList->pFileName) || (0 == pFileList->FileNameLength))
        return STAT_ERR;

    return STAT_OK;
}

#endif
