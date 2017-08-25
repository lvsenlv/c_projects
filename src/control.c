/*************************************************************************
	> File Name: control.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 9th,2017 Wednesday 08:49:11
 ************************************************************************/

#include "control.h"
#include "control_str.h"
#include <string.h>
#include <locale.h>
#include <sys/stat.h>
#include <unistd.h>


char **g_pStr = pEnStr;
char g_FlagLanguage = 0; //0 is English and 1 is Chinese

void CTL_InitConsole(void)
{
    setlocale(LC_ALL,"");
    initscr();
    cbreak();
    nonl();
    noecho();
    keypad(stdscr, true);
    attron(A_REVERSE | A_BOLD);
    mvaddnstr(LINES-1, 0, g_pStr[CTL_STR_ESC_EXIT], COLS);
    attroff(A_REVERSE | A_BOLD);
    refresh();
}

void CTL_DrawStdScreen(void)
{
    clear();
    border('|', '|', '-', '-', '+', '+', '+', '+');
    int LabelSize = strlen(g_pStr[CTL_STR_LABEL]);

    attron(A_REVERSE);
    if(g_FlagLanguage)
    {
        mvaddstr(0, (COLS-LabelSize*2/3)/2, g_pStr[CTL_STR_LABEL]);
    }
    else
    {
        mvaddstr(0, (COLS-LabelSize)/2, g_pStr[CTL_STR_LABEL]);
    }

    attron(A_BOLD);
    mvaddstr(LINES-2, 2, g_pStr[CTL_STR_END_LINE]);
    attroff(A_REVERSE | A_BOLD);
    refresh();
}


G_STATUS CTL_ChooseLanguage(void)
{
    int lines = 6, cols = 20;
    WINDOW *win = newwin(lines, cols, (LINES-lines)/2, (COLS-cols)/2);
    wborder(win, '+', '+', '-', '-', '+', '+', '+', '+');

    int EnStrStartX = 0, ChStrStartX = 0;
    EnStrStartX = (cols - (sizeof(CTL_STR_CHOOSE_LANG_EN)-1)) / 2;
    ChStrStartX = (cols - ((sizeof(CTL_STR_CHOOSE_LANG_CH)-1)*2/3)) / 2;
    mvwaddstr(win, 3, ChStrStartX, CTL_STR_CHOOSE_LANG_CH);
    wattron(win, A_REVERSE);
    mvwaddstr(win, 2, EnStrStartX, CTL_STR_CHOOSE_LANG_EN);
    wattroff(win, A_REVERSE);

    keypad(win, true);
    int key = 0;
    char flag = 0;
    while(1)
    {
        key = wgetch(win);
        
        if((KEY_UP == key) || (KEY_DOWN == key))
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
            mvwaddstr(win, 2, EnStrStartX, CTL_STR_CHOOSE_LANG_EN);
            wattron(win, A_REVERSE);
            mvwaddstr(win, 3, ChStrStartX, CTL_STR_CHOOSE_LANG_CH);
            wattroff(win, A_REVERSE);
        }
        else
        {
            mvwaddstr(win, 3, ChStrStartX, CTL_STR_CHOOSE_LANG_CH);
            wattron(win, A_REVERSE);
            mvwaddstr(win, 2, EnStrStartX, CTL_STR_CHOOSE_LANG_EN);
            wattroff(win, A_REVERSE);
        }
    }

    g_pStr = (1 == flag) ? pChStr : pEnStr;
    g_FlagLanguage = flag;
    
    delwin(win);
    touchline(stdscr, (LINES-lines)/2, lines);
    refresh();
    return STAT_OK;
}

G_STATUS CTL_ChooseFunc(char *pFunc)
{
    mvaddstr(2, 2, g_pStr[CTL_STR_MENU_FUNC2]);
    mvaddstr(3, 2, g_pStr[CTL_STR_MENU_FUNC3]);
    mvaddstr(4, 2, g_pStr[CTL_STR_MENU_FUNC4]);
    attron(A_REVERSE);
    mvaddstr(1, 2, g_pStr[CTL_STR_MENU_FUNC1]);
    attroff(A_REVERSE);
    refresh();

    int key;
    int CurPosY = 1;
    while(1)
    {
        key = getch();

        if(KEY_DOWN == key)
        {
            mvaddstr(CurPosY, 2, g_pStr[CTL_STR_MENU_FUNC1 - 1 + CurPosY]);
            CurPosY++;
        }
        else if(KEY_UP == key)
        {
            mvaddstr(CurPosY, 2, g_pStr[CTL_STR_MENU_FUNC1 - 1 + CurPosY]);
            CurPosY--;
        }        
        else if(27 == key) //Esc key
            return STAT_EXIT;
        else if(13 == key) //Enter key
            break;
        else
            continue;

        if(0 == CurPosY)
        {
            CurPosY = CTL_MENU_FUNC_NUM - 1;
        }
        else if(CurPosY >= CTL_MENU_FUNC_NUM)
        {
            CurPosY = 1;
        }

        attron(A_REVERSE);
        mvaddstr(CurPosY, 2, g_pStr[CTL_STR_MENU_FUNC1 - 1 + CurPosY]);
        attroff(A_REVERSE);
    }

    *pFunc = (char)CurPosY;
    return STAT_OK;
}

