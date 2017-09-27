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
//static G_STATUS AfterEncryptDecrypt(PROCESS_STATUS ProcessStatus);
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



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
G_STATUS CTL_MENU_EncryptDecrypt(CTL_MENU func)
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

    PROCESS_STATUS ProcessStatus;

    if(S_IFREG & FileInfo.st_mode)
    {
        FileList_t HeadNode, FileList;

        HeadNode.FileName = NULL;
        HeadNode.FileNameLenght = 0;
        HeadNode.FileSize = 1;
        HeadNode.pNext = &FileList;
        
        FileList.FileName = FileName;
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

        ProcessStatus = EncryptDecrypt(pFileList, func);

        FreeFileList(pFileList);
    }

    AfterEncryptDecrypt(ProcessStatus);

    return STAT_OK;
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
                            wprintw(win[i], "%s", PthreadArg[i].pCurFileList->FileName);
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
                            wprintw(win[i], "%s", PthreadArg[i].pCurFileList->FileName);
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
            
            if(pNewFileList->FileName != NULL)
                free(pNewFileList->FileName);
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
            
            pNewFileList->FileName = pFileName;
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
        
        FileNameLenght = strlen(pEntry->d_name);
        if(STAT_OK != IsEncryptFile(pEntry->d_name, FileNameLenght))
            continue;

        FileNameLenght += FolderNameLenght + 2; //contains '/' or '\' and '\0'
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
            
            if(pNewFileList->FileName != NULL)
                free(pNewFileList->FileName);
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
            
            pNewFileList->FileName = pFileName;
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
        printf("%s\n", pFileList->FileName);
        pFileList = pFileList->pNext;
    }
}



//Log related
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
G_STATUS AfterEncryptDecrypt(PROCESS_STATUS ProcessStatus)
{
    WINDOW *win = newwin(CTL_RESULT_WIN_LINES, CTL_RESULT_WIN_COLS, 
            (LINES-CTL_RESULT_WIN_LINES)/2, (COLS-CTL_RESULT_WIN_COLS)/2);
    int key;

    CTL_SHOW_CONSOLE_END_LINE();
    keypad(win, true);
    
    if(PROCESS_STATUS_SUCCESS == ProcessStatus)
    {
        CTL_SET_COLOR(win, CTL_PANEL_GREEN);
        wattron(win, A_BOLD);
        mvwaddstr(win, CTL_RESULT_WIN_LINES/3, 
            (CTL_RESULT_WIN_COLS-sizeof(STR_SUCCESS)+1)/2, STR_SUCCESS);
        wattroff(win, A_BOLD);
        
        CTL_SET_COLOR(win, CTL_PANEL_CYAN);
        wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
        mvwaddstr(win, CTL_RESULT_WIN_LINES/3, 
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
    
    CTL_SET_COLOR(win, CTL_PANEL_RED);
    wattron(win, A_BOLD);
    mvwaddstr(win, CTL_RESULT_WIN_LINES/3, (CTL_RESULT_WIN_COLS-sizeof(STR_FAIL)+1)/2, STR_FAIL);
    wattroff(win, A_BOLD);
        
    CTL_SET_COLOR(win, CTL_PANEL_CYAN);
    wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
    
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

    if(flag)
        return STAT_GO_BACK;

    CTL_HIDE_CONSOLE_END_LINE();

    G_STATUS status;
    log_t log;
    
    status = ParseLog(&log);
    if(STAT_OK != status)
        return status;

    WINDOW *WinLabel = newwin(1, COLS, LINES-1, 0);
    CTL_SET_COLOR(WinLabel, CTL_PANEL_CYAN);
    wattron(WinLabel, A_REVERSE);
    mvwaddstr(win, 0, 0, STR_RESULT_WIN_LABEL);
    wattroff(WinLabel, A_REVERSE);
    wrefresh(WinLabel);
    
    win = newwin(LINES-1, COLS, 0, 0);
    int i;
    log_t *pCurLog = log.pNext;
    char TopFlag = 0; //1 means it has been in top position, i.e can't to page up again

    //scrollok(win, true);
    wmove(win, 0, 0);

    while(1)
    {
        key = wgetch(win);
        if(KEY_NPAGE == key)
        {
            for(i = 0; i < LINES-1; i++)
            {
                if(NULL == pCurLog)
                    break;
    
                CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
                wprintw(win, "%s\n", pCurLog->pFileName);
                CTL_SET_COLOR(win, CTL_PANEL_RED);
                wprintw(win, "%s\n", pCurLog->pReason);
                
                if(NULL == pCurLog->pNext)
                    break;
                    
                pCurLog = pCurLog->pNext; 
            }
            
            TopFlag = 0;
        }
        else if(KEY_PPAGE == key)
        {
            if(TopFlag)
                continue;
            
            for(i = 0; i < LINES-1; i++)
            {
                if(NULL == pCurLog)
                {
                    break;
                }
                
                if(NULL == pCurLog->pPrevious)
                    break;
                
                pCurLog = pCurLog->pPrevious;
            }

            if(pCurLog == &log)
                TopFlag = 1;

            wclear(win);
            
            for(i = 0; i < LINES-1; i++)
            {
                if(NULL == pCurLog)
                    break;
    
                CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
                wprintw(win, "%s\n", pCurLog->pFileName);
                CTL_SET_COLOR(win, CTL_PANEL_RED);
                wprintw(win, "%s\n", pCurLog->pReason);
                
                if(NULL == pCurLog->pNext)
                    break;
                    
                pCurLog = pCurLog->pNext; 
            }
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
    touchwin(win);
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
    pHeadNode->pFileName = NULL;
    pHeadNode->pReason = NULL;
    pHeadNode->pNext = NULL;
    pHeadNode->pPrevious = NULL;
    
    char buf[CYT_FILE_NAME_LENGHT+BUF_SIZE];
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
        pTmp = (char *)malloc(len);
        if(NULL == pTmp)
        {
            DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
            return STAT_ERR;
        }

        memcpy(pTmp, buf, len);
        pNewNode->pFileName = pTmp;

        if((NULL != fgets(buf, sizeof(buf), g_pDispFile)) && (0 == feof(g_pDispFile)))
        {
            len = strlen(buf);
            buf[len-1] = '\0'; //buf[len-1] is '\n' before
            pTmp = (char *)malloc(len);
            if(NULL == pTmp)
            {
                DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
                return STAT_ERR;
            }

            memcpy(pTmp, buf, len);
            pNewNode->pReason = pTmp;
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
        fprintf(stdout, "%s\n", CurNode->pFileName);
        fprintf(stdout, "%s\n", CurNode->pReason);
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
    pFileList->FileName = NULL;
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
        if(CurNode->FileName != NULL)
            free(CurNode->FileName);
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
        if(NULL != CurNode->pFileName)
            free(CurNode->pFileName);
        if(NULL != CurNode->pReason)
            free(CurNode->pReason);
        free(CurNode);
        CurNode = TmpNode;
    }
}

