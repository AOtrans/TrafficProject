#ifndef INIUTIL_H
#define INIUTIL_H

#include <map>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

#define CONFIGLEN           256

enum INI_RES
{
    INI_SUCCESS,            //成功
    INI_ERROR,              //普通错误
    INI_OPENFILE_ERROR,     //打开文件失败
    INI_NO_ATTR            //无对应的键值
};

//              子键索引    子键值
typedef map<std::string,std::string> KEYMAP;
//              主键索引 主键值
typedef map<std::string,KEYMAP> MAINKEYMAP;
// config 文件的基本操作类

class IniUtil
{
public:
    // 构造函数
    IniUtil();

    // 析够函数
    virtual ~IniUtil();
public:
    //获取整形的键值
    int  GetInt(const char* mAttr, const char* cAttr );
    //获取键值的字符串
    char *GetStr(const char* mAttr, const char* cAttr );
    // 打开config 文件
    INI_RES OpenFile(const char* pathName, const char* type);
    // 关闭config 文件
    INI_RES CloseFile();
protected:
    // 读取config文件
    INI_RES GetKey(const char* mAttr, const char* cAttr, char* value);
protected:
    // 被打开的文件局柄
    FILE* m_fp;
    char  m_szKey[ CONFIGLEN ];
    MAINKEYMAP m_Map;
};

#endif // INIUTIL_H
