#ifndef __HELLO_NET_M_CORE_S_LOGGER__
#define __HELLO_NET_M_CORE_S_LOGGER__
#include "m_core.hpp"
#include "s_base.hpp"

/**
*@brief 仿log4j日志对象appender，可独立使用
*/
class CHNLog : CHNNoCopyable
{
public:
    //这些级别是从低到高的日志级别。设置为某个级别后，对应级别及其以上的级别会写入日志。
    //比如在这里定义了INFO级别， 则INFO/WARN/ERROR级别的日志会写入文件。特殊的eOFF级别被设置后会彻底关闭日志。
    enum eHNLogThreshold
    {
        eTRACE = 0,
        eDEBUG,
        eINFO,
        eWARN,
        eERROR,
        eOFF
    };
public:
    explicit CHNLog(size_t buffsize = HN_MAXHEAP)
        :m_Threshold(eDEBUG)
        , m_MaxFileSize(0)
        , m_DatePattern("Default.log")
        , m_bErrPrint(true)
        , m_bSplitBytes(false)
        , m_pLogFile(NULL)
        , m_pFmtBuff(NULL)
        , m_fmtBufSize(buffsize)
    {
        m_pFmtBuff = new char[m_fmtBufSize];
        memset(m_pFmtBuff, 0, m_fmtBufSize);
        SetLogAttr("");
    }
    ~CHNLog()
    {
        HNCloseFILE(m_pLogFile);
        HNDeleteArr(m_pFmtBuff);
    }

