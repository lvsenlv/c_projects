/*************************************************************************
	> File Name: control.h
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 2017年08月09日 星期三 08时49分29秒
 ************************************************************************/

#ifndef __CONTROL_H
#define __CONTROL_H

#include "common.h"
#ifdef __LINUX
#include <ncurses.h>
#elif defined __WINDOWS
#include "curses_win.h"
#endif

#define CTL_ExitConsole()                   endwin()
#define CTL_MakeCursorInvis()               {move(LINES-1, COLS-1);refresh();}

typedef enum
{
    CTL_MENU_FUNC_INSTRUCTION = 1,
    CTL_MENU_FUNC_ENCRYPT_FILE,
    CTL_MENU_FUNC_ENCRYPT_FOLDER,
    CTL_MENU_FUNC_DECRYPT_FILE,
    CTL_MENU_FUNC_DECRYPT_FOLDER,
    CTL_MENU_FUNC_CHOOSE_LANGUAGE,
    CTL_MENU_FUNC_NUM,
}CTL_MENU_FUNC;

#if defined __WINDOWS
enum 
{
    false,
    true,
};
#endif

extern char **g_pStr;

void CTL_InitConsole(void);
G_STATUS CTL_ChooseLanguage(void);
void CTL_DrawStdScreen(void);
G_STATUS CTL_ChooseFunc(char *pFunc);
G_STATUS CTL_ShowInstruction(void);
G_STATUS CTL_GetFileName(char *pFileName);
G_STATUS CTL_GetPassord(char *pPassword);
G_STATUS CTL_MakeChoice(WINDOW *pWin, int lines, int cols, char *pStr);

#endif

