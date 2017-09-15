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
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#define CYT_FILE_NAME_LENGHT                CTL_FILE_NAME_LENGHT
#define CYT_PASSWORD_LENGHT                 CTL_PASSWORD_LENGHT_MAX
#define ENCRYPT_FILE_SUFFIX_NAME            ".ept"
#define DECRYPT_FILE_SUFFIX_NAME            ".dpt"
#define FILE_LIST_NAME                      "./file_list"

#define CYT_SMALL_FILE_SIZE                 ((int64_t)(1024*1024*100)) //50Mb

extern FILE *g_pDispFile;
#define     DISP_LOG(file, reason) \
            fprintf(g_pDispFile, "[FileName]: %s\n[Reason]: %s\n", file, reason)

extern char g_password[CYT_PASSWORD_LENGHT];

typedef enum
{
    PROCESS_STATUS_BUSY = 0,
    PROCESS_STATUS_SUCCESS,
    PROCESS_STATUS_ERR,
    PROCESS_STATUS_FATAL_ERR,
}PROCESS_STATUS;

typedef struct FileListStruct
{
    char *FileName;
    int FileNameLenght;
    int64_t FileSize;
    struct FileListStruct *pNext;
}FileList_t;

typedef struct PthreadArg
{
    int *pRatioFactor;
    pthread_mutex_t *pLock;
    PROCESS_STATUS ProcessStatus;
    FileList_t *pFileList;

    //use in multi threads
    FileList_t *pCurFileList;
    const char *pCurFileName;
}PthreadArg_t;

static inline void FreeFileList(FileList_t *pHeadNode)
{
    FileList_t *CurNode = pHeadNode->pNext;
    FileList_t *TmpNode;
    while(CurNode != NULL)
    {
        TmpNode = CurNode->pNext;
        free(CurNode);
        CurNode = TmpNode;
    }
}

G_STATUS EncryptDecrypt(char func);

#endif

