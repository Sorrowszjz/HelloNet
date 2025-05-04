#ifndef __HELLO_NET_M_CORE_S_FILEUTIL__
#define __HELLO_NET_M_CORE_S_FILEUTIL__
#include "m_core.hpp"
#include "s_base.hpp"

/**
*@brief      RAII的chdir封装，
*            提供一个函数切换到一个目标目录(同时保存当前目录)
*            对象析构的时候切换回原来的目录
*/
class CHNAutoChDir : CHNNoCopyable
{
public:
    CHNAutoChDir(){}
    ~CHNAutoChDir()
    {
        if (m_szSavedDir[0] == 0)
            return;
        if (0 != HNChDir(m_szSavedDir))
        {
            printf("HNChDir(%s) fail.", m_szSavedDir);
        }
    }

    /**
    *@brief     切换到一个目标目录
    *@param     pszDestDir        [in]    目标目录
    *@return    是否成功
    */
    CHNResult<> ChDirTo(const char* pszDestDir)
    {
        CHNResult<> result;
        if (pszDestDir == NULL || pszDestDir[0] == 0)
            return result.SetFail("params invalid.");
        memset(m_szSavedDir, 0, sizeof(m_szSavedDir));
        if (NULL == getcwd(m_szSavedDir, sizeof(m_szSavedDir) - 1))
        {
            memset(m_szSavedDir, 0, sizeof(m_szSavedDir));
            return result.SetSystemFail();
        }
        if (0 != HNChDir(pszDestDir))
        {
            return result.SetSystemFail();
        }
        return result;
    }

private:
    char m_szSavedDir[MAX_PATH];
};

/**
*@brief        文件操作类
*/
class CHNFileUtil : CHNNoCopyable
{
public:
    /**
    *@brief     [仅用于路径]自动根据平台转化分隔符的snprintf
    *@param     fmt    [in]    格式化字符串
    *@return    格式化后的字符串
    */
    static std::string HNPathSprintf(const char* fmt, ...)
    {
        if (fmt == NULL)
            return "";

        char buf[MAX_PATH];
        memset(buf, 0, sizeof(buf));
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
        va_end(ap);

        std::string strFmt = buf;
#if defined(HNOS_WIN)
        CHNStrUtil::HNReplaceAll(strFmt, "/", "\\");
        CHNStrUtil::HNReplaceAll(strFmt, "\\\\", "\\");
#else
        CHNStrUtil::HNReplaceAll(strFmt, "\\", "/");
        CHNStrUtil::HNReplaceAll(strFmt, "//", "/");
#endif
        return strFmt;
    }

    /**
    *@brief     初步检查一个路径是不是合法的
    *@param     pszSrcFullPath    [in]    输入的包括文件名的全路径。路径分隔符支持\和/
    *@return    成功：CHNResult.Succ    失败：CHNResult.ErrDesc[错误描述]
    */
    static CHNResult<> HNPathCheck(const char* path)
    {
        CHNResult<> result;
        if (path == NULL)
        {
            return result.SetFail("path is NULL.");
        }
        if (path[0] == 0)
        {
            return result.SetFail("path is EMPTY.");
        }
        size_t nPathLen = strlen(path);
        if (nPathLen >= MAX_PATH)
        {
            return result.SetFail("path is TOO LONG.nPathLen:%u", nPathLen);
        }
        return result;
    }

    /**
    *@brief     根据输入字符串判断路径是不是一个文件名还是一个路径
    *@param     path    [in]    输入的包括文件名的全路径。路径分隔符支持\和/
    *@return    是否为文件名
    */
    static bool HNPathIsFileName(const char* path)
    {
        if (path == NULL)
        {
            return false;
        }
        if (path[0] == 0 || path[0] == '.' || path[0] == '/' || path[0] == '\\')
        {
            return false;
        }
#if defined(HNOS_WIN)
        //找到冒号位置
        if (strstr(path, ":"))
        {
            return false;
        }
#endif
        if (strstr(path, "/") || strstr(path, "\\"))
        {
            return false;
        }
        return true;
    }

    /**
    *@brief     从文件路径中取文件名
    *@param     path[in]    输入的包括文件名的全路径。路径分隔符支持\和/
    *@param     suffix[in]  可选。规定文件扩展名。如果文件有 suffix，则不会输出这个扩展名。
    *@return    基本名。失败为空
    */
    static std::string HNBaseName(const char* path, const char* suffix = NULL)
    {
        std::string ret;
        if(path == NULL || path[0] == 0)
            return ret;

        //提取文件名
        const char* pIndex = path;
        size_t nSlashPos = 0;
        for(size_t i = 0 ; *pIndex != '\0'; i++)
        {
#if defined(HNOS_WIN)
            if(*pIndex == '/' || *pIndex == '\\' || *pIndex == ':' )
#else
            if(*pIndex == '/'  )
#endif
                nSlashPos = i;
            pIndex++;
        }
        if(nSlashPos == -1)
            ret = path;
        else
            ret = path + nSlashPos + 1;
        if(ret.empty() || suffix == NULL || suffix[0] == 0)
            return ret;

        //移除后缀名
        if(CHNStrUtil::HNStrIsEndWith(ret.c_str(), suffix, false))
        {
            ret.resize(ret.length()-strlen(suffix));
        }
        return ret;
    }
    
    /**
    *@brief     根据输入字符串判断路径是不是一个绝对路径
    *@param     path    [in]    输入路径。路径分隔符支持\和/
    *@return    是否为绝对路径
    */
    static bool HNPathIsAbsolute(const char* path)
    {
        if (path == NULL || path[0] == 0)
        {
            return false;
        }
        auto len = strlen(path);
#if defined(HNOS_WIN)
        //找到冒号位置
        if (len >= 2 && path[1] == ':')
        {
            return true;
        }
#else
        if(path[0] == '/' || path[0] == '\\')
        {
            return true;
        }
#endif
        return false;
    }

    /**
    *@brief     判断路径是否合法且对应路径的磁盘存在
    *@param     pszSrcFullPath    [in]    输入的包括文件名的全路径。路径分隔符支持\和/
    *@return    是否合法且磁盘存在。如果路径中没有":"，则认为是相对路径，总是返回true
    */
    static bool HNPathValidAndDiskExist(const char* path)
    {
        if (path == NULL)
        {
            return false;
        }
        if (path[0] == 0)
        {
            return false;
        }
        size_t nPathLen = strlen(path);
        if (nPathLen >= MAX_PATH)
        {
            return false;
        }

#if defined(HNOS_WIN)
        //找到冒号位置
        std::string strDisk;
        for (size_t i = 0; i < nPathLen; i++)
        {
            strDisk += path[i];

            //找到冒号
            if (path[i] == ':')
                break;
        }

        //没有找到冒号，认为是相对路径
        if (strDisk.empty())
            return true;

        //找到了冒号，判断磁盘是否存在
        if (::access(strDisk.c_str(), F_OK) != 0)
        {
            return false;
        }
#endif
        return true;
    }

    /**
    *@brief        创建0777权限的文件夹
    */
    static CHNResult<> HNMakeDir(const char* filename)
    {
        CHNResult<> result;

        //如果目录存在，直接返回成功
        if (::access(filename, F_OK) == 0)
        {
            return result;
        }
#if defined(HNOS_WIN)
        int nMakeRet = ::_mkdir(filename);
#else
        //S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH
        //成功0 失败-1
        ::umask(0);
        int nMakeRet = ::mkdir(filename, 0777);
#endif
        if (nMakeRet != 0)
        {
            //文件已存在，也认为创建成功
            int nErrorNo = HNGetLastError();
            if (ERROR_ALREADY_EXISTS != nErrorNo)
            {
                return result.SetSystemFail(nErrorNo);
            }
        }
        return result;
    }

