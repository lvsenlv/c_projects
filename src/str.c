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
    "2. This project adopt  \"non-storage\" way, i.e don't save user's password.",
    "   It will generate the factor to encrypt based on password.",
    "   Decrption is the same with encryption. Thus, arbitrary password can decrypt,",
    "   you will get corrupted files if password differs from that used to encrypt.",
    "   因此，任意密码均可解密，但与加密密码不一致时，将得到损坏的文件，",
    "   故，请牢记您的密码，密码丢失将永久无法解密；",
    "3. 为保证数据的安全性，本软件仅在原先文件的基础上生成加密文件，",
    "   原先文件依旧存在，可通过新的加密文件，解密得到原先的文件，",
    "   建议用户完成加密后，手动将旧文件删除，否则解密时会将其覆盖，",
    "   同理，解密完成后，确认得到正确文件后，建议删除原先的加密文件，",
    "   用户可通过菜单下的对应选项，进行快速清理旧文件；",
    "",
    "温馨提示：",
    "   本软件可加密单个文件，即可针对不同文件使用不同的加密密码，",
    "   但不建议这么做，过多的密码易混淆且遗忘，易造成无法解密",
    "",
    "若有任何使用问题，请发送邮件至lvsen46000@163.com",
    NULL,
};

