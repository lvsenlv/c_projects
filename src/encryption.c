/*************************************************************************
	> File Name: encryption.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 3rd,2017 Thursday 08:37:45
 ************************************************************************/

#ifdef __LINUX
#define _FILE_OFFSET_BITS 64 //Make sure st_size is 64bits instead of 32bits
#endif

#include "encryption.h"
#include "core_code.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

static PROCESS_STATUS EncryptDecrypt(FileList_t *pFileList, CTL_MENU func);
static PROCESS_STATUS EncryptDecrypt_Plus(FileList_t *pFileList, CTL_MENU func);
static G_STATUS AfterEncryptDecrypt(PROCESS_STATUS ProcessStatus);
static FileList_t *ScanDirectory(char *pFolderName);
static FileList_t *ScanEncryptFile(char *pFolderName);
static G_STATUS ParseLog(log_t *pHeadNode);
static inline void DeleteTailSymbol(char *pFileName, int FileNameLenght);
static inline void InitFileListNode(FileList_t *pFileList);
static inline void FreeFileList(FileList_t *pHeadNode);
static inline G_STATUS IsEncryptFile(const char *pFileName, int FileNameLenght);
static inline void FreeLog(log_t *pHeadNode);

char g_password[CTL_PASSWORD_LENGHT_MAX];
pthread_mutex_t g_LogLock = PTHREAD_MUTEX_INITIALIZER;
static int g_SuccessFailCountTable[2] = {0, 0};



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
G_STATUS CTL_MENU_EncryptDecrypt(CTL_MENU func)
{
    if((func != CTL_MENU_ENCRYPT) && (func != CTL_MENU_DECRYPT))
    {
        CTL_ErrExit(STR_ERR_INVALID_FUNC);
    }
    
    G_STATUS status;
    char FileName[CYT_FILE_NAME_LENGHT];
#ifdef __LINUX
        struct stat FileInfo;
#elif defined __WINDOWS
        struct _stati64 FileInfo;
#endif

    int FileNameLenght;
    while(1)
    {
        status = CTL_GetFileName(FileName);
        if(status != STAT_OK)
            return status;

#ifdef __WINDOWS
        ConvertNameFormat(FileName);
#endif        
        FileNameLenght = strlen(FileName);
        DeleteTailSymbol(FileName, FileNameLenght);

#ifdef __LINUX
        if(lstat(FileName, &FileInfo) == 0)
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

    PROCESS_STATUS ProcessStatus = PROCESS_STATUS_ERR;

    if(S_IFREG & FileInfo.st_mode)
    {
        FileList_t HeadNode, FileList;

        HeadNode.pFileName = NULL;
        HeadNode.FileNameLenght = 0;
        HeadNode.FileSize = 1;
        HeadNode.pNext = &FileList;
        
        FileList.pFileName = FileName;
        FileList.FileNameLenght = FileNameLenght;
        FileList.FileSize = FileInfo.st_size;
        FileList.pNext = NULL;

        ProcessStatus = EncryptDecrypt_Plus(&HeadNode, func);
    }
    else if(S_IFDIR & FileInfo.st_mode)
    {
        FileList_t *pFileList = NULL;
        
        pFileList = (CTL_MENU_ENCRYPT == func) ? 
            ScanDirectory(FileName) : ScanEncryptFile(FileName);
            
        if(NULL == pFileList)
        {
            DISP_ERR_PLUS("%s: %s", STR_FAIL_TO_SCAN_DIRECTORY, FileName);
            return STAT_ERR;
        }

        char buf[BUF_SIZE];
        if(CTL_MENU_ENCRYPT == func)
        {
            snprintf(buf, sizeof(buf), "%s%s", STR_IN_ENCRYPTING, FileName);
            status = CTL_ConfirmOperation(buf, FileNameLenght+sizeof(STR_IN_ENCRYPTING)-1);
        }
        else
        {
            snprintf(buf, sizeof(buf), "%s%s", STR_IN_DECRYPTING, FileName);
            status = CTL_ConfirmOperation(buf, FileNameLenght+sizeof(STR_IN_DECRYPTING)-1);
        }
        
        if(STAT_OK != status)
            return status;
        
        ProcessStatus = EncryptDecrypt(pFileList, func);

        FreeFileList(pFileList);
    }

    status = AfterEncryptDecrypt(ProcessStatus);

    return status;
}



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//Note: pFileList must be the HeadNode
static PROCESS_STATUS EncryptDecrypt(FileList_t *pFileList, CTL_MENU func)
{
#ifdef PTHREAD_NUM_MAX
#if (PTHREAD_NUM_MAX < 2)
    #error STR_ERR_PTHREAD_NUM_TOO_SMALL
#elif (PTHREAD_NUM_MAX > 8)
    #error STR_ERR_PTHREAD_NUM_TOO_BIG
#endif
#endif

#ifdef __DEBUG
    if(NULL == pFileList)
    {
        DISP_ERR(STR_ERR_INVALID_FILE_LIST);
        return PROCESS_STATUS_ELSE_ERR;
    }
#endif

    //Create the window >>>
    WINDOW *ScheduleWin[PTHREAD_NUM_MAX];
    WINDOW *win[PTHREAD_NUM_MAX];
    int i;

    for(i = 0; i < PTHREAD_NUM_MAX; i++)
    {
        ScheduleWin[i] = newwin(1, COLS, i*LINES/PTHREAD_NUM_MAX, 0);
    }
    
    for(i = 0; i < PTHREAD_NUM_MAX-1; i++) //PTHREAD_NUM_MAX-1 : the last win should tackle additionally
    {
        /*
            LINES-PTHREAD_NUM_MAX : schedule wins take cost of PTHREAD_NUM_MAX lines
            i*LINES/PTHREAD_NUM_MAX + 1 : schedule wins occupy the position i*LINES/PTHREAD_NUM_MAX, 
                so the win should start from to next line.
        */
        win[i] = newwin((LINES-PTHREAD_NUM_MAX)/PTHREAD_NUM_MAX, COLS, 
            i*LINES/PTHREAD_NUM_MAX + 1, 0);
    }

    /*
        It must be under considered the situation that the result of LINES/PTHREAD_NUM_MAX 
            is decimal fraction.
        Thus, use "LINES-PTHREAD_NUM_MAX - (LINES-PTHREAD_NUM_MAX)*(PTHREAD_NUM_MAX-1)/PTHREAD_NUM_MAX"
            instead of "(LINES-PTHREAD_NUM_MAX)/PTHREAD_NUM_MAX"
    */
    win[PTHREAD_NUM_MAX-1] = newwin(LINES-PTHREAD_NUM_MAX - 
        (LINES-PTHREAD_NUM_MAX)*(PTHREAD_NUM_MAX-1)/PTHREAD_NUM_MAX, 
        COLS, LINES*(PTHREAD_NUM_MAX-1)/PTHREAD_NUM_MAX + 1, 0);
    
    for(i = 0; i < PTHREAD_NUM_MAX; i++)
    {
        CTL_SET_COLOR(ScheduleWin[i], CTL_PANEL_CYAN);
        CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
        
        wmove(win[i], 0, 0);
        scrollok(win[i], true);
        
        wattron(ScheduleWin[i], A_UNDERLINE);
        mvwhline(ScheduleWin[i], 0, 0, ' ', COLS);

        mvwprintw(ScheduleWin[i], 0, 0, "%s%d->%s", STR_TASK, i+1, 
            (CTL_MENU_ENCRYPT == func) ? STR_IN_ENCRYPTING : STR_IN_DECRYPTING);
        mvwprintw(ScheduleWin[i], 0, COLS*2/5, "%s0", STR_SUCCESS_COUNT);
        mvwprintw(ScheduleWin[i], 0, COLS*3/5, "%s0", STR_FAIL_COUNT);
        mvwprintw(ScheduleWin[i], 0, COLS*4/5, "%s0%%", STR_RATE);
    
        wrefresh(ScheduleWin[i]);
        wrefresh(win[i]);
    }

    g_pCurFilelist = pFileList->pNext; //First FileList is head node
    
    int RatioFactor[PTHREAD_NUM_MAX];
    int denominator[PTHREAD_NUM_MAX];
    PthreadArg_t PthreadArg[PTHREAD_NUM_MAX];
    pthread_t PthreadID[PTHREAD_NUM_MAX];
    int ret = 0;
    
    for(i = 0; i < PTHREAD_NUM_MAX; i++)
    {
        RatioFactor[i] = 0;
        denominator[i] = 100;
        InitPthreadArg(&PthreadArg[i]);
        
        PthreadArg[i].pFunc = (CTL_MENU_ENCRYPT == func) ? encrypt : decrypt;
        PthreadArg[i].pRatioFactor = &RatioFactor[i];
        PthreadArg[i].pCurFileList = g_pCurFilelist;
        PthreadArg[i].ProcessStatus = PROCESS_STATUS_BUSY;
        PthreadArg[i].SuccessCount = 0;
        PthreadArg[i].FailCount = 0;
        PthreadArg[i].RefreshFlag = REFRESH_FLAG_FALSE;
    }
     
    for(i = 0; i < PTHREAD_NUM_MAX; i++)
    {   
        ret = pthread_create(&PthreadID[i], NULL, Pthread_EncryptDecrypt, &PthreadArg[i]);
	    if(ret != 0)
	    {
		    DISP_ERR(STR_ERR_PTHREAD_CREATE);
            return PROCESS_STATUS_ELSE_ERR;
	    }
    }
    
    char ExitFlag = 0;
    char flag = 0; //Set 1 if it has been refreshed <=> if(REFRESH_FLAG_TRUE <= PthreadArg[i].RefreshFlag)
	
	while(1)
    {
        if(ExitFlag)
            break;
        
        ExitFlag = 1;
        
        for(i = 0; i < PTHREAD_NUM_MAX; i++)
        {
            if(PROCESS_STATUS_BUSY == PthreadArg[i].ProcessStatus)
            {
                ExitFlag = 0;
                mvwprintw(ScheduleWin[i], 0, (COLS*4/5 + sizeof(STR_RATE)-1), 
                    "%d%%  ", RatioFactor[i]/denominator[i]);
                
                if(REFRESH_FLAG_TRUE <= PthreadArg[i].RefreshFlag)
                {
                    flag = 1;
                    switch(PthreadArg[i].RefreshFlag)
                    {
                        case REFRESH_FLAG_SUCCESS:
                            mvwprintw(ScheduleWin[i], 0, (COLS*2/5 + sizeof(STR_SUCCESS_COUNT)-1), 
                                "%d", PthreadArg[i].SuccessCount);
                            mvwprintw(ScheduleWin[i], 0, (COLS*4/5 + sizeof(STR_RATE)-1), "100%%");
                            CTL_SET_COLOR(win[i], CTL_PANEL_GREEN);
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s\n", STR_SUCCESS);
                            wattroff(win[i], A_BOLD);
                            CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
                            break;
                        case REFRESH_FLAG_FAIL:
                            mvwprintw(ScheduleWin[i], 0, (COLS*3/5 + sizeof(STR_FAIL_COUNT)-1), 
                                "%d", PthreadArg[i].FailCount);
                            CTL_SET_COLOR(win[i], CTL_PANEL_RED);
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s\n", STR_FAIL);
                            wattroff(win[i], A_BOLD);
                            CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
                            break;
                        default:
                            RatioFactor[i] = 0;
                            denominator[i] = (int)(PthreadArg[i].pCurFileList->FileSize / CYT_SMALL_FILE_SIZE);
                            if(0 == denominator[i])
                                denominator[i] = 100;
                            wprintw(win[i], "%s", PthreadArg[i].pCurFileList->pFileName);
                            break;
                    }
                    
                    wrefresh(win[i]);
                    PthreadArg[i].RefreshFlag = REFRESH_FLAG_FALSE; //Clear refresh flag, continue en/decrypting
                }
            
                wrefresh(ScheduleWin[i]);
            }
        }

        if(flag)
        {
            flag = 0;
            continue;
        }
        
        usleep(REFRESH_INTERVAL);
	}
        
    for(i = 0; i < PTHREAD_NUM_MAX; i++)
    {
        delwin(ScheduleWin[i]);
        delwin(win[i]);
    }
    
    touchwin(stdscr);
    refresh();

    for(i = 0; i < PTHREAD_NUM_MAX; i++)
    {
        g_SuccessFailCountTable[0] += PthreadArg[i].SuccessCount;
        g_SuccessFailCountTable[1] += PthreadArg[i].FailCount;
    }
    
    for(i = 0; i < PTHREAD_NUM_MAX; i++)
    {
        if(PROCESS_STATUS_SUCCESS != PthreadArg[i].ProcessStatus)
        {
            if(PROCESS_STATUS_FATAL_ERR == PthreadArg[i].ProcessStatus)
                return PROCESS_STATUS_FATAL_ERR;
                
            return PROCESS_STATUS_ERR;
        }
    }
        
    return PROCESS_STATUS_SUCCESS;
}

//Create pthreads according to pFileList->FileSize(HeadNode)
static PROCESS_STATUS EncryptDecrypt_Plus(FileList_t *pFileList, CTL_MENU func)
{
#ifdef PTHREAD_NUM_MAX
#if (PTHREAD_NUM_MAX < 2)
    #error STR_ERR_PTHREAD_NUM_TOO_SMALL
#elif (PTHREAD_NUM_MAX > 8)
    #error STR_ERR_PTHREAD_NUM_TOO_BIG
#endif
#endif

#ifdef __DEBUG
    if(NULL == pFileList)
    {
        DISP_ERR(STR_ERR_INVALID_FILE_LIST);
        return PROCESS_STATUS_ELSE_ERR;
    }
#endif

    //Create the window >>>
    WINDOW *ScheduleWin[PTHREAD_NUM_MAX];
    WINDOW *win[PTHREAD_NUM_MAX];
    int i, CycleIndex;

    CycleIndex = (PTHREAD_NUM_MAX <= pFileList->FileSize) ? 
        PTHREAD_NUM_MAX : pFileList->FileSize;

    for(i = 0; i < CycleIndex; i++)
    {
        ScheduleWin[i] = newwin(1, COLS, i*LINES/CycleIndex, 0);
    }
    
    for(i = 0; i < CycleIndex-1; i++) //PTHREAD_NUM_MAX-1 : the last win should tackle additionally
    {
        /*
            LINES-PTHREAD_NUM_MAX : schedule wins take cost of PTHREAD_NUM_MAX lines
            i*LINES/PTHREAD_NUM_MAX + 1 : schedule wins occupy the position i*LINES/PTHREAD_NUM_MAX, 
                so the win should start from to next line.
        */
        win[i] = newwin((LINES-CycleIndex)/CycleIndex, COLS, 
            i*LINES/CycleIndex + 1, 0);
    }

    /*
        It must be under considered the situation that the result of LINES/PTHREAD_NUM_MAX 
            is decimal fraction.
        Thus, use "LINES-PTHREAD_NUM_MAX - (LINES-PTHREAD_NUM_MAX)*(PTHREAD_NUM_MAX-1)/PTHREAD_NUM_MAX"
            instead of "(LINES-PTHREAD_NUM_MAX)/PTHREAD_NUM_MAX"
    */
    win[CycleIndex-1] = newwin(LINES-CycleIndex - 
        (LINES-CycleIndex)*(CycleIndex-1)/CycleIndex, 
        COLS, LINES*(CycleIndex-1)/CycleIndex + 1, 0);
    
    for(i = 0; i < CycleIndex; i++)
    {
        CTL_SET_COLOR(ScheduleWin[i], CTL_PANEL_CYAN);
        CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
        
        wmove(win[i], 0, 0);
        scrollok(win[i], true);
        
        wattron(ScheduleWin[i], A_UNDERLINE);
        mvwhline(ScheduleWin[i], 0, 0, ' ', COLS);

        mvwprintw(ScheduleWin[i], 0, 0, "%s%d->%s", STR_TASK, i+1, 
            (CTL_MENU_ENCRYPT == func) ? STR_IN_ENCRYPTING : STR_IN_DECRYPTING);
        mvwprintw(ScheduleWin[i], 0, COLS*2/5, "%s0", STR_SUCCESS_COUNT);
        mvwprintw(ScheduleWin[i], 0, COLS*3/5, "%s0", STR_FAIL_COUNT);
        mvwprintw(ScheduleWin[i], 0, COLS*4/5, "%s0%%", STR_RATE);
    
        wrefresh(ScheduleWin[i]);
        wrefresh(win[i]);
    }

    g_pCurFilelist = pFileList->pNext; //First FileList is head node
    
    int RatioFactor[PTHREAD_NUM_MAX];
    int denominator[PTHREAD_NUM_MAX];
    PthreadArg_t PthreadArg[PTHREAD_NUM_MAX];
    pthread_t PthreadID[PTHREAD_NUM_MAX];
    int ret = 0;
    
    for(i = 0; i < CycleIndex; i++)
    {
        RatioFactor[i] = 0;
        denominator[i] = 100;
        InitPthreadArg(&PthreadArg[i]);
        
        PthreadArg[i].pFunc = (CTL_MENU_ENCRYPT == func) ? encrypt : decrypt;
        PthreadArg[i].pRatioFactor = &RatioFactor[i];
        PthreadArg[i].pCurFileList = g_pCurFilelist;
        PthreadArg[i].ProcessStatus = PROCESS_STATUS_BUSY;
        PthreadArg[i].SuccessCount = 0;
        PthreadArg[i].FailCount = 0;
        PthreadArg[i].RefreshFlag = REFRESH_FLAG_FALSE;
    }
     
    for(i = 0; i < CycleIndex; i++)
    {   
        ret = pthread_create(&PthreadID[i], NULL, Pthread_EncryptDecrypt, &PthreadArg[i]);
	    if(ret != 0)
	    {
		    DISP_ERR(STR_ERR_PTHREAD_CREATE);
            return PROCESS_STATUS_ELSE_ERR;
	    }
    }
    
    char ExitFlag = 0;
    char flag = 0; //Set 1 if it has been refreshed <=> if(REFRESH_FLAG_TRUE <= PthreadArg[i].RefreshFlag)
	
	while(1)
    {
        if(ExitFlag)
            break;
        
        ExitFlag = 1;
        
        for(i = 0; i < CycleIndex; i++)
        {
            if(PROCESS_STATUS_BUSY == PthreadArg[i].ProcessStatus)
            {
                ExitFlag = 0;
                mvwprintw(ScheduleWin[i], 0, (COLS*4/5 + sizeof(STR_RATE)-1), 
                    "%d%%  ", RatioFactor[i]/denominator[i]);
                
                if(REFRESH_FLAG_TRUE <= PthreadArg[i].RefreshFlag)
                {
                    flag = 1;
                    switch(PthreadArg[i].RefreshFlag)
                    {
                        case REFRESH_FLAG_SUCCESS:
                            mvwprintw(ScheduleWin[i], 0, (COLS*2/5 + sizeof(STR_SUCCESS_COUNT)-1), 
                                "%d", PthreadArg[i].SuccessCount);
                            mvwprintw(ScheduleWin[i], 0, (COLS*4/5 + sizeof(STR_RATE)-1), "100%%");
                            CTL_SET_COLOR(win[i], CTL_PANEL_GREEN);
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s\n", STR_SUCCESS);
                            wattroff(win[i], A_BOLD);
                            CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
                            break;
                        case REFRESH_FLAG_FAIL:
                            mvwprintw(ScheduleWin[i], 0, (COLS*3/5 + sizeof(STR_FAIL_COUNT)-1), 
                                "%d", PthreadArg[i].FailCount);
                            //mvwhline(ScheduleWin[i], 0, (COLS*4/5 + sizeof(STR_RATE)-1), ' ', 4);
                            //mvwprintw(ScheduleWin[i], 0, (COLS*4/5 + sizeof(STR_RATE)-1), "0%%");
                            CTL_SET_COLOR(win[i], CTL_PANEL_RED);
                            wattron(win[i], A_BOLD);
                            wprintw(win[i], " %s\n", STR_FAIL);
                            wattroff(win[i], A_BOLD);
                            CTL_SET_COLOR(win[i], CTL_PANEL_YELLOW);
                            break;
                        default:
                            RatioFactor[i] = 0;
                            denominator[i] = (int)(PthreadArg[i].pCurFileList->FileSize / CYT_SMALL_FILE_SIZE);
                            if(0 == denominator[i])
                                denominator[i] = 100;
                            //mvwhline(ScheduleWin[i], 0, (COLS*4/5 + sizeof(STR_RATE)-1), ' ', 4);
                            wprintw(win[i], "%s", PthreadArg[i].pCurFileList->pFileName);
                            break;
                    }
                    
                    wrefresh(win[i]);
                    PthreadArg[i].RefreshFlag = REFRESH_FLAG_FALSE; //Clear refresh flag, continue en/decrypting
                }
            
                wrefresh(ScheduleWin[i]);
            }
        }

        if(flag)
        {
            flag = 0;
            continue;
        }
        
        usleep(REFRESH_INTERVAL);
	}
        
    for(i = 0; i < CycleIndex; i++)
    {
        delwin(ScheduleWin[i]);
        delwin(win[i]);
    }
    
    touchwin(stdscr);
    refresh();
    
    for(i = 0; i < CycleIndex; i++)
    {
        if(PROCESS_STATUS_SUCCESS != PthreadArg[i].ProcessStatus)
        {
            if(PROCESS_STATUS_FATAL_ERR == PthreadArg[i].ProcessStatus)
                return PROCESS_STATUS_FATAL_ERR;
                
            return PROCESS_STATUS_ERR;
        }
    }
        
    return PROCESS_STATUS_SUCCESS;
}



//File list related
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static FileList_t *ScanDirectory(char *pFolderName)
{
    DIR *pDir;
    pDir = opendir(pFolderName);
    if(NULL == pDir)
        return NULL;
    
    FileList_t *pHeadNode = (FileList_t *)malloc(sizeof(FileList_t));
    if(NULL == pHeadNode)
    {
        closedir(pDir);
        return NULL;
    }
    
    InitFileListNode(pHeadNode);
    
    FileList_t *pCurFileList = pHeadNode, *pNewFileList;
    struct dirent *pEntry;
    int FolderNameLenght = strlen(pFolderName);
    int FileNameLenght;
    char *pFileName;
#ifdef __LINUX
    struct stat FileInfo;
#elif defined __WINDOWS
    struct _stati64 FileInfo;
#endif

    while(1)
    {
        pEntry = readdir(pDir);
        if(NULL == pEntry)
            break;

        //Ignore ".", ".."
        if('.' == pEntry->d_name[0])
        {
            if('\0' == pEntry->d_name[1])
                continue;
            else if(('.' == pEntry->d_name[1]) && ('\0' == pEntry->d_name[2]))
                continue;
        }

        FileNameLenght = FolderNameLenght + strlen(pEntry->d_name) + 2; //contains '/' or '\' and '\0'
        pFileName = (char *)malloc(FileNameLenght);
        if(NULL == pFileName)
        {
            FreeFileList(pHeadNode);
            closedir(pDir);
            return NULL;
        }

        snprintf(pFileName, FileNameLenght, "%s/%s", pFolderName, pEntry->d_name);
        
#ifdef __LINUX
        if(0 != lstat(pFileName, &FileInfo))
#elif defined __WINDOWS
        if(0 != _stati64(pFileName, &FileInfo))
#endif
        {
            free(pFileName);
            continue;
        }
        
        if((DT_DIR == pEntry->d_type) && (FileInfo.st_mode & S_IFDIR))
        {
            pNewFileList = ScanDirectory(pFileName);
            if(NULL == pNewFileList)
            {
                free(pFileName);
                continue;
            }
            
            pHeadNode->FileSize += pNewFileList->FileSize;
            pCurFileList->pNext = pNewFileList->pNext;
            pCurFileList = pNewFileList;

            while(pCurFileList->pNext != NULL)
            {
                pCurFileList = pCurFileList->pNext;
            }
            
            if(pNewFileList->pFileName != NULL)
                free(pNewFileList->pFileName);
            free(pNewFileList);
        }
        else if((DT_REG == pEntry->d_type) && (FileInfo.st_mode & S_IFREG))
        {
            pNewFileList = (FileList_t *)malloc(sizeof(FileList_t));
            if(NULL == pNewFileList)
            {
                free(pFileName);
                FreeFileList(pHeadNode);
                closedir(pDir);
                return NULL;
            }
            
            pNewFileList->pFileName = pFileName;
            pNewFileList->FileNameLenght = FileNameLenght;
            pNewFileList->FileSize = FileInfo.st_size;
            pNewFileList->pNext = NULL;
            
            pCurFileList->pNext = pNewFileList;
            pCurFileList = pNewFileList;

            pHeadNode->FileSize++;
        }
        else
        {
            free(pFileName);
            continue;
        }
    }

    if(NULL == pHeadNode->pNext)
    {
        FreeFileList(pHeadNode);
        pHeadNode = NULL;
    }

    closedir(pDir);
    return pHeadNode;
}

static FileList_t *ScanEncryptFile(char *pFolderName)
{
    DIR *pDir;
    pDir = opendir(pFolderName);
    if(NULL == pDir)
        return NULL;
    
    FileList_t *pHeadNode = (FileList_t *)malloc(sizeof(FileList_t));
    if(NULL == pHeadNode)
    {
        closedir(pDir);
        return NULL;
    }
    
    InitFileListNode(pHeadNode);
    
    FileList_t *pCurFileList = pHeadNode, *pNewFileList;
    struct dirent *pEntry;
    int FolderNameLenght = strlen(pFolderName);
    int FileNameLenght;
    char *pFileName;
#ifdef __LINUX
    struct stat FileInfo;
#elif defined __WINDOWS
    struct _stati64 FileInfo;
#endif

    while(1)
    {
        pEntry = readdir(pDir);
        if(NULL == pEntry)
            break;
            
        //Ignore ".", "..", "...", ".ept" or "..ept" or "...ept"
        if('.' == pEntry->d_name[0])
        {
            if('\0' == pEntry->d_name[1])
                continue;
            else if('.' == pEntry->d_name[1])
            {
                if('\0' == pEntry->d_name[2])
                    continue;
                else if('.' == pEntry->d_name[2])
                {
                    if('\0' == pEntry->d_name[3])
                        continue;
                    else if(('e' == pEntry->d_name[3]) && ('p' == pEntry->d_name[4]) && 
                            ('t' == pEntry->d_name[5]) && ('\0' == pEntry->d_name[6]))
                        continue;
                }
                else if(('e' == pEntry->d_name[2]) && ('p' == pEntry->d_name[3]) && 
                        ('t' == pEntry->d_name[4]) && ('\0' == pEntry->d_name[5]))
                    continue;
            }
            else if(('e' == pEntry->d_name[1]) && ('p' == pEntry->d_name[2]) && 
                    ('t' == pEntry->d_name[3]) && ('\0' == pEntry->d_name[4]))
                continue;

        }
        
        FileNameLenght = FolderNameLenght + strlen(pEntry->d_name) + 2; //contains '/' or '\' and '\0'
        pFileName = (char *)malloc(FileNameLenght);
        if(NULL == pFileName)
        {
            FreeFileList(pHeadNode);
            closedir(pDir);
            return NULL;
        }

        snprintf(pFileName, FileNameLenght, "%s/%s", pFolderName, pEntry->d_name);
        
#ifdef __LINUX
        if(0 != lstat(pFileName, &FileInfo))
#elif defined __WINDOWS
        if(0 != _stati64(pFileName, &FileInfo))
#endif
        {
            free(pFileName);
            continue;
        }
        
        if((DT_DIR == pEntry->d_type) && (FileInfo.st_mode & S_IFDIR))
        {
            pNewFileList = ScanEncryptFile(pFileName);
            if(NULL == pNewFileList)
            {
                free(pFileName);
                continue;
            }
            
            pHeadNode->FileSize += pNewFileList->FileSize;
            pCurFileList->pNext = pNewFileList->pNext;
            pCurFileList = pNewFileList;

            while(pCurFileList->pNext != NULL)
            {
                pCurFileList = pCurFileList->pNext;
            }
            
            if(pNewFileList->pFileName != NULL)
                free(pNewFileList->pFileName);
            free(pNewFileList);
        }
        else if((DT_REG == pEntry->d_type) && (FileInfo.st_mode & S_IFREG))
        {
            if(STAT_OK != IsEncryptFile(pEntry->d_name, FileNameLenght-FolderNameLenght-2))
                continue;
            
            pNewFileList = (FileList_t *)malloc(sizeof(FileList_t));
            if(NULL == pNewFileList)
            {
                free(pFileName);
                FreeFileList(pHeadNode);
                closedir(pDir);
                return NULL;
            }
            
            pNewFileList->pFileName = pFileName;
            pNewFileList->FileNameLenght = FileNameLenght;
            pNewFileList->FileSize = FileInfo.st_size;
            pNewFileList->pNext = NULL;
            
            pCurFileList->pNext = pNewFileList;
            pCurFileList = pNewFileList;

            pHeadNode->FileSize++;
        }
        else
        {
            free(pFileName);
            continue;
        }
    }

    if(NULL == pHeadNode->pNext)
    {
        FreeFileList(pHeadNode);
        pHeadNode = NULL;
    }

    closedir(pDir);
    return pHeadNode;
}

void DispFileList(FileList_t *pHeadNode)
{
    FileList_t *pFileList = pHeadNode->pNext;

    while(pFileList != NULL)
    {
        printf("%s\n", pFileList->pFileName);
        pFileList = pFileList->pNext;
    }
}



//Log related
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static G_STATUS AfterEncryptDecrypt(PROCESS_STATUS ProcessStatus)
{
    if(PROCESS_STATUS_ELSE_ERR == ProcessStatus)
        return STAT_ERR;
    
    WINDOW *win = newwin(CTL_RESULT_WIN_LINES, CTL_RESULT_WIN_COLS, 
            (LINES-CTL_RESULT_WIN_LINES)/2, (COLS-CTL_RESULT_WIN_COLS)/2);
    int key;
    char buf[COLS - 4];

    CTL_SHOW_CONSOLE_END_LINE();
    keypad(win, true);
    
    if(PROCESS_STATUS_SUCCESS == ProcessStatus)
    {
        CTL_SET_COLOR(win, CTL_PANEL_GREEN);
        wattron(win, A_BOLD);
        snprintf(buf, sizeof(buf), "Success, in total:%d", g_SuccessFailCountTable[0]);
        mvwaddstr(win, CTL_RESULT_WIN_LINES/3, 
            (CTL_RESULT_WIN_COLS-strlen(buf))/2, buf);
        wattroff(win, A_BOLD);
        
        CTL_SET_COLOR(win, CTL_PANEL_GREEN);
        wborder(win, '*', '*', '*', '*', '*', '*', '*', '*');
        mvwaddstr(win, CTL_RESULT_WIN_LINES*2/3, 
            (CTL_RESULT_WIN_COLS-sizeof(STR_GO_BACK)+1)/2, STR_GO_BACK);
        wrefresh(win);
        
        while(1)
        {
            key = wgetch(win);
            if(13 == key) //Enter key
                break;
            else if(27 == key) //Esc key
            {
                delwin(win);
                return STAT_EXIT;
            }
            else
                continue;
        }
        
        delwin(win);
        touchline(stdscr, (LINES-CTL_RESULT_WIN_LINES)/2, CTL_RESULT_WIN_LINES);
        refresh();

        return STAT_OK;
    }

    snprintf(buf, sizeof(buf), "Success:%d   Fail:%d", g_SuccessFailCountTable[0], 
        g_SuccessFailCountTable[1]);
    CTL_SET_COLOR(win, CTL_PANEL_GREEN);
    wattron(win, A_BOLD);
    mvwprintw(win, CTL_RESULT_WIN_LINES/3, (CTL_RESULT_WIN_COLS-strlen(buf))/2, 
        "Success:%d   ", g_SuccessFailCountTable[0]);
    CTL_SET_COLOR(win, CTL_PANEL_RED);
    wprintw(win, "Fail:%d", g_SuccessFailCountTable[1]);
    wattroff(win, A_BOLD);
        
    CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
    wborder(win, '*', '*', '*', '*', '*', '*', '*', '*');
    
    int Str1StartX = (CTL_RESULT_WIN_COLS - sizeof(STR_VIEW_LOG)+1 - sizeof(STR_GO_BACK)+1)/3;
    int Str2StartX = CTL_RESULT_WIN_COLS - Str1StartX - sizeof(STR_GO_BACK)+1;
    char flag = 0;

    mvwaddstr(win, 4, Str2StartX, STR_GO_BACK);
    wattron(win, A_REVERSE);
    mvwaddstr(win, 4, Str1StartX, STR_VIEW_LOG);
    wattroff(win, A_REVERSE);
    
    while(1)
    {
        key = wgetch(win);
        if((KEY_LEFT == key) || (KEY_RIGHT == key))
        {
            flag ^= 1;
        }        
        else if(27 == key) //Esc key
        {            
            delwin(win);
            return STAT_EXIT;
        }
        else if(13 == key) //Enter key
            break;
        else
            continue;

        if(flag)
        {
            mvwaddstr(win, 4, Str1StartX, STR_VIEW_LOG);
            wattron(win, A_REVERSE);
            mvwaddstr(win, 4, Str2StartX, STR_GO_BACK);
            wattroff(win, A_REVERSE);
        }
        else
        {
            mvwaddstr(win, 4, Str2StartX, STR_GO_BACK);
            wattron(win, A_REVERSE);
            mvwaddstr(win, 4, Str1StartX, STR_VIEW_LOG);
            wattroff(win, A_REVERSE);
        }
    }
    
    delwin(win);
    touchline(stdscr, (LINES-CTL_RESULT_WIN_LINES)/2, CTL_RESULT_WIN_LINES);
    CTL_HIDE_CONSOLE_END_LINE();

    if(flag)
        return STAT_GO_BACK;

    G_STATUS status;
    log_t log;
    
    status = ParseLog(&log);
    if(STAT_OK != status)
        return status;

    WINDOW *WinLabel = newwin(1, COLS, LINES-1, 0);
    
    keypad(WinLabel, true);
    CTL_SET_COLOR(WinLabel, CTL_PANEL_CYAN);
    wattron(WinLabel, A_REVERSE);
    mvwaddstr(WinLabel, 0, 0, STR_RESULT_WIN_LABEL);
    wrefresh(WinLabel);

    win = newwin(LINES-1, COLS, 0, 0);
    log_t *pCurNode = log.pNext;
    char BottomFlag = 0;            //1 means it has been in bottom position, i.e can't to page down again
    int PageCount = 1;              //PageCount must be initialize as 1
    int CurPage = 0, LogCount = 0;
    int i, LineCount;

    if(NULL == pCurNode)
    {
        BottomFlag = 1;
        CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
        mvwaddstr(win, (LINES-1)/2, (COLS-sizeof(STR_LOG_IS_NULL)+1)/2, STR_LOG_IS_NULL);
        wrefresh(win);
        PageCount = 0;
    }
    else
    {
        LineCount = 0;
        while(NULL != pCurNode->pNext)
        {
            LineCount += pCurNode->lines;
            if((LineCount + pCurNode->pNext->lines) > LINES-1)
            {
                PageCount++;
                LineCount = 0;
            }
            pCurNode = pCurNode->pNext;
        }
    
        pCurNode = log.pNext;
    
        wmove(win, 0, 0);
        for(i = 0, LineCount = 0; i < (LINES-1)/2; i++)
        {
            CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
            if(pCurNode->flag)
            {
                wprintw(win, "%s", pCurNode->pEntry);
            }
            else
            {
                wprintw(win, "%s\n", pCurNode->pEntry);
            }
            CTL_SET_COLOR(win, CTL_PANEL_RED);
            wprintw(win, "%s\n", pCurNode->pDetail);
            
            if(NULL == pCurNode->pNext) //It means only one page at the first time of display
            {
                BottomFlag = 1;     
                break;
            }

            LineCount += pCurNode->lines;
            pCurNode = pCurNode->pNext;
            if((LineCount + pCurNode->lines) > (LINES-1))
                break;
        }
        wrefresh(win);
        mvwprintw(WinLabel, 0, sizeof(STR_RESULT_WIN_LABEL)-1, "[%d/%d] ", PageCount, CurPage+1);
    }

    while(1)
    {
        key = wgetch(WinLabel);
        
        if(KEY_NPAGE == key)
        {
            if(BottomFlag)
                continue;
                
            wclear(win);
            wmove(win, 0, 0);
            for(i = 0, LineCount = 0; i < (LINES-1)/2; i++)
            {
                CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
                if(pCurNode->flag)
                {
                    wprintw(win, "%s", pCurNode->pEntry);
                }
                else
                {
                    wprintw(win, "%s\n", pCurNode->pEntry);
                }
                CTL_SET_COLOR(win, CTL_PANEL_RED);
                wprintw(win, "%s\n", pCurNode->pDetail);

                if(NULL == pCurNode->pNext)
                {
                    BottomFlag = 1;
                    break;
                }

                LineCount += pCurNode->lines;
                pCurNode = pCurNode->pNext;
                if((LineCount + pCurNode->lines) > (LINES-1))
                {
                    i++;
                    break;
                }
            }
            wrefresh(win);

            LogCount = i;
            CurPage++;

            mvwprintw(WinLabel, 0, sizeof(STR_RESULT_WIN_LABEL)-1, "[%d/%d] ", PageCount, CurPage+1);            
        }
        else if(KEY_PPAGE == key)
        {
            if(CurPage <= 0)
                continue;

            for(i = LogCount; i > 0; i--)
            {
                pCurNode = pCurNode->pPrevious;
            }
                
            for(i = 0, LineCount = 0; i < (LINES-1)/2; i++)
            {
                pCurNode = pCurNode->pPrevious;
                LineCount += pCurNode->lines;
                
                if(&log == pCurNode->pPrevious)
                    break;
                if((LineCount + pCurNode->pPrevious->lines) > (LINES-1))
                    break;
            }

            BottomFlag = 0;
            CurPage--;

            wclear(win);
            wmove(win, 0, 0);
            for(i = 0, LineCount = 0; i < (LINES-1)/2; i++)
            {
                CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
                if(pCurNode->flag)
                {
                    wprintw(win, "%s", pCurNode->pEntry);
                }
                else
                {
                    wprintw(win, "%s\n", pCurNode->pEntry);
                }
                CTL_SET_COLOR(win, CTL_PANEL_RED);
                wprintw(win, "%s\n", pCurNode->pDetail);
                
                LineCount += pCurNode->lines;
                pCurNode = pCurNode->pNext;
                if((LineCount + pCurNode->lines) > (LINES-1))
                {
                    i++;
                    break;
                }
            }
            wrefresh(win);
            LogCount = i;
            
            mvwprintw(WinLabel, 0, sizeof(STR_RESULT_WIN_LABEL)-1, "[%d/%d] ", PageCount, CurPage+1);
        }
        else if(27 == key) //Esc key
        {
            delwin(WinLabel);
            delwin(win);
            return STAT_EXIT;
        }
        else if(13 == key) //Enter key
            break;
        else
            continue;
    }

    FreeLog(&log);
    delwin(WinLabel);
    delwin(win);
    touchwin(stdscr);
    refresh();
    
    return STAT_OK;
}

static G_STATUS ParseLog(log_t *pHeadNode)
{
#ifdef __DEBUG    
    if(NULL == pHeadNode)
    {
        DISP_ERR(STR_ERR_INVALID_LOG_FILE);
        return STAT_ERR;
    }
#endif
    
    long CurLogPostion = ftell(g_pDispFile);
    rewind(g_pDispFile);

    log_t *pCurNode = pHeadNode, *pNewNode;
    pHeadNode->pEntry = NULL;
    pHeadNode->pDetail = NULL;
    pHeadNode->lines = 0;
    pHeadNode->flag = 0;
    pHeadNode->pNext = NULL;
    pHeadNode->pPrevious = NULL;
    
    char buf[CYT_FILE_NAME_LENGHT*2];
    char *pTmp;
    int len;

    while((NULL != fgets(buf, sizeof(buf), g_pDispFile)) && (0 == feof(g_pDispFile)))
    {
        pNewNode = (log_t *)malloc(sizeof(log_t));
        if(NULL == pNewNode)
        {
            DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
            return STAT_ERR;
        }

        len = strlen(buf);
        buf[len-1] = '\0'; //buf[len-1] is '\n' before
        len--;
        if(COLS == len)
        {
            pNewNode->flag = 1;
            pNewNode->lines = 1;
        }
        else
        {
            pNewNode->flag = 0;
            pNewNode->lines = len/COLS + 1;
        }
            
        pTmp = (char *)malloc(len+1);
        if(NULL == pTmp)
        {
            DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
            return STAT_ERR;
        }

        memcpy(pTmp, buf, len+1); //len+1: end with '\0'
        pNewNode->pEntry = pTmp;

        if((NULL != fgets(buf, sizeof(buf), g_pDispFile)) && (0 == feof(g_pDispFile)))
        {
            len = strlen(buf);
            buf[len-1] = '\0'; //buf[len-1] is '\n' before
            buf[COLS-1] = '\0'; //Make sure content of buf occupy only one line
            pNewNode->lines++;
                
            pTmp = (char *)malloc(len);
            if(NULL == pTmp)
            {
                DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
                return STAT_ERR;
            }

            memcpy(pTmp, buf, len);
            pNewNode->pDetail = pTmp;
        }
        else
        {
            DISP_ERR(STR_ERR_INVALID_LOG_FILE);
            return STAT_ERR;
        }
        
        pNewNode->pNext = NULL;
        pNewNode->pPrevious = pCurNode;
        
        pCurNode->pNext = pNewNode;
        pCurNode = pNewNode;
    }

    fseek(g_pDispFile, CurLogPostion, SEEK_SET);
    return STAT_OK;
}

void DispLog(log_t *pHeadNode)
{
    log_t *CurNode = pHeadNode->pNext;

    while(NULL != CurNode)
    {
        fprintf(stdout, "%s\n", CurNode->pEntry);
        fprintf(stdout, "%s\n", CurNode->pDetail);
        CurNode = CurNode->pNext;
    }
}



//Static inline functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static inline void DeleteTailSymbol(char *pFileName, int FileNameLenght)
{
    if(('/' == pFileName[FileNameLenght-1]) || (92 == pFileName[FileNameLenght-1])) //'\' ASCII is 92
    {
        pFileName[FileNameLenght-1] = '\0';
    }
}

static inline void InitFileListNode(FileList_t *pFileList)
{
    pFileList->pFileName = NULL;
    pFileList->FileNameLenght = 0;
    pFileList->FileSize = 0;
    pFileList->pNext = NULL;
}

static inline void FreeFileList(FileList_t *pHeadNode)
{
    FileList_t *CurNode = pHeadNode;
    FileList_t *TmpNode;
    while(CurNode != NULL)
    {
        TmpNode = CurNode->pNext;
        if(CurNode->pFileName != NULL)
            free(CurNode->pFileName);
        free(CurNode);
        CurNode = TmpNode;
    }
}

static inline G_STATUS IsEncryptFile(const char *pFileName, int FileNameLenght)
{
    if(('t' == pFileName[FileNameLenght-1]) && ('p' == pFileName[FileNameLenght-2]) && 
       ('e' == pFileName[FileNameLenght-3]) && ('.' == pFileName[FileNameLenght-4]))
       return STAT_OK;

    return STAT_ERR;
}

static inline void FreeLog(log_t *pHeadNode)
{
    log_t *CurNode = pHeadNode->pNext;
    log_t *TmpNode;
    while(CurNode != NULL)
    {
        TmpNode = CurNode->pNext;
        if(NULL != CurNode->pEntry)
            free(CurNode->pEntry);
        if(NULL != CurNode->pDetail)
            free(CurNode->pDetail);
        free(CurNode);
        CurNode = TmpNode;
    }
}