    /**
    *@brief     根据平台转化路径格式，windows分隔符为'\'，否则为'/'
    *           同时消除一些路径的错误，比如连续两个/\等
    *           如将 "test_serio/\\mshelp.rar.split\\0" ==> "test_serio/mshelp.rar.split/0"
    */
    static void HNPathRecovery(std::string& path)
    {
        bool skipNextSlash = false;
        std::string newPath;
        newPath.reserve(path.length() + 1);
        for (size_t i = 0; i < path.length(); i++)
        {
            if (path[i] == '/' || path[i] == '\\')
            {
                if (skipNextSlash)
                    continue;
#if defined(HNOS_WIN)
                newPath.push_back('\\');
#else
                newPath.push_back('/');
#endif
                skipNextSlash = true;
                continue;
            }
            skipNextSlash = false;
            newPath.push_back(path[i]);
        }
        path = std::move(newPath);
    }

    /**
    *@brief      对于文件夹的路径格式，如果最后一个字符不是'/''\'，则添加'/'或'\'
    */
    static void HNDirFormat(char* szPath)
    {
        if (szPath == NULL || szPath[0] == 0)
            return;
        size_t nLastCharPos = strlen(szPath) - 1;
        if (nLastCharPos > 0 && szPath[nLastCharPos] != '\\' && szPath[nLastCharPos] != '/')
        {
#if defined(HNOS_WIN)
            szPath[nLastCharPos + 1] = '\\';
#else
            szPath[nLastCharPos + 1] = '/';
#endif
            szPath[nLastCharPos + 2] = 0;
        }
    }

    /**
    *@brief      对于文件夹的路径格式，如果最后一个字符不是'/''\'，则添加'/'或'\'
    */
    static void HNDirFormat(std::string& strPath)
    {
        if (strPath.empty())
            return;
        size_t nLastCharPos = strPath.length() - 1;
        if (nLastCharPos > 0 && strPath[nLastCharPos] != '\\' && strPath[nLastCharPos] != '/')
        {
#if defined(HNOS_WIN)
            strPath += '\\';
#else
            strPath += '/';
#endif
        }
    }

    /**
    *@brief     [仅linux生效]将任意windows路径按照规则不冲突地映射为linux路径，规则如下：
    *             1.假设windows路径为 c:\ssts\resource
    *             2.如果路径第二个字符为冒号，移除冒号，变成 c\ssts\resource
    *             3.将所有\转化为/， 变成c/ssts/resource
    *             4.在前面加上prefix(假设为/sdcard/tmp/)，变成 /sdcard/tmp/c/ssts/resource
    *@param     path    [in]    输入的路径。
    *@param     prefix  [in]    转化后的前缀路径，如果prefix为NULL或空，等同于“/”。
    *@return    结果路径，如果path不是windows路径，直接返回path
    */
    static std::string HNPathConvert2Linux(const char* path, const char* prefix = "/sdcard/tmp/")
    {
        if (NULL == path)
            return "";

#if defined(HNOS_WIN)
        return path;
#else

        size_t len = strlen(path);
        if (len < 2 || path[1] != ':')
            return path;

        std::string strResult = "/";
        if (NULL != prefix && 0 != prefix[0])
        {
            strResult = prefix;
            char lastChar = prefix[strlen(prefix) - 1];
            if (lastChar != '/' && lastChar != '\\')
                strResult += "/";
        }

        for (size_t i = 0; i < len; i++)
        {
            if (i == 1)
                continue;

            if (path[i] == '\\')
                strResult += '/';
            else
                strResult += path[i];
        }
        return strResult;
#endif
    }

    /**
    *@brief        比较两个路径是否指向同一个文件
    */
    static bool HNFilePathEqual(const char* szPath1, const char* szPath2)
    {
        if (szPath1 == NULL && szPath2 == NULL) return true;
        if (szPath1 == NULL || szPath2 == NULL) return false;

        size_t nLen1 = strlen(szPath1);
        size_t nLen2 = strlen(szPath2);
        if (nLen1 != nLen2) return false;

        for (size_t i = 0; i < nLen1; i++)
        {
            if ((szPath1[i] == '/' && szPath2[i] == '\\') ||
                (szPath1[i] == '\\' && szPath2[i] == '/'))
            {
                continue;
            }

#if defined(HNOS_WIN)
            //windows路径不区分大小写
            if (tolower(szPath1[i]) != tolower(szPath2[i])) return false;
#else
            if (szPath1[i] != szPath2[i]) return false;
#endif    
        }
        return true;
    }

    /**
    *@brief        获取文件大小
    */
    static CHNResult<int64_t>  HNGetFileSize(const char* szFilePath)
    {
        CHNResult<int64_t> result;
#if defined(HNOS_WIN) 
        WIN32_FILE_ATTRIBUTE_DATA fileAttr;
        BOOL bRet = GetFileAttributesExA(szFilePath, GetFileExInfoStandard, &fileAttr);
        if (!bRet)
        {
            return result.SetSystemFail();
        }
        int64_t nSize = ((int64_t)fileAttr.nFileSizeHigh << 32) + fileAttr.nFileSizeLow;
        return result.SetSucc(nSize);
#else
        struct stat64 struStats;
        if (lstat64(szFilePath, &struStats) != 0)
        {
            return result.SetSystemFail();
        }
        else if (S_ISDIR(struStats.st_mode))
        {
            return result.SetFail("HNGetFileSize lstat64 S_ISDIR:%s", szFilePath);
        }
        else
        {
            return result.SetSucc((int64_t)struStats.st_size);
        }
#endif    
    }

    /**
    *@brief        获取文件大小
    */
    static CHNResult<int64_t> HNGetFILESize(FILE* fp)
    {
        CHNResult<int64_t> result;
        if (fp == NULL)
        {
            return result.SetFail("fp == NULL");
        }
        fseek(fp, 0L, SEEK_END);
        auto nFileLen = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        return result.SetSucc((int64_t)nFileLen);
    }

    /**
    *@brief     获取文件扩展名
    *@param     pszPath    [in]    文件路径。可以是全路径也可以是相对路径
    *@return    扩展名
    */
    static std::string HNGetFileExt(const char * pszPath)
    {
        if (NULL == pszPath || 0 == pszPath[0])
            return "";
        for (int i = (int)strlen(pszPath) - 1; i >= 0; --i)
        {
            if (pszPath[i] == '/' || pszPath[i] == '\\')
            {
                break;
            }
            if (pszPath[i] == '.')
            {
                std::string ext(pszPath + i + 1);
                return ext;
            }
        }
        return "";
    }

    /**
    *@brief     将一个全路径分离成目录名和文件名。如将"c:\a\1.txt"分离成"c:\a\"和1.txt
    *@param     pszSrcFullPath[in]    输入的包括文件名的全路径。路径分隔符支持\和/
    *@return    是否成功。成功：std::pair.first[目录名]    std::pair.second[文件名]
    */
    static CHNResult<std::pair<std::string,std::string> > HNSeperatePath(const char * pszSrcFullPath)
    {
        CHNResult<std::pair<std::string,std::string> > result;
        auto result2 = HNPathCheck(pszSrcFullPath);
        if (!result2)
        {
            return result.SetFail("HNPathCheck err:%s",result2.ErrDesc());
        }

        //未找到\\或/则认为是纯文件名
        if (CHNStrUtil::HNStrStri(pszSrcFullPath, "/") == NULL &&
            CHNStrUtil::HNStrStri(pszSrcFullPath, "\\") == NULL)
        {
            return result.SetSucc(std::make_pair("", pszSrcFullPath));
        }

        //从后到前找分隔符
        int nPathLen = (int)strlen(pszSrcFullPath);
        for (int i = nPathLen - 1; i >= 0; i--)
        {
            if (pszSrcFullPath[i] == '\\' || pszSrcFullPath[i] == '/')
            {
                std::string dir(pszSrcFullPath, i + 1);
                std::string filename(pszSrcFullPath + i + 1, nPathLen - i -1);
                return result.SetSucc(std::make_pair(std::move(dir),std::move(filename)));
            }
            if (pszSrcFullPath[i] == ':' && i == nPathLen - 1)
            {
                std::string strPath = pszSrcFullPath;
                strPath += "/";
                return result.SetSucc(std::make_pair(std::move(strPath), ""));
            }
        }
        return result.SetFail("parse path err.pszSrcFullPath:%s", pszSrcFullPath);
    }

