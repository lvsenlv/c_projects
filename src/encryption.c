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

static G_STATUS EncryptDecryptFile(char func);
static G_STATUS EncryptDecryptFolder(char func);
static void *Pthread_ProcessFile(void *arg);
static void *Pthread_ProcessFolder(void *arg);
static G_STATUS CreateFileList(char *pFolderName);
static G_STATUS ParseFileList(FileList_t *pHeadNode);
static G_STATUS EncryptFile(char *pFileName, int64_t FileSize, int *pRatioFactor);
static G_STATUS DecryptFile(char *pFileName, int64_t FileSize, int *pRatioFactor);
static inline void DeleteEncyptSuffix(char *pFileName);
static inline void ConvertFileFormat(char *pFileName);

char g_password[CTL_PASSWORD_LENGHT_MAX];
FileList_t g_FileList;
FileList_t *g_CurFile = &g_FileList;
int g_RatioFactor[4];
int16_t g_PthreadNum = 0;
pthread_mutex_t FileLock = PTHREAD_MUTEX_INITIALIZER;

G_STATUS EncryptDecrypt(char func)
{
    if((func != CTL_MENU_ENCRYPT) && (func != CTL_MENU_DECRYPT))
    {
        DISP_ERR(STR_ERR_INVALID_FUNC);
        return STAT_ERR;
    }
    
    G_STATUS status;
    char FileName[CYT_FILE_NAME_LENGHT];
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
        g_FileList.FileName = FileName;
        g_FileList.FileNameLenght = strlen(g_FileList.FileName);
        g_FileList.FileSize = FileInfo.st_size;
        g_FileList.pNext = NULL;
        status = EncryptDecryptFile(func);
    }
    else if(S_IFDIR & FileInfo.st_mode)
    {
        g_FileList.FileName = FileName; //it means folder name
        g_FileList.FileNameLenght = 0;
        g_FileList.FileSize = 0;        //it means the number of files
        g_FileList.pNext = NULL;
        status = EncryptDecryptFolder(func);
    }

    return STAT_OK;
}

/*
    g_FileList must be initialized before following func is invoked
*/
static G_STATUS EncryptDecryptFile(char func)
{
    int lines = 0, cols = 0;
    int FileNameLenght = g_FileList.FileNameLenght;

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
    CTL_SET_COLOR(win, CTL_PANEL_CYAN);
    mvwhline(win, 0, 0, '-', cols);
    mvwhline(win, lines-1, 0, '-', cols);
    CTL_RESET_COLOR(win, CTL_PANEL_CYAN);

    CTL_SET_COLOR(win, CTL_PANEL_GREEN);
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
    CTL_RESET_COLOR(win, CTL_PANEL_GREEN);

    CTL_SET_COLOR(win, CTL_PANEL_YELLOW);    
    mvwaddstr(win, lines-2, 0, STR_RATE);
    wrefresh(win);

    PthreadArg_t PthreadArg;
    PthreadArg.func = func;
    PthreadArg.pRatioFactor = &g_RatioFactor[0];
    *PthreadArg.pRatioFactor = 0;    
    PthreadArg.ProcessStatus = PROCESS_STATUS_BUSY;

    PthreadArg.lock = PTHREAD_MUTEX_INITIALIZER;
    
    pthread_t PthreadID;
    int ret = 0;
    ret = pthread_create(&PthreadID, NULL, Pthread_ProcessFile, &PthreadArg);
	if(ret)
	{
		DISP_ERR(STR_ERR_PTHREAD_CREATE);
        return STAT_ERR;
	}

    int denominator = (int)(g_FileList.FileSize / CYT_SMALL_FILE_SIZE);
    if(0 == denominator)
        denominator = 100;
    
    while(PROCESS_STATUS_BUSY == PthreadArg.ProcessStatus)
    {
        mvwprintw(win, lines-2, (cols-3)/2, "%d%%", g_RatioFactor[0]/denominator);
        wrefresh(win);
        usleep(500000);
    }
    CTL_RESET_COLOR(win, CTL_PANEL_YELLOW);

    if(PROCESS_STATUS_SUCCESS == PthreadArg.ProcessStatus)
    {
        CTL_SET_COLOR(win, CTL_PANEL_GREEN);
        mvwhline(win, lines-2, 0, ' ', cols);
        mvwaddstr(win, lines-2, (cols-sizeof(STR_SUCCESS)+1)/2, STR_SUCCESS);
        CTL_RESET_COLOR(win, CTL_PANEL_GREEN);
        wrefresh(win);
    }
    else
    {
        CTL_SET_COLOR(win, CTL_PANEL_RED);
        mvwhline(win, lines-2, 0, ' ', cols);
        mvwaddstr(win, lines-2, (cols-sizeof(STR_FAIL)+1)/2, STR_FAIL);
        CTL_RESET_COLOR(win, CTL_PANEL_RED);
        wrefresh(win);
    }

    wgetch(win);

    delwin(win);
    touchwin(stdscr);
    refresh();
    return STAT_OK;
}

