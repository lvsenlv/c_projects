/*************************************************************************
	> File Name: str.h
	> Author: 
	> Mail: 
	> Created Time: August 10th,2017 Thursday 13:42:50
 ************************************************************************/

#ifndef __STR_H
#define __STR_H

#include "decoder.h"
#include <string.h>

#define STR_BACK                               ((0 == g_LanguageFlag) ? STR_EN_BACK                         : STR_CH_BACK                          )
#define STR_BEYOND_RANGE_OF_DISP               ((0 == g_LanguageFlag) ? STR_EN_BEYOND_RANGE_OF_DISP         : STR_CH_BEYOND_RANGE_OF_DISP          )
#define STR_CONSOLE_END_LINE                   ((0 == g_LanguageFlag) ? STR_EN_CONSOLE_END_LINE             : STR_CH_CONSOLE_END_LINE              )
#define STR_CONSOLE_LABEL                      ((0 == g_LanguageFlag) ? STR_EN_CONSOLE_LABEL                : STR_CH_CONSOLE_LABEL                 )
#define STR_CONTINUE                           ((0 == g_LanguageFlag) ? STR_EN_CONTINUE                     : STR_CH_CONTINUE                      )
#define STR_DISP_ERR_INFO                      ((0 == g_LanguageFlag) ? STR_EN_DISP_ERR_INFO                : STR_CH_DISP_ERR_INFO                 )
#define STR_ESC_EXIT                           ((0 == g_LanguageFlag) ? STR_EN_ESC_EXIT                     : STR_CH_ESC_EXIT                      )
#define STR_EXIT_PROJECT                       ((0 == g_LanguageFlag) ? STR_EN_EXIT_PROJECT                 : STR_CH_EXIT_PROJECT                  )
#define STR_FILE_IS_NULL                       ((0 == g_LanguageFlag) ? STR_EN_FILE_IS_NULL                 : STR_CH_FILE_IS_NULL                  )
#define STR_FILE_LIST_IS_NULL                  ((0 == g_LanguageFlag) ? STR_EN_FILE_LIST_IS_NULL            : STR_CH_FILE_LIST_IS_NULL             )
#define STR_FILE_NOT_EXIST                     ((0 == g_LanguageFlag) ? STR_EN_FILE_NOT_EXIST               : STR_CH_FILE_NOT_EXIST                )
#define STR_GO_BACK                            ((0 == g_LanguageFlag) ? STR_EN_GO_BACK                      : STR_CH_GO_BACK                       )
#define STR_IF_CONTINUE                        ((0 == g_LanguageFlag) ? STR_EN_IF_CONTINUE                  : STR_CH_IF_CONTINUE                   )
#define STR_IN_ENCRYPTING                      ((0 == g_LanguageFlag) ? STR_EN_IN_ENCRYPTING                : STR_CH_IN_ENCRYPTING                 )
#define STR_IN_DECRYPTING                      ((0 == g_LanguageFlag) ? STR_EN_IN_DECRYPTING                : STR_CH_IN_DECRYPTING                 )
#define STR_INPUT                              ((0 == g_LanguageFlag) ? STR_EN_INPUT                        : STR_CH_INPUT                         )
#define STR_INPUT_FILE_NAME                    ((0 == g_LanguageFlag) ? STR_EN_INPUT_FILE_NAME              : STR_CH_INPUT_FILE_NAME               )
#define STR_INPUT_FILE_NAME_EG                 ((0 == g_LanguageFlag) ? STR_EN_INPUT_FILE_NAME_EG           : STR_CH_INPUT_FILE_NAME_EG            )
#define STR_INPUT_PASSWORD                     ((0 == g_LanguageFlag) ? STR_EN_INPUT_PASSWORD               : STR_CH_INPUT_PASSWORD                )
#define STR_INPUT_PASSWORD_CONFIRM             ((0 == g_LanguageFlag) ? STR_EN_INPUT_PASSWORD_CONFIRM       : STR_CH_INPUT_PASSWORD_CONFIRM        )
#define STR_INVALID_FUNC                       ((0 == g_LanguageFlag) ? STR_EN_INVALID_FUNC                 : STR_CH_INVALID_FUNC                  )
#define STR_INVALID_PARAMTER                   ((0 == g_LanguageFlag) ? STR_EN_INVALID_PARAMETER            : STR_CH_INVALID_PARAMETER             )
#define STR_LANGUAGE                           ((0 == g_LanguageFlag) ? STR_EN_LANGUAGE                     : STR_CH_LANGUAGE                      )
#define STR_LOG_IS_NULL                        ((0 == g_LanguageFlag) ? STR_EN_LOG_IS_NULL                  : STR_CH_LOG_IS_NULL                   )
#define STR_MENU                               ((0 == g_LanguageFlag) ? STR_EN_MENU                         : STR_CH_MENU                          )
#define STR_NO                                 ((0 == g_LanguageFlag) ? STR_EN_NO                           : STR_CH_NO                            )
#define STR_NOT_A_FILE                         ((0 == g_LanguageFlag) ? STR_EN_NOT_A_FILE                   : STR_CH_NOT_A_FILE                    )
#define STR_NOT_NOT_SUPPORT_FUNCTION           ((0 == g_LanguageFlag) ? STR_EN_NOT_SUPPORT_FUNCTION         : STR_CH_NOT_SUPPORT_FUNCTION          )
#define STR_NULL                               ((0 == g_LanguageFlag) ? STR_EN_NULL                         : STR_CH_NULL                          )
#define STR_PASSWORD_NOT_MATCH                 ((0 == g_LanguageFlag) ? STR_EN_PASSWORD_NOT_MATCH           : STR_CH_PASSWORD_NOT_MATCH            )
#define STR_PASSWORD_NULL                      ((0 == g_LanguageFlag) ? STR_EN_PASSWORD_NULL                : STR_CH_PASSWORD_NULL                 )
#define STR_PASSWORD_TOO_SHORT                 ((0 == g_LanguageFlag) ? STR_EN_PASSWORD_TOO_SHORT           : STR_CH_PASSWORD_TOO_SHORT            )
#define STR_PRESS_ENTER_TO_GO_BACK             ((0 == g_LanguageFlag) ? STR_EN_PRESS_ENTER_TO_GO_BACK       : STR_CH_PRESS_ENTER_TO_GO_BACK        )
#define STR_PRO_REASON_NO_ENCRYPT_FILE         ((0 == g_LanguageFlag) ? STR_EN_PRO_REASON_NO_ENCRYPT_FILE   : STR_CH_PRO_REASON_NO_ENCRYPT_FILE    )
#define STR_PRO_REASON_NO_DECRYPT_FILE         ((0 == g_LanguageFlag) ? STR_EN_PRO_REASON_NO_DECRYPT_FILE   : STR_CH_PRO_REASON_NO_DECRYPT_FILE    )
#define STR_RATE                               ((0 == g_LanguageFlag) ? STR_EN_RATE                         : STR_CH_RATE                          )
#define STR_RETRY                              ((0 == g_LanguageFlag) ? STR_EN_RETRY                        : STR_CH_RETRY                         )
#define STR_SHOW_FILE_LABEL                    ((0 == g_LanguageFlag) ? STR_EN_SHOW_FILE_LABEL              : STR_CH_SHOW_FILE_LABEL               )
#define STR_SUCCESS                            ((0 == g_LanguageFlag) ? STR_EN_SUCCESS                      : STR_CH_SUCCESS                       )
#define STR_SUCCESS_COUNT                      ((0 == g_LanguageFlag) ? STR_EN_SUCCESS_COUNT                : STR_CH_SUCCESS_COUNT                 )
#define STR_TASK                               ((0 == g_LanguageFlag) ? STR_EN_TASK                         : STR_CH_TASK                          )
#define STR_TOTAL_FILE                         ((0 == g_LanguageFlag) ? STR_EN_TOTAL_FILE                   : STR_CH_TOTAL_FILE                    )
#define STR_VIEW_LOG                           ((0 == g_LanguageFlag) ? STR_EN_VIEW_LOG                     : STR_CH_VIEW_LOG                      )
#define STR_YES                                ((0 == g_LanguageFlag) ? STR_EN_YES                          : STR_CH_YES                           )
                                                                                                                                                  