    /**
    *@brief     获取工作目录（工作目录可能被修改）
    *@return    是否成功，成功时结果在StrResult1
    */
    static CHNResult<std::string> HNGetWorkDir()
    {
        CHNResult<std::string> result;
        char szPath[MAX_PATH] = { 0 };
        if (NULL == getcwd(szPath, sizeof(szPath) - 2))
        {
            return result.SetSystemFail();
        }
        HNDirFormat(szPath);
        return result.SetSucc(szPath);
    }

    /**
    *@brief     获取调用这个模块的调用者可执行文件的目录（一般固定）
    *@return    是否成功，成功时结果在std::pair.first；如果有额外的信息（文件名）在std::pair.second
    */
    static CHNResult<std::pair<std::string,std::string> >  HNGetExecuteFileDir()
    {
        CHNResult<std::pair<std::string,std::string> > result;
        char szPath[MAX_PATH] = { 0 };
#if defined(HNOS_WIN)
        if (0 == GetModuleFileNameA(GetModuleHandleA(NULL), szPath, sizeof(szPath) - 2))
#else
        if (-1 == readlink("/proc/self/exe", szPath, sizeof(szPath) - 2))
#endif
        {
            return result.SetSystemFail();
        }
        return HNSeperatePath(szPath);
    }

    /**
    *@brief     获取这个模块自身所在的目录（一般固定）
    *@return    是否成功，成功时结果在std::pair.first；如果有额外的信息（文件名）在std::pair.second
    */
    static CHNResult<std::pair<std::string,std::string> > HNGetSelfFileDir()
    {
        CHNResult<std::pair<std::string,std::string> > result;
        char szPath[MAX_PATH] = { 0 };
#if defined(HNOS_WIN)
        auto Module = HNGetSelfModuleHandle();
        if (NULL == Module)
        {
            return result.SetSystemFail();
        }
        if (0 == GetModuleFileNameA(Module, szPath, sizeof(szPath) - 2))
        {
            return result.SetSystemFail();
        }
#else
        Dl_info dl_info;
        if (0 == dladdr((const void*)CHNFileUtil::HNGetSelfFileDir, &dl_info))
        {
            return result.SetSystemFail();
        }
        strncpy(szPath, dl_info.dli_fname, sizeof(szPath) - 1);
#endif
        return HNSeperatePath(szPath);
    }

    /**
    *@brief     识别这个路径对应的资源类型 TODO win7下C:\usr\KIOSK\TestTools\SPTest\dist\publicconfig\file.txt 和/usr/KIOSK/TestTools/SPTest/dist/publicconfig/file.txt等价，有问题
    *@param     pszSrcFullPath    [in]    输入的包括文件名的全路径。路径分隔符支持\和/
    *@return    类型
    */
#define HNFILE_TYPE_INVALID     0
#define HNFILE_TYPE_FILE        1
#define HNFILE_TYPE_DIR         2
    static int HNGetPathType(const char* pszSrcFullPath)
    {
        if (pszSrcFullPath == NULL || pszSrcFullPath[0] == 0)
        {
            return HNFILE_TYPE_INVALID;
        }

#if defined(HNOS_WIN)
        //TODO:无法识别快捷方式
        if(pszSrcFullPath[0] == '/')
            return HNFILE_TYPE_INVALID;
        DWORD dwAttribute = GetFileAttributesA(pszSrcFullPath);
        if (dwAttribute == INVALID_FILE_ATTRIBUTES)
        {
            return HNFILE_TYPE_INVALID;
        }
        else if (dwAttribute&FILE_ATTRIBUTE_DIRECTORY)
        {
            return HNFILE_TYPE_DIR;
        }
        else
        {
            return HNFILE_TYPE_FILE;
        }
#else
        //TODO:非目录则为文件的逻辑
        struct stat64 struStats;
        if (lstat64(pszSrcFullPath, &struStats) != 0)
        {
            return HNFILE_TYPE_INVALID;
        }
        else if (S_ISDIR(struStats.st_mode))
        {
            return HNFILE_TYPE_DIR;
        }
        else
        {
            return HNFILE_TYPE_FILE;
        }
#endif
    }

    /**
    *@brief     读取文件时，智能跳过BOM。处理文本文件才需要这样做
    *@param     fin[in]    已经打开的输入文件流
    *@return    int [文件类型] (根据前几个字节文件头判断的结果。仅供参考)
    *@note      此函数会移动文件指针
    */
#define HN_TEXT_ANSI           (0)       //[记事本ANSI]                  无BOM
#define HN_TEXT_UCS2_LE_BOM    (1)       //[记事本Unicode]               FF FE
#define HN_TEXT_UCS2_BE_BOM    (2)       //[记事本Unicode big endian]    FE FF    
#define HN_TEXT_UTF8_BOM       (3)       //[记事本UTF-8]                 EF BB BF
#define HN_TEXT_UTF8           (4)       //[]                            无BOM
    static CHNResult<int> HNSkipBOM(std::ifstream& fin)
    {
        CHNResult<int> result;
        if (!fin.good())
        {
            return result.SetFail("fin is NOT good.");
        }
        char szHead[3] = { 0 };
        fin.read(szHead, sizeof(szHead));
        std::streamsize count = fin.gcount();
        if (count >= 2)
        {
            if (szHead[0] == '\xFF' && szHead[1] == '\xFE')
            {
                fin.seekg(2, fin.beg);
                return result.SetSucc(HN_TEXT_UCS2_LE_BOM);
            }
            if (szHead[0] == '\xFE' && szHead[1] == '\xFF')
            {
                fin.seekg(2, fin.beg);
                return result.SetSucc(HN_TEXT_UCS2_BE_BOM);
            }
            if (count == 3 && szHead[0] == '\xEF' && szHead[1] == '\xBB' &&  szHead[2] == '\xBF')
            {
                fin.seekg(3, fin.beg);
                return result.SetSucc(HN_TEXT_UTF8_BOM);
            }
        }
        fin.seekg(0, fin.beg);
        return result.SetSucc(HN_TEXT_ANSI);
    }