    /**
    *@brief 开箱即用的默认日志对象指针，你也可以自己创建其他日志对象
    *        警告！！默认的日志对象指针第一行日志是非线程安全的，后续的所有日志是线程安全的
    *        所以，一般的做法是在main函数第一行写一行日志，后面的应用逻辑创建线程后在多线程环境下写的日志才线程安全
    */
    static CHNLog* Default()
    {
        static CHNLog o;
        return &o;
    }

public:
    /**
    *@brief     设置写日志属性
    *@param     DatePattern       [in]    日志保存的文件全路径。空代表写入exe所在目录。通配符支持%Y %m %d，例如 D:\\KIOSK\\Log\\%Y-%m\\%Y-%m-%d\\APPLOG\\MyApp.%Y-%m-%d.txt
    *@param     Threshold         [in]    日志阈值，通过阈值控制什么级别的日志会被写入
    *@param     bErrPrint         [in]    是否将警告和错误日志打印到控制台
    *@param     bSplitBytes       [in]    二进制数据是否要用空格隔开
    *@param     MaxFileSize       [in]    日志文件最大大小(字节)，等于0则不限制【暂不支持】
    *@return    是否成功。失败时使用默认配置
    */
    CHNResult<> SetLogAttr(const std::string& DatePattern, eHNLogThreshold Threshold = eDEBUG,
        bool bErrPrint = true, bool bSplitBytes = false, uint64_t MaxFileSize = 0)
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);
        CHNResult<> result;
        m_Threshold = Threshold;
        m_MaxFileSize = MaxFileSize;
        m_bErrPrint = bErrPrint;
        m_bSplitBytes = bSplitBytes;
        
        auto result2 = CHNFileUtil::HNGetExecuteFileDir();
        if (!result2)
        {
            return result.SetFail("HNGetExecuteFileDir fail:%s",result2.ErrDesc());
        }
        m_DatePattern = result2.Get().first;
        m_DatePattern += result2.Get().second;
        m_DatePattern += ".%Y-%m-%d.log";

        //日志文件格式
        if (!DatePattern.empty() && DatePattern.length() < MAX_PATH)
        {
            auto result3 = CHNTimeValue::Now().Format(DatePattern.c_str());
            if (!result3)
            {
                return result.SetFail("Format fail:%s",result3.ErrDesc());
            }
            m_DatePattern = DatePattern;
        }
        CHNFileUtil::HNPathRecovery(m_DatePattern);

        //设置的时候要关闭上次打开的重新确定写日志状态
        HNCloseFILE(m_pLogFile);
        return result;
    }

    /**
    *@brief     写日志，日志写完毕后添加换行符，单条日志长度有限制
    *@param     pFileName    [in]    当条日志的文件名称，可为空
    *@param     pFuncName    [in]    当条日志的函数名称，可为空
    *@param     ulLine       [in]    当条日志在文件中的行数
    *@param     nLevel       [in]    当条日志的日志等级
    *@param     pszFmt       [in]    日志内容格式串/日志内容
    *@return    是否成功
    */
    void WriteLogFmt(const char* pFileName, const char* pFuncName, unsigned long ulLine, eHNLogThreshold nLevel, const char* pszFmt, ...)
    {
        if (pszFmt == NULL || (int)nLevel >= (int)eOFF || (int)nLevel < (int)m_Threshold)
        {
            return;
        }
        CHNGuard<CHNRecMutex> guard(m_Mutex);

        //检查Logger不通过不写
        CHNTimeValue tvNow = CHNTimeValue::Now();
        if (!CheckLogger(tvNow))
        {
            return;
        }

        //格式化时间
        auto result = tvNow.Format("%Y-%m-%d %H:%M:%S SSS");
        if (!result)
        {
            printf("tvNow.Format fail:%s.", result.ErrDesc());
            return;
        }

        //格式化args
        va_list ap;
        va_start(ap, pszFmt);
        if (-1 == vsnprintf(m_pFmtBuff, m_fmtBufSize - 1, pszFmt, ap))
        {
            m_pFmtBuff[m_fmtBufSize - 1] = ('\0');
        }
        va_end(ap);

        //写入
        static const char * const m_LOG_LEVEL_NAME[] = { "TRC", "DBG", "INF", "WAN", "ERR", "OFF" };
        int nThradID = HNGetTID();
        fprintf(m_pLogFile, "[%s][%s][%05d]{%s:%03lu} %s\n",
            m_LOG_LEVEL_NAME[(int)nLevel], result.Get().c_str(), nThradID, CHNStrUtil::HNGetFileNameShort(pFileName), ulLine, m_pFmtBuff);
        if (m_bErrPrint && nLevel > eINFO)
        {
            printf("[%s][%s][%05d]{%s:%03lu} %s\n", m_LOG_LEVEL_NAME[(int)nLevel], result.Get().c_str(), nThradID, CHNStrUtil::HNGetFileNameShort(pFileName), ulLine, m_pFmtBuff);
        }

        //INFO以上级别触发立即写入
        if (nLevel >= eINFO)
        {
            fflush(m_pLogFile);
        }
    }


    /**
    *@brief     写二进制数据日志，日志写完毕后添加换行符
    *           二进制日志没有长度限制，所以如果要写巨大数据，请用这个接口。
    *@param     pFileName    [in]    当条日志的文件名称，可为空
    *@param     pFuncName    [in]    当条日志的函数名称，可为空
    *@param     ulLine       [in]    当条日志在文件中的行数
    *@param     nLevel       [in]    当条日志的日志等级
    *@param     pszTips      [in]    前导提示字符串
    *@param     pszBytes     [in]    数据日志内容
    *@param     nBytesLen    [in]    数据日志内容长度
    *@return    是否成功
    */
    void WriteLogBytes(const char* pFileName, const char* pFuncName, unsigned long ulLine, eHNLogThreshold nLevel, const char* pszTips, const void* pszBytes, unsigned int nBytesLen)
    {
        if (pszTips == NULL || (int)nLevel >= (int)eOFF || (int)nLevel < (int)m_Threshold)
        {
            return;
        }
        CHNGuard<CHNRecMutex> guard(m_Mutex);

        //检查Logger不通过不写
        CHNTimeValue tvNow = CHNTimeValue::Now();
        if (!CheckLogger(tvNow))
        {
            return;
        }

        //格式化时间
        auto result = tvNow.Format("%Y-%m-%d %H:%M:%S SSS");
        if (!result)
        {
            printf("tvNow.Format fail:%s.", result.ErrDesc());
            return;
        }

        //写入将字节数组转换为可显示的字符串
        static const char * const m_LOG_LEVEL_NAME[] = { "TRC", "DBG", "INF", "WAN", "ERR", "OFF" };
        std::string hexStr = CHNStrUtil::HNBytes2Dsp((const unsigned char*)pszBytes, nBytesLen, m_bSplitBytes ? " " : "");
        int nThradID = HNGetTID();
        fprintf(m_pLogFile, "[%s][%s][%05d]{%s:%03lu} %s %s\n",
            m_LOG_LEVEL_NAME[(int)nLevel], result.Get().c_str(), nThradID, CHNStrUtil::HNGetFileNameShort(pFileName), ulLine, pszTips, hexStr.c_str());
        if (m_bErrPrint && nLevel > eINFO)
        {
            printf("[%s][%s][%05d]{%s:%03lu} %s%s\n", m_LOG_LEVEL_NAME[(int)nLevel], result.Get().c_str(), nThradID, CHNStrUtil::HNGetFileNameShort(pFileName), ulLine, pszTips, hexStr.c_str());
        }

        //INFO以上级别触发立即写入
        if (nLevel >= eINFO)
        {
            fflush(m_pLogFile);
        }
    }

    /**
    *@brief     写纯日志，日志类不添加任何内容，只记录用户数据
    *@param     nLevel        [in]    当条日志的日志等级
    *@param     pszFmt        [in]    日志内容格式串/日志内容
    *@return    是否成功
    */
    void WriteLogPlain(eHNLogThreshold nLevel, const char* pszFmt, ...)
    {
        if (pszFmt == NULL || (int)nLevel >= (int)eOFF || (int)nLevel < (int)m_Threshold)
        {
            return;
        }
        CHNGuard<CHNRecMutex> guard(m_Mutex);

        //检查Logger不通过不写
        CHNTimeValue tvNow = CHNTimeValue::Now();
        if (!CheckLogger(tvNow))
        {
            return;
        }
        //格式化args
        va_list ap;
        va_start(ap, pszFmt);
        if (-1 == vsnprintf(m_pFmtBuff, m_fmtBufSize - 1, pszFmt, ap))
        {
            m_pFmtBuff[m_fmtBufSize - 1] = ('\0');
        }
        va_end(ap);
        //写入
        fprintf(m_pLogFile, "%s", m_pFmtBuff);
        if (m_bErrPrint && nLevel > eINFO)
        {
            printf("%s", m_pFmtBuff);
        }
        //INFO以上级别触发立即写入
        if (nLevel >= eINFO)
        {
            fflush(m_pLogFile);
        }
    }