#define STR_FAIL                               ((0 == g_LanguageFlag) ? STR_EN_FAIL                         : STR_CH_FAIL                          )
#define STR_FAIL_COUNT                         ((0 == g_LanguageFlag) ? STR_EN_FAIL_COUNT                   : STR_CH_FAIL_COUNT                    )
#define STR_FAIL_TO_CREATE_OPEN_FILE           ((0 == g_LanguageFlag) ? STR_EN_FAIL_TO_CREATE_OPEN_FILE     : STR_CH_FAIL_TO_CREATE_OPEN_FILE      )
#define STR_FAIL_TO_CREATE_PTHREAD             ((0 == g_LanguageFlag) ? STR_EN_FAIL_TO_CREATE_PTHREAD       : STR_CH_FAIL_TO_CREATE_PTHREAD        )
#define STR_FAIL_TO_DELETE_OLD_FILE            ((0 == g_LanguageFlag) ? STR_EN_FAIL_TO_DELETE_OLD_FILE      : STR_CH_FAIL_TO_DELETE_OLD_FILE       )
#define STR_FAIL_TO_GET_FILE_FOLDER_INFO       ((0 == g_LanguageFlag) ? STR_EN_FAIL_TO_GET_FILE_FOLDER_INFO : STR_CH_FAIL_TO_GET_FILE_FOLDER_INFO  )
#define STR_FAIL_TO_GET_FILE_INFO              ((0 == g_LanguageFlag) ? STR_EN_FAIL_TO_GET_FILE_INFO        : STR_CH_FAIL_TO_GET_FILE_INFO         )
#define STR_FAIL_TO_OPEN_FILE                  ((0 == g_LanguageFlag) ? STR_EN_FAIL_TO_OPEN_FILE            : STR_CH_FAIL_TO_OPEN_FILE             )
#define STR_FAIL_TO_OPEN_LOG_FILE              ((0 == g_LanguageFlag) ? STR_EN_FAIL_TO_OPEN_LOG_FILE        : STR_CH_FAIL_TO_OPEN_LOG_FILE         )
#define STR_FAIL_TO_READ_FILE                  ((0 == g_LanguageFlag) ? STR_EN_FAIL_TO_READ_FILE            : STR_CH_FAIL_TO_READ_FILE             )
#define STR_FAIL_TO_SCAN_DIRECTORY             ((0 == g_LanguageFlag) ? STR_EN_FAIL_TO_SCAN_DIRECTORY       : STR_CH_FAIL_TO_SCAN_DIRECTORY        )
#define STR_FAIL_TO_WRITE_FILE                 ((0 == g_LanguageFlag) ? STR_EN_FAIL_TO_WRITE_FILE           : STR_CH_FAIL_TO_WRITE_FILE            )
                                                                                                                                                 
