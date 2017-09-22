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

    //error details should be writed into log file, i.e use DISP_LOG
    PROCESS_STATUS_ERR,
    PROCESS_STATUS_FATAL_ERR,

    //error details should be writed into g_ErrBuf, i.e use DISP_ERR
    PROCESS_STATUS_ELSE_ERR,
}PROCESS_STATUS;

typedef struct FileListStruct
{
    char *FileName;
    int FileNameLenght;
    int64_t FileSize;
    struct FileListStruct *pNext;
}FileList_t;

static inline G_STATUS CheckFileList(__IO FileList_t *pFileList)
{
#ifdef __DEBUG
    if(NULL == pFileList)
        return STAT_ERR;
#endif

    if((NULL == pFileList->FileName) || (0 == pFileList->FileNameLenght))
        return STAT_ERR;

    return STAT_OK;
}

G_STATUS EncryptDecrypt(char func);
G_STATUS CreateFileList_Plus(FileList_t *pHeadNode, char *pFolderName);
FileList_t *ScanDirectory(char *pFolderName);
void DispFileList(FileList_t *pHeadNode);

#endif

