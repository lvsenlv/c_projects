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
#include "core_code.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static G_STATUS EncryptFile(FileList_t *pFileList);
static G_STATUS DecryptFile(FileList_t *pFileList);
static G_STATUS EncryptFolder(FileList_t *pFileList);
static G_STATUS DecryptFolder(FileList_t *pFileList);
static G_STATUS CreateFileList(char *pFolderName);
static G_STATUS ParseFileList(FileList_t *pHeadNode);
static inline void InitFileList(FileList_t *pFileList);
static inline void InitPthreadArg(PthreadArg_t *pArg_t);

char g_password[CTL_PASSWORD_LENGHT_MAX];
FileList_t g_FileList;
int g_RatioFactor[4];

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

        if(CTL_MENU_ENCRYPT == func)
        {
            status = EncryptFile(&g_FileList);
        }
        else
        {
            status = DecryptFile(&g_FileList);
        }
    }
    else if(S_IFDIR & FileInfo.st_mode)
    {
        g_FileList.FileName = FileName; //it means folder name
        g_FileList.FileNameLenght = 0;
        g_FileList.FileSize = 0;        //it means the number of files
        g_FileList.pNext = NULL;
        
        if(CTL_MENU_ENCRYPT == func)
        {
            status = EncryptFolder(&g_FileList);
        }
        else
        {
            status = DecryptFolder(&g_FileList);
        }
    }

    return STAT_OK;
}



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static G_STATUS EncryptFile(FileList_t *pFileList)
{
    int lines = 0, cols = 0;

    if((sizeof(STR_IN_ENCRYPTING)-1 + pFileList->FileNameLenght) <= CTL_ENCRYPT_FILE_WIN_COLS)
    {
        lines = 4;
        cols = pFileList->FileNameLenght + sizeof(STR_IN_ENCRYPTING) - 1;
    }
    else
    {
        lines = ((sizeof(STR_IN_ENCRYPTING)-1 + pFileList->FileNameLenght)
            /CTL_ENCRYPT_FILE_WIN_COLS) + 4;
        cols = CTL_ENCRYPT_FILE_WIN_COLS;
    }
    
    WINDOW *win = newwin(lines, cols, (LINES-lines)/2, (COLS-cols)/2);
    CTL_SET_COLOR(win, CTL_PANEL_CYAN);
    mvwhline(win, 0, 0, '-', cols);
    mvwhline(win, lines-1, 0, '-', cols);
    CTL_RESET_COLOR(win, CTL_PANEL_CYAN);

    CTL_SET_COLOR(win, CTL_PANEL_GREEN);
    if((sizeof(STR_IN_ENCRYPTING)-1 + pFileList->FileNameLenght)%cols != 0)
    {
        mvwprintw(win, 1, 0, "%s%s\n", STR_IN_ENCRYPTING, pFileList->FileName);
    }
    else
    {
        mvwprintw(win, 1, 0, "%s%s", STR_IN_ENCRYPTING, pFileList->FileName);
    }
    CTL_RESET_COLOR(win, CTL_PANEL_GREEN);

    CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
    mvwaddstr(win, lines-2, 0, STR_RATE);
    wrefresh(win);

    PthreadArg_t PthreadArg;
    InitPthreadArg(&PthreadArg);
    PthreadArg.pRatioFactor = &g_RatioFactor[0];
    *PthreadArg.pRatioFactor = 0;
    PthreadArg.pLock = &g_StatusLock[0];
    PthreadArg.ProcessStatus = PROCESS_STATUS_BUSY;
    PthreadArg.pFileList = &g_FileList;
    pthread_t PthreadID;
    int ret = 0;
    ret = pthread_create(&PthreadID, NULL, Pthread_EncryptFile, &PthreadArg);
	if(ret)
	{
		DISP_ERR(STR_ERR_PTHREAD_CREATE);
        return STAT_ERR;
	}

    int denominator = (int)(pFileList->FileSize / CYT_SMALL_FILE_SIZE);
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

static G_STATUS DecryptFile(FileList_t *pFileList)
{
    return STAT_OK;
}

//Adapt 4 threads to process encrypting or decrypting
static G_STATUS EncryptFolder(FileList_t *pFileList)
{
    G_STATUS status = STAT_OK;

    status = CreateFileList(pFileList->FileName);
    if(status != STAT_OK)
        return status;

    status = ParseFileList(pFileList);
    if(status != STAT_OK)
    {            
        FreeFileList(pFileList);
        return status;
    }

    if(pFileList->FileSize == 0)
    {
        DISP(STR_FILE_LIST_IS_NULL);
        return STAT_ERR;
    }
    
    return STAT_OK;
}

static G_STATUS DecryptFolder(FileList_t *pFileList)
{
    return STAT_OK;
}



//file list related
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
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



//static inline functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static inline void InitFileList(FileList_t *pFileList)
{
    pFileList->FileName = NULL;
    pFileList->FileNameLenght = 0;
    pFileList->FileSize = 0;
    pFileList->pNext = NULL;
}

static inline void InitPthreadArg(PthreadArg_t *pArg_t)
{
    pArg_t->pCurFileList = NULL;
    pArg_t->pCurFileName = NULL;
    pArg_t->pFileList = NULL;
    pArg_t->pLock = (pthread_mutex_t *)0;
    pArg_t->pRatioFactor = NULL;
    pArg_t->ProcessStatus = PROCESS_STATUS_BUSY;
}