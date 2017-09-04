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

#define BUF_SIZE_SMALL                      ((uint32_t)(1024*1024))         //1Mb
#define BUF_SIZE_MEDIUM                     ((uint32_t)(1024*1024*10))      //10Mb
#define BUF_SIZE_LARGE                      ((uint32_t)(1024*1024*100))     //100Mb

extern char g_password[CYT_PASSWORD_LENGHT];

G_STATUS encrypt(void);

#endif

