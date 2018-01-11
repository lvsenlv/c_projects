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
    "1. Hardware character:",
    "   Please adopt the adapted version of software",
    "   E.g. encrypt_win10_4th_400Mb :",
    "   win10 - Software can be run on Windows10",
    "   4th   - Software adopts 4 threads to run",
    "   400Mb - Software requires 400Mb running momery space at least",
    "",
    "2. \"Not storage\"  character:",
    "   Any input password can be used to encrypt or decrypt",
    "   You would get corrupted file if use non-corresponding password to decrypt",
    "",
    "Tips:",
    "   * It's unable to crack, please confirm your password would not be forgot",
    "",
    "   * If make pristine encrypt file lose by using incorrect password to decrypt",
    "     You can use that password to encrypt the file for solving this problem",
    "",
    "With any questions, please contact to lvsen46000@163.com",
    NULL,
};

#ifdef __LINUX
char *ChInstruction[] = { 
    "请仔细阅读",
    "1. 硬件特性: ",
    "   请根据不同的硬件配置，选择不同的版本使用，以保证高性能及高稳定性",
    "   如 encrypt_win10_4th_400Mb :",
    "   win10 - 程序仅可于 Windows10 平台使用",
    "   4th   - 程序采用 4 线程进行加密",
    "   400Mb - 程序大约需要占用 400Mb 运行内存",
    "",
    "2. \"无存储\"特性: ",
    "   任意输入的密码均可进行加密或解密",
    "   若解密密码与加密密码不一致, 将得到损坏的文件",
    "",
    "***温馨提示***",
    "   * 此程序所得的加密结果, 无法破解, 请务必牢记您的加密时输入的密码",
    "",
    "   * 倘若您因为输入了与加密不一致的解密密码, 并解密成功, 导致原始加密文件被删除",
    "     请使用该解密密码, 再对解密后的文件进行一次加密即可恢复",
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
    "   任意输入的密码均可进行加密或解密",
    "   若解密密码与加密密码不一致, 将得到损坏的文件",
    "",
    "***温馨提示***",
    "   * 此程序所得的加密结果, 无法破解, 请务必牢记您的加密时输入的密码",
    "     若遗忘, 则将永久无法解密得到正确格式的文件",
    "",
    "   * 倘若您因为输入了与加密不一致的解密密码, 并解密成功, 导致原始加密文件被删除",
    "     请使用该解密密码, 再对解密后的文件进行一次加密即可恢复",
    "",
    "   * 请使用对运行内存要求相同的版本进行解密, 否则将得到损坏的文件",
    "     如, 使用 encrypt_win10_4th_400Mb 进行加密",
    "     则必须使用 encrypt_***_***_400Mb 的版本进行解密",
    "",
    "如有任何问题, 请发送邮件至lvsen46000@163.com ",
    NULL,
};
#endif

void InitStr(void)
{
    g_LanguageFlag = 0;
}