#define STR_ERR_FAIL_TO_MALLOC                 ((0 == g_LanguageFlag) ? STR_EN_ERR_FAIL_TO_MALLOC           : STR_CH_ERR_FAIL_TO_MALLOC            )
#define STR_ERR_INVALID_COLS                   ((0 == g_LanguageFlag) ? STR_EN_ERR_INVALID_COLS             : STR_CH_ERR_INVALID_COLS              )
#define STR_ERR_INVALID_LINES                  ((0 == g_LanguageFlag) ? STR_EN_ERR_INVALID_LINES            : STR_CH_ERR_INVALID_LINES             )
#define STR_ERR_PTHREAD_NUM_TOO_BIG            ((0 == g_LanguageFlag) ? STR_EN_ERR_PTHREAD_NUM_TOO_BIG      : STR_CH_ERR_PTHREAD_NUM_TOO_BIG       )
#define STR_ERR_PTHREAD_NUM_TOO_SMALL          ((0 == g_LanguageFlag) ? STR_EN_ERR_PTHREAD_NUM_TOO_SMALL    : STR_CH_ERR_PTHREAD_NUM_TOO_SMALL     )

/*************************************************************************
                                 English
 ************************************************************************/
#define STR_EN_BACK                             "[ back ]"
#define STR_EN_BEYOND_RANGE_OF_DISP             "Beyond range of displaying"
#define STR_EN_CONSOLE_END_LINE                 "Press Esc to exit project | Version 1.0.0 | Copyright 2017-2017"
#define STR_EN_CONSOLE_LABEL                    "Encryption System"
#define STR_EN_CONTINUE                         "[ continue ]"
#define STR_EN_DISP_ERR_INFO                    "Error Info"
#define STR_EN_ESC_EXIT                         "Press Esc to exit project"
#define STR_EN_EXIT_PROJECT                     "Continue to exit project ?"
#define STR_EN_FILE_IS_NULL                     "It is an empty file"
#define STR_EN_FILE_LIST_IS_NULL                "Folder is empty"
#define STR_EN_FILE_NOT_EXIST                   "The file does not exist"
#define STR_EN_GO_BACK                          "[ go back ]"
#define STR_EN_IF_CONTINUE                      "If continue ?"
#define STR_EN_IN_ENCRYPTING                    "Encrypting: "
#define STR_EN_IN_DECRYPTING                    "Decrypting: "
#define STR_EN_INPUT                            "Input: "
#define STR_EN_INPUT_FILE_NAME                  "Please input the file or folder name with full path"
#ifdef __LINUX
#define STR_EN_INPUT_FILE_NAME_EG               "E.g, /root/document/..."
#elif defined __WINDOWS
#define STR_EN_INPUT_FILE_NAME_EG               "E.g, C:\\User\\Administrator\\..."
#endif 
#define STR_EN_INPUT_PASSWORD                   "Password: "
#define STR_EN_INPUT_PASSWORD_CONFIRM           "Confirm password: "
#define STR_EN_INVALID_FUNC                     "Invalid input function"
#define STR_EN_INVALID_PARAMETER                "Invalid paramter"
#define STR_EN_LANGUAGE                         "English"
#define STR_EN_LOG_IS_NULL                      "Log is empty"
#define STR_EN_MENU                             "Menu"
#define STR_EN_NO                               "[ no ]"
#define STR_EN_NOT_A_FILE                       "It is not a file"
#define STR_EN_NOT_SUPPORT_FUNCTION             "Not support this function  temporarily"
#define STR_EN_NULL                             "NULL"
#define STR_EN_PASSWORD_NOT_MATCH               "Two password do not match"
#define STR_EN_PASSWORD_NULL                    "Length of key could not be null"
#define STR_EN_PASSWORD_TOO_SHORT               "Password too short, at least 8 bits"
#define STR_EN_PRESS_ENTER_TO_GO_BACK           "[ press Enter to go back ]"
#define STR_EN_PRO_REASON_NO_DECRYPT_FILE       "Protable cause: absent encrytable file in directory"
#define STR_EN_PRO_REASON_NO_ENCRYPT_FILE       "Protable cause: absent decrytable file in directory"
#define STR_EN_RATE                             "Rate: "
#define STR_EN_RETRY                            "[ retry ]"
#define STR_EN_SHOW_FILE_LABEL                  "Pgdn: next  |  Pgup: last  |  Enter: go back  |  "
#define STR_EN_SUCCESS                          "[ success ]"
#define STR_EN_SUCCESS_COUNT                    "Success: "
#define STR_EN_TASK                             "Task"
#define STR_EN_TOTAL_FILE                       "Total files"
#define STR_EN_VIEW_LOG                         "[ view log ]"
#define STR_EN_YES                              "[ yes ]"

