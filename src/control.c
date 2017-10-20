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
#endif
#include <signal.h>
#include <sys/stat.h>

static inline void FreePtrLinkList(PtrLinkList_t *pHeadNode);
static inline void FreeFileContent(FileContent_t *pHeadNode);
static inline G_STATUS ReplaceChar(char *ptr, char SrcChar, char TarChar);
static G_STATUS MoveString(char *pHead, char *pTail);

void CTL_InitConsole(void)
{
    InitStr(); //It must be initialized firstly

#ifdef __LINUX
    setlocale(LC_ALL, "");
#endif

    //signal(SIGINT, SIG_IGN);
    
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

G_STATUS CTL_ConfirmOperation(const char *pStr, int StrLenght)
{
    return STAT_OK;
}

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
        if((KEY_DOWN == key) || (KEY_UP == key))
        {
            flag ^= 1;
        }        
        else if(13 == key) //Enter key
            break;
        else
            continue;

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
    
    if(flag)
    {
        g_menu = g_ChMenu;
        g_LanguageFlag = 1;
    }
    else
    {
        g_menu = g_EnMenu;
        g_LanguageFlag = 0;
    }

    delwin(win);
    touchline(stdscr, (LINES-CTL_CHOOSE_LANGUAGE_WIN_LINES)/2, 
        CTL_CHOOSE_LANGUAGE_WIN_LINES);
    refresh();
}

void CTL_ShowMenu(CTL_MENU *pFunc)
{
    WINDOW *win = newwin(CTL_MENU_WIN_LINES, CTL_MENU_WIN_COLS, 
        (LINES-CTL_MENU_WIN_LINES)/2, (COLS-CTL_MENU_WIN_COLS)/2);

    CTL_SET_COLOR(win, CTL_PANEL_CYAN);
    wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
    wattron(win, A_REVERSE);
    mvwaddstr(win, 0, (CTL_MENU_WIN_COLS-GetWidth(STR_MENU))/2, STR_MENU);
    wattroff(win, A_REVERSE);

    int key;
    int CurPosY = 2;
    while(NULL != g_menu[CurPosY-1])
    {
        mvwaddstr(win, CurPosY, 2, g_menu[CurPosY-1]);
        CurPosY++;
    }

    CurPosY = 1;
    
    wattron(win, A_REVERSE);
    mvwaddstr(win, CurPosY, 2, g_menu[CurPosY-1]);
    wattroff(win, A_REVERSE);
    wrefresh(win);
    
    keypad(win, true);
    while(1)
    {
        key = wgetch(win);

        if((KEY_DOWN == key) || (9 == key))
        {
            mvwaddstr(win, CurPosY, 2, g_menu[CurPosY-1]);
            CurPosY++;
        }
        else if(KEY_UP == key)
        {
            mvwaddstr(win, CurPosY, 2, g_menu[CurPosY-1]);
            CurPosY--;
        }
        else if(27 == key) //Esc key
        {
            CTL_ForceExit();
        }
        else if(13 == key) //Enter key
            break;
        else
            continue;

        if(CurPosY < 1)
        {
            CurPosY = CTL_MENU_MAX;
        }
        else if((CurPosY-1) >= CTL_MENU_MAX)
        {
            CurPosY = 1;
        }

        wattron(win, A_REVERSE);
        mvwaddstr(win, CurPosY, 2, g_menu[CurPosY-1]);
        wattroff(win, A_REVERSE);
    }

    *pFunc = CurPosY-1;
    delwin(win);
    touchline(stdscr, (LINES-CTL_MENU_WIN_LINES)/2, CTL_MENU_WIN_LINES);
    refresh();
}

