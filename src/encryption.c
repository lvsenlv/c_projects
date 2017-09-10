/*************************************************************************
	> File Name: encryption.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 3rd,2017 Thursday 08:37:45
 ************************************************************************/

#ifdef __LINUX
#define _FILE_OFFSET_BITS 64 //make sure st_size is 64bits instead of 32bits
#endif

#include "encryption.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

static G_STATUS EncryptDecryptFile(char func);
static G_STATUS ParseFileList(void);
static void *ProcessFile(void *arg);
static void *ProcessFolder(void *arg);
static G_STATUS EncryptFile(char *pFileName, int64_t FileSize);
static G_STATUS DecryptFile(char *pFileName, int64_t FileSize);
static inline void DeleteEncyptSuffix(char *pFileName);
static inline void ConvertFileFormat(char *pFileName);

char g_password[CTL_PASSWORD_LENGHT_MAX];
FileList_t g_FileList;
FileList_t *g_CurFile = &g_FileList;
int32_t g_ProcessCount = 0;
PROCESS_STATUS g_ProcessStatus = PROCESS_STATUS_FAIL;
pthread_mutex_t FileLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CountLock = PTHREAD_MUTEX_INITIALIZER;

G_STATUS EncryptDecrypt(char func)
{
    if((func != CTL_MENU_ENCRYPT) && (func != CTL_MENU_DECRYPT))
    {
        endwin();
        CLEAR_STR_SCR();
        DISP_ERR(STR_ERR_INVALID_FUNC);
        return STAT_ERR;
    }
    
    G_STATUS status;
    char FileName[CYT_FILE_NAME_LENGTH];
#ifdef __LINUX
        struct stat FileInfo; 
#elif defined __WINDOWS
        struct _stati64 FileInfo;
#endif

    while(1)
    {
        status = CTL_GetFileName(FileName);
        if(status != STAT_OK)
            return status;

#ifdef __LINUX
        if(stat(FileName, &FileInfo) == 0)
#elif defined __WINDOWS
        if(_stati64(FileName, &FileInfo) == 0)
#endif
            break;

        status = CTL_MakeChoice("%s", STR_FAIL_TO_GET_FILE_FOLDER_INFO);
        if(STAT_RETRY == status)
            continue;
        else
           return status;
    }

    status = CTL_GetPassord(g_password);
    if(status != STAT_OK)
        return status;

    if(S_IFREG & FileInfo.st_mode)
    {
        g_ProcessCount = 1;
        g_FileList.FileName = FileName;
        g_FileList.FileSize = FileInfo.st_size;
        g_FileList.pNext = NULL;
        status = EncryptDecryptFile(func);
    }
    else if(S_IFDIR & FileInfo.st_mode)
    {
        status = ParseFileList();
    }

    return STAT_OK;
}