#define STR_EN_FAIL                             "[ fail ]"
#define STR_EN_FAIL_COUNT                       "Fail: "
#define STR_EN_FAIL_TO_CREATE_OPEN_FILE         "Fail to create or open file"
#define STR_EN_FAIL_TO_CREATE_PTHREAD           "Fail to create pthread"
#define STR_EN_FAIL_TO_DELETE_OLD_FILE          "Fail to delete old file"
#define STR_EN_FAIL_TO_GET_FILE_FOLDER_INFO     "Fail to get file or folder info"
#define STR_EN_FAIL_TO_GET_FILE_INFO            "Fail to get file info"
#define STR_EN_FAIL_TO_OPEN_FILE                "Fail to open file"
#define STR_EN_FAIL_TO_OPEN_LOG_FILE            "Fail to open log file"
#define STR_EN_FAIL_TO_READ_FILE                "Fail to read file, make sure it is readable"
#define STR_EN_FAIL_TO_SCAN_DIRECTORY           "Fail to scan directory"
#define STR_EN_FAIL_TO_WRITE_FILE               "Fail to write to file, make sure it is writeable"

#define STR_EN_ERR_FAIL_TO_MALLOC               "Error: not enough running memory space"
#define STR_EN_ERR_INVALID_COLS                 "Error: need wider console, at least: "
#define STR_EN_ERR_INVALID_LINES                "Error: need higher console, at least: "
#define STR_EN_ERR_PTHREAD_NUM_TOO_BIG          "Error: Macro PTHREAD_NUM_MAX must be equare to or less than 8"
#define STR_EN_ERR_PTHREAD_NUM_TOO_SMALL        "Error: Macro PTHREAD_NUM_MAX must be equare to or bigger than 2"



