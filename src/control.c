/*************************************************************************
	> File Name: control.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 9th,2017 Wednesday 08:49:11
 ************************************************************************/

#include "control.h"
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#ifdef __LINUX
#include <locale.h>
#include <signal.h>
#endif
#include <sys/stat.h>

static int CountLines(char *pBuf, int cols);
static G_STATUS MoveString(char *pHead, char *pTail);
static inline void FreeFileContent(FileContent_t *pHeadNode);
static inline G_STATUS ReplaceChar(char *ptr, char SrcChar, char TarChar);



/*
 *  Briefs: Use for initializing the standard screen
 *  Return: None
 *  Note:   1. Must be invoked before any operations
 *          2. Must invoke CTL_***Exit related function at the end of project
 */
void CTL_InitConsole(void)
{
    InitStr(); //It must be initialized firstly

#ifdef __LINUX
    setlocale(LC_ALL, "");
    //signal(SIGINT, SIG_IGN);
#endif
    
    initscr();
    cbreak();
    nonl();
    noecho();
    keypad(stdscr, true);
    
    if(COLS < CTL_CONSOLE_COLS)
    {
        CTL_ErrExit("%s%d\n%s%d\n", STR_EN_ERR_INVALID_COLS, CTL_CONSOLE_COLS, 
            STR_CH_ERR_INVALID_COLS, CTL_CONSOLE_COLS);
    }
    
    if(LINES < CTL_CONSOLE_LINES)
    {
        CTL_ErrExit("%s%d\n%s%d\n", STR_EN_ERR_INVALID_LINES, CTL_CONSOLE_LINES, 
            STR_CH_ERR_INVALID_LINES, CTL_CONSOLE_LINES);
    }
    
    if(OK == start_color())
    {
        init_pair(CTL_PANEL_CYAN, COLOR_CYAN, COLOR_BLACK);
        init_pair(CTL_PANEL_RED, COLOR_RED, COLOR_BLACK);
        init_pair(CTL_PANEL_YELLOW, COLOR_YELLOW, COLOR_BLACK);
        init_pair(CTL_PANEL_GREEN, COLOR_GREEN, COLOR_BLACK);
        init_pair(CTL_PANEL_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    }
    
    CTL_SET_COLOR(stdscr, CTL_PANEL_CYAN);
}

/*
 *  Briefs: Draw the standard screen
 *  Return: None
 *  Note:   None
 */
void CTL_DrawStdConsole(void)
{
    clear();
    border('|', '|', '-', '-', '+', '+', '+', '+');
    
    attron(A_REVERSE);
    mvaddstr(0, (COLS-GetWidth(STR_CONSOLE_LABEL))/2, STR_CONSOLE_LABEL);
    mvaddstr(LINES-2, 2, STR_CONSOLE_END_LINE);

    int i;
    for(i = 0; i < (COLS-GetWidth(STR_CONSOLE_END_LINE)-4); i++)
    {
        addch(' ');
    }
    attroff(A_REVERSE);
    refresh();
}

/*
 *  Briefs: Choose language
 *  Return: None
 *  Note:   None
 */
void CTL_ChooseLanguage(void)
{
    WINDOW *win = newwin(CTL_CHOOSE_LANGUAGE_WIN_LINES, CTL_CHOOSE_LANGUAGE_WIN_COLS, 
        (LINES-CTL_CHOOSE_LANGUAGE_WIN_LINES)/2, (COLS-CTL_CHOOSE_LANGUAGE_WIN_COLS)/2);
        
    CTL_SET_COLOR(win, CTL_PANEL_CYAN);
    wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
    
    int Str1StartX = (CTL_CHOOSE_LANGUAGE_WIN_COLS - GetWidth(STR_EN_LANGUAGE))/2 + 1;
    int Str2StartX = (CTL_CHOOSE_LANGUAGE_WIN_COLS - GetWidth(STR_CH_LANGUAGE))/2;
    
    mvwaddstr(win, 2, Str2StartX, STR_CH_LANGUAGE);
    wattron(win, A_REVERSE);
    mvwaddstr(win, 1, Str1StartX, STR_EN_LANGUAGE);
    wattroff(win, A_REVERSE);
    
    char flag = 0;
    int key;
    
    keypad(win, true); 
    while(1)
    {
        key = wgetch(win);
        switch(key)
        {
            case CTL_KEY_DOWN:
            case CTL_KEY_UP:
            case CTL_KEY_TAB:
                flag ^= 1;
                break;
            case CTL_KEY_ENTER:
                goto while_out;
            default :
                continue;
        }
        
        if(flag)
        {
            mvwaddstr(win, 1, Str1StartX, STR_EN_LANGUAGE);
            wattron(win, A_REVERSE);
            mvwaddstr(win, 2, Str2StartX, STR_CH_LANGUAGE);
            wattroff(win, A_REVERSE);
        }
        else
        {
            mvwaddstr(win, 2, Str2StartX, STR_CH_LANGUAGE);
            wattron(win, A_REVERSE);
            mvwaddstr(win, 1, Str1StartX, STR_EN_LANGUAGE);
            wattroff(win, A_REVERSE);
        }
    }

while_out :
    
    g_LanguageFlag = (flag) ? LAN_CH : LAN_EN;

    delwin(win);
    touchline(stdscr, (LINES-CTL_CHOOSE_LANGUAGE_WIN_LINES)/2, 
        CTL_CHOOSE_LANGUAGE_WIN_LINES);
    refresh();
}

/*
 *  Briefs: Show menu
 *  Return: None
 *  Note:   None
 */
void CTL_ShowMenu(CTL_MENU *pFunc)
{
    WINDOW *win = newwin(CTL_MENU_WIN_LINES, CTL_MENU_WIN_COLS, 
        (LINES-CTL_MENU_WIN_LINES)/2, (COLS-CTL_MENU_WIN_COLS)/2);

    CTL_SET_COLOR(win, CTL_PANEL_CYAN);
    wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
    wattron(win, A_REVERSE);
    mvwaddstr(win, 0, (CTL_MENU_WIN_COLS-GetWidth(STR_MENU))/2, STR_MENU);
    wattroff(win, A_REVERSE);

    int CurPosY = 2;
    char **menu = (g_LanguageFlag == LAN_EN) ? g_EnMenu : g_ChMenu;
    
    while(NULL != menu[CurPosY-1])
    {
        mvwaddstr(win, CurPosY, 2, menu[CurPosY-1]);
        CurPosY++;
    }

    CurPosY = 1;
    wattron(win, A_REVERSE);
    mvwaddstr(win, CurPosY, 2, menu[0]);
    wattroff(win, A_REVERSE);
    wrefresh(win);
    
    int key;
    
    keypad(win, true);
    while(1)
    {
        key = wgetch(win);
        switch(key)
        {
            case CTL_KEY_DOWN:
            case CTL_KEY_TAB:
                mvwaddstr(win, CurPosY, 2, menu[CurPosY-1]);
                CurPosY++;
                break;
            case CTL_KEY_UP:
                mvwaddstr(win, CurPosY, 2, menu[CurPosY-1]);
                CurPosY--;
                break;
            case CTL_KEY_ESC:
                CTL_ForceExit();
                break;
            case CTL_KEY_ENTER:
                goto while_out;
            default :
                continue;
        }

        if(1 > CurPosY)
        {
            CurPosY = CTL_MENU_MAX;
        }
        else if(CTL_MENU_MAX <= (CurPosY-1))
        {
            CurPosY = 1;
        }

        wattron(win, A_REVERSE);
        mvwaddstr(win, CurPosY, 2, menu[CurPosY-1]);
        wattroff(win, A_REVERSE);
    }

while_out :

    *pFunc = CurPosY-1;
    delwin(win);
    touchline(stdscr, (LINES-CTL_MENU_WIN_LINES)/2, CTL_MENU_WIN_LINES);
    refresh();
}

/*  
 *  Briefs: Show instruction
 *  Return: None
 *  Note:   None
 */
void CTL_ShowInstruction(void)
{
    WINDOW *win = newwin(LINES, COLS, 0, 0);
    char **ptr = (g_LanguageFlag == LAN_EN) ? EnInstruction : ChInstruction;

    CTL_SET_COLOR(win, CTL_PANEL_GREEN);
    wattron(win, A_REVERSE);
    //mvwaddstr(win, 0, (COLS - strlen(*ptr))/2, *ptr++); //MinGW doesn't support
    mvwaddstr(win, 0, (COLS - GetWidth(*ptr))/2, *ptr);
    ptr++;
    wattroff(win, A_REVERSE);

    wmove(win, 1, 0);
    while(NULL != *ptr)
    {
        wprintw(win, "%s\n", *ptr++);
    }

    wattron(win, A_REVERSE);
    mvwaddstr(win, LINES-1, 0, STR_PRESS_ENTER_TO_GO_BACK);
    wattroff(win, A_REVERSE);

    int key;
    keypad(win, true);
    while(1)
    {
        key = wgetch(win);
        switch(key)
        {
            case CTL_KEY_ENTER:
                goto while_out;
            default :
                continue;
        }
    }
    
while_out :    

    delwin(win);
    touchwin(stdscr);
    refresh();
}


/*  
 *  Briefs: Show file
 *  Return: Always STAT_OK
 *  Note:   Only support UTF-8
 */
G_STATUS CTL_ShowFile(const char *pFileName)
{
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //Parse file
    if(0 != access(pFileName, F_OK))
    {
        CTL_DispWarning("%s: %s\n", STR_FILE_NOT_EXIST, pFileName);
        return STAT_OK;
    }

#ifdef __LINUX
    struct stat FileInfo;
#elif defined __WINDOWS
    struct _stati64 FileInfo;
#endif

#ifdef __LINUX
    if(0 != lstat(pFileName, &FileInfo))
#elif defined __WINDOWS
    if(0 != _stati64(pFileName, &FileInfo))
#endif
    {
        CTL_DispWarning("%s: %s\n", STR_FAIL_TO_GET_FILE_INFO, pFileName);
        return STAT_OK;
    }
    
    if(!(S_IFREG & FileInfo.st_mode))
    {
        CTL_DispWarning("%s: %s\n", STR_NOT_A_FILE, pFileName);
        return STAT_OK;
    }
    
    if(CTL_FILE_SIZE_MAX < FileInfo.st_size)
    {
        return STAT_OK;
    }
    
    FILE *fp = fopen(pFileName, "rb");
    if(NULL == fp)
    {
        CTL_DispWarning("%s: %s\n", STR_FAIL_TO_OPEN_FILE, pFileName);
        return STAT_OK;
    }
    
    char *buf = (char *)malloc(CTL_SINGLE_LINE_WIDTH);
    if(NULL == buf)
    {
        fclose(fp);
        CTL_DispWarning(STR_ERR_FAIL_TO_MALLOC);
        return STAT_OK;
    }
    
    FileContent_t HeadNode;
    HeadNode.pContent = NULL;
    HeadNode.pPrev = NULL;
    HeadNode.pNext = NULL;
    
    FileContent_t *pTmpNode = &HeadNode;
    FileContent_t *pNewNode;

    while(NULL != fgets(buf, CTL_SINGLE_LINE_WIDTH, fp))
    {
        pNewNode = (FileContent_t *)malloc(sizeof(FileContent_t));
        if(NULL == pNewNode)
        {
            fclose(fp);
            free(buf);
            FreeFileContent(&HeadNode);
            CTL_DispWarning(STR_ERR_FAIL_TO_MALLOC);
            return STAT_OK;
        }
        
        ReplaceChar(buf, '\n', '\0');
        pNewNode->ContentWidth = GetWidth(buf);
        if(0 == pNewNode->ContentWidth)
        {
            pNewNode->ContentWidth = 1;
        }
        
        if(0 == (pNewNode->ContentWidth % COLS))
        {
            pNewNode->flag = 1;
            pNewNode->ContentLines = pNewNode->ContentWidth / COLS;
        }
        else
        {
            pNewNode->flag = 0;
            pNewNode->ContentLines = pNewNode->ContentWidth / COLS + 1;
        }

        pNewNode->pContent = buf;
        pNewNode->pPrev = pTmpNode;
        pNewNode->pNext = NULL;

        pTmpNode->pNext = pNewNode;
        pTmpNode = pNewNode;
        
        if(0 != feof(fp)) //End of file
            break;
        
        buf = (char *)malloc(CTL_SINGLE_LINE_WIDTH);
        if(NULL == buf)
        {
            fclose(fp);
            FreeFileContent(&HeadNode);
            CTL_DispWarning(STR_ERR_FAIL_TO_MALLOC);
            return STAT_OK;
        }
    }

    fclose(fp);
    fp = NULL;



    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //Show file
    WINDOW *WinLabel = newwin(1, COLS, LINES-1, 0);
    CTL_SET_COLOR(WinLabel, CTL_PANEL_CYAN);
    wattron(WinLabel, A_REVERSE);
    mvwaddstr(WinLabel, 0, 0, STR_SHOW_FILE_LABEL);
    wrefresh(WinLabel);

    WINDOW *win = newwin(LINES-1, COLS, 0, 0);
    win = newwin(LINES-1, COLS, 0, 0);
    FileContent_t *pCurNode = HeadNode.pNext;
    char BottomFlag = 0;            //1 means it has been in bottom position, i.e can't page down again
    int PageCount = 1;              //PageCount must be initialize as 1
    int CurPage = 0, LogCount = 0;
    int i, LineCount;
    int key;
    int LabelWidth = GetWidth(STR_SHOW_FILE_LABEL);

    if(NULL == pCurNode)
    {
        BottomFlag = 1;
        CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
        mvwaddstr(win, (LINES-1)/2, (COLS-GetWidth(STR_LOG_IS_NULL))/2, STR_LOG_IS_NULL);
        wrefresh(win);
        PageCount = 0;
    }
    else
    {
        LineCount = 0;
        while(NULL != pCurNode->pNext)
        {
            LineCount += pCurNode->ContentLines;
            if((LINES-1) < (LineCount + pCurNode->pNext->ContentLines))
            {
                PageCount++;
                LineCount = 0;
            }
            pCurNode = pCurNode->pNext;
        }
    
        pCurNode = HeadNode.pNext;
    
        wmove(win, 0, 0);
        for(i = 0, LineCount = 0; i < (LINES-1); i++)
        {
            CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
            wprintw(win, (pCurNode->flag) ? "%s" : "%s\n", pCurNode->pContent);
            CTL_SET_COLOR(win, CTL_PANEL_RED);
            
            if(NULL == pCurNode->pNext) //It means only one page at the first time of display
            {
                BottomFlag = 1;
                break;
            }

            LineCount += pCurNode->ContentLines;
            pCurNode = pCurNode->pNext;
            if((LINES-1) < (LineCount + pCurNode->ContentLines))
                break;
        }
        wrefresh(win);
        mvwprintw(WinLabel, 0, LabelWidth, "[%d/%d] ", PageCount, CurPage+1);
    }
    
    keypad(WinLabel, true);
    while(1)
    {
        key = wgetch(WinLabel);
        switch(key)
        {
            case CTL_KEY_NPAGE:
                if(BottomFlag)
                    continue;
                
                wclear(win);
                wmove(win, 0, 0);
                for(i = 0, LineCount = 0; i < (LINES-1); i++)
                {
                    CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
                    wprintw(win, (pCurNode->flag) ? "%s" : "%s\n", pCurNode->pContent);
                    CTL_SET_COLOR(win, CTL_PANEL_RED);
    
                    if(NULL == pCurNode->pNext)
                    {
                        BottomFlag = 1;
                        break;
                    }

                    LineCount += pCurNode->ContentLines;
                    pCurNode = pCurNode->pNext;
                    if((LINES-1) < (LineCount + pCurNode->ContentLines))
                    {
                        i++;
                        break;
                    }
                }
                wrefresh(win);
    
                LogCount = i;
                CurPage++;
    
                mvwprintw(WinLabel, 0, LabelWidth, "[%d/%d] ", PageCount, CurPage+1);
                break;
            case CTL_KEY_PPAGE:
                if(0 >= CurPage)
                    continue;

                for(i = LogCount; i > 0; i--)
                {
                    pCurNode = pCurNode->pPrev;
                }
                
                for(i = 0, LineCount = 0; i < (LINES-1); i++)
                {
                    pCurNode = pCurNode->pPrev;
                    LineCount += pCurNode->ContentLines;
                
                    if(&HeadNode == pCurNode->pPrev)
                        break;
                    if((LINES-1) < (LineCount + pCurNode->pPrev->ContentLines))
                        break;
                }

                BottomFlag = 0;
                CurPage--;

                wclear(win);
                wmove(win, 0, 0);
                for(i = 0, LineCount = 0; i < (LINES-1); i++)
                {
                    CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
                    wprintw(win, (pCurNode->flag) ? "%s" : "%s\n", pCurNode->pContent);
                    CTL_SET_COLOR(win, CTL_PANEL_RED);
                
                    LineCount += pCurNode->ContentLines;
                    pCurNode = pCurNode->pNext;
                    if((LINES-1) < (LineCount + pCurNode->ContentLines))
                    {
                        i++;
                        break;
                    }
                }
                wrefresh(win);
                LogCount = i;
            
                mvwprintw(WinLabel, 0, LabelWidth, "[%d/%d] ", PageCount, CurPage+1);
                break;
            case CTL_KEY_ENTER:
                goto while_out;
            default :
                continue;
        }
    }

while_out :

    FreeFileContent(&HeadNode);
    delwin(WinLabel);
    delwin(win);
    touchwin(stdscr);
    refresh();

    return STAT_OK;
}

/*  
 *  Briefs: Get file name and set to pFileName
 *  Return: STAT_BACK / STAT_OK
 *  Note:   The size of pFileName must be CTL_FILE_NAME_LENGHT or bigger
 */
G_STATUS CTL_GetFileName(char *pFileName)
{
    WINDOW *win = newwin(CTL_GET_FILE_NAME_WIN_LINES, CTL_GET_FILE_NAME_WIN_COLS, 
        (LINES-CTL_GET_FILE_NAME_WIN_LINES)/2, (COLS-CTL_GET_FILE_NAME_WIN_COLS)/2);
    CTL_SET_COLOR(win, CTL_PANEL_CYAN);

    keypad(win, false);
    while(1)
    {
        echo();
        
        mvwhline(win, 0, 0, '-', CTL_GET_FILE_NAME_WIN_COLS);
        mvwhline(win, CTL_GET_FILE_NAME_WIN_LINES-1, 0, '-', CTL_GET_FILE_NAME_WIN_COLS);
        
        mvwaddstr(win, 1, 0, STR_INPUT_FILE_NAME);
        mvwaddstr(win, 2, 0, STR_INPUT_FILE_NAME_EG);
        mvwaddstr(win, 3, 0, STR_INPUT);
        mvwgetnstr(win, 3, GetWidth(STR_INPUT), pFileName, CTL_FILE_NAME_LENGHT-1); //End with '\0'
        noecho();
        
        if(0 == access(pFileName, F_OK))
            break;

        if(STAT_BACK == CTL_MakeChoice(STR_FILE_NOT_EXIST))
        {
            delwin(win);
            touchline(stdscr, (LINES-CTL_GET_FILE_NAME_WIN_LINES)/2, 
                CTL_GET_FILE_NAME_WIN_LINES);
            refresh();
            return STAT_BACK;
        }
        
        wclear(win);
    }

    delwin(win);
    touchline(stdscr, (LINES-CTL_GET_FILE_NAME_WIN_LINES)/2, 
        CTL_GET_FILE_NAME_WIN_LINES);
    refresh();

    return STAT_OK;
}

/*  
 *  Briefs: Get password and set to pPassword
 *  Return: STAT_BACK / STAT_OK
 *  Note:   The size of pPassword must be CTL_PASSWORD_LENGHT_MAX or bigger
 */
G_STATUS CTL_GetPassord(char *pPassword)
{
    WINDOW *win = newwin(CTL_GET_PASSWORD_WIN_LINES, CTL_GET_PASSWORD_WIN_COLS, 
        (LINES-CTL_GET_PASSWORD_WIN_LINES)/2, (COLS-CTL_GET_PASSWORD_WIN_COLS)/2);
    CTL_SET_COLOR(win, CTL_PANEL_CYAN);

    G_STATUS status;
    char buf[CTL_PASSWORD_LENGHT_MAX];
    int key, i;
    int CurPosX;
    
    keypad(win, true);
    while(1)
    {
        
        mvwhline(win, 0, 0, '-', CTL_GET_PASSWORD_WIN_COLS);
        mvwhline(win, CTL_GET_PASSWORD_WIN_LINES-1, 0, '-', CTL_GET_PASSWORD_WIN_COLS);

        i = 0;
        CurPosX = GetWidth(STR_INPUT_PASSWORD);
        mvwaddstr(win, 3, 0, STR_INPUT_PASSWORD_CONFIRM);
        mvwaddstr(win, 2, 0, STR_INPUT_PASSWORD);
        
        //Input password >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        while(1)
        {
            key = wgetch(win);
            if((32 <= key) && (126 >= key)) //Space key to "~" key
            {
                if((CTL_PASSWORD_LENGHT_MAX-1) <= i)
                    continue;
                    
                waddch(win, '*');
                CurPosX++;
                pPassword[i++] = (char)key;
            }
            else if(CTL_KEY_BACKSPACE == key)
            {
                if(0 < i)
                {
                    i--;
                    mvwaddch(win, 2, --CurPosX, ' ');
                }
            }
            else if((CTL_KEY_ENTER == key) || (CTL_KEY_TAB == key))
                break;
            else 
                continue;
            
            wmove(win, 2, CurPosX);
        }
        pPassword[i] = '\0';

        if(CTL_PASSWORD_LENGHT_MIN > i)
        {
            status = CTL_MakeChoice(STR_PASSWORD_TOO_SHORT);
            if(STAT_BACK == status)
            {
                delwin(win);
                touchline(stdscr, (LINES-CTL_GET_PASSWORD_WIN_LINES)/2, 
                    CTL_GET_PASSWORD_WIN_LINES);
                refresh();
                return STAT_BACK;
            }

            wclear(win);
            continue;
        }

        //Input password again >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        i = 0;
        CurPosX = GetWidth(STR_INPUT_PASSWORD_CONFIRM);
        wmove(win, 3, CurPosX);
        
        while(1)
        {
            key = wgetch(win);
            if((32 <= key) && (126 >= key)) //Space key to "~" key
            {
                if((CTL_PASSWORD_LENGHT_MAX-1) <= i)
                    continue;
                waddch(win, '*');
                CurPosX++;
                buf[i++] = (char)key;
            }
            else if(CTL_KEY_BACKSPACE == key)
            {
                if(0 < i)
                {
                    i--;
                    mvwaddch(win, 3, --CurPosX, ' ');
                }
            }            
            else if(CTL_KEY_ENTER == key)
                break;
            else 
                continue;
            
            wmove(win, 3, CurPosX);
        }
        buf[i] = '\0';

        if(strcmp(pPassword, buf) == 0)
            break;

        status = CTL_MakeChoice(STR_PASSWORD_NOT_MATCH);
        if(STAT_BACK == status)
        {
            delwin(win);
            touchline(stdscr, (LINES-CTL_GET_PASSWORD_WIN_LINES)/2, 
                CTL_GET_PASSWORD_WIN_LINES);
            refresh();
            return STAT_BACK;
        }

        wclear(win);
        continue;
    }

    delwin(win);
    touchline(stdscr, (LINES-CTL_GET_PASSWORD_WIN_LINES)/2, 
        CTL_GET_PASSWORD_WIN_LINES);
    refresh();
    
    return STAT_OK;
}

/*  
 *  Briefs: Similar with printf
 *  Return: Always STAT_OK
 *  Note:   Only support UTF-8
 */
G_STATUS CTL_DispWarning(const char *pFormat, ...)
{
    va_list varg;
    char buf[1024];
    
    va_start(varg, pFormat);
    vsnprintf(buf, sizeof(buf), pFormat, varg);
    va_end(varg);

    //Count lines >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    int lines = CountLines(buf, CTL_WARNING_WIN_COLS);

    if(0 == lines)
    {
        lines = 1;
    }

    lines += 3;
    if(LINES < lines)
    {
        CTL_DispWarning(STR_BEYOND_RANGE_OF_DISP);
        return STAT_OK;
    }
    
    //Display content >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    WINDOW *win = newwin(lines, CTL_WARNING_WIN_COLS, 
        (LINES-lines)/2, (COLS-CTL_WARNING_WIN_COLS)/2);
    
    CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
    mvwhline(win, 0, 0, '*', CTL_WARNING_WIN_COLS);
    mvwhline(win, lines-1, 0, '*', CTL_WARNING_WIN_COLS);

    mvwaddstr(win, 1, 0, buf);
    wattron(win, A_REVERSE);
    mvwaddstr(win, lines-2, (CTL_WARNING_WIN_COLS-GetWidth(STR_CONTINUE))/2, STR_CONTINUE);
    wattroff(win, A_REVERSE);

    wrefresh(win);

    wgetch(win);

    delwin(win);
    touchline(stdscr, (LINES-lines)/2, lines);
    refresh();
    
    return STAT_OK;
}