/*
    g_FileList must be initialized before following func is invoked
    Adapt 4 threads to process encrypting or decrypting.
*/
static G_STATUS EncryptDecryptFolder(char func)
{
    G_STATUS status = STAT_OK;

    while(1)
    {
        status = CreateFileList(g_FileList.FileName);
        if(status != STAT_OK)
        {        
            status = CTL_MakeChoice("%s", g_buf);
            if(status != STAT_RETRY)
                return status;
                
            continue;
        }

        status = ParseFileList(&g_FileList);
        if(status != STAT_OK)
        {            
            FreeFileList(&g_FileList);
            status = CTL_MakeChoice("%s", g_buf);
            if(STAT_RETRY != status)
                return status;
        }

        if(g_FileList.FileSize != 0)
            break;

        
        
    }
    
    WINDOW *win1 = newwin(LINES/4, COLS, 0, 0);
    WINDOW *win2 = newwin(LINES/4, COLS, LINES/4, 0);
    WINDOW *win3 = newwin(LINES/4, COLS, LINES/2, 0);
    WINDOW *win4 = newwin(LINES/4, COLS, LINES*3/4, 0);

    scrollok(win1, true);
    scrollok(win2, true);
    scrollok(win3, true);
    scrollok(win4, true);

    CTL_SET_COLOR(win1, CTL_PANEL_MAGENTA);
    CTL_SET_COLOR(win2, CTL_PANEL_MAGENTA);
    CTL_SET_COLOR(win3, CTL_PANEL_MAGENTA);
    CTL_SET_COLOR(win4, CTL_PANEL_MAGENTA);
    wattron(win1, A_REVERSE);
    mvwaddstr(win1, 0, 0, STR_TASK);
    wattroff(win1, A_REVERSE);
    mvwhline(win2, 0, 0, '-', COLS);
    mvwhline(win3, 0, 0, '-', COLS);
    mvwhline(win4, 0, 0, '-', COLS);
    CTL_RESET_COLOR(win1, CTL_PANEL_MAGENTA);
    CTL_RESET_COLOR(win2, CTL_PANEL_MAGENTA);
    CTL_RESET_COLOR(win3, CTL_PANEL_MAGENTA);
    CTL_RESET_COLOR(win4, CTL_PANEL_MAGENTA);

    wmove(win2, 1, 0);
    wmove(win3, 1, 0);
    wmove(win4, 1, 0);

    wrefresh(win1);
    wrefresh(win2);
    wrefresh(win3);
    wrefresh(win4);

    wgetch(win1);

    FreeFileList(&g_FileList);
    delwin(win1);
    delwin(win2);
    delwin(win3);
    delwin(win4);
    touchwin(stdscr);
    refresh();

    return STAT_OK;
}

