/*************************************************************************
	> File Name: control.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 9th,2017 Wednesday 08:49:11
 ************************************************************************/

#include "control.h"
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>

G_STATUS CTL_InitConsole(void)
{
    initscr();
    cbreak();
    nonl();
    noecho();
    keypad(stdscr, true);
    if(OK == start_color())
    {
        init_pair(CTL_PANEL_CYAN, COLOR_CYAN, COLOR_BLACK);        
        init_pair(CTL_PANEL_RED, COLOR_RED, COLOR_BLACK);
        init_pair(CTL_PANEL_YELLOW, COLOR_YELLOW, COLOR_BLACK);
        init_pair(CTL_PANEL_GREEN, COLOR_GREEN, COLOR_BLACK);
    }    

    if(COLS < CTL_CONSOLE_COLS)
    {
        endwin();
        DISP_ERR_PLUS("%s%d\n", STR_ERR_INVALID_COLS, CTL_CONSOLE_COLS);
        return STAT_ERR;
    }
    
    if(LINES < CTL_CONSOLE_LINES)
    {
        endwin();
        DISP_ERR_PLUS("%s%d\n", STR_ERR_INVALID_LINES, CTL_CONSOLE_LINES);
        return STAT_ERR;
    }

    clear();
    CTL_SET_WIN_COLOR(stdscr, CTL_PANEL_CYAN);
    border('|', '|', '-', '-', '+', '+', '+', '+');
    
    attron(A_REVERSE);
    mvaddstr(0, (COLS+1-sizeof(STR_CONSOLE_LABEL))/2, STR_CONSOLE_LABEL);
    mvaddnstr(LINES-2, 2, STR_CONSOLE_END_LINE, COLS-3);
    
    int i;
    for(i = 0; i < (COLS-sizeof(STR_CONSOLE_END_LINE)-3); i++)
    {
        addch(' ');
    }
    attroff(A_REVERSE);
    refresh();    

    return STAT_OK;
}

G_STATUS CTL_ShowMenu(char *pFunc)
{
    WINDOW *win = newwin(CTL_MENU_WIN_LINES, CTL_MENU_WIN_COLS, 
        (LINES-CTL_MENU_WIN_LINES)/2, (COLS-CTL_MENU_WIN_COLS)/2);

    CTL_SET_WIN_COLOR(win, CTL_PANEL_CYAN);
    wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
    wattron(win, A_REVERSE);
    mvwaddstr(win, 0, (CTL_MENU_WIN_COLS+1-sizeof(STR_MENU))/2, STR_MENU);
    wattroff(win, A_REVERSE);

    int key;
    int CurPosY = 3, CurMenuPos = 1;
    while(g_pMenuStr[CurMenuPos] != NULL)
    {
        mvwaddstr(win, CurPosY++, 2, g_pMenuStr[CurMenuPos++]);
    }
    wattron(win, A_REVERSE);
    mvwaddstr(win, 2, 2, g_pMenuStr[0]);
    wattroff(win, A_REVERSE);
    wrefresh(win);

    CurPosY = 2;
    CurMenuPos = 0;
    keypad(win, true);
    while(1)
    {
        key = wgetch(win);

        if((KEY_DOWN == key) || (9 == key))
        {
            mvwaddstr(win, CurPosY++, 2, g_pMenuStr[CurMenuPos++]);
        }
        else if(KEY_UP == key)
        {
            mvwaddstr(win, CurPosY--, 2, g_pMenuStr[CurMenuPos--]);
        }        
        else if(27 == key) //Esc key
            return STAT_EXIT;
        else if(13 == key) //Enter key
            break;
        else
            continue;

        if(CurMenuPos < 0)
        {
            CurMenuPos = CTL_MENU_COUNT - 2;
            CurPosY = CurMenuPos + 2;
        }
        else if(CurMenuPos >= (CTL_MENU_COUNT-1))
        {
            CurMenuPos = 0;
            CurPosY = 2;
        }

        wattron(win, A_REVERSE);
        mvwaddstr(win, CurPosY, 2, g_pMenuStr[CurMenuPos]);
        wattroff(win, A_REVERSE);
    }

    *pFunc = CurMenuPos + 1;
    delwin(win);
    touchline(stdscr, (LINES-CTL_MENU_WIN_LINES)/2, CTL_MENU_WIN_LINES);
    refresh();
    
    return STAT_OK;
    
}