protected:

    /**
    *@brief     检查是否应该写入日志
    *@param     tvNow        [in]    时间
    *@return    是否成功。
    */
    bool CheckLogger(const CHNTimeValue &tvNow)
    {
        //如果文件已经打开且日期相同，无需任何操作
        if (m_pLogFile != NULL && tvNow.IsEqualDay(m_tmCurrent))
        {
            return true;
        }

        //关闭已有文件
        HNCloseFILE(m_pLogFile);

        //记录最新日期
        auto result = tvNow.Format(m_tmCurrent);
        if (!result)
        {
            printf("tvNow.Format(m_tmCurrent) fail.err:%s", result.ErrDesc());
            return false;
        }

        //格式化日期文件串
        auto result2 = tvNow.Format(m_DatePattern.c_str());
        if (!result2 || result2.Get().empty())
        {
            printf("tvNow.Format(m_DatePattern:%s) fail.err:%s", m_DatePattern.c_str(), result2.ErrDesc());
            return false;
        }
        std::string strFilePath = result2.Get();

        //确保路径存在
        result = CHNFileUtil::HNCheckAndCreateDir(strFilePath.c_str());
        if (!result)
        {
            printf("HNCheckAndCreateDir(%s) fail.err:%s", strFilePath.c_str(), result.ErrDesc());
            return false;
        }

        //(重新)打开文件
        m_pLogFile = fopen(strFilePath.c_str(), "ab+");
        if (NULL == m_pLogFile)
        {
            printf("fopen(%s) fail.err:%d", strFilePath.c_str(), (int)HNGetLastError());
            return false;
        }
        return true;
    }

    //日志配置
    eHNLogThreshold m_Threshold;
    uint64_t        m_MaxFileSize;
    std::string     m_DatePattern;
    bool            m_bErrPrint;
    bool            m_bSplitBytes;

    //内部结构
    CHNRecMutex     m_Mutex;
    FILE*           m_pLogFile;
    char*           m_pFmtBuff;
    size_t          m_fmtBufSize;

    //当前文件的年月日。
    struct tm       m_tmCurrent;
};
//你可以在include "pcquicklib.h" 之前用定义HN_PROJ_LOGGER你自己的日志对象，否则默认使用单例对象
#ifndef HN_PROJ_LOGGER
#define HN_PROJ_LOGGER    CHNLog::Default()
#endif
//日志宏
#define HN_TRACE(_logFmt, ...)  if(HN_PROJ_LOGGER) HN_PROJ_LOGGER->WriteLogFmt(__FILE__,__FUNCTION__,__LINE__,CHNLog::eTRACE,_logFmt, ## __VA_ARGS__)
#define HN_DEBUG(_logFmt, ...)  if(HN_PROJ_LOGGER) HN_PROJ_LOGGER->WriteLogFmt(__FILE__,__FUNCTION__,__LINE__,CHNLog::eDEBUG,_logFmt, ## __VA_ARGS__)
#define HN_INFO(_logFmt, ...)   if(HN_PROJ_LOGGER) HN_PROJ_LOGGER->WriteLogFmt(__FILE__,__FUNCTION__,__LINE__,CHNLog::eINFO,_logFmt, ## __VA_ARGS__)
#define HN_WARN(_logFmt, ...)   if(HN_PROJ_LOGGER) HN_PROJ_LOGGER->WriteLogFmt(__FILE__,__FUNCTION__,__LINE__,CHNLog::eWARN,_logFmt, ## __VA_ARGS__)
#define HN_ERROR(_logFmt, ...)  if(HN_PROJ_LOGGER) HN_PROJ_LOGGER->WriteLogFmt(__FILE__,__FUNCTION__,__LINE__,CHNLog::eERROR,_logFmt, ## __VA_ARGS__)
#define HNBYTE_TRACE(pszTips,pszBytes,nBytesLen) if(HN_PROJ_LOGGER) HN_PROJ_LOGGER->WriteLogBytes(__FILE__,__FUNCTION__,__LINE__,CHNLog::eTRACE,pszTips,pszBytes,nBytesLen)
#define HNBYTE_DEBUG(pszTips,pszBytes,nBytesLen) if(HN_PROJ_LOGGER) HN_PROJ_LOGGER->WriteLogBytes(__FILE__,__FUNCTION__,__LINE__,CHNLog::eDEBUG,pszTips,pszBytes,nBytesLen)
#define HNBYTE_INFO(pszTips,pszBytes,nBytesLen)  if(HN_PROJ_LOGGER) HN_PROJ_LOGGER->WriteLogBytes(__FILE__,__FUNCTION__,__LINE__,CHNLog::eINFO,pszTips,pszBytes,nBytesLen)
#define HNBYTE_WARN(pszTips,pszBytes,nBytesLen)  if(HN_PROJ_LOGGER) HN_PROJ_LOGGER->WriteLogBytes(__FILE__,__FUNCTION__,__LINE__,CHNLog::eWARN,pszTips,pszBytes,nBytesLen)
#define HNBYTE_ERROR(pszTips,pszBytes,nBytesLen) if(HN_PROJ_LOGGER) HN_PROJ_LOGGER->WriteLogBytes(__FILE__,__FUNCTION__,__LINE__,CHNLog::eERROR,pszTips,pszBytes,nBytesLen)



#endif //__HELLO_NET_M_CORE_S_LOGGER__