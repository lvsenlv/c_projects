/*************************************************************************
	> File Name: control_str.c
	> Author: lvsenlv
	> Mail: lvsen46000@163.com
	> Created Time: 2017年08月18日 星期五 08时51分45秒
 ************************************************************************/

#include "common.h"
#include "control_str.h"

char *pEnStr[] = {
    "Press Esc to exit project",
    "Encryption System",
    "Press Esc to exit project | Version 1.0.0 | Copyright 2017-2017",
    "1. Instruction",
    "2. Encrypt single file",
    "3. Encrypt folder",
    "4. Decrypt single file",
    "5. Decrypt folder",
    "6. Change language",    
    "Please input the file name with full path",
#ifdef __LINUX
    "E.g, /root/document/file.txt",
#elif defined __WINDOWS
    "E.g, C:/User/Administrator/file.txt",
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

char *pChStr[] = {
    "按Esc键退出程序",
    "加密系统",
    "按Esc键退出程序 | 版本号 1.0.0 | Copyright 2017-2017",
    "1. 使用说明", 
    "2. 加密单个文件",
    "3. 加密文件夹",
    "4. 解密单个文件",
    "5. 解密文件夹",
    "6. 更改语言",       
    "请输入文件名，若不在当前文件夹则需完整路径",
#ifdef __LINUX
    "例如, /root/document/文件.txt",
#elif defined __WINDOWS
    "例如, C:/User/Administrator/文件.txt",
#endif
    "输入: ",
    "[重试]",
    "[返回]",
    "此文件不存在",
    "密码：",
    "密码太短，至少8位",
    "确认密码：",
    "两次密码不匹配",
    "[按Enter键返回]",
    "加密：",
    "解密：",
    "按任意键继续",
    "[成功]",
    "错误：无法获取文件信息",
    "错误：无法打开文件",
    "错误，运行内存空间不足",
    "错误：读取文件失败，请确保文件可读",
    "错误：密码长度不能为0",
    "错误：无法创建或打开文件",
    "错误：写入文件失败，请确保文件可写",
    "错误：无法删除旧文件",
};

char *pEnInstruction[] = {
    "No more info temporarily",
    NULL,
};

char *pChInstruction[] = {
    "数据无价，请认真阅读",
    "1. 此版本目前仅可用于Linux系统，所有功能均处于测试阶段；",
    "2. 本软件采用\"无存储\"式加密方法，即不存储用户密码，",
    "   软件会根据用户密码，生成加密因子进行加密，解密时同理，",
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