//Always return STAT_OK
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
        CTL_DispWarning("%s: %s\n", STR_FAIL_TO_GET_FILE_FOLDER_INFO, pFileName);
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
    
    //char *buf = (char *)malloc(CTL_SINGLE_LINE_WIDTH);
    char *buf = NULL;
    if(NULL == buf)
    {
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
            free(buf);
            FreeFileContent(&HeadNode);
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
            FreeFileContent(&HeadNode);
            return STAT_OK;
        }
    }

    fclose(fp);
    fp = NULL;



    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //Show file
    WINDOW *WinLabel = newwin(1, COLS, LINES-1, 0);
    keypad(WinLabel, true);
    CTL_SET_COLOR(WinLabel, CTL_PANEL_CYAN);
    wattron(WinLabel, A_REVERSE);
    mvwaddstr(WinLabel, 0, 0, STR_SHOW_FILE_LABEL);
    wrefresh(WinLabel);

    WINDOW *win = newwin(LINES-1, COLS, 0, 0);
    win = newwin(LINES-1, COLS, 0, 0);
    FileContent_t *pCurNode = HeadNode.pNext;
    char BottomFlag = 0;            //1 means it has been in bottom position, i.e can't to page down again
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
            if((LineCount + pCurNode->pNext->ContentLines) > LINES-1)
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
            if((LineCount + pCurNode->ContentLines) > (LINES-1))
                break;
        }
        wrefresh(win);
        mvwprintw(WinLabel, 0, LabelWidth, "[%d/%d] ", PageCount, CurPage+1);
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
                if((LineCount + pCurNode->ContentLines) > (LINES-1))
                {
                    i++;
                    break;
                }
            }
            wrefresh(win);

            LogCount = i;
            CurPage++;

            mvwprintw(WinLabel, 0, LabelWidth, "[%d/%d] ", PageCount, CurPage+1);            
        }
        else if(KEY_PPAGE == key)
        {
            if(CurPage <= 0)
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
                if((LineCount + pCurNode->pPrev->ContentLines) > (LINES-1))
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
                if((LineCount + pCurNode->ContentLines) > (LINES-1))
                {
                    i++;
                    break;
                }
            }
            wrefresh(win);
            LogCount = i;
            
            mvwprintw(WinLabel, 0, LabelWidth, "[%d/%d] ", PageCount, CurPage+1);
        }
        else if(13 == key) //Enter key
            break;
        else
            continue;
    }

    FreeFileContent(&HeadNode);
    delwin(WinLabel);
    delwin(win);
    touchwin(stdscr);
    refresh();

    return STAT_OK;
}

#if 0
void CTL_MENU_ShowInstruction(void)
{
    WINDOW *win = newwin(LINES, COLS, 0, 0);
    char **ptr = pInstruction;

    CTL_SET_COLOR(win, CTL_PANEL_GREEN);
    wattron(win, A_REVERSE);
    //mvwaddstr(win, 0, (COLS - strlen(*ptr))/2, *ptr++); //MinGW doesn't support
    mvwaddstr(win, 0, (COLS - strlen(*ptr))/2, *ptr);
    ptr++; 
    wattroff(win, A_REVERSE);

    wmove(win, 1, 0);
    while(*ptr != NULL)
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
        if(13 == key) //Enter key
            break;
        else
            continue;
    }

    delwin(win);
    touchwin(stdscr);
    refresh();
}

/*
    Return: STAT_GO_BACK, STAT_OK
*/
G_STATUS CTL_GetFileName(char *pFileName)
{
    WINDOW *win = newwin(CTL_GET_FILE_NAME_WIN_LINES, CTL_GET_FILE_NAME_WIN_COLS, 
        (LINES-CTL_GET_FILE_NAME_WIN_LINES)/2, (COLS-CTL_GET_FILE_NAME_WIN_COLS)/2);
    CTL_SET_COLOR(win, CTL_PANEL_CYAN);

    G_STATUS status;
    keypad(win, false);
    
    while(1)
    {
        echo();
        CTL_HIDE_CONSOLE_END_LINE(); //Shield the end line of standard screen 
        
        mvwhline(win, 0, 0, '-', CTL_GET_FILE_NAME_WIN_COLS);
        mvwhline(win, CTL_GET_FILE_NAME_WIN_LINES-1, 0, '-', CTL_GET_FILE_NAME_WIN_COLS);
        
        mvwaddstr(win, 1, 0, STR_INPUT_FILE_NAME);
        mvwaddstr(win, 2, 0, STR_INPUT_FILE_NAME_EG);
        mvwaddstr(win, 3, 0, STR_INPUT);
        mvwgetnstr(win, 3, sizeof(STR_INPUT)-1, pFileName, CTL_FILE_NAME_LENGHT-1);
        if(0 == access(pFileName, F_OK))
            break;

        noecho();
        CTL_SHOW_CONSOLE_END_LINE();
        status = CTL_MakeChoice("%s", STR_FILE_NOT_EXIST);
        if(STAT_GO_BACK == status)
        {
            delwin(win);
            return STAT_GO_BACK;
        }
        
        wclear(win);
    }

    noecho();
    delwin(win);
    touchline(stdscr, (LINES-CTL_GET_FILE_NAME_WIN_LINES)/2, 
        CTL_GET_FILE_NAME_WIN_LINES);
    CTL_SHOW_CONSOLE_END_LINE();

    return STAT_OK;
}