    /**
    *@brief     读取文件时，智能跳过BOM。处理文本文件才需要这样做
    *@param     fp[in]    已经打开的文件指针
    *@return    int [文件类型] (根据前几个字节文件头判断的结果。仅供参考)
    *@note      此函数会移动文件指针
    */
    static CHNResult<int> HNSkipBOM(FILE* fp)
    {
        CHNResult<int> result;
        if (fp == NULL)
        {
            return result.SetFail("fp is NULL.");
        }
        
        char szHead[3] = { 0 };
        auto count = fread(szHead, 1, 3, fp);
        if (count >= 2)
        {
            if (szHead[0] == '\xFF' && szHead[1] == '\xFE')
            {
                fseek(fp, 2 , SEEK_SET);
                return result.SetSucc(HN_TEXT_UCS2_LE_BOM);
            }
            if (szHead[0] == '\xFE' && szHead[1] == '\xFF')
            {
                fseek(fp, 2 , SEEK_SET);
                return result.SetSucc(HN_TEXT_UCS2_BE_BOM);
            }
            if (count == 3 && szHead[0] == '\xEF' && szHead[1] == '\xBB' &&  szHead[2] == '\xBF')
            {
                fseek(fp, 3 , SEEK_SET);
                return result.SetSucc(HN_TEXT_UTF8_BOM);
            }
        }
        fseek(fp, 0 , SEEK_SET);
        return result.SetSucc(HN_TEXT_ANSI);
    }

    /**
    *@brief     创建这个文件
    *@param     pszPath    [in]    路径。
    *@return    成功：CHNResult.Succ    失败：CHNResult.ErrDesc[错误描述]
    */
    static CHNResult<> HNCreateFile(const char* pszPath)
    {
        CHNResult<> result;
        if (pszPath == NULL || pszPath[0] == 0)
        {
            return result.SetFail("path is NULL/EMPTY.");
        }

        int fd = ::open(pszPath, O_CREAT, S_IREAD | S_IWRITE | S_IEXEC);
        if (fd < 0)
        {
            return result.SetSystemFail();
        }
        close(fd);
        return result;
    }

    /**
    *@brief     删除这个文件
    *@param     pszPath    [in]            路径。
    *@return    成功：CHNResult.Succ    失败：CHNResult.ErrDesc[错误描述]
    */
    static CHNResult<> HNDeleteFile(const char* pszPath)
    {
        CHNResult<> result;
        if (pszPath == NULL || pszPath[0] == 0)
        {
            return result.SetFail("path is NULL/EMPTY.");
        }
        if (0 != ::remove(pszPath))
        {
            //文件本身不存在，也算删除成功
            long errcode = HNGetLastError();
            if (ERROR_FILE_NOT_FOUND == errcode || ERROR_PATH_NOT_FOUND == errcode)
                return result;
            return result.SetSystemFail(errcode);
        }
        return result;
    }

    /**
    *@brief     删除这个空目录
    *@param     pszPath    [in]            路径。
    *@return    成功：CHNResult.Succ    失败：CHNResult.ErrDesc[错误描述]
    */
    static CHNResult<> HNDeleteEmptyDir(const char* pszPath)
    {
        CHNResult<> result;
        if (pszPath == NULL || pszPath[0] == 0)
        {
            return result.SetFail("path is NULL/EMPTY.");
        }
        if (0 != ::rmdir(pszPath))
        {
            //文件夹本身不存在，也算删除成功
            long errcode = HNGetLastError();
            if (ERROR_FILE_NOT_FOUND == errcode || ERROR_PATH_NOT_FOUND == errcode)
                return result;
            return result.SetSystemFail(errcode);
        }
        return result;
    }

    /**
    *@brief     创建目录，如果子目录不存在，会依次创建
    *@param     pszPath    [in]    路径。
    *@return    成功：CHNResult.Succ    失败：CHNResult.ErrDesc[错误描述]
    */
    static CHNResult<> HNCreateDir(const char* pszPath)
    {
        CHNResult<> result;
        if (pszPath == NULL)
        {
            return result.SetFail("path is NULL.");
        }
        if (pszPath[0] == 0)
            return result;

        size_t nPathLen = strlen(pszPath);
        if (nPathLen >= MAX_PATH)
        {
            return result.SetFail("path is TOO LONG.pszPath:%s", pszPath);
        }

        //尝试直接创建，看能不能成功
        char pszCheckPath[MAX_PATH + 2] = { 0 };
        memset(pszCheckPath, 0, sizeof(pszCheckPath));
        strncpy(pszCheckPath, pszPath, sizeof(pszCheckPath));
        if (pszCheckPath[nPathLen - 1] == '/')
        {
            pszCheckPath[nPathLen - 1] = '\0';
        }
        result = HNMakeDir(pszCheckPath);
        if (result)
        {
            return result;
        }

        //从前向后创建
        char *p = pszCheckPath + 1;
        while (1)
        {
            while (*p && *p != '\\' && *p != '/')
                p++;
            char hold = *p;
            *p = 0;

            result = HNMakeDir(pszCheckPath);
            if (!result)
            {
                return result;
            }

            if (hold == 0)
                break;
            *p++ = hold;
        }
        return result;
    }


    /**
    *@brief     遍历操作目录，不支持通配符。
    *@param     pszPath[in]     路径。
    *@param     callback[in]    回调函数。回调函数返回失败时将会退出遍历
    *@param     bRecusive[in]   是否递归处理。
    *@return    是否成功
    */
    static CHNResult<> HNScanDir(const char* pszPath,
        std::function<CHNResult<>(const char* pszPath, const char* pszFileName, bool bDir)> callback,
        bool bRecursive = false)
    {
        auto result = HNPathCheck(pszPath);
        if (!result)
        {
            return result;
        }

        char szScanDir[MAX_PATH] = { 0 };
        strcpy(szScanDir, pszPath);
        HNDirFormat(szScanDir);
#if defined(HNOS_WIN)
        strcat(szScanDir, "*.*");

        _finddata_t findData;
        intptr_t handle = _findfirst(szScanDir, &findData);
        if (handle == -1)
        {
            return result.SetSystemFail();
        }
        do
        {
            if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
            {
                continue;
            }

            //找到了
            strcpy(szScanDir, pszPath);
            HNDirFormat(szScanDir);
            strcat(szScanDir, findData.name);
            if (findData.attrib & _A_SUBDIR)
            {
                //找到文件夹
                if (bRecursive)
                {
                    result = HNScanDir(szScanDir, callback, true);
                    if (!result)
                    {
                        _findclose(handle);
                        return result;
                    }
                }
                else
                {
                    if (callback)
                    {
                        result = callback(szScanDir, findData.name, true);
                        if (!result)
                        {
                            _findclose(handle);
                            return result;
                        }
                    }
                }
            }
            else
            {
                //找到文件
                if (callback)
                {
                    result = callback(szScanDir, findData.name, false);
                    if (!result)
                    {
                        _findclose(handle);
                        return result;
                    }
                }
            }
        } while (_findnext(handle, &findData) == 0);
        _findclose(handle);
#else
        DIR *handle;
        struct dirent *findData, finddirbuf;
        if ((handle = opendir(szScanDir)) == NULL)
        {
            return result.SetSystemFail();
        }
        while (readdir_r(handle, &finddirbuf, &findData) == 0)
        {
            if (!findData)
                break;
            if (strcmp(findData->d_name, ".") == 0 || strcmp(findData->d_name, "..") == 0)
            {
                continue;
            }
            
            strcpy(szScanDir, pszPath);
            HNDirFormat(szScanDir);
            strcat(szScanDir, findData->d_name);

            //判断是不是文件夹
            if (findData->d_type == DT_DIR)
            {
                //找到文件夹
                if (bRecursive)
                {
                    result = HNScanDir(szScanDir, callback, true);
                    if (!result)
                    {
                        closedir(handle);
                        return result;
                    }
                }
                else
                {
                    if (callback)
                    {
                        result = callback(szScanDir, findData->d_name, true);
                        if (!result)
                        {
                            closedir(handle);
                            return result;
                        }
                    }
                }
            }
            else
            {
                //找到文件
                if (callback)
                {
                    result = callback(szScanDir, findData->d_name, false);
                    if (!result)
                    {
                        closedir(handle);
                        return result;
                    }
                }
            }
        }
        closedir(handle);
#endif

        //处理文件夹自身
        if (callback)
        {
            return callback(pszPath, "", true);
        }
        return result;
    }

