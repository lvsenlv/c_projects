/*************************************************************************
	> File Name: control_str.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: August 18th,2017 Friday 08:51:45
 ************************************************************************/

#include <stdio.h>
#include "common.h"
#include "str.h"

char g_LanguageFlag = LAN_EN;

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

#ifdef __LINUX
char *ChInstruction[] = { 
    "请仔细阅读",
    "",
    "如有任何问题, 请发送邮件至lvsen46000@163.com ",
    NULL,
};
#elif defined __WINDOWS
char *ChInstruction[] = {
    "请仔细阅读",
    "1. 硬件特性: ",
    "   请根据不同的硬件配置，选择不同的版本使用，以保证高性能及高稳定性",
    "   如 encrypt_win10_4th_400Mb :",
    "   win10 - 程序仅可于 Windows10 平台使用",
    "   4th   - 程序采用 4 线程进行加密",
    "   400Mb - 程序大约需要占用 400Mb 运行内存",
    "   注意: ",
    "   打开\"设备管理器->处理器\", 请根据其中列出的 CPU 数量，选择对应数量的线程版本",
    "   请确保空闲的运行内存大于程序所要求的大小",
    "",
    "2. \"无存储\"特性: ",
    "   程序不会存储用户密码, 程序会根据用户输入的密码, 生成对应的\"因子\" ",
    "   并采用该因子进行加密或解密",
    "   注意: ",
    "   任意输入的密码均可进行加密或解密, 若解密与加密密码不一致, 将得到错误的解密结果",
    "",
    "3. \"安全\"特性: ",
    "   完成加密后, 若删除原先的旧文件失败, 原先的文件不受影响",
    "   完成解密后, 若删除对应的加密文件失败, 原先加密的文件不受影响",
    "",
    "***温馨提示***",
    "   * 此程序所得的加密结果, 无法破解, 请务必牢记您的加密时输入的密码",
    "     若遗忘, 则将永久无法解密, 若您介意, 请选用 plus 版本进行加密",
    "",
    "   * 倘若您因为输入了不一致的解密密码, 并解密成功, 导致原始加密文件被删除",
    "     请使用该解密密码, 再对解密后的文件进行一次加密即可恢复",
    "",
    "如有任何问题, 请发送邮件至lvsen46000@163.com ",
    NULL,
};
#endif

void InitStr(void)
{
    g_LanguageFlag = 0;
}