void CTL_ShowInstruction(void)
{
    WINDOW *win = newwin(LINES, COLS, 0, 0);

    char **ptr;
    wattron(win, A_REVERSE);
    if(g_FlagLanguage)
    {
        ptr = pChInstruction;
        mvwaddstr(win, 0, (COLS - strlen(*ptr)*2/3)/2, *ptr);
    }
    else
    {
        ptr = pEnInstruction;
        mvwaddstr(win, 0, (COLS - strlen(*ptr))/2, *ptr);        
    }
    ptr++;
    wattroff(win, A_REVERSE);

    wmove(win, 1, 0);
    while(*ptr != NULL)
    {
        wprintw(win, "%s\n", *ptr++);
    }

    wattron(win, A_REVERSE);
    mvwaddstr(win, LINES-1, 0, g_pStr[CTL_STR_ENTER_KEY_GO_BACK]);
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
    int lines = 7, cols = strlen(g_pStr[CTL_STR_INPUT_FILE_NAME]);
    if(g_FlagLanguage)
    {
        cols = cols * 2 / 3;
    }

    WINDOW *win = newwin(lines, cols, (LINES-lines)/2, (COLS-cols)/2);

    G_STATUS status;
    while(1)
    {
        mvhline(LINES-2, 2, ' ', COLS-3);
        refresh();
            
        mvwhline(win, 0, 0, '*', cols);
        mvwhline(win, 6, 0, '*', cols);

        echo();
        keypad(win, false);
        mvwaddstr(win, 1, 0, g_pStr[CTL_STR_INPUT_FILE_NAME]);
        mvwaddstr(win, 2, 0, g_pStr[CTL_STR_INPUT_FILE_NAME_EG]);
        mvwaddstr(win, 3, 0, g_pStr[CTL_STR_INPUT]);
        mvwgetnstr(win, 3, strlen(g_pStr[CTL_STR_INPUT]), pFileName, CTL_FILE_NAME_LENGHT-1);
        if(0 == access(pFileName, F_OK))
            break;

        attron(A_REVERSE | A_BOLD);
        mvaddstr(LINES-2, 2, g_pStr[CTL_STR_END_LINE]);
        attroff(A_REVERSE | A_BOLD);
        refresh();

        status = CTL_MakeChoice(NULL, lines, cols, g_pStr[CTL_STR_FILE_NOT_EXIST]);
        if(STAT_EXIT == status)
        {
            delwin(win);
            return STAT_EXIT;
        }
        else if(STAT_GO_BACK == status)
        {
            delwin(win);
            touchline(stdscr, (LINES-lines)/2, lines);
            attron(A_REVERSE | A_BOLD);
            mvaddstr(LINES-2, 2, g_pStr[CTL_STR_END_LINE]);
            attroff(A_REVERSE | A_BOLD);
            refresh();
            return STAT_GO_BACK;
        }

        wclear(win);
    }

    delwin(win);
    touchline(stdscr, (LINES-lines)/2, lines);
    attron(A_REVERSE | A_BOLD);
    mvaddstr(LINES-2, 2, g_pStr[CTL_STR_END_LINE]);
    attroff(A_REVERSE | A_BOLD);
    refresh();

    return STAT_OK;
}