void CTL_ShowInstruction(void)
{
    WINDOW *win = newwin(LINES, COLS, 0, 0);
    char **ptr = pInstruction;

    CTL_SET_WIN_COLOR(win, CTL_PANEL_GREEN);
    wattron(win, A_REVERSE);
    mvwaddstr(win, 0, (COLS - strlen(*ptr))/2, *ptr++);
    wattroff(win, A_REVERSE);

    wmove(win, 1, 0);
    while(*ptr != NULL)
    {
        wprintw(win, "%s\n", *ptr++);
    }

    wattron(win, A_REVERSE);
    mvwaddstr(win, LINES-1, 0, STR_GO_BACK);
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

G_STATUS CTL_GetFileName(char *pFileName)
{
    WINDOW *win = newwin(CTL_GET_FILE_NAME_WIN_LINES, CTL_GET_FILE_NAME_WIN_COLS, 
        (LINES-CTL_GET_FILE_NAME_WIN_LINES)/2, (COLS-CTL_GET_FILE_NAME_WIN_COLS)/2);
    CTL_SET_WIN_COLOR(win, CTL_PANEL_CYAN);

    G_STATUS status;
    keypad(win, false);    
    
    while(1)
    {
        echo();
        CTL_HIDE_CONSOLE_END_LINE(); //shield the end line of standard screen 
        
        mvwhline(win, 0, 0, '-', CTL_GET_FILE_NAME_WIN_COLS);
        mvwhline(win, CTL_GET_FILE_NAME_WIN_LINES-1, 0, '-', CTL_GET_FILE_NAME_WIN_COLS);
        
        mvwaddstr(win, 1, 0, STR_INPUT_FILE_NAME);
        mvwaddstr(win, 2, 0, STR_INPUT_FILE_NAME_EG);
        mvwaddstr(win, 3, 0, STR_INPUT);
        mvwgetnstr(win, 3, sizeof(STR_INPUT)-1, pFileName, CTL_FILE_NAME_LENGHT-1);
        if(0 == access(pFileName, F_OK))
            break;

        CTL_SHOW_CONSOLE_END_LINE();
        status = CTL_MakeChoice("%s", STR_FILE_NOT_EXIST);
        if(STAT_EXIT == status)
        {
            delwin(win);
            return STAT_EXIT;
        }
        else if(STAT_GO_BACK == status)
        {
            noecho();
            delwin(win);
            touchline(stdscr, (LINES-CTL_GET_FILE_NAME_WIN_LINES)/2, 
                CTL_GET_FILE_NAME_WIN_LINES);
            CTL_SHOW_CONSOLE_END_LINE();
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

G_STATUS CTL_GetPassord(char *pPassword)
{
    WINDOW *win = newwin(CTL_GET_PASSWORD_WIN_LINES, CTL_GET_PASSWORD_WIN_COLS, 
        (LINES-CTL_GET_PASSWORD_WIN_LINES)/2, (COLS-CTL_GET_PASSWORD_WIN_COLS)/2);
    CTL_SET_WIN_COLOR(win, CTL_PANEL_CYAN);

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
                delwin(win);
                return STAT_EXIT;
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
            if(STAT_EXIT == status)
            {
                delwin(win);
                return STAT_EXIT;
            }
            else if(STAT_GO_BACK == status)
            {
                delwin(win);
                touchline(stdscr, (LINES-CTL_GET_PASSWORD_WIN_LINES)/2, 
                    CTL_GET_PASSWORD_WIN_LINES);
                refresh();
                return STAT_GO_BACK;
            }
            else if(STAT_RETRY == status)
            {
                wclear(win);
                continue;
            }
            else
                return status;
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
                delwin(win);
                return STAT_EXIT;
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
        if(STAT_EXIT == status)
        {
            delwin(win);
            return STAT_EXIT;
        }
        else if(STAT_GO_BACK == status)
        {
            delwin(win);
            touchline(stdscr, (LINES-CTL_GET_PASSWORD_WIN_LINES)/2, 
                CTL_GET_PASSWORD_WIN_LINES);
            refresh();
            return STAT_GO_BACK;
        }
        else if(STAT_RETRY == status)
        {
            wclear(win);
            continue;
        }
        else
            return status;
    }

    delwin(win);
    touchline(stdscr, (LINES-CTL_GET_PASSWORD_WIN_LINES)/2, 
        CTL_GET_PASSWORD_WIN_LINES);
    refresh();
    return STAT_OK;
}



/*
    rules: "%s" means string
    e.g: CTL_MakeChoice("%s%s", "It occurs fatal error", "Please make a choice");
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

    //parse format
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
                    endwin();
                    DISP_ERR(STR_ERR_FAIL_TO_MALLOC);
                    return STAT_ERR;
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

    lines += 5;
    if(lines > CTL_CONSOLE_LINES)
    {
        FreePtrLinkList(&StrHeadNode);
        endwin();
        DISP_ERR_PLUS("%s%d\n", STR_ERR_INVALID_LINES, lines);
        return STAT_ERR;
    }

    cols += 4;
    if(cols > CTL_CONSOLE_COLS)
    {
        FreePtrLinkList(&StrHeadNode);
        DISP_ERR_PLUS("%s%d\n", STR_ERR_INVALID_COLS, cols);
        return STAT_ERR;
    }
#ifdef __DEBUG    
    if(lines < 6) //top and bottom symbols is 4 lines, choice string is 1 lines
    {
        FreePtrLinkList(&StrHeadNode);
        DISP_ERR_PLUS("%s%d\n", STR_ERR_STR_IS_NULL, lines);
        return STAT_ERR;
    }
    if(cols < 20)  //STR_RETRY and STR_BACK are 16 bytes in total, side symbols are 4 bytes
    {
        FreePtrLinkList(&StrHeadNode);
        DISP_ERR_PLUS("%s%d\n", STR_ERR_STR_TOO_SHORT, cols);
        return STAT_ERR;
    }
#endif

    WINDOW *win = newwin(lines, cols, (LINES-lines)/2, (COLS-cols)/2);
    CTL_SET_WIN_COLOR(win, CTL_PANEL_YELLOW);
    wborder(win, '*', '*', '*', '*', '*', '*', '*', '*');

    int CurPosY = 2;
    pStrNode = StrHeadNode.pNext_t;
    while(pStrNode != NULL)
    {
        mvwaddstr(win, CurPosY++, 2, pStrNode->ptr);
        pStrNode = pStrNode->pNext_t;
    }

    int Str1StartX = (cols-16)/3; //STR_RETRY and STR_BACK are about 16 bytes
    int Str2StartX = cols - Str1StartX - 8; //STR_BACK is 8 bytes
    int StartPosY = lines-1-2; //bottom symbols are 2 lines, and the line counts from 0
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
            FreePtrLinkList(&StrHeadNode);
            delwin(win);
            return STAT_EXIT;
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

    delwin(win);
    touchline(stdscr, (LINES-lines)/2, lines);
    refresh();

    FreePtrLinkList(&StrHeadNode);

    if(flag)
        return STAT_GO_BACK;

    return STAT_RETRY;
}