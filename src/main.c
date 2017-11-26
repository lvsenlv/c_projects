/*************************************************************************
	> File Name: main.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 9th,2017 Wednesday 08:48:52
 ************************************************************************/

#include "control.h"
#include "encryption.h"
#include "extra.h"

int main(void)
{
#ifdef __WINDOWS
    system("mode con lines=48 cols=120");
#endif

    LoginAdministrator();

    CTL_MENU func = CTL_MENU_MAX;

    CTL_InitConsole();
#ifdef __LINUX
    CTL_ChooseLanguage();
#endif

    while(1)
    {
        CTL_DrawStdConsole();
        CTL_ShowMenu(&func);
        
        CTL_HIDE_CONSOLE_END_LINE(); //Shield the end line of standard screen 
        switch(func)
        {
            case CTL_MENU_SHOW_INSTRUCTION:
                CTL_ShowInstruction();
                break;
            case CTL_MENU_CHANGE_LANGUAGE:
                CTL_ChooseLanguage();
                break;
            case CTL_MENU_ENCRYPT:
            case CTL_MENU_DECRYPT:
                CTL_EncryptDecrypt(func);
            default :
                break;
        }
        CTL_SHOW_CONSOLE_END_LINE();
    }
 
    CTL_ForceExit();
} 