/*************************************************************************
                                 Chinese
 ************************************************************************/
#define STR_CH_BACK                             "[ 返回 ]"
#define STR_CH_BEYOND_RANGE_OF_DISP             "超出显示范围"
#define STR_CH_CONSOLE_END_LINE                 "按 Esc 键退出系统 | 版本号 1.0.0 | 版权期 2017-2017"
#define STR_CH_CONSOLE_LABEL                    "加密系统"
#define STR_CH_CONTINUE                         "[ 继续 ]"
#define STR_CH_DISP_ERR_INFO                    "错误信息"
#define STR_CH_ESC_EXIT                         "按 Esc 键退出程序"
#define STR_CH_EXIT_PROJECT                     "继续退出程序? "
#define STR_CH_FILE_IS_NULL                     "文件为空"
#define STR_CH_FILE_LIST_IS_NULL                "文件夹为空"
#define STR_CH_FILE_NOT_EXIST                   "文件不存在"
#define STR_CH_GO_BACK                          "[ 返回 ]"
#define STR_CH_IF_CONTINUE                      "是否继续? "
#define STR_CH_IN_ENCRYPTING                    "加密: "
#define STR_CH_IN_DECRYPTING                    "解密: "
#define STR_CH_INPUT                            "输入: "
#define STR_CH_INPUT_FILE_NAME                  "请输入文件或文件夹的名称（含完整路径）"
#ifdef __LINUX
#define STR_CH_INPUT_FILE_NAME_EG               "例如, /root/document/..."
#elif defined __WINDOWS
#define STR_CH_INPUT_FILE_NAME_EG               "例如, C:\\User\\Administrator\\..."
#endif 
#define STR_CH_INPUT_PASSWORD                   "输入密码: "
#define STR_CH_INPUT_PASSWORD_CONFIRM           "确认密码: "
#define STR_CH_INVALID_FUNC                     "无效的输入选项"
#define STR_CH_INVALID_PARAMETER                "无效的参数"
#define STR_CH_LANGUAGE                         "中文"
#define STR_CH_LOG_IS_NULL                      "日志为空"
#define STR_CH_MENU                             "菜单"
#define STR_CH_NO                               "[ 否 ]"
#define STR_CH_NOT_A_FILE                       "不是一个文件"
#define STR_CH_NOT_SUPPORT_FUNCTION             "暂时不提供此功能        "
#define STR_CH_NULL                             "空"
#define STR_CH_PASSWORD_NOT_MATCH               "两次密码不匹配"
#define STR_CH_PASSWORD_NULL                    "密码不能为空"
#define STR_CH_PASSWORD_TOO_SHORT               "密码太短, 至少需要8位"
#define STR_CH_PRESS_ENTER_TO_GO_BACK           "[ 按 Enter 键返回 ]"
#define STR_CH_PRO_REASON_NO_DECRYPT_FILE       "可能原因: 目录中无可解密文件"
#define STR_CH_PRO_REASON_NO_ENCRYPT_FILE       "可能原因: 目录中无可加密文件"
#define STR_CH_RATE                             "进度: "
#define STR_CH_RETRY                            "[ 重试 ]"
#define STR_CH_SHOW_FILE_LABEL                  "Pgdn: 下一页  |  Pgup: 上一页  |  Enter: 返回  |  "
#define STR_CH_SUCCESS                          "[ 成功 ]"
#define STR_CH_SUCCESS_COUNT                    "成功: "
#define STR_CH_TASK                             "线程"
#define STR_CH_TOTAL_FILE                       "文件总数"
#define STR_CH_VIEW_LOG                         "[ 日志 ]"
#define STR_CH_YES                              "[ 是 ]"
    
