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
#ifdef __WINDOWS
#include <windows.h>
#endif

#define CYT_FILE_NAME_LENGHT                CTL_FILE_NAME_LENGHT
#define CYT_PASSWORD_LENGHT                 CTL_PASSWORD_LENGHT_MAX
#define ENCRYPT_FILE_SUFFIX_NAME            ".ept"

#define CYT_SMALL_FILE_SIZE                 ((int64_t)(1024*1024*100)) //50Mb
#define PTHREAD_NUM_MAX                     2
#define REFRESH_INTERVAL                    (200*1000) //Unit: us

#ifdef __WINDOWS
#define sleep(time)                         Sleep(time*1000) //Unit: ms
#define usleep(time)                        Sleep(time/1000) //Unit: ms
#endif

extern char g_password[CYT_PASSWORD_LENGHT];
pthread_mutex_t g_LogLock;
extern FILE *g_pDispFile;
#define     DISP_LOG(file, reason) \
            { \
                pthread_mutex_lock(&g_LogLock); \
                g_ti = time(NULL); \
                g_time = localtime(&g_ti); \
                fprintf(g_pDispFile, "[%4d-%02d-%02d %02d:%02d:%02d]: %s\n[Reason]: %s\n", \
                    g_time->tm_year+1900, g_time->tm_mon, g_time->tm_mday, \
                    g_time->tm_hour, g_time->tm_min, g_time->tm_sec, file, reason); \
                pthread_mutex_unlock(&g_LogLock); \
            }

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
    char *FileName;
    int FileNameLenght;
    int64_t FileSize;
    struct FileListStruct *pNext;
}FileList_t;

typedef struct LogStruct
{
    char *pFileName;
    char *pReason;
    int16_t lines;
    char flag;      //1 means the parameter lines is equal to COLS
    struct LogStruct *pNext;
    struct LogStruct *pPrevious;
}log_t;

static inline G_STATUS CheckFileListArg(__IO FileList_t *pFileList)
{
#ifdef __DEBUG
    if(NULL == pFileList)
        return STAT_ERR;
#endif

    if((NULL == pFileList->FileName) || (0 == pFileList->FileNameLenght))
        return STAT_ERR;

    return STAT_OK;
}

#ifdef __WINDOWS
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

G_STATUS CTL_MENU_EncryptDecrypt(CTL_MENU func);
void DispFileList(FileList_t *pHeadNode);
G_STATUS AfterEncryptDecrypt(PROCESS_STATUS ProcessStatus);

#endif