    /**
    *@brief     按字节扫描操作文件。
    *@param     pszPath     [in]    路径。
    *@param     callback    [in]    回调函数。回调函数返回失败时将会退出遍历
    *@param     memsize     [in]    分多次读取时每次分配的内存空间大小
    *@return    成功：CHNResult.Succ    失败：CHNResult.ErrDesc[错误描述]
    */
    static CHNResult<> HNScanFile(const char* pszPath,
        std::function<CHNResult<>(const char* buf, size_t len)> callback,
        size_t memsize = HN_MAXHEAP)
    {
        auto result = HNPathCheck(pszPath);
        if (!result)
        {
            return result;
        }
        if (memsize == 0 || memsize > 1024 * 1024 * 1024)
        {
            return result.SetFail("param err.memsize:%d", memsize);
        }
        FILE* fp = fopen(pszPath, "rb");
        if (fp == NULL)
        {
            return result.SetSystemFail();
        }
        CHNAutoObj autoFile([&fp](){HNCloseFILE(fp); });

        //分段读取
        char* pszBuf = new char[memsize];
        if (pszBuf == NULL)
        {
            return result.SetSystemFail();
        }
        CHNAutoObj autoBuf([&pszBuf](){HNDeleteArr(pszBuf); });

        while (true)
        {
            size_t nReadLen = fread(pszBuf, 1, memsize, fp);
            if (nReadLen > memsize) return result.SetSystemFail();
            if (nReadLen == 0) break;

            //处理扫描到的字节
            if (callback)
            {
                result = callback(pszBuf, nReadLen);
                if (!result)
                {
                    return result;
                }
            }
        }
        return result;
    }

    /**
    *@brief     按行‘\n’扫描读取文本文件
    *@param     pszPath        [in]    路径。
    *@param     lineEndChar    [in]    line end char
    *@return    按行分割的数据，失败返回空
    */
    static std::vector<std::string> HNReadFileLines(const char* pszPath, const char  lineEndChar = '\n')
    {
        std::vector<std::string> ret;
        auto result = HNPathCheck(pszPath);
        if (!result)
        {
            return ret;
        }

        //打开文件
        std::ifstream fin(pszPath);
        if (!fin.is_open())
        {
            return ret;
        }
        CHNAutoObj autoFile([&fin](){fin.close(); });

        auto result2 = HNSkipBOM(fin);
        if (!result2)
            return ret;
        if (result2.Get() == HN_TEXT_UCS2_LE_BOM || result2.Get() == HN_TEXT_UCS2_BE_BOM)
        {
            return ret;
        }

        //逐行读取并解析文件
        std::string    strLine;
        while (std::getline(fin, strLine, lineEndChar))
        {
            ret.push_back(strLine);
        }
        return ret;
    }

    /**
    *@brief     按行‘\n’扫描操作文本文件,每扫描到一行，执行回调函数。
    *@param     pszPath     [in]    路径。
    *@param     callback    [in]    回调函数。回调函数返回失败时将会退出遍历
    *@param     lineEndChar [in]    line end char
    *@return    成功：CHNResult.Succ    失败：CHNResult.ErrDesc[错误描述]
    */
    static CHNResult<> HNScanFileLine(const char* pszPath,
        std::function<CHNResult<>(std::string &strLine)> callback,
        const char  lineEndChar = '\n')
    {
        auto result = HNPathCheck(pszPath);
        if (!result)
        {
            return result;
        }

        //打开文件
        std::ifstream fin(pszPath);
        if (!fin.is_open())
        {
            return result.SetSystemFail();
        }
        CHNAutoObj autoFile([&fin](){fin.close(); });

        auto result2 = HNSkipBOM(fin);
        if(!result2)
        {
            return result.SetFail("HNSkipBOM fail:%s",result2.ErrDesc());
        }
        if (result2.Get() == HN_TEXT_UCS2_LE_BOM || result2.Get() == HN_TEXT_UCS2_BE_BOM)
        {
            return result.SetFail("ifstream (%s) fail. unsupport unicode text file.", pszPath);
        }

        //逐行读取并解析文件
        std::string    strLine;
        while (std::getline(fin, strLine, lineEndChar))
        {
            if (callback)
            {
                result = callback(strLine);
                if (!result) return result;
            }
        }
        return result;
    }

    /**
    *@brief     读取文件所有内容，不能读取超过2G的文件。
    *@param     pszFilePath    [in]        路径。
    *@param     szBuff         [in][out]   用户分配的空间，读出数据后保存在这。
    *@param     nBufLen        [in]        用户分配的空间大小
    *@return    成功：CHNResult.IntResult[实际读出的大小]     失败：CHNResult.ErrDesc[错误描述]
    */
    static CHNResult<size_t> HNReadFile(const char* pszFilePath, unsigned char* szBuff, unsigned int nBufLen)
    {
        CHNResult<size_t> result;
        auto result2 = HNPathCheck(pszFilePath);
        if (!result2)
        {
            return result.SetFail("HNPathCheck fail:%s",result2.ErrDesc());
        }
        if (szBuff == NULL)
        {
            return result.SetFail("szBuff is NULL");
        }

        FILE* fp = fopen(pszFilePath, "rb");
        if (fp == NULL)
        {
            return result.SetSystemFail();
        }
        CHNAutoObj autoFile([&fp](){HNCloseFILE(fp); });

        //获取文件大小
        fseek(fp, 0L, SEEK_END);
        int nFileLen = ftell(fp);
        if (nFileLen < 0)
        {
            return result.SetSystemFail();
        }
        fseek(fp, 0L, SEEK_SET);

        //判断文件大小是否超出buf大小。
        if ((int)nBufLen <= nFileLen)
        {
            return result.SetFail("buf len(%d) <= file size(%d).", nBufLen, nFileLen);
        }

        //复制
        size_t nReadLen = fread(szBuff, 1, nBufLen, fp);
        if (nReadLen >= (int)nBufLen)
        {
            return result.SetFail("fread (%s) nReadLen(%d) >= nBufLen(%d)", pszFilePath, nReadLen, nBufLen);
        }
        return result.SetSucc((int)nReadLen);
    }

    /**
    *@brief     读取文件所有内容，不能读取超过2G的文件。
    *@param     pszFilePath    [in]        路径。
    *@return    文件内容，如果失败，内容为空
    */
    static CHNBuffer HNReadFileAll(const char* pszFilePath)
    {
        auto result = HNPathCheck(pszFilePath);
        if (!result)
        {
            return CHNBuffer();
        }

        FILE* fp = fopen(pszFilePath, "rb");
        if (fp == NULL)
        {
            return CHNBuffer();
        }
        CHNAutoObj autoFile([&fp](){HNCloseFILE(fp); });

        //获取文件大小
        fseek(fp, 0L, SEEK_END);
        long nFileLen = ftell(fp);
        if (nFileLen < 0)
        {
            return CHNBuffer();
        }
        fseek(fp, 0L, SEEK_SET);

        //复制
        CHNBuffer buffer(nFileLen + 1);
        size_t nReadLen = fread(buffer.data(), 1, buffer.capacity(), fp);
        if (nReadLen > buffer.capacity())
        {
            return CHNBuffer();
        }
        buffer.updatesize(nReadLen);
        return buffer;
    }