/*  
 *  Briefs: Similar with CTL_DispWarning
 *  Return: STAT_BACK / STAT_RETRY
 *  Note:   Only support UTF-8
 */
G_STATUS CTL_MakeChoice(const char *pFormat, ...)
{
    va_list varg;
    char buf[1024];
    
    va_start(varg, pFormat);
    vsnprintf(buf, sizeof(buf), pFormat, varg);
    va_end(varg);

    //Count lines >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    int lines = CountLines(buf, CTL_MAKE_CHOICE_WIN_COLS);
    if(0 == lines)
    {
        lines = 1;
    }

    lines += 3;
    if(LINES < lines)
    {
        CTL_DispWarning(STR_BEYOND_RANGE_OF_DISP);
        return STAT_RETRY;
    }
    
    //Display content >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    WINDOW *win = newwin(lines, CTL_MAKE_CHOICE_WIN_COLS, 
        (LINES-lines)/2, (COLS-CTL_MAKE_CHOICE_WIN_COLS)/2);
    
    CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
    mvwhline(win, 0, 0, '*', CTL_MAKE_CHOICE_WIN_COLS);
    mvwhline(win, lines-1, 0, '*', CTL_MAKE_CHOICE_WIN_COLS);
    mvwaddstr(win, 1, 0, buf);

    char *pStr1 = STR_RETRY;
    char *pStr2 = STR_BACK;
    int Str1StartX = (CTL_MAKE_CHOICE_WIN_COLS - GetWidth(pStr1) - GetWidth(pStr2))/3;
    int Str2StartX = CTL_MAKE_CHOICE_WIN_COLS - Str1StartX - GetWidth(pStr2);
    int StartPosY = lines - 2;
    
    mvwaddstr(win, StartPosY, Str2StartX, pStr2);
    wattron(win, A_REVERSE);
    mvwaddstr(win, StartPosY, Str1StartX, pStr1);
    wattroff(win, A_REVERSE);

    char flag = 0;
    int key;
    
    keypad(win, true);
    while(1)
    {
        key = wgetch(win);
        switch(key)
        {
            case CTL_KEY_LEFT:
            case CTL_KEY_RIGHT:
                flag ^= 1;
                break;
            case CTL_KEY_ENTER:
                goto while_out;
            default :
                continue;
        }

        if(flag)
        {
            mvwaddstr(win, StartPosY, Str1StartX, pStr1);
            wattron(win, A_REVERSE);
            mvwaddstr(win, StartPosY, Str2StartX, pStr2);
            wattroff(win, A_REVERSE);
        }
        else
        {
            mvwaddstr(win, StartPosY, Str2StartX, pStr2);
            wattron(win, A_REVERSE);
            mvwaddstr(win, StartPosY, Str1StartX, pStr1);
            wattroff(win, A_REVERSE);
        }
    }