/*
    Return: STAT_GO_BACK, STAT_OK
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
        mvwhline(win, 5, 0, '-', CTL_GET_PASSWORD_WIN_COLS);

        i = 0;
        CurPosX = sizeof(STR_INPUT_PASSWORD)-1;
        mvwaddstr(win, 3, 0, STR_INPUT_PASSWORD_CONFIRM);
        mvwaddstr(win, 2, 0, STR_INPUT_PASSWORD);
        while(1)
        {
            key = wgetch(win);
            if((key >= 32) && (key <= 126))
            {
                if(i >= (CTL_PASSWORD_LENGHT_MAX-1))
                    continue;
                waddch(win, '*');
                CurPosX++;
                pPassword[i++] = (char)key;
            }
            #ifdef __LINUX
            else if(KEY_BACKSPACE == key)
            #elif defined __WINDOWS
            else if(8 == key)  //Backspace key
            #endif
            {
                if(i > 0)
                {
                    i--;
                    mvwaddch(win, 2, --CurPosX, ' ');
                }
            }            
            else if(27 == key) //Esc key
            {
                CTL_SafeExit(win);
                continue;
            }
            else if((13 == key) || (9 == key)) //Enter key or Tab key
                break;
            else 
                continue;
            
            wmove(win, 2, CurPosX);
        }
        pPassword[i] = '\0';

        if(i < CTL_PASSWORD_LENGHT_MIN)
        {
            status = CTL_MakeChoice("%s", STR_PASSWORD_TOO_SHORT);
            if(STAT_GO_BACK == status)
            {
                delwin(win);
                return STAT_GO_BACK;
            }

            wclear(win);
            continue;
        }

        i = 0;
        CurPosX = CTL_GET_PASSWORD_WIN_COLS - CTL_PASSWORD_LENGHT_MAX;
        wmove(win, 3, CurPosX);
        while(1)
        {
            key = wgetch(win);
            if((key >= 32) && (key <= 126))
            {
                if(i >= (CTL_PASSWORD_LENGHT_MAX-1))
                    continue;
                waddch(win, '*');
                CurPosX++;
                buf[i++] = (char)key;
            }
            #ifdef __LINUX
            else if(KEY_BACKSPACE == key)
            #elif defined __WINDOWS
            else if(8 == key)  //Backspace key
            #endif
            {
                if(i > 0)
                {
                    i--;
                    mvwaddch(win, 3, --CurPosX, ' ');
                }
            }            
            else if(27 == key) //Esc key
            {
                CTL_SafeExit(win);
                continue;
            }
            else if(13 == key) //Enter key
                break;
            else 
                continue;
            
            wmove(win, 3, CurPosX);
        }
        buf[i] = '\0';

        if(strcmp(pPassword, buf) == 0)
            break;

        status = CTL_MakeChoice("%s", STR_PASSWORD_NOT_MATCH);
        if(STAT_GO_BACK == status)
        {
            delwin(win);
            return STAT_GO_BACK;
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
    Rules: "%s" means string
    E.g: CTL_MakeChoice("%s%s", "It occurs fatal error", "Please make a choice");
    Return: STAT_RETRY, STAT_GO_BACK
*/
G_STATUS CTL_MakeChoice(const char*format, ...)
{
    va_list pArg;
    va_start(pArg, format);

    PtrLinkList_t StrHeadNode;
    PtrLinkList_t *pStrNode = &StrHeadNode;
    PtrLinkList_t *pTmpNode;
    const char *pFormat = format;
    int lines = 0, cols = 20, tmp = 0;

    //Parse format
    while(*pFormat != '\0')
    {
        if('%' == *pFormat++)
        {
            if('s' == *pFormat)
            {
                lines++;
                pTmpNode = (PtrLinkList_t *)malloc(sizeof(PtrLinkList_t));
                if(NULL == pTmpNode)
                {
                    CTL_ErrExit(STR_ERR_FAIL_TO_MALLOC);
                }
                
                pTmpNode->ptr = va_arg(pArg, char *);
                tmp = strlen(pTmpNode->ptr);
                if(tmp > cols)
                {
                    cols = tmp;
                }
                
                pTmpNode->pNext_t = NULL;
                pStrNode->pNext_t = pTmpNode;
                pStrNode = pTmpNode;
            }
        }
    }

    va_end(pArg);

    lines += 6;
    if(lines > CTL_CONSOLE_LINES)
    {
        //FreePtrLinkList(&StrHeadNode);
        CTL_ErrExit("%s%d\n", STR_ERR_INVALID_LINES, lines);
    }

    cols += 4;
    if(cols > CTL_CONSOLE_COLS)
    {
        //FreePtrLinkList(&StrHeadNode);
        CTL_ErrExit("%s%d\n", STR_ERR_INVALID_COLS, cols);
    }
#ifdef __DEBUG
    if(lines < 7) //Top and bottom symbols is 4 lines, choice string is 3 lines
    {
        //FreePtrLinkList(&StrHeadNode);
        CTL_ErrExit("%s%d\n", STR_ERR_STR_IS_NULL, lines);
    }
    if(cols < 20)  //STR_RETRY and STR_BACK are 16 bytes in total, side symbols are 4 bytes
    {
        //FreePtrLinkList(&StrHeadNode);
        CTL_ErrExit("%s%d\n", STR_ERR_STR_TOO_SHORT, cols);
    }
#endif

    WINDOW *win = newwin(lines, cols, (LINES-lines)/2, (COLS-cols)/2);
    CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
    wborder(win, '*', '*', '*', '*', '*', '*', '*', '*');

    int CurPosY = 2;
    pStrNode = StrHeadNode.pNext_t;
    while(pStrNode != NULL)
    {
        mvwaddstr(win, CurPosY++, 2, pStrNode->ptr);
        pStrNode = pStrNode->pNext_t;
    }

    int Str1StartX = (cols-16)/3; //STR_RETRY and STR_BACK are about 16 bytes
    int Str2StartX = cols - Str1StartX - sizeof(STR_BACK)+1;
    int StartPosY = lines-1-2; //Bottom symbols are 2 lines, and the line counts from 0
    mvwaddstr(win, StartPosY, Str2StartX, STR_BACK);
    wattron(win, A_REVERSE);
    mvwaddstr(win, StartPosY, Str1StartX, STR_RETRY);
    wattroff(win, A_REVERSE);

    char flag = 0;
    int key;
    keypad(win, true);
    while(1)
    {
        key = wgetch(win);
        if((KEY_LEFT == key) || (KEY_RIGHT == key))
        {
            flag ^= 1;
        }        
        else if(27 == key) //Esc key
        {
            CTL_SafeExit(win);
            
            FreePtrLinkList(&StrHeadNode);
            delwin(win);
            touchline(stdscr, (LINES-lines)/2, lines);
            refresh();
            return STAT_RETRY;
        }
        else if(13 == key) //Enter key
            break;
        else
            continue;

        if(flag)
        {
            mvwaddstr(win, StartPosY, Str1StartX, STR_RETRY);
            wattron(win, A_REVERSE);
            mvwaddstr(win, StartPosY, Str2StartX, STR_BACK);
            wattroff(win, A_REVERSE);
        }
        else
        {
            mvwaddstr(win, StartPosY, Str2StartX, STR_BACK);
            wattron(win, A_REVERSE);
            mvwaddstr(win, StartPosY, Str1StartX, STR_RETRY);
            wattroff(win, A_REVERSE);
        }
    }

    FreePtrLinkList(&StrHeadNode);
    delwin(win);
    touchline(stdscr, (LINES-lines)/2, lines);
    refresh();

    if(flag)
        return STAT_GO_BACK;

    return STAT_RETRY;
}