	/**
    *@brief     从文件中读取固定长度的内容，不能读取超过2G的文件。
    *@param     pszFilePath    [in]        路径。
	*@param     seekgstart     [in]        文件流的位置。
	*@param     readlen        [in]        读固定长度的数据。
    *@return    文件内容，如果失败，内容为空
    */
    static CHNBuffer HNReadFileSeekg(const char* pszFilePath, unsigned int seekgstart, unsigned int readlen)
    {
        auto result = HNPathCheck(pszFilePath);
        if (!result)
        {
            return CHNBuffer();
        }

        FILE* fp = fopen(pszFilePath, "rb");
        if (fp == NULL)
        {
            return CHNBuffer();
        }
        CHNAutoObj autoFile([&fp](){HNCloseFILE(fp); });

        //获取文件大小
        fseek(fp, 0L, SEEK_END);
        long nFileLen = ftell(fp);
        if (nFileLen < 0)
        {
            return CHNBuffer();
        }
        fseek(fp, seekgstart*readlen, SEEK_SET);
		if((seekgstart+1)*readlen > nFileLen)
			readlen = nFileLen - (seekgstart)*readlen;
        //复制
        CHNBuffer buffer(readlen + 1);
        size_t nReadLen = fread(buffer.data(), 1, buffer.capacity(), fp);
        if (nReadLen > buffer.capacity())
        {
            return CHNBuffer();
        }
        buffer.updatesize(readlen);
        return buffer;
    }

    /**
    *@brief     检查路径是否存在，不存在则创建目录。
    *@param     pszFilePath    [in]        文件全路径。
    *@return    是否成功
    */
    static CHNResult<> HNCheckAndCreateDir(const char* pszFilePath)
    {
        CHNResult<> result;
        auto result2 = HNSeperatePath(pszFilePath);
        if (!result2)
        {
            return result.SetFail("HNSeperatePath fail:%s",result2.ErrDesc());
        }
        std::string tempdir = HNPathConvert2Linux(result2.Get().first.c_str());
        return HNCreateDir(tempdir.c_str());
    }

    /**
    *@brief     写文件。
    *@param     fp            [in]        已经打开的文件指针。
    *@param     szBuff        [in]        要写入的内容。
    *@param     nBufLen       [in]        要写入的内容长度。
    *@return    是否成功
    */
    static CHNResult<> HNWriteFile(FILE* fp, const void* szBuff, size_t nBufLen)
    {
        CHNResult<> result;
        if (fp == NULL)
        {
            return result.SetSystemFail();
        }
        while (true)
        {
            const unsigned char *pWritePos = (const unsigned char *)szBuff;
            size_t nWriteLen = fwrite(pWritePos, 1, nBufLen, fp);
            if (nWriteLen > (int)nBufLen)
            {
                return result.SetFail("fwrite() nWriteLen(%d) > nBufLen(%d).", nWriteLen, nBufLen);
            }
            if (nWriteLen == nBufLen) break;

            pWritePos += nWriteLen;
            nBufLen -= nWriteLen;
        }
        return result;
    }

    /**
    *@brief     写文件。
    *@param     pszFilePath    [in]        路径。
    *@param     szBuff         [in]        要写入的内容。
    *@param     nBufLen        [in]        要写入的内容长度。
    *@param     bAppend        [in]        是否要将待写入的数据追加到末尾。
    *@return    成功：CHNResult.IntResult     失败：CHNResult.ErrDesc[错误描述]
    */
    static CHNResult<> HNWriteFile(const char* pszFilePath, const void* szBuff, size_t nBufLen, bool bAppend = false)
    {
        auto result = HNPathCheck(pszFilePath);
        if (!result)
        {
            return result;
        }
        if (szBuff == NULL)
        {
            return result.SetFail("buf is NULL");
        }
        //新增创建文件目录
        result = HNCheckAndCreateDir(pszFilePath);
        if (!result)
        {
            return result;
        }

        FILE* fp = fopen(pszFilePath, bAppend ? "ab+" : "wb");
        if (fp == NULL)
        {
            return result.SetSystemFail();
        }
        CHNAutoObj autoFile([&fp](){HNCloseFILE(fp); });
        return HNWriteFile(fp, szBuff, nBufLen);
    }

    /**
    *@brief     删除文件夹 HNDeleteDir
    *@param     pszPath        [in]    路径。
    *@param     bRecusive      [in]    是否递归删除子文件夹。(是：将会清空文件夹，危险！！ 否：将忽略pszPath目录的子文件夹(以及子文件夹的文件))
    *@return    成功：CHNResult.Succ  失败：CHNResult.ErrDesc[错误描述]
    */
    static CHNResult<> HNDeleteDir(const char* pszPath, bool bRecursive = false)
    {
        //文件夹本身不存在，直接返回成功
        CHNResult<> result;
        if (pszPath != NULL && 0 != ::access(pszPath, F_OK))
        {
            return result;
        }
        return HNScanDir(pszPath,
            [](const char* pszPath, const char* pszFileName, bool bDir)->CHNResult<>
        {
            if (bDir)    return HNDeleteEmptyDir(pszPath);
            else        return HNDeleteFile(pszPath);
        },
            bRecursive);
    }

    /**
    *@brief     移动文件夹，递归操作
    *@param     pszSrcPath  [in]    源路径
    *@param     pszDestPath [in]    目标路径
    *@return    成功：CHNResult.Succ  失败：CHNResult.ErrDesc[错误描述]
    */
    static CHNResult<> HNMoveDir(const char* pszSrcPath, const char* pszDestPath)
    {
        CHNResult<> result;
        if (pszDestPath == NULL || pszDestPath[0] == 0 || pszSrcPath == NULL || pszSrcPath[0] == 0)
        {
            return result.SetFail("params invalid.");
        }
        std::string strSrcPath = pszSrcPath;
        std::string strDestPath = pszDestPath;
        CHNFileUtil::HNDirFormat(strSrcPath);
        CHNFileUtil::HNDirFormat(strDestPath);

        return HNScanDir(pszSrcPath,
            [&strDestPath, &strSrcPath](const char* pszPath, const char* pszFileName, bool bDir)->CHNResult<>
        {
            CHNResult<> result;
            if (!bDir)
            {
                std::string strThisDestPath = strDestPath + (pszPath + strSrcPath.length());
                result = CHNFileUtil::HNMoveFile(pszPath, strThisDestPath.c_str());
            }
            return result;
        },
            true);
    }

