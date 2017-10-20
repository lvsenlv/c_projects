/*************************************************************************
	> File Name: control_str.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 18th,2017 Friday 08:51:45
 ************************************************************************/

#include <stdio.h>
#include "str.h"

char **g_menu = NULL;
char g_LanguageFlag; //0 is English, 1 is Chinese

char *g_EnMenu[] = {
    "1. Instruction",
    "2. Encrypt",
    "3. Decrypt",
    "4. Change language",
    NULL,
};

char *g_ChMenu[] = {
    "1. 说明",
    "2. 加密",
    "3. 解密",
    "4. 变更语言",
    NULL,
};

char *EnInstruction[] = {
    "Please read it carefully",
    "1. This project supports Windows or Linux, and all functions are in developing.",
    "",
    "2. This project adopts \"non-storage\" way, i.e, don't save user's password.",
    "   It will generate the factor to encrypt the file based on password.",
    "   Decryption is same with encryption. Thus, arbitrary password can decrypt,",
    "   you will get corrupted files if password differs from that used to encrypt.",
    "   Note: it is unable to decrypte foever if the password is lost.",
    "",
    "3. To ensure data security, this program will generate decrypted files,",
    "   i.e, the encrypted file will not be deleted.",
#ifdef __LINUX    
    "   You can run \"clean.sh\" to remove all encrypted files.",
#elif defined __WINDOWS
    "   You can run \"clean.bat\" to remove all encrypted files.",
#endif
    "   Note: when decrypted, the source file would be coverd if exist.",
    "",
    "Tips:",
    "   It is not recommend that Encrypt multi files by running encrypt single files for many times, "
    "i.e, encrypt multi files using different passwords.",
    "   Note: it is easy to be confused and fogotten if there are lots of passwords.",
    "",
    "With any questions, please contact to lvsen46000@163.com",
    NULL,
};

char *ChInstruction;

void InitStr(void)
{
    g_menu = g_EnMenu;
    g_LanguageFlag = 0;
}