while_out :

    delwin(win);
    touchline(stdscr, (LINES-lines)/2, lines);
    refresh();
    
    if(flag)
        return STAT_BACK;
    
    return STAT_RETRY;
}

/*  
 *  Briefs: Similar with CTL_MakeChoice
 *  Return: STAT_BACK / STAT_OK
 *  Note:   Only support UTF-8
 */
G_STATUS CTL_ConfirmOperation(const char *pFormat, ...)
{
    va_list varg;
    char buf[1024];
    
    va_start(varg, pFormat);
    vsnprintf(buf, sizeof(buf), pFormat, varg);
    va_end(varg);

    //Count lines >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    int lines = CountLines(buf, CTL_CONFIRM_WIN_COLS);
    if(0 == lines)
    {
        lines = 1;
    }

    lines += 3;
    if(LINES < lines)
    {
        CTL_DispWarning(STR_BEYOND_RANGE_OF_DISP);
        return STAT_BACK;
    }
    
    //Display content >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    WINDOW *win = newwin(lines, CTL_CONFIRM_WIN_COLS, 
        (LINES-lines)/2, (COLS-CTL_CONFIRM_WIN_COLS)/2);
    
    CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
    mvwhline(win, 0, 0, '*', CTL_CONFIRM_WIN_COLS);
    mvwhline(win, lines-1, 0, '*', CTL_CONFIRM_WIN_COLS);
    mvwaddstr(win, 1, 0, buf);

    char *pStr1 = STR_YES;
    char *pStr2 = STR_NO;
    int Str1StartX = (CTL_CONFIRM_WIN_COLS - GetWidth(pStr1) - GetWidth(pStr2))/3;
    int Str2StartX = CTL_CONFIRM_WIN_COLS - Str1StartX - GetWidth(pStr2);
    int StartPosY = lines - 2;
    
    mvwaddstr(win, StartPosY, Str2StartX, pStr2);
    wattron(win, A_REVERSE);
    mvwaddstr(win, StartPosY, Str1StartX, pStr1);
    wattroff(win, A_REVERSE);

    char flag = 0;
    int key;
    
    keypad(win, true);
    while(1)
    {
        key = wgetch(win);
        switch(key)
        {
            case CTL_KEY_LEFT:
            case CTL_KEY_RIGHT:
                flag ^= 1;
                break;
            case CTL_KEY_ENTER:
                goto while_out;
            default :
                continue;
        }

        if(flag)
        {
            mvwaddstr(win, StartPosY, Str1StartX, pStr1);
            wattron(win, A_REVERSE);
            mvwaddstr(win, StartPosY, Str2StartX, pStr2);
            wattroff(win, A_REVERSE);
        }
        else
        {
            mvwaddstr(win, StartPosY, Str2StartX, pStr2);
            wattron(win, A_REVERSE);
            mvwaddstr(win, StartPosY, Str1StartX, pStr1);
            wattroff(win, A_REVERSE);
        }
    }

