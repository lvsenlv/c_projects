/*************************************************************************
	> File Name: main.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 9th,2017 Wednesday 08:48:52
 ************************************************************************/

#include "common.h"
#include "control.h"
#include "str.h"
#include "encryption.h"

int main(void)
{    
    G_STATUS status;
    char func = 0;

    status = CTL_InitConsole();
    if(status != STAT_OK)
        exit(0);

    while(1)
    {
        status = CTL_ShowMenu(&func);
        if(status != STAT_OK)
            CTL_ExitConsole();

        if(CTL_MENU_SHOW_INSTRUCTION == func)
            CTL_ShowInstruction();
        else if((CTL_MENU_ENCRYPT == func) || (CTL_MENU_DECRYPT == func))
        {
            status = EncryptDecrypt(func);
            if(STAT_EXIT == status)
            {
                CTL_ExitConsole();
            }            
            else if(STAT_ERR == status)
            {
                exit(0);
            }
            else if(STAT_GO_BACK == status)
            {
                continue;
            }
        }
    }
    
    while(1);
    
    CTL_ExitConsole();
} 