//Static inline functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static inline void FreePtrLinkList(PtrLinkList_t *pHeadNode)
{
    PtrLinkList_t *pCurNode = pHeadNode->pNext_t;
    PtrLinkList_t *pTmpNode;
    while(pCurNode != NULL)
    {
        pTmpNode = pCurNode->pNext_t;
        free(pCurNode);
        pCurNode = pTmpNode;
    }
}

G_STATUS CTL_ConfirmOperation(const char *pStr, int StrLenght)
{
    int cols;
    if(CTL_CONFIRM_WIN_COLS < StrLenght)
    {
        cols = StrLenght+4;
    }
    else
    {
        cols = CTL_CONFIRM_WIN_COLS;
    }

    
    WINDOW *win = newwin(CTL_CONFIRM_WIN_LINES, cols, 
        (LINES-CTL_CONFIRM_WIN_LINES)/2, (COLS-cols)/2);
    
    CTL_SET_COLOR(win, CTL_PANEL_CYAN);
    wborder(win, '*', '*', '*', '*', '*', '*', '*', '*');

    mvwaddstr(win, 2, (cols-StrLenght)/2, pStr);

    int Str1StartX = (cols - sizeof(STR_CONTINUE)+1 - sizeof(STR_GO_BACK)+1)/3;
    int Str2StartX = cols - Str1StartX - sizeof(STR_CONTINUE)+1;
    mvwaddstr(win, 4, Str2StartX, STR_CONTINUE);
    wattron(win, A_REVERSE);
    mvwaddstr(win, 4, Str1StartX, STR_GO_BACK);
    wattroff(win, A_REVERSE);

    char flag = 0;
    int key;
    keypad(win, true);
    while(1)
    {
        key = wgetch(win);
        if((KEY_LEFT == key) || (KEY_RIGHT == key))
        {
            flag ^= 1;
        }        
        else if(13 == key) //Enter key
            break;
        else
            continue;

        if(flag)
        {
            mvwaddstr(win, 4, Str1StartX, STR_GO_BACK);
            wattron(win, A_REVERSE);
            mvwaddstr(win, 4, Str2StartX, STR_CONTINUE);
            wattroff(win, A_REVERSE);
        }
        else
        {
            mvwaddstr(win, 4, Str2StartX, STR_CONTINUE);
            wattron(win, A_REVERSE);
            mvwaddstr(win, 4, Str1StartX, STR_GO_BACK);
            wattroff(win, A_REVERSE);
        }
    }

    delwin(win);
    touchline(stdscr, (LINES-CTL_CONFIRM_WIN_LINES)/2, CTL_CONFIRM_WIN_LINES);
    refresh();

    if(0 == flag)
        return STAT_GO_BACK;

    return STAT_OK;
}
#endif