static G_STATUS EncryptDecryptFile(char func)
{
    int lines = 0, cols = 0;
    int FileNameLenght = strlen(g_FileList.FileName);

    if(CTL_MENU_ENCRYPT == func)
    {
        if((sizeof(STR_IN_ENCRYPTING)-1 + FileNameLenght) <= CTL_ENCRYPT_FILE_WIN_COLS)
        {
            lines = 4;
            cols = FileNameLenght + sizeof(STR_IN_ENCRYPTING) - 1;
        }
        else
        {
            lines = ((sizeof(STR_IN_DECRYPTING)-1 + FileNameLenght)
                /CTL_ENCRYPT_FILE_WIN_COLS) + 4;
            cols = CTL_ENCRYPT_FILE_WIN_COLS;
        }
    }
    else
    {
        if((sizeof(STR_IN_DECRYPTING)-1 + FileNameLenght) <= CTL_DECRYPT_FILE_WIN_COLS)
        {
            lines = 4;
            cols = FileNameLenght + sizeof(STR_IN_DECRYPTING) - 1;
        }
        else
        {
            lines = ((sizeof(STR_IN_DECRYPTING)-1 + FileNameLenght)
                /CTL_DECRYPT_FILE_WIN_COLS) + 4;
            cols = CTL_DECRYPT_FILE_WIN_COLS;
        }
    }
    
    WINDOW *win = newwin(lines, cols, (LINES-lines)/2, (COLS-cols)/2);
    mvwhline(win, 0, 0, '-', cols);
    mvwhline(win, lines-1, 0, '-', cols);
    
    if(CTL_MENU_ENCRYPT == func)
    {
        if((sizeof(STR_IN_DECRYPTING)-1 + FileNameLenght)%cols != 0)
            mvwprintw(win, 1, 0, "%s%s\n", STR_IN_ENCRYPTING, g_FileList.FileName);
        else
            mvwprintw(win, 1, 0, "%s%s", STR_IN_ENCRYPTING, g_FileList.FileName);
    }
    else
    {
        if((sizeof(STR_IN_DECRYPTING)-1 + FileNameLenght)%cols != 0)
            mvwprintw(win, 1, 0, "%s%s\n", STR_IN_DECRYPTING, g_FileList.FileName);
        else
            mvwprintw(win, 1, 0, "%s%s", STR_IN_DECRYPTING, g_FileList.FileName);
    }
    
    wrefresh(win);

    //G_STATUS status = STAT_OK;
    g_ProcessStatus = PROCESS_STATUS_IN_PROCESSING;
    pthread_t PthreadID;
    int ret = 0;
    ret = pthread_create(&PthreadID, NULL, ProcessFile, &func);
	if(ret)
	{
		DISP_ERR(STR_ERR_PTHREAD_CREATE);
        return STAT_ERR;
	}

    int CurPosY = lines - 2, CurPosX = 0;
    while(PROCESS_STATUS_IN_PROCESSING == g_ProcessStatus)
    {
        if(CurPosX > cols)
        {
            mvwhline(win, CurPosY, 0, ' ', cols);
            wmove(win, CurPosY, 0);
            CurPosX = 0;
        }
        
        waddch(win, '.');
        wrefresh(win);
        CurPosX++;
        sleep(1);
    }

    if(PROCESS_STATUS_SUCCESS == g_ProcessStatus)
    {
        mvwaddstr(win, CurPosY, (cols-sizeof(STR_SUCCESS)+1)/2, STR_SUCCESS);
        wrefresh(win);
    }

    wgetch(win);

    delwin(win);
    touchwin(stdscr);
    refresh();
    return STAT_OK;
}

static G_STATUS ParseFileList(void)
{    
    if(access(FILE_LIST_NAME, F_OK) != 0)
    {
        endwin();
        CLEAR_STR_SCR();
        DISP_ERR(STR_ERR_FILE_LIST_NOT_EXIST);
        return STAT_ERR;
    }

#ifdef __LINUX
    struct stat FileInfo;
    if(stat(FILE_LIST_NAME, &FileInfo) != 0)
#elif defined __WINDOWS
    struct _stati64 FileInfo;
    if(_stati64(FILE_LIST_NAME, &FileInfo) != 0)
#endif
    {
        endwin();
        CLEAR_STR_SCR();
        DISP_ERR(STR_ERR_FAIL_TO_GET_FILE_INFO);
        return STAT_ERR;
    }

    if(!(S_IFREG & FileInfo.st_mode))
    {
        endwin();
        CLEAR_STR_SCR();
        DISP_ERR(STR_ERR_INVALID_FILE_LIST);
        return STAT_ERR;
    }    

    FILE *fp = NULL;
    fp = fopen(FILE_LIST_NAME, "rb");
    if(NULL == fp)
    {
        endwin();
        CLEAR_STR_SCR();
        DISP_ERR(STR_ERR_FAIL_TO_OPEN_FILE_LIST);
        return STAT_ERR;
    }

    char *FileName = NULL;
    FileName = (char *)malloc(CYT_FILE_NAME_LENGTH);
    if(NULL == FileName)
    {
        endwin();
        CLEAR_STR_SCR();
        DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
        return STAT_ERR;
    }
    
    while((fgets(FileName, CYT_FILE_NAME_LENGTH, fp) != 0) && (0 == feof(fp)))
    {
        
    }
    
    return STAT_OK;
}

/*************************************************************************
                                Pthread
************************************************************************/
static void *ProcessFile(void *arg)
{
    g_ProcessStatus = PROCESS_STATUS_IN_PROCESSING;
    G_STATUS status;
    if(CTL_MENU_ENCRYPT == *(char *)arg)
        status = EncryptFile(g_FileList.FileName, g_FileList.FileSize);
    else
        status = DecryptFile(g_FileList.FileName, g_FileList.FileSize);

    g_ProcessStatus = (status != STAT_OK) ? PROCESS_STATUS_FAIL : PROCESS_STATUS_SUCCESS;
    
    return NULL;
}