#define STR_CH_FAIL                             "[ 失败 ]"
#define STR_CH_FAIL_COUNT                       "失败: "
#define STR_CH_FAIL_TO_CREATE_OPEN_FILE         "无法创建或打开文件"
#define STR_CH_FAIL_TO_CREATE_PTHREAD           "无法创建线程"
#define STR_CH_FAIL_TO_DELETE_OLD_FILE          "无法删除旧文件"
#define STR_CH_FAIL_TO_GET_FILE_FOLDER_INFO     "无法获取文件或文件夹的信息"
#define STR_CH_FAIL_TO_GET_FILE_INFO            "无法获取文件的信息"
#define STR_CH_FAIL_TO_OPEN_FILE                "无法打开文件"
#define STR_CH_FAIL_TO_OPEN_LOG_FILE            "无法打开日志文件"
#define STR_CH_FAIL_TO_READ_FILE                "无法读取文件, 请确保文件可读"
#define STR_CH_FAIL_TO_SCAN_DIRECTORY           "无法扫描目录"
#define STR_CH_FAIL_TO_WRITE_FILE               "无法写入文件, 请确保文件可写"
    
#define STR_CH_ERR_FAIL_TO_MALLOC               "错误: 运行内存空间"
#define STR_CH_ERR_INVALID_COLS                 "错误: 窗口宽度不足, 至少需要宽度: "
#define STR_CH_ERR_INVALID_LINES                "错误: 窗口高度不足, 至少需要高度: "
#define STR_CH_ERR_PTHREAD_NUM_TOO_BIG          "错误: 宏定义 PTHREAD_NUM_MAX 必须小于等于 8"
#define STR_CH_ERR_PTHREAD_NUM_TOO_SMALL        "错误: 宏定义 PTHREAD_NUM_MAX 必须大于等于 2"

#define LAN_EN                                  0
#define LAN_CH                                  (!LAN_EN)

extern char *g_EnMenu[];
extern char *g_ChMenu[];
extern char g_LanguageFlag;
extern char *EnInstruction[];
extern char *ChInstruction[];


//Inline function >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

static inline int GetWidth(const char *ptr)
{
    int width = UTF8_GetWidth(ptr);
    if(0 == width)
        return strlen(ptr);

    return width;
}

#if 0
/*
    It's only compliant for UTF-8 format
    Return the width of string
    Note: the charater not in ASCII table occupy 2 unit width
*/
static inline int GetWidth(const char *ptr)
{
    int count = 0;

    while('\0' != *ptr)
    {
        if(*ptr >> 7)
        {
            //Char in ASCII table is 1 byte, else most characters of UTF-8 is 3 bytes
            ptr += 3;
            count += 2;
        }
        else
        {
            ptr++;
            count++;
        }
    }

    return count;
}
#endif


void InitStr(void);

#endif
