/*************************************************************************
	> File Name: core_code.h
	> Author: 
	> Mail: 
	> Created Time: September 15th,2017 Friday 16:25:33
 ************************************************************************/

#ifndef __CORE_CODE_H
#define __CORE_CODE_H

#include "encryption.h"

extern pthread_mutex_t g_StatusLock[4];

void *Pthread_EncryptFile(void *arg);
void *Pthread_DecryptFile(void *arg);
void *Pthread_EncryptFolder(void *arg);
void *Pthread_DecryptFolder(void *arg);

#endif