static void *ProcessFolder(void *arg)
{
    FileList_t *CurFile;
    G_STATUS status;
    G_STATUS (*pFunc)(char *, int64_t);

    if(CTL_MENU_ENCRYPT == *(char *)arg)
        pFunc = EncryptFile;
    else
        pFunc = DecryptFile;

    pthread_mutex_lock(&FileLock);
    CurFile = g_CurFile;
    g_CurFile = g_CurFile->pNext;
    pthread_mutex_unlock(&FileLock);
    
    while(CurFile != NULL);
    {
        status = (*pFunc)(CurFile->FileName, CurFile->FileSize);
        if(status != STAT_OK)
        {
            if(STAT_EXIT == status)
                return NULL;

            pthread_mutex_lock(&CountLock);
            if(g_ProcessCount != 0)
                g_ProcessCount--;
            pthread_mutex_unlock(&CountLock);
        }            
        
        pthread_mutex_lock(&FileLock);
        CurFile = g_CurFile;
        g_CurFile = g_CurFile->pNext;
        pthread_mutex_unlock(&FileLock);
    }
    
    return NULL;
}

/*************************************************************************
                   Core codes of encryption algorithm
************************************************************************/
static G_STATUS EncryptFile(char *pFileName, int64_t FileSize)
{
    FILE *fp = NULL;
    fp = fopen(pFileName, "rb");
    if(NULL == fp)
    {
        DISP_LOG(pFileName, STR_FOPEN_ERR);
        return STAT_ERR;
    }
    fseek(fp, 0, SEEK_SET);

    char NewFileName[CYT_FILE_NAME_LENGTH];
    snprintf(NewFileName, sizeof(NewFileName), "%s%s", pFileName, ENCRYPT_FILE_SUFFIX_NAME);
    FILE *NewFp = NULL;
    NewFp = fopen(NewFileName, "wb+");
    if(NULL == NewFp)
    {
        fclose(fp);
        DISP_LOG(NewFileName, STR_FAIL_TO_CREATE_OPEN_FILE);
        return STAT_ERR;
    }

    uint8_t *pData = NULL;
    pData = (uint8_t *)malloc(sizeof(uint8_t) * CYT_SMALL_FILE_SIZE);
    if(NULL == pData)
    {
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_ERR_FAIL_TO_MALLOC);
        return STAT_EXIT;
    }
        
    int64_t size = 0;
    int32_t EncyptFactor = 0;
    int32_t PasswordLenght = 0;
    const char *pPassword = g_password;

    //get encrypt factor
    while(*pPassword != '\0')
    {
        EncyptFactor += (uint32_t)*pPassword;
        pPassword++;
        PasswordLenght++;
    }
    if(0 == PasswordLenght)
    {        
        free(pData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_PASSWORD_NULL);
        return STAT_ERR;
    }
    EncyptFactor %= 8;
    if(0 == EncyptFactor)
        EncyptFactor = 1;

    uint8_t SubFactor1 = 0xFF >> (8-EncyptFactor);
    uint8_t SubFactor2 = 8 - EncyptFactor;
    uint8_t *pTmp = NULL, *pTmp2 = NULL;
    uint8_t TmpData = 0;
    int32_t i = 0, index = 0;
    int32_t CycleIndex = (int32_t)(FileSize / CYT_SMALL_FILE_SIZE);

    int32_t RestDataCount = CYT_SMALL_FILE_SIZE - PasswordLenght;
    uint8_t *pBackupData = NULL;
    pBackupData = (uint8_t *)malloc(PasswordLenght * sizeof(uint8_t));
    if(NULL == pBackupData)
    {
        free(pData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_ERR_FAIL_TO_MALLOC);
        return STAT_EXIT;
    }    
    
    for(index = 0; index < CycleIndex; index++)
    {
        //read data from original file
        memset(pData, 0, sizeof(uint8_t)*CYT_SMALL_FILE_SIZE);
        size = fread(pData, sizeof(uint8_t), CYT_SMALL_FILE_SIZE, fp);
        if(size != CYT_SMALL_FILE_SIZE)
        {
            free(pData);
            free(pBackupData);
            fclose(fp);
            fclose(NewFp);
            DISP_LOG(pFileName, STR_FAIL_TO_READ_FILE);        
            return STAT_ERR;
        }
        
        //proccess 1
        pTmp = pData;
        for(i = 0; i < CYT_SMALL_FILE_SIZE; i++)
        {
            TmpData = 0;
            TmpData = *pTmp & SubFactor1;
            TmpData <<= SubFactor2;
            TmpData |= *pTmp >> ((uint8_t)EncyptFactor);
            *pTmp++ = TmpData;
        }

        //proccess 2
        pTmp = pBackupData;
        pTmp2 = pData + CYT_SMALL_FILE_SIZE - 1;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp++ = *pTmp2--;
        }

        pTmp = pData + CYT_SMALL_FILE_SIZE - 1;
        pTmp2 = pData + RestDataCount - 1;
        for(i = 0; i < RestDataCount; i++)
        {
            *pTmp-- = *pTmp2--;
        }

        pTmp = pData;
        pTmp2 = pBackupData;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp++ = *pTmp2++;
        }

        //proccess 3
        pPassword = g_password;
        pTmp = pData + CYT_SMALL_FILE_SIZE - 1;
        for(i = 0; i < PasswordLenght; i++)
        {
            TmpData = *pTmp;
            *pTmp = pData[(uint32_t)(*pPassword)];
            pData[(uint32_t)(*pPassword)] = TmpData;
            pTmp--;
            pPassword++;
        }

        //write encyption data to new file
        size = fwrite(pData, sizeof(uint8_t), CYT_SMALL_FILE_SIZE, NewFp);
        if(size != CYT_SMALL_FILE_SIZE)
        {        
            free(pData);
            free(pBackupData);
            fclose(fp);
            fclose(NewFp);
            DISP_LOG(NewFileName, STR_FAIL_TO_WRITE_FILE);
            return STAT_ERR;
        }
    }

    //encrypt rest data
    int32_t RestDataSize = (int32_t)(FileSize % CYT_SMALL_FILE_SIZE);
    
    //read data from original file
    memset(pData, 0, sizeof(uint8_t)*CYT_SMALL_FILE_SIZE);
    size = fread(pData, sizeof(uint8_t), RestDataSize, fp);
    if(size != RestDataSize)
    {
        free(pData);
        free(pBackupData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_FAIL_TO_READ_FILE);        
        return STAT_ERR;
    }
    
    //proccess 1
    pTmp = pData;
    for(i = 0; i < RestDataSize; i++)
    {
        TmpData = 0;
        TmpData = *pTmp & SubFactor1;
        TmpData <<= SubFactor2;
        TmpData |= *pTmp >> ((uint8_t)EncyptFactor);
        *pTmp++ = TmpData;
    }

    //proccess 2
    if(RestDataSize <= PasswordLenght)
    {
        pTmp = pData;
        pPassword = g_password;
        for(i = 0; i < RestDataSize; i++)
        {
            *pTmp ^= *pPassword;
            pPassword++;
            pTmp++;
        }
    }
    else
    {
        pTmp = pBackupData;
        pTmp2 = pData + RestDataSize - 1;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp++ = *pTmp2--;
        }

        RestDataCount = RestDataSize - PasswordLenght;
        pTmp = pData + RestDataSize - 1;
        pTmp2 = pData + RestDataCount - 1;
        for(i = 0; i < RestDataCount; i++)
        {
            *pTmp-- = *pTmp2--;
        }

        pTmp = pData;
        pTmp2 = pBackupData;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp++ = *pTmp2++;
        }
    }

    //proccess 3
    if(RestDataSize > 256)
    {
        pPassword = g_password;
        pTmp = pData + RestDataSize - 1;
        for(i = 0; i < PasswordLenght; i++)
        {
            TmpData = *pTmp;
            *pTmp = pData[(uint32_t)(*pPassword)];
            pData[(uint32_t)(*pPassword)] = TmpData;
            pTmp--;
            pPassword++;
        }
    }
    
    //write encyption data to new file
    size = fwrite(pData, sizeof(uint8_t), RestDataSize, NewFp);
    if(size != RestDataSize)
    {        
        free(pData);
        free(pBackupData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(NewFileName, STR_FAIL_TO_WRITE_FILE);
        return STAT_ERR;
    }

    free(pData);
    free(pBackupData);
    fclose(fp);
    fclose(NewFp);
    fp = NULL;
    NewFp = NULL;

#ifdef __LINUX
    snprintf(NewFileName, sizeof(NewFileName), "rm -rf %s", pFileName);
#elif defined __WINDOWS
    ConvertFileFormat(pFileName);
    snprintf(NewFileName, sizeof(NewFileName), "del /a /f /q %s >nul 2>nul", pFileName);
#endif
    fp = popen(NewFileName, "r");
    if(NULL == fp)
    {
        DISP_LOG(pFileName, STR_FAIL_TO_DELETE_OLD_FILE);
        return STAT_ERR;
    }

    pclose(fp);    
    return STAT_OK;    
}