    /**
    *@brief     复制文件
    *@param     pszSrcPath    [in]    源路径。
    *@param     pszDestPath   [in]    目标路径。目标路径可以是文件夹(以/或\结尾)(此时目标文件名和源文件名相同)。
    *@param     bOverWrite    [in]    是否覆盖已有文件，如果不覆盖且目标文件已存在，则会失败。
    *@return    成功：CHNResult.Succ    失败：CHNResult.ErrDesc[错误描述]
    */
    static CHNResult<> HNCopyFile(const char* pszSrcPath, const char* pszDestPath, bool bOverWrite = true)
    {
        CHNResult<> result;
        auto result2 = HNSeperatePath(pszDestPath);
        if (!result2)    return result.SetFail("HNSeperatePath fail:%s",result2.ErrDesc());
        HNCreateDir(result2.Get().first.c_str());

        result2 = HNSeperatePath(pszSrcPath);
        if (!result2)    return result.SetFail("HNSeperatePath fail:%s",result2.ErrDesc());

        //如果目标路径是文件夹，则加入源文件名
        char szDestFilePath[MAX_PATH * 2] = { 0 };
        memset(szDestFilePath, 0, sizeof(szDestFilePath));
        strncpy(szDestFilePath, pszDestPath, sizeof(szDestFilePath));
        size_t nLastDestPathPos = strlen(pszDestPath) - 1;
        if (nLastDestPathPos > 0 && (pszDestPath[nLastDestPathPos] == '\\' || pszDestPath[nLastDestPathPos] == '/'))
        {
            strcat(szDestFilePath, result2.Get().second.c_str());
            if (strlen(szDestFilePath) >= MAX_PATH)
            {
                return result.SetFail("dest file path is TOO LONG.");
            }
        }
        if (HNFilePathEqual(pszSrcPath, szDestFilePath))
        {
            return result.SetFail("src and dest file is EQUAL.");
        }

        //不覆盖模式，判断目标文件是否存在
        if (!bOverWrite)
        {
            FILE* fp = fopen(szDestFilePath, "rb");
            if (fp != NULL)
            {
                fclose(fp);
                return result.SetFail("dest file(%s) is EXIST.", szDestFilePath);
            }
        }

        //打开两个文件
        FILE* fpSrc = fopen(pszSrcPath, "rb");
        if (fpSrc == NULL)
        {
            return result.SetSystemFail();
        }
        CHNAutoObj AutoFileSrc([&fpSrc](){HNCloseFILE(fpSrc); });

        FILE* fpDest = fopen(szDestFilePath, "wb");
        if (fpDest == NULL)
        {
            return result.SetSystemFail();
        }
        CHNAutoObj AutoFileDest([&fpDest](){HNCloseFILE(fpDest); });

        //按字节复制
        char szBuf[HN_MAXURL] = { 0 };
        while (true)
        {
            memset(szBuf, 0, sizeof(szBuf));

            size_t nReadLen = fread(szBuf, 1, sizeof(szBuf), fpSrc);
            if (nReadLen > sizeof(szBuf))
            {
                return result.SetFail("fread (%s) nReadLen(%d) > sizeof(szBuf)(%d).", pszSrcPath, nReadLen, sizeof(szBuf));
            }
            if (nReadLen == 0) break;

            while (true)
            {
                char *pWritePos = szBuf;
                size_t nWriteLen = fwrite(pWritePos, 1, nReadLen, fpDest);
                if (nWriteLen > nReadLen)
                {
                    return result.SetFail("fwrite(%s) nWriteLen(%d) > nReadLen(%d).", szDestFilePath, nWriteLen, nReadLen);
                }
                if (nWriteLen == nReadLen) break;

                pWritePos += nWriteLen;
                nReadLen -= nWriteLen;
            }
        }
        return result.SetSucc();
    }

    /**
    *@brief     移动文件。目标文件必须不存在。
    *@param     pszSrcPath    [in]    源路径。
    *@param     pszDestPath   [in]    目标路径。目标路径可以是文件夹(以/或\结尾)(此时目标文件名和源文件名相同)。
    *@param     bOverWrite    [in]    是否覆盖已有文件，如果不覆盖且目标文件已存在，则会失败。
    *@return    成功：CHNResult.Succ    失败：CHNResult.ErrDesc[错误描述]
    */
    static CHNResult<> HNMoveFile(const char* pszSrcPath, const char* pszDestPath, bool bOverWrite = true)
    {
        CHNResult<> result;
        auto result2 = HNSeperatePath(pszDestPath);
        if (!result2)    return result.SetFail("HNSeperatePath fail:%s",result2.ErrDesc());

        //确保目标文件夹存在
        CHNFileUtil::HNCreateDir(result2.Get().first.c_str());

        //如果目标路径是文件夹，则加入源文件名
        std::string strDestPath = pszDestPath;
        if (result2.Get().second.empty())
        {
            result2 = HNSeperatePath(pszSrcPath);
            if (!result2)    return result.SetFail("HNSeperatePath fail:%s",result2.ErrDesc());
            strDestPath += result2.Get().second;
        }
        if (strDestPath.length() >= MAX_PATH)
        {
            return result.SetFail("dest file path is TOO LONG.");
        }
        if (HNFilePathEqual(pszSrcPath, strDestPath.c_str()))
        {
            return result.SetFail("src and dest file is EQUAL.");
        }

        {
            //判断目标文件是否存在
            FILE* fp = fopen(strDestPath.c_str(), "rb");
            if (fp != NULL)
            {
                fclose(fp);
                if (bOverWrite)
                {
                    result = HNDeleteFile(strDestPath.c_str());
                    if (!result) return result;
                }
                else
                {
                    return result.SetFail("dest file(%s) is EXIST.", strDestPath.c_str());
                }
            }
        }

        //移动
        if (0 != rename(pszSrcPath, strDestPath.c_str()))
        {
            return result.SetSystemFail();
        }
        return result.SetSucc();
    }

    /**
    *@brief     取文件时间
    *@param     pszPath    [in]  文件路径。
    *@param     tmstr      [out] 标准格式时间字符串 2010-01-01 00:00:00
    *@return    是否成功
    */
    static bool HNFileTimeStr(const char* pszPath, std::string* tmstr)
    {
#if defined(HNOS_WIN)
        WIN32_FIND_DATAA ff32;
        HANDLE hFind = FindFirstFileA(pszPath, &ff32);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            return false;
        }
        FILETIME ftLocal;
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime), &ftLocal);
        SYSTEMTIME stLocal;
        ZeroMemory(&stLocal, sizeof(SYSTEMTIME));
        FileTimeToSystemTime(&ftLocal, &stLocal);
        char tmp[256];
        sprintf(tmp, "%04d-%02d-%02d %02d:%02d:%02d", stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour, stLocal.wMinute, stLocal.wSecond);   // 文件创建时间
        *tmstr = tmp;
        FindClose(hFind);
        return true;
