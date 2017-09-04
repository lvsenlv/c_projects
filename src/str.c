/*************************************************************************
	> File Name: control_str.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 18th,2017 Friday 08:51:45
 ************************************************************************/

#include <stdio.h>
#include "str.h"

char *g_pMenuStr[] = {
    "1. Instruction",
    "2. Encrypt",
    "3. Decrypt",
    NULL,
};

char *pEnStr[] = {
    "Press Esc to exit project",
    "Encryption System",
    "Press Esc to exit project | Version 1.0.0 | Copyright 2017-2017",
    "1. Instruction",
    "2. Encrypt",
    "3. Decrypt",
    "4. Change language",    
    "Please input the file or folder name with full path",
#ifdef __LINUX
    "E.g, /root/document/...",
#elif defined __WINDOWS
    "E.g, C:/User/Administrator/...",
#endif    
    "Input: ",
    "[Retry]",
    "[Back]",
    "The file does not exist",
    "Password: ",
    "Password too short, at least 8 bits",
    "Confirm password: ",
    "Two password do not match",
    "[Enter key to go back]",
    "Encrypt : ",
    "Decrypt : ",
    "Press any key to continue",
    "[Success]",
    "Error : Fail to get file info",
    "Error : Fail to open file",
    "Error : Not enough memory space for running",
    "Error: Fail to read file, make sure it is readable",
    "Error: Length of key could not be null",
    "Error: Fail to create or open file",
    "Error: Fail to write to file, make sure it is writeable",
    "Error: Fail to delete old file",
};

char *pInstruction[] = {
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