static G_STATUS DecryptFile(char *pFileName, int64_t FileSize)
{
    FILE *fp = NULL;
    fp = fopen(pFileName, "rb");
    if(NULL == fp)
    {
        DISP_LOG(pFileName, STR_FOPEN_ERR);
        return STAT_ERR;
    }
    fseek(fp, 0, SEEK_SET);

    char NewFileName[CYT_FILE_NAME_LENGTH];
    snprintf(NewFileName, sizeof(NewFileName), "%s", pFileName);
    DeleteEncyptSuffix(NewFileName);
    FILE *NewFp = NULL;
    NewFp = fopen(NewFileName, "wb+");
    if(NULL == NewFp)
    {
        fclose(fp);
        DISP_LOG(NewFileName, STR_FAIL_TO_CREATE_OPEN_FILE);
        return STAT_ERR;
    }

    uint8_t *pData = NULL;
    pData = (uint8_t *)malloc(sizeof(uint8_t) * CYT_SMALL_FILE_SIZE);
    if(NULL == pData)
    {
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_ERR_FAIL_TO_MALLOC);
        return STAT_EXIT;
    }
        
    int64_t size = 0;
    int32_t EncyptFactor = 0;
    int32_t PasswordLenght = 0;
    const char *pPassword = g_password;

    //get decrypt factor
    while(*pPassword != '\0')
    {
        EncyptFactor += (uint32_t)*pPassword;
        pPassword++;
        PasswordLenght++;
    }
    if(0 == PasswordLenght)
    {        
        free(pData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_PASSWORD_NULL);
        return STAT_ERR;
    }
    EncyptFactor %= 8;
    if(0 == EncyptFactor)
        EncyptFactor = 1;

    uint8_t SubFactor1 = 0xFF >> EncyptFactor;
    uint8_t SubFactor2 = 8 - EncyptFactor;
    uint8_t *pTmp = NULL, *pTmp2 = NULL;
    uint8_t TmpData = 0;
    int32_t i = 0, index = 0;
    int32_t CycleIndex = (int32_t)(FileSize / CYT_SMALL_FILE_SIZE);

    int32_t RestDataCount = CYT_SMALL_FILE_SIZE - PasswordLenght;
    uint8_t *pBackupData = NULL;
    pBackupData = (uint8_t *)malloc(PasswordLenght * sizeof(uint8_t));
    if(NULL == pBackupData)
    {
        free(pData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_ERR_FAIL_TO_MALLOC);
        return STAT_EXIT;
    }    
    
    for(index = 0; index < CycleIndex; index++)
    {
        //read data from original file
        memset(pData, 0, sizeof(uint8_t)*CYT_SMALL_FILE_SIZE);
        size = fread(pData, sizeof(uint8_t), CYT_SMALL_FILE_SIZE, fp);
        if(size != CYT_SMALL_FILE_SIZE)
        {
            free(pData);
            free(pBackupData);
            fclose(fp);
            fclose(NewFp);
            DISP_LOG(pFileName, STR_FAIL_TO_READ_FILE);
            return STAT_ERR;
        }
        
        //proccess 3
        pPassword = g_password + PasswordLenght - 1;
        pTmp = pData + CYT_SMALL_FILE_SIZE - PasswordLenght;
        for(i = 0; i < PasswordLenght; i++)
        {
            TmpData = *pTmp;
            *pTmp = pData[(uint32_t)(*pPassword)];
            pData[(uint32_t)(*pPassword)] = TmpData;
            pTmp++;
            pPassword--;
        }

        //proccess 2
        pTmp = pBackupData;
        pTmp2 = pData;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp++ = *pTmp2++;
        }

        pTmp = pData;
        pTmp2 = pData + PasswordLenght;
        for(i = 0; i < RestDataCount; i++)
        {
            *pTmp++ = *pTmp2++;
        }

        pTmp = pData + CYT_SMALL_FILE_SIZE - 1;
        pTmp2 = pBackupData;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp-- = *pTmp2++;
        }

        //proccess 1
        pTmp = pData;
        for(i = 0; i < CYT_SMALL_FILE_SIZE; i++)
        {
            TmpData = 0;
            TmpData = *pTmp & SubFactor1;
            TmpData <<= EncyptFactor;
            TmpData |= *pTmp >> ((uint8_t)SubFactor2);
            *pTmp++ = TmpData;
        }

        //write encyption data to new file
        size = fwrite(pData, sizeof(uint8_t), CYT_SMALL_FILE_SIZE, NewFp);
        if(size != CYT_SMALL_FILE_SIZE)
        {        
            free(pData);
            free(pBackupData);
            fclose(fp);
            fclose(NewFp);
            DISP_LOG(NewFileName, STR_FAIL_TO_WRITE_FILE);
            return STAT_ERR;
        }
    }

    //encrypt rest data
    int32_t RestDataSize = (int32_t)(FileSize % CYT_SMALL_FILE_SIZE);
    
    //read data from original file
    memset(pData, 0, sizeof(uint8_t)*CYT_SMALL_FILE_SIZE);
    size = fread(pData, sizeof(uint8_t), RestDataSize, fp);
    if(size != RestDataSize)
    {
        free(pData);
        free(pBackupData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(pFileName, STR_FAIL_TO_READ_FILE);        
        return STAT_ERR;
    }
    
    //proccess 3
    if(RestDataSize > 256)
    {
        pPassword = g_password + PasswordLenght - 1;
        pTmp = pData + RestDataSize - PasswordLenght;
        for(i = 0; i < PasswordLenght; i++)
        {
            TmpData = *pTmp;
            *pTmp = pData[(uint32_t)(*pPassword)];
            pData[(uint32_t)(*pPassword)] = TmpData;
            pTmp++;
            pPassword--;
        }
    }       

    //proccess 2
    if(RestDataSize <= PasswordLenght)
    {
        pTmp = pData;
        pPassword = g_password;
        for(i = 0; i < RestDataSize; i++)
        {
            *pTmp ^= *pPassword;
            pPassword++;
            pTmp++;
        }
    }
    else
    {
        pTmp = pBackupData;
        pTmp2 = pData;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp++ = *pTmp2++;
        }

        RestDataCount = RestDataSize - PasswordLenght;
        pTmp = pData;
        pTmp2 = pData + PasswordLenght;
        for(i = 0; i < RestDataCount; i++)
        {
            *pTmp++ = *pTmp2++;
        }

        pTmp = pData + RestDataSize - 1;
        pTmp2 = pBackupData;
        for(i = 0; i < PasswordLenght; i++)
        {
            *pTmp-- = *pTmp2++;
        }
    }    

    //proccess 1
    pTmp = pData;
    for(i = 0; i < RestDataSize; i++)
    {
        TmpData = 0;
        TmpData = *pTmp & SubFactor1;
        TmpData <<= EncyptFactor;
        TmpData |= *pTmp >> ((uint8_t)SubFactor2);
        *pTmp++ = TmpData;
    }

    
    //write encyption data to new file
    size = fwrite(pData, sizeof(uint8_t), RestDataSize, NewFp);
    if(size != RestDataSize)
    {        
        free(pData);
        free(pBackupData);
        fclose(fp);
        fclose(NewFp);
        DISP_LOG(NewFileName, STR_FAIL_TO_WRITE_FILE);
        return STAT_ERR;
    }

    free(pData);
    free(pBackupData);
    fclose(fp);
    fclose(NewFp);
   
    return STAT_OK;    
}

static inline void DeleteEncyptSuffix(char *pFileName)
{
    int len = strlen(pFileName);
    char *ptr = pFileName + len - 1;

    while((*ptr != '.') && (len > 0))
    {
        ptr--;
        len--;
    }

    if(len != 0)
        *ptr = '\0';
}

static inline void ConvertFileFormat(char *pFileName)
{
    while(*pFileName != '\0')
    {
        if('/' == *pFileName)
            *pFileName = 92;    //'\' ASCII is 92 
        pFileName++;
    }
}