#else
        struct stat s;        /* results of stat() */
        struct tm* ptm;
        time_t tm_t = 0;
        bool bret = false;
        if (strcmp(pszPath, "-") != 0)
        {
            char name[MAX_PATH + 1];
            int len = strlen(pszPath);
            if (len > MAX_PATH)
                len = MAX_PATH;

            strncpy(name, pszPath, MAX_PATH - 1);
            /* strncpy doesnt append the trailing NULL, of the string is too long. */
            name[MAX_PATH] = '\0';

            if (name[len - 1] == '/')
                name[len - 1] = '\0';
            /* not all systems allow stat'ing a file with / appended */
            if (stat(name, &s) == 0)
            {
                tm_t = s.st_mtime;
                bret = true;
            }
        }
        ptm = localtime(&tm_t);

        char tmp[256];
        sprintf(tmp, "%04d-%02d-%02d %02d:%02d:%02d",
            ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
        *tmstr = tmp;

        return bret;
#endif
    }

    /**
    *@brief     取文件最后修改时间，精确到秒
    *@param     pszPath    [in]  文件路径。
    *@return    失败返回0，成功返回对应时间
    */
    static CHNTimeValue HNGetFileLastModifyTime(const char* pszPath)
    {
        if (pszPath == NULL || pszPath[0] == 0)
            return CHNTimeValue(0);
        auto fp = fopen(pszPath, "r");
        if (NULL == fp)
            return CHNTimeValue(0);
        struct stat fileStat;
        ::fstat(fileno(fp), &fileStat);
        time_t modify_time = fileStat.st_mtime;
        fclose(fp);
        return CHNTimeValue(modify_time * 1000);
    }

    /**
    *@brief     针对一个目录中的目录和文件，将这些文件名根据基础目录获取一个简化目录.如basepath：c:\123 , pszpath: c:\123\web\q.xml ==> 输出：web\q.xml
    *@param     pszPath    [in]  文件路径
    *@param     basePath   [in]  基础路径，pszPath必须是基础路径里面的子文件
    *@return    简化后的目录
    */
    static std::string HNIgnoreBasePath(const char* pszPath, const char* basePath)
    {
        if (pszPath == NULL || basePath == NULL || pszPath[0] == 0 || basePath[0] == 0)
        {
            return "";
        }
        const char* pEnd = strstr(pszPath, basePath);
        if (NULL == pEnd)
        {
            return "";
        }
        size_t baseLen = strlen(basePath);
        if (pszPath[baseLen] == '/' || pszPath[baseLen] == '\\')
        {
            return pszPath + baseLen + 1;
        }
        else
        {
            return pszPath + baseLen;
        }
    }

    /**
    *@brief     获取文件版本信息
    *@param     pszPath    [in]  文件路径
    *@return    是否成功，成功则std::string为版本信息
    */
    static CHNResult<std::string> HNGetFileVersionInfo(const char* pszPath)
    {
        CHNResult<std::string> result;
#if defined(HNOS_WIN)
        char *pVerValue = NULL;
        unsigned long dwTemp = 0;
        unsigned long dwSize = 0;
        unsigned char *pData = NULL;
        unsigned int uLen = 0;

        dwSize = GetFileVersionInfoSizeA(pszPath, &dwTemp);
        if (dwSize == 0)
        {
            return result.SetSystemFail();
        }
        pData = new unsigned char[dwSize + 1];
        CHNAutoObj autoDelete([&pData](){HNDeleteArr(pData); });
        if (!GetFileVersionInfoA(pszPath, 0, dwSize, pData))
        {
            return result.SetSystemFail();
        }

        //取版本号
        VS_FIXEDFILEINFO* pInfo;
        char strVersion[MAX_PATH] = { 0 };
        if (!VerQueryValueA(pData, "\\", (void**)&pInfo, &uLen) || uLen >= MAX_PATH)
        {
            return result.SetSystemFail();
        }
        snprintf(strVersion, sizeof(strVersion) - 1, ("%d.%d.%d.%d"),
            HIWORD(pInfo->dwFileVersionMS), LOWORD(pInfo->dwFileVersionMS),
            HIWORD(pInfo->dwFileVersionLS), LOWORD(pInfo->dwFileVersionLS));

        //取Translation
        char strTranslation[MAX_PATH] = { 0 };
        if (!VerQueryValueA(pData, ("\\VarFileInfo\\Translation"), (void **)&pVerValue, &uLen) || NULL == pVerValue || uLen >= MAX_PATH)
        {
            return result.SetSystemFail();
        }
        snprintf(strTranslation, sizeof(strTranslation) - 1, "%04x%04x",
            *((unsigned short int *)pVerValue), *((unsigned short int *)&pVerValue[2]));

        //文件描述
        char strSubBlock[MAX_PATH] = { 0 };
        snprintf(strSubBlock, sizeof(strSubBlock) - 1, "\\StringFileInfo\\%s\\FileDescription", strTranslation);
        if (!VerQueryValueA(pData, strSubBlock, (void **)&pVerValue, &uLen) || NULL == pVerValue || uLen >= MAX_PATH)
        {
            return result.SetSystemFail();
        }
        std::string vinfo = strVersion;
        vinfo += " ";
        vinfo += (pVerValue ? pVerValue : "");
        result.SetSucc(std::move(vinfo));
#else
        result.SetFail("unsupported");
#endif
        return result;
    }

#if defined(HNOS_WIN)
    /**
    *@brief     取模块自身句柄，例如dll里面获取自身的模块
    *@return    句柄，失败返回NULL
    */
    static HMODULE HNGetSelfModuleHandle()
    {
        MEMORY_BASIC_INFORMATION mbi;
        return ((::VirtualQuery(HNGetSelfModuleHandle, &mbi, sizeof(mbi)) != 0) ? (HMODULE)mbi.AllocationBase : NULL);
    }
#endif
};

/**
*@brief        对文件描述符操作进行RAII封装
*/
class CHNFdHandle : CHNNoCopyable
{
public:
    explicit CHNFdHandle(bool bAppend = true)
        :m_fd(-1)
        , m_append(bAppend)
    {
    }
    ~CHNFdHandle()
    {
        HNFdClose();
    }

    /**
    *@brief     文件描述符是否有效
    */
    bool HNFdValid()
    {
        return (m_fd >= 0);
    }

    /**
    *@brief     打开文件描述符
    *@param     path    [in]    文件路径。
    *@return    失败返回-1。成功返回[文件描述符]
    */
    int HNFdOpen(const char* path)
    {
        if (HNFdValid())
        {
            HNFdClose();
        }
#if defined(HNOS_WIN)
        if (m_append)
            m_fd = ::_open(path, _O_APPEND | _O_CREAT | _O_BINARY | _O_RDWR, _S_IREAD | _S_IWRITE);
        else
            m_fd = ::_open(path, _O_CREAT | _O_BINARY | _O_RDWR, _S_IREAD | _S_IWRITE);
#else
        //保证root权限进程创建的文件也能正常被别的进程访问
        ::umask(0);
        if (m_append)
            m_fd = ::open(path, O_APPEND | O_CREAT | O_RDWR, S_IREAD | S_IWRITE | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        else
            m_fd = ::open(path, O_CREAT | O_RDWR, S_IREAD | S_IWRITE | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
#endif
        return m_fd;
    }

    /**
    *@brief     将数据写入磁盘
    */
    void HNFdFlush()
    {
        if (!HNFdValid())
            return;
#if defined(HNOS_WIN)
        ::_commit(m_fd);
#elif defined(HNOS_ANDROID) || defined(HNOS_MAC) 
        ::fsync(m_fd);
#else
        ::syncfs(m_fd);
#endif
    }

    /**
    *@brief     关闭文件描述符
    */
    void HNFdClose()
    {
        if (!HNFdValid())
            return;
#if defined(HNOS_WIN)
        ::_close(m_fd);
#else
        ::close(m_fd);
#endif
        m_fd = -1;
    }

    /**
    *@brief     写入数据
    *@param     data    [in]    数据。
    *@param     len     [in]    数据长度。
    *@return    失败返回-1。成功返回[写入的字节数]
    */
    int HNFdWrite(const void* data, unsigned int len)
    {
        if (m_fd < 0)
            return -1;
#if defined(HNOS_WIN)
        return ::_write(m_fd, data, len);
#else
        return ::write(m_fd, data, len);
#endif
    }

    /**
    *@brief     读取数据。如果此时fd中的数据如果小于要读取的数据，就会引起阻塞。
    *@param     data    [out]    数据缓冲区。
    *@param     len     [in]    要读取的数据长度。
    *@return    失败返回-1。读到文件尾返回0，成功返回[读取成功的字节数]
    */
    int HNFdRead(void* buf, unsigned int len)
    {
        if (m_fd < 0)
            return -1;
#if defined(HNOS_WIN)
        return ::_read(m_fd, buf, len);
#else
        return ::read(m_fd, buf, len);
#endif
    }

    /**
    *@brief     移动fd指针
    *@param     offset    [out]    相对于whence的偏移量。
    *@param     whence    [in]    搜寻的开始位置。SEEK_SET【从文件头部开始】
    *                                         SEEK_CUR【从文件当前读写的指针位置开始】
    *                                         SEEK_END【从文件结尾开始】
    *@return    失败返回-1。成功返回结果偏移量位置，以文件开头的字节为单位。
    */
    int HNFdSeek(long offset, int whence)
    {
        if (m_fd < 0)
            return -1;
#if defined(HNOS_WIN)
        return ::_lseek(m_fd, offset, whence);
#else
        return ::lseek(m_fd, offset, whence);
#endif
    }

private:
    int  m_fd;
    bool m_append;
};



#endif //__HELLO_NET_M_CORE_S_FILEUTIL__