G_STATUS CTL_GetPassord(char *pPassword)
{
    int lines = 6, cols = strlen(g_pStr[CTL_STR_INPUT_PASSWORD_CONFIRM]);
    if(g_FlagLanguage)
        cols = cols * 2 / 3;
    cols += CTL_PASSWORD_LENGHT_MAX;

    WINDOW *win = newwin(lines, cols, (LINES-lines)/2, (COLS-cols)/2);

    char buf[CTL_PASSWORD_LENGHT_MAX];
    int key, i;
    int CurPosX;
    G_STATUS status;
    keypad(win, true);
    noecho();
    while(1)
    {
        mvwhline(win, 0, 0, '*', cols);
        mvwhline(win, 5, 0, '*', cols);

        i = 0;
        CurPosX = strlen(g_pStr[CTL_STR_INPUT_PASSWORD]);
        if(g_FlagLanguage)
            CurPosX = CurPosX * 2 / 3;
        mvwaddstr(win, 3, 0, g_pStr[CTL_STR_INPUT_PASSWORD_CONFIRM]);
        mvwaddstr(win, 2, 0, g_pStr[CTL_STR_INPUT_PASSWORD]);
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
            else if(13 == key) //Enter key
                break;
            else 
                continue;
            
            wmove(win, 2, CurPosX);
        }
        pPassword[i] = '\0';

        if(i < CTL_PASSWORD_LENGHT_MIN)
        {
            status = CTL_MakeChoice(win, lines, cols, g_pStr[CTL_STR_PASSWORD_TOO_SHORT]);
            if(STAT_EXIT == status)
            {
                delwin(win);
                return STAT_EXIT;
            }
            else if(STAT_GO_BACK == status)
            {
                delwin(win);
                touchline(stdscr, (LINES-lines)/2, lines);
                refresh();
                return STAT_GO_BACK;
            }
            wclear(win);
            continue;
        }

        i = 0;
        CurPosX = cols-CTL_PASSWORD_LENGHT_MAX;
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

        status = CTL_MakeChoice(win, lines, cols, g_pStr[CTL_STR_PASSWORD_NOT_MATCH]);
        if(STAT_EXIT == status)
        {
            delwin(win);
            return STAT_EXIT;
        }
        else if(STAT_GO_BACK == status)
        {
            delwin(win);
            touchline(stdscr, (LINES-lines)/2, lines);
            refresh();
            return STAT_GO_BACK;
        }
        
        wclear(win);
    }

    delwin(win);
    touchline(stdscr, (LINES-lines)/2, lines);
    refresh();
    return STAT_OK;
}

G_STATUS CTL_MakeChoice(WINDOW *pWin, int lines, int cols, char *pStr)
{
    if(cols < 18) //CTL_STR_RETRY and CTL_STR_BACK are about 12 bytes. Rest 6 bytes is space
        cols = COLS/2;

    WINDOW *win = newwin(lines, cols, (LINES-lines)/2, (COLS-cols)/2);
    wborder(win, '#', '#', '#', '#', '#', '#', '#', '#');

    int Str1StartX = (cols-12)/3; //CTL_STR_RETRY and CTL_STR_BACK are about 12 bytes
    int Str2StartX = cols - Str1StartX - 6;
    int StartPosY = (lines-2)/2 + 1;
    if(g_FlagLanguage)
    {
        mvwaddstr(win, (lines-2)/2, (cols - strlen(pStr)*2/3)/2, pStr);
    }
    else
    {
        mvwaddstr(win, (lines-2)/2, (cols-strlen(pStr))/2, pStr);
    }
    mvwaddstr(win, StartPosY, Str2StartX, g_pStr[CTL_STR_BACK]);
    wattron(win, A_REVERSE);
    mvwaddstr(win, StartPosY, Str1StartX, g_pStr[CTL_STR_RETRY]);
    wattroff(win, A_REVERSE);

    int flag = 0;
    int key;
    noecho();
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
            delwin(win);
            return STAT_EXIT;
        }
        else if(13 == key) //Enter key
            break;
        else
            continue;

        if(flag)
        {
            mvwaddstr(win, StartPosY, Str1StartX, g_pStr[CTL_STR_RETRY]);
            wattron(win, A_REVERSE);
            mvwaddstr(win, StartPosY, Str2StartX, g_pStr[CTL_STR_BACK]);
            wattroff(win, A_REVERSE);
        }
        else
        {
            mvwaddstr(win, StartPosY, Str2StartX, g_pStr[CTL_STR_BACK]);
            wattron(win, A_REVERSE);
            mvwaddstr(win, StartPosY, Str1StartX, g_pStr[CTL_STR_RETRY]);
            wattroff(win, A_REVERSE);
        }
    }

    delwin(win);
    touchline(stdscr, (LINES-lines)/2, lines);
    refresh();
    touchline(pWin, (LINES-lines)/2, lines);
    wrefresh(pWin);

    if(flag)
        return STAT_GO_BACK;

    return STAT_RETRY;
}