//Always return STAT_OK
G_STATUS CTL_DispWarning(const char *pFormat, ...)
{
    va_list varg;
    char buf[1024];
    
    va_start(varg, pFormat);
    vsnprintf(buf, sizeof(buf), pFormat, varg);
    va_end(varg);

    int lines = 1, count = 0;
    char *pBuf = buf;

    //Count lines
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
        
        if(CTL_WARNING_WIN_COLS <= count)
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

    if(0 == lines)
    {
        lines = 1;
    }
    else if(1 < lines)
    {
        if(CTL_WARNING_WIN_COLS >= count)
        {
            //lines++;
        }
    }

    lines += 3;
    if(LINES < lines)
    {
        CTL_DispWarning("超出显示范围");
        return STAT_OK;
    }
    
    WINDOW *win = newwin(lines, CTL_WARNING_WIN_COLS, 
        (LINES-lines)/2, (COLS-CTL_WARNING_WIN_COLS)/2);
    
    CTL_SET_COLOR(win, CTL_PANEL_YELLOW);
    mvwhline(win, 0, 0, '*', CTL_WARNING_WIN_COLS);
    mvwhline(win, lines-1, 0, '*', CTL_WARNING_WIN_COLS);

    int CurPosY = 1;
    count = 0;

    mvwaddstr(win, 1, 0, buf);
    wattron(win, A_REVERSE);
    mvwaddstr(win, lines-2, (CTL_WARNING_WIN_COLS-GetWidth(STR_CONTINUE))/2, STR_CONTINUE);
    wattroff(win, A_REVERSE);

    wrefresh(win);

    wgetch(win);

    delwin(win);
    touchline(stdscr, (LINES-lines)/2, lines);
    refresh();
}

//Move string from pTail address to PHead address
static G_STATUS MoveString(char *pHead, char *pTail)
{
    if((NULL == pHead) || (NULL == pTail) || (pHead >= pTail) || ('\0' == *pHead))
        return STAT_OK;

    while('\0' != *pTail)
    {
        *pHead++ = *pTail++;
    }
    *pHead = '\0';

    return STAT_OK;
}



//Static inline functions
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static inline void FreeFileContent(FileContent_t *pHeadNode)
{
    FileContent_t *CurNode = pHeadNode->pNext;
    FileContent_t *TmpNode;
    while(CurNode != NULL)
    {
        TmpNode = CurNode->pNext;
        if(NULL != CurNode->pContent)
            free(CurNode->pContent);
        free(CurNode);
        CurNode = TmpNode;
    }
}

//Find out the first SrcChar and set as TarChar
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
