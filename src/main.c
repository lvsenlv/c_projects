/*************************************************************************
	> File Name: main.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 9th,2017 Wednesday 08:48:52
 ************************************************************************/

#include "control.h"
#include "encryption.h"
#include "extra.h"

static inline void CheckError(G_STATUS status);

int main(void)
{
    char buf[128];
    LoginAdministrator();

    G_STATUS status;
    CTL_MENU func = CTL_MENU_MAX;

    CTL_InitConsole();
    CTL_ChooseLanguage();

    while(1)
    {
        CTL_DrawStdConsole();
        CTL_ShowMenu(&func);
        
        CTL_HIDE_CONSOLE_END_LINE(); //Shield the end line of standard screen 
        
        if(CTL_MENU_CHANGE_LANGUAGE == func)
        {
            CTL_ChooseLanguage();
        }
        else if(CTL_MENU_SHOW_INSTRUCTION == func)
        {
            CTL_ShowInstruction();
        }
        
        CTL_GetPassord(buf);
        CTL_GetFileName(buf);
        CTL_GetPassord(buf);
        CTL_GetFileName(buf);
        
        CTL_SHOW_CONSOLE_END_LINE();

//        if(CTL_MENU_SHOW_INSTRUCTION == func)
//        {
//            CTL_MENU_ShowInstruction();
//            continue;
//        }
//        else if((CTL_MENU_ENCRYPT == func) || (CTL_MENU_DECRYPT == func))
//        {
//            status = CTL_MENU_EncryptDecrypt(func);
//            CheckError(status);
//            if(STAT_EXIT == status)
//            {
//                CTL_SafeExit(stdscr);
//            }
//            else if(STAT_GO_BACK == status)
//            {
//                continue;
//            }
//        }
    }
    
    CTL_SafeExit(stdscr);
} 

static inline void CheckError(G_STATUS status)
{
    if(STAT_ERR == status)
    {
        endwin();
        fprintf(stderr, "%s", g_ErrBuf);
        exit(-1);
    }
}