while_out :

    delwin(win);
    touchline(stdscr, (LINES-lines)/2, lines);
    refresh();
    
    if(flag)
        return STAT_BACK;
    
    return STAT_OK;
}



//Static functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*  
 *  Briefs: Calculate the number of lines of pBuf according to cols
 *  Return: The number of lines in pBuf
 *  Note:   The paramter of cols means the width of window
 */
static int CountLines(char *pBuf, int cols)
{
    if((NULL == pBuf) || ('\0' == *pBuf))
        return 0;
    
    int lines = 1; //Must initialize as 1
    int count = 0;
    
    while('\0' != *pBuf)
    {
        if('\r' == *pBuf)
        {
            pBuf++;
            continue;
        }
        
        if('\n' == *pBuf)
        {
            count = 0;
            if('\0' != *(pBuf + 1))
            {
                lines++;
            }
            pBuf++;
            continue;
        }

        if('\t' == *pBuf)
        {
            *pBuf = ' ';
        }

        count += (*pBuf >> 7) ? 2 : 1;
        
        if(cols <= count)
        {
            count = 0;
            lines++;
            if(*pBuf >> 7)
            {
                if('\n' == *(pBuf + 3) || ('\0' == *(pBuf + 3)))
                {
                    lines--;
                    MoveString(pBuf+3, pBuf+4);
                }
                else if('\0' == *(pBuf + 3))
                {
                    lines--;
                }
            }
            else
            {
                if('\n' == *(pBuf + 1) || ('\0' == *(pBuf + 1)))
                {
                    lines--;
                    MoveString(pBuf+1, pBuf+2);
                }
                else if('\0' == *(pBuf + 1))
                {
                    lines--;
                }
            }
        }
        
        pBuf += (*pBuf >> 7) ? 3 : 1;
    }

    return lines;
}