static G_STATUS CreateFileList(char *pFolderName)
{
    char buf[CYT_FILE_NAME_LENGHT+64];
    snprintf(buf, sizeof(buf), "find %s -type f > %s 2>/dev/null", pFolderName, FILE_LIST_NAME);

    FILE *fp = NULL;
    fp = popen(buf, "r");
    if(NULL == fp)
    {
        pclose(fp);
        DISP_ERR(STR_FAIL_TO_CREATE_FILE_LIST);
        return STAT_ERR;
    }

    pclose(fp);
    return STAT_OK;
}

static G_STATUS ParseFileList(FileList_t *pHeadNode)
{    
    if(access(FILE_LIST_NAME, F_OK) != 0)
    {
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
        DISP_ERR(STR_ERR_FAIL_TO_GET_FILE_INFO);
        return STAT_ERR;
    }

    if(!(S_IFREG & FileInfo.st_mode))
    {
        DISP_ERR(STR_ERR_INVALID_FILE_LIST);
        return STAT_ERR;
    }    

    FILE *fp = NULL;
    fp = fopen(FILE_LIST_NAME, "rb");
    if(NULL == fp)
    {
        DISP_ERR(STR_ERR_FAIL_TO_OPEN_FILE_LIST);
        return STAT_ERR;
    }

    char *FileName = NULL;
    FileName = (char *)malloc(CYT_FILE_NAME_LENGHT);
    if(NULL == FileName)
    {
        DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
        return STAT_ERR;
    }    

    int FileNameLenght;
    FileList_t *NewNode, *CurNode = pHeadNode;
    pHeadNode->FileSize = 0;
    char *pTmp;
    while((fgets(FileName, CYT_FILE_NAME_LENGHT, fp) != 0) && (0 == feof(fp)))
    {
        FileNameLenght = strlen(FileName);
        if(0 == FileNameLenght)
            continue;
        
        FileName[FileNameLenght-1] = '\0';
        FileNameLenght--;
        
        if(access(FileName, F_OK) != 0)
            continue;        
        
#ifdef __LINUX
        if(stat(FileName, &FileInfo) != 0)
#elif defined __WINDOWS
        if(_stati64(FileName, &FileInfo) != 0)
#endif
            continue;

        if(!(S_IFREG & FileInfo.st_mode))
            continue;
        
        NewNode = (FileList_t *)malloc(sizeof(FileList_t));
        if(NULL == NewNode)
        {
            fclose(fp);
            DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
            return STAT_ERR;
        }
        
        pTmp = (char *)malloc(FileNameLenght);
        if(NULL == pTmp)
        {
            fclose(fp);
            DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
            return STAT_ERR;
        }

        memcpy(pTmp, FileName, FileNameLenght);        
        NewNode->FileName = pTmp;
        NewNode->FileNameLenght = FileNameLenght;
        NewNode->FileSize = FileInfo.st_size;
        NewNode->pNext = NULL;
        
        CurNode->pNext = NewNode;
        CurNode = NewNode;

        pHeadNode->FileSize++;
    }

    free(FileName);
    fclose(fp);
    return STAT_OK;
}



/*************************************************************************
                                Pthread
************************************************************************/
static void *Pthread_ProcessFile(void *arg)
{
    PthreadArg_t *pArg_t = (PthreadArg_t *)arg;
    G_STATUS status;
    
    if(CTL_MENU_ENCRYPT == pArg_t->func)
        status = EncryptFile(g_FileList.FileName, g_FileList.FileSize, pArg_t->pRatioFactor);
    else
        status = DecryptFile(g_FileList.FileName, g_FileList.FileSize, pArg_t->pRatioFactor);

    pthread_mutex_lock(&pArg_t->lock);
    if(status != STAT_OK)
    {
        
        if(STAT_ERR == status)
            pArg_t->ProcessStatus = PROCESS_STATUS_FAIL;
        else if(STAT_EXIT == status)
            pArg_t->ProcessStatus = PROCESS_STATUS_EXIT;
    }
    else
    {
        pArg_t->ProcessStatus = PROCESS_STATUS_SUCCESS;
    }
    pthread_mutex_unlock(&pArg_t->lock);
    
    return NULL;
}

