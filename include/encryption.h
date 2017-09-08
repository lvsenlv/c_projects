/*************************************************************************
	> File Name: encryption.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 3rd,2017 Thursday 08:38:01
 ************************************************************************/

#ifndef __ENCRYPTION_H
#define __ENCRYPTION_H

#include "common.h"
#include <sys/stat.h>
#include <unistd.h>
#include "control.h"

#define CYT_FILE_NAME_LENGTH                CTL_FILE_NAME_LENGHT
#define CYT_PASSWORD_LENGHT                 CTL_PASSWORD_LENGHT_MAX
#define ENCRYPT_FILE_SUFFIX_NAME            ".ept"
#define DECRYPT_FILE_SUFFIX_NAME            ".dpt"

#define CYT_SMALL_FILE_SIZE                 ((int64_t)(1024*1024*100)) //50Mb

extern FILE *g_pDispFile;
#define     DISP_LOG(file, reason) \
            fprintf(g_pDispFile, "[FileName]: %s\n[Reason]: %s\n", file, reason)

extern char g_password[CYT_PASSWORD_LENGHT];

typedef struct FileListStruct
{
    char *FileName;
    int64_t FileSize;
    struct FileListStruct *pNext;
}FileList_t;

typedef enum 
{
    PROCESS_STATUS_IN_PROCESSING = 0,
    PROCESS_STATUS_SUCCESS,
    PROCESS_STATUS_FAIL,
}PROCESS_STATUS;

G_STATUS EncryptDecrypt(char func);

#endif