/*  
 *  Briefs: Move string from pTail address to pHead address
 *  Return: STAT_ERR / STAT_OK
 *  Note:   1. It's only used to operate a string, i.e pHead and pTail arg
               the address of the same string.
            2. pTail must be bigger than pHead
 */
static G_STATUS MoveString(char *pHead, char *pTail)
{
    if((NULL == pHead) || (NULL == pTail) || (pHead >= pTail) || ('\0' == *pHead))
        return STAT_ERR;

    while('\0' != *pTail)
    {
        *pHead++ = *pTail++;
    }
    *pHead = '\0';

    return STAT_OK;
}



//Static inline functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

/*  
 *  Briefs: Free link list of FileContent_t
 *  Return: None
 *  Note:   None
 */
static inline void FreeFileContent(FileContent_t *pHeadNode)
{
    FileContent_t *CurNode = pHeadNode->pNext;
    FileContent_t *TmpNode;
    while(NULL != CurNode)
    {
        TmpNode = CurNode->pNext;
        if(NULL != CurNode->pContent)
            free(CurNode->pContent);
        free(CurNode);
        CurNode = TmpNode;
    }
}

/*  
 *  Briefs: Find out the first SrcChar and set as TarChar
 *  Return: STAT_ERR / STAT_OK
 *  Note:   None
 */
static inline G_STATUS ReplaceChar(char *ptr, char SrcChar, char TarChar)
{
    if(NULL == ptr)
        return STAT_ERR;
    
    while('\0' != *ptr)
    {
        if(SrcChar == *ptr)
        {
            *ptr = TarChar;
            return STAT_OK;
        }
        ptr++;
    }
    
    return STAT_ERR;
}