static void *Pthread_ProcessFolder(void *arg)
{
    PthreadArg_t *pArg_t = (PthreadArg_t *)arg;
    G_STATUS (*pFunc)(char *, int64_t, int *);
    FileList_t *CurFile;
    G_STATUS status = STAT_OK;    

    if(CTL_MENU_ENCRYPT == pArg_t->func)
        pFunc = EncryptFile;
    else
        pFunc = DecryptFile;

    pthread_mutex_lock(&FileLock);
    CurFile = g_CurFile;
    g_CurFile = g_CurFile->pNext;
    pthread_mutex_unlock(&FileLock);

    while(CurFile != NULL);
    {
        
        pthread_mutex_lock(&pArg_t->lock);
        pArg_t->CurFileName = CurFile->FileName;
        pthread_mutex_unlock(&pArg_t->lock);        
        
        status = (*pFunc)(CurFile->FileName, CurFile->FileSize, pArg_t->pRatioFactor);
        pthread_mutex_lock(&pArg_t->lock);
        if(status != STAT_OK)
        {
            if(STAT_EXIT == status)
            {
                *pArg_t->pRatioFactor = PROCESS_STATUS_EXIT;
                pthread_mutex_unlock(&pArg_t->lock);
                return NULL;
            }
            
            *pArg_t->pRatioFactor = PROCESS_STATUS_FAIL;            
            
        }
        else
        {
            *pArg_t->pRatioFactor = PROCESS_STATUS_SUCCESS;
        }
        pthread_mutex_unlock(&pArg_t->lock);
                
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
static G_STATUS EncryptFile(char *pFileName, int64_t FileSize, int *pRatioFactor)
{
    FILE *fp = NULL;
    fp = fopen(pFileName, "rb");
    if(NULL == fp)
    {
        DISP_LOG(pFileName, STR_FOPEN_ERR);
        return STAT_ERR;
    }
    fseek(fp, 0, SEEK_SET);

    char NewFileName[CYT_FILE_NAME_LENGHT];
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
    int EncyptFactor = 0;
    int PasswordLenght = 0;
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
    int i = 0, index = 0;
    int CycleIndex = (int)(FileSize / CYT_SMALL_FILE_SIZE);

    int RestDataCount = CYT_SMALL_FILE_SIZE - PasswordLenght;
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
        *pRatioFactor = index*100;
        
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

        *pRatioFactor += 10;
        
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

        *pRatioFactor += 50;

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
        
        *pRatioFactor += 10;

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

        *pRatioFactor += 10;

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
    int RestDataSize = (int)(FileSize % CYT_SMALL_FILE_SIZE);
    
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

static G_STATUS DecryptFile(char *pFileName, int64_t FileSize, int *pRatioFactor)
{
    FILE *fp = NULL;
    fp = fopen(pFileName, "rb");
    if(NULL == fp)
    {
        DISP_LOG(pFileName, STR_FOPEN_ERR);
        return STAT_ERR;
    }
    fseek(fp, 0, SEEK_SET);

    char NewFileName[CYT_FILE_NAME_LENGHT];
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
    int EncyptFactor = 0;
    int PasswordLenght = 0;
    const char *pPassword = g_password;

    //get decrypt factor
    while(*pPassword != '\0')
    {
        EncyptFactor += (uint)*pPassword;
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
    int i = 0, index = 0;
    int CycleIndex = (int)(FileSize / CYT_SMALL_FILE_SIZE);

    int RestDataCount = CYT_SMALL_FILE_SIZE - PasswordLenght;
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
        *pRatioFactor = index*100;
        
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

        *pRatioFactor += 10;
        
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

        *pRatioFactor += 10;

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

        *pRatioFactor += 10;

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

        *pRatioFactor += 50;

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
    int RestDataSize = (int)(FileSize % CYT_SMALL_FILE_SIZE);
    
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
