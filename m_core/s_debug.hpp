#ifndef __HELLO_NET_M_CORE_S_DEBUG__
#define __HELLO_NET_M_CORE_S_DEBUG__
#include "m_core.hpp"
#include "s_base.hpp"
/**
*@brief        生成dump，目前只支持windows
*/
class CHNDumpHelper : CHNNoCopyable
{
public:
    /**
    *@brief     配置生成dump文件，注意这个函数在整个进程里面只能有一个地方被调用。
    *           在动态库里面调用此函数可能不会生成dump文件，建议在exe里面调用
    *@param     path        [in]    生成dump文件的路径。为空时默认为模块文件所在目录
    *@param     filename    [in]    dump文件的文件名。为空时默认为模块文件名称.dmp
    *@return    成功：CHNResult.Succ    失败：CHNResult.ErrDesc[错误描述]
    */
    static CHNResult<> SetDump(const char* path, const char* filename)
    {
        CHNResult<> result;
        auto result2 = CHNFileUtil::HNGetExecuteFileDir();
        if (!result2)
        {
            return result.SetFail("HNGetExecuteFileDir fail:%s",result2.ErrDesc());
        }
        std::string strPath = result2.Get().first;
        std::string strFile = result2.Get().second;
        strFile += ".dmp";
        if (path != NULL && path[0] != 0)
        {
            strPath = path;
            CHNFileUtil::HNCreateDir(path);
        }
        if (filename != NULL && filename[0] != 0)
        {
            strFile = filename;
        }
#if defined(HNOS_WIN)
        CHNDumpHelper::instance().m_DumpFilePath = strPath;
        int nLastCharPos = (int)strPath.length() - 1;
        if (nLastCharPos > 0 && strPath[nLastCharPos] != '\\' && strPath[nLastCharPos] != '/')
        {
            CHNDumpHelper::instance().m_DumpFilePath += "/";
        }
        CHNDumpHelper::instance().m_DumpFilePath += strFile;
        ::SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)HNExceptionCrashHandler);
        return result;
#else
        return result.SetFail("unsupported.");
#endif
    }

private:
    explicit CHNDumpHelper() :m_DumpFilePath(""){}
    virtual ~CHNDumpHelper(){}
    static CHNDumpHelper& instance()
    {
        static CHNDumpHelper obj;
        return obj;
    }
    std::string m_DumpFilePath;

#if defined(HNOS_WIN)
    /**
    *@brief        CRT运行异常
    */
    static LONG HNExceptionCrashHandler(EXCEPTION_POINTERS *pException)
    {
        // 创建Dump文件
        HANDLE hDumpFile = CreateFileA(CHNDumpHelper::instance().m_DumpFilePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hDumpFile != INVALID_HANDLE_VALUE)
        {
            // Dump信息
            MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
            dumpInfo.ExceptionPointers = pException;
            dumpInfo.ThreadId = HNGetTID();
            dumpInfo.ClientPointers = TRUE;
            // 写入Dump文件内容
            if (!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL))
            {
                printf("MiniDumpWriteDump (%s) fail! err:%d", CHNDumpHelper::instance().m_DumpFilePath.c_str(), (int)HNGetLastError());
            }
            CloseHandle(hDumpFile);
        }
        return EXCEPTION_EXECUTE_HANDLER;
    }
#endif
};

#endif // __HELLO_NET_M_CORE_S_DEBUG__