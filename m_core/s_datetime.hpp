#ifndef __HELLO_NET_M_CORE_S_DATATIME__
#define __HELLO_NET_M_CORE_S_DATATIME__
#include "s_base.hpp"
/**
*@brief        时间类，统一提供时间处理功能
*/
class CHNTimeValue
{
public:
    /**
    *@brief        用(从1970年1月1日以来的毫秒数)构造时间，默认初始化为0
    */
    CHNTimeValue() :m_TimeMsValue(0){}
    ~CHNTimeValue(){}

    /**
    *@brief        通过传入的时间初始化时间值，必须显式构造，主要是为了防止将日历时间和+-的毫秒时间混淆
    */
    explicit CHNTimeValue(int64_t nTimeMsValue) :m_TimeMsValue(nTimeMsValue){}

    /**
    *@brief        重载=(赋值)操作符，将time_t、timeval、时间字符串(例:2016-05-08 12:34:56)转换为CHNTimeValue
    */
    CHNTimeValue& operator= (int64_t nTimeMsValue)  { m_TimeMsValue = nTimeMsValue; return *this; }
    CHNTimeValue& operator= (timeval tvTime)        { m_TimeMsValue = ((int64_t)tvTime.tv_sec) * 1000 + (((int64_t)tvTime.tv_usec) / 1000); return *this; }

    /**
    *@brief        重载比较操作符，判断两个CHNTimeValue的大小
    */
    bool operator==(const CHNTimeValue& rhs) const        { return (m_TimeMsValue == rhs.m_TimeMsValue); }
    bool operator!=(const CHNTimeValue& rhs) const        { return (m_TimeMsValue != rhs.m_TimeMsValue); }
    bool operator> (const CHNTimeValue& rhs) const        { return (m_TimeMsValue > rhs.m_TimeMsValue); }
    bool operator< (const CHNTimeValue& rhs) const        { return (m_TimeMsValue < rhs.m_TimeMsValue); }
    bool operator>=(const CHNTimeValue& rhs) const        { return (m_TimeMsValue >= rhs.m_TimeMsValue); }
    bool operator<=(const CHNTimeValue& rhs) const        { return (m_TimeMsValue <= rhs.m_TimeMsValue); }

    /**
    *@brief        重载-(时间)操作符，表示两个时间之间的间隔(毫秒)
    */
    int64_t operator-(const CHNTimeValue& rhs) const      { return m_TimeMsValue - rhs.m_TimeMsValue; }

    /**
    *@brief        重载+ -(毫秒)操作符，表示将对象的时间增加或减少多少毫秒，返回一个新对象
    */
    const CHNTimeValue operator+(int64_t nMs) const    { return CHNTimeValue(m_TimeMsValue + nMs); }
    const CHNTimeValue operator-(int64_t nMs) const    { return CHNTimeValue(m_TimeMsValue - nMs); }

    /**
    *@brief        重载+= -=(毫秒)操作符，表示在当前对象的基础上将时间增加或减少多少毫秒
    */
    CHNTimeValue& operator += (int64_t nMs)            { m_TimeMsValue += nMs; return *this; }
    CHNTimeValue& operator -= (int64_t nMs)            { m_TimeMsValue -= nMs; return *this; }

    /**
    *@brief        取出日历时间
    */
    int64_t GetValue(void) const                    { return m_TimeMsValue; }

    /**
    *@brief        将时间字符串(例:2016-05-08 12:34:56)转换为CHNTimeValue
    */
    CHNResult<> SetValue(const char * pszTime)
    {
        //解析原串到tm结构体
        struct tm tmTimeMsValue;
        auto result = CHNTimeValue::Convert(pszTime, tmTimeMsValue);
        if (!result) result;

        //tm转换为秒数
        int64_t tMakeTime = mktime(&tmTimeMsValue);
        if (tMakeTime < 0)
        {
            return result.SetSystemFail();
        }

        m_TimeMsValue = ((int64_t)tMakeTime) * 1000;
        return result;
    }

    /**
    *@brief     按照C标准格式化时间到字符串，扩展HN_MSFORMATSTR代表毫秒,存在多个HN_MSFORMATSTR时只有第一个才有效。
    *@param     pszFormatStr[in]    格式串，如"%Y-%m-%d %H:%M:%S SSS"
    *@param     utc[in]             true:utc时间；false:本地时间
    *@return    是否成功。成功：[格式化后的字符串]    
    */
    CHNResult<std::string> Format(const char * pszFormatStr = "%Y-%m-%d %H:%M:%S SSS", bool utc = false) const
    {
        CHNResult<std::string> result;
        if (pszFormatStr == NULL || pszFormatStr[0] == 0)
        {
            return result.SetSucc("");
        }
        if ((strlen(pszFormatStr) + 19) > MAX_PATH)
        {
            return result.SetFail("params INVALID or param len TOO LONG.pszFormatStr:%s", pszFormatStr);
        }

        //分离成秒和毫秒
        struct    tm    tmValue;
        unsigned int uMsValue = 0;
        auto result2 = Format(tmValue, &uMsValue,utc);
        if(!result2)
        {
            return result.SetFail("Format fail:%s",result2.ErrDesc());
        }

        //按照C标准格式化时间
        char szFormatted[MAX_PATH] = { 0 };
        size_t nSize = strftime(szFormatted, sizeof(szFormatted), pszFormatStr, &tmValue);
        if (nSize == 0)
        {
            return result.SetSystemFail();
        }

        //处理毫秒
        char * pMsPos = strstr(szFormatted, HN_MSFORMATSTR);
        if (NULL == pMsPos)
        {
            return result.SetSucc(szFormatted);
        }
        pMsPos[0] = '0' + uMsValue / 100;
        pMsPos[1] = '0' + uMsValue / 10 % 10;
        pMsPos[2] = '0' + uMsValue % 10;

        return result.SetSucc(szFormatted);
    }

    /**
    *@brief     格式化时间到tm[可读取出年月日时分秒]和毫秒部分[0-999]。
    *@param     tmValue     [out]    输出到tm
    *@param     uMsValue    [out]    输出到毫秒部分，如果传入空指针，会忽略毫秒
    *@param     utc         [in]    true:utc时间；false:本地时间
    *@return    是否成功
    */
    CHNResult<> Format(struct tm &tmValue, unsigned int * uMsValue = NULL, bool utc = false) const
    {
        //分离成秒
        CHNResult<> result;
        time_t nSec = m_TimeMsValue / 1000;

        //将time_t转换为tm结构体
        if(utc)
        {
    #if defined(HNOS_WIN)
            if (0 != gmtime_s(&tmValue, &nSec))
    #else
            if (NULL == gmtime_r(&nSec, &tmValue))
    #endif
            {
                return result.SetSystemFail();
            }
        }
        else
        {
    #if defined(HNOS_WIN)
            if (0 != localtime_s(&tmValue, &nSec))
    #else
            if (NULL == localtime_r(&nSec, &tmValue))
    #endif
            {
                return result.SetSystemFail();
            }
        }

        //分离毫秒
        if (uMsValue != NULL)
        {
            *uMsValue = (unsigned int)(m_TimeMsValue % 1000);
        }
        return result;
    }

    /**
    *@brief     判断当前时间是否已经超过this对象最后一次设定的时间。使用TickCount
    *@param     nTimeOutMs     [out]    毫秒。如果nTimeoutMs < 0 代表永不超时(总是返回false).
    *@return    是否超时
    */
    bool IsTimeOut(int64_t nTimeOutMs) const
    {
        return ((nTimeOutMs < 0) ? false : (CHNTimeValue::TickCount().GetValue() - m_TimeMsValue > nTimeOutMs));
    }

    /**
    *@brief     和另外时间比较，判断是否在同一天。如果比较产生异常，默认返回false
    *@param     tmOther        [out]    另外的时间tm对象
    *@return    是否在同一天
    */
    bool IsEqualDay(const struct tm& tmOther) const
    {
        struct tm tmCurr;
        auto result = Format(tmCurr);
        if (!result)
            return false;
        return (tmOther.tm_mday == tmCurr.tm_mday && tmOther.tm_mon == tmCurr.tm_mon && tmOther.tm_year == tmCurr.tm_year);
    }

    /**
    *@brief     返回当前系统的日历时间(从1970年1月1日以来的毫秒数)
    */
    static CHNTimeValue Now()
    {
#if defined(HNOS_WIN)
        struct timeb tbTime;
        ftime(&tbTime);
        return CHNTimeValue(((int64_t)tbTime.time) * 1000 + tbTime.millitm);
#else
        /* 需要加上编译选项 -lrt */
        timeval tv;
        int nRet = gettimeofday(&tv, NULL);
        if (nRet != 0)
        {
            throw ("CHNTimeValue::Now::gettimeofday() fail");
            return CHNTimeValue(0);
        }
        return CHNTimeValue((((int64_t)tv.tv_sec) * 1000) + (((int64_t)tv.tv_usec) / 1000));
#endif
    }

    /**
    *@brief      设置系统时间 时间字符串(输入示例:2016-05-08 12:34:56)
    */
    static CHNResult<> SetSystemTime(const char * pszTime)
    {
        //解析原串到tm结构体
        struct tm tmTimeMsValue;
        auto result = CHNTimeValue::Convert(pszTime, tmTimeMsValue);
        if (!result) result;
#if defined(HNOS_WIN)
        SYSTEMTIME st = { (unsigned short)(1900 + tmTimeMsValue.tm_year),
            (unsigned short)(1 + tmTimeMsValue.tm_mon),
            (unsigned short)tmTimeMsValue.tm_wday,
            (unsigned short)tmTimeMsValue.tm_mday,
            (unsigned short)tmTimeMsValue.tm_hour,
            (unsigned short)tmTimeMsValue.tm_min,
            (unsigned short)tmTimeMsValue.tm_sec, 0 };
        //提升权限
        result = CHNPrivilege::Adjust(SE_SYSTEMTIME_NAME);
        if (!result)
        {
            return result;
        }
        if (!::SetLocalTime(&st))
        {
            return result.SetSystemFail();
        }
#else
        struct timeval     time_tv;
        time_tv.tv_sec = ::mktime(&tmTimeMsValue);
        if (time_tv.tv_sec < 0)
        {
            return result.SetSystemFail();
        }
        time_tv.tv_usec = 0;
        if (settimeofday(&time_tv, NULL) != 0)
        {
            return result.SetSystemFail();
        }
#endif
        return result;
    }

    /**
    *@brief     返回系统启动到现在的毫秒数即TickCount.
    */
    static CHNTimeValue TickCount()
    {
#if defined(HNOS_WIN)
        return CHNTimeValue(::GetTickCount());

        /*
        //高精度计时，执行有点慢，放弃使用。
        static LARGE_INTEGER TicksPerSecond = { 0 };
        if (!TicksPerSecond.QuadPart)
        {
        if(!QueryPerformanceFrequency(&TicksPerSecond))
        {
        CHNAssertLog::obj().PushAssert("CHNTimeValue::TickCount::QueryPerformanceFrequency() fail.err: %s", CHNResult::HNGetSysErrorMsg(HNGetLastError()));
        return CHNTimeValue(0);
        }
        }
        LARGE_INTEGER Tick;
        if(!QueryPerformanceCounter(&Tick))
        {
        CHNAssertLog::obj().PushAssert("CHNTimeValue::TickCount::QueryPerformanceCounter() fail.err: %s", CHNResult::HNGetSysErrorMsg(HNGetLastError()));
        return CHNTimeValue(0);
        }

        int64_t Seconds = Tick.QuadPart / TicksPerSecond.QuadPart;
        int64_t LeftPart = Tick.QuadPart - (TicksPerSecond.QuadPart*Seconds);
        int64_t MillSeconds = LeftPart * 1000 / TicksPerSecond.QuadPart;
        int64_t nRet = Seconds * 1000 + MillSeconds;
        if(nRet <= 0)
        {
        CHNAssertLog::obj().PushAssert("CHNTimeValue::TickCount::nRet <= 0l.err: %s", CHNResult::HNGetSysErrorMsg(HNGetLastError()));
        return CHNTimeValue(0);
        }
        return CHNTimeValue(nRet);*/
#else
        /* 需要加上编译选项 -lrt */
        struct timespec ts;
        int nRet = clock_gettime(CLOCK_MONOTONIC, &ts);
        if (nRet != 0)
        {
            throw ("CHNTimeValue::Now::clock_gettime(CLOCK_MONOTONIC) fail.err");
            return CHNTimeValue(0);
        }
        return CHNTimeValue((((int64_t)ts.tv_sec) * 1000) + (((int64_t)ts.tv_nsec) / 1000000));
#endif
    }

    /**
    *@brief     补齐linux下的GetLocalTime函数
    */
    static bool HNGetLocalTime(SYSTEMTIME_WIN *pSystem)
    {
        if (pSystem == NULL)
        {
            return false;
        }
#if defined(HNOS_WIN)
        ::GetLocalTime((SYSTEMTIME*)pSystem);
#else
        //获取time_t
        timeval tv;
        int nRet = gettimeofday(&tv, NULL);
        if (nRet != 0)
        {
            return false;
        }
        int64_t m_TimeMsValue = (((int64_t)tv.tv_sec) * 1000) + (((int64_t)tv.tv_usec) / 1000);
        time_t nSec = m_TimeMsValue / 1000;

        //将time_t转换为tm结构体
        tm tmValue;
        if (NULL == localtime_r(&nSec, &tmValue))
        {
            return false;
        }

        pSystem->wYear = tmValue.tm_year + 1900;
        pSystem->wMonth = tmValue.tm_mon + 1;
        pSystem->wDay = tmValue.tm_mday;
        pSystem->wHour = tmValue.tm_hour;
        pSystem->wMinute = tmValue.tm_min;
        pSystem->wSecond = tmValue.tm_sec;
        pSystem->wDayOfWeek = tmValue.tm_wday;
        pSystem->wMilliseconds = (unsigned short)(m_TimeMsValue % 1000);
#endif
        return true;
    }

    /**
    *@brief     将系统时间转化为字符串(例:2016-05-08 12:34:56.323 0)
    *@return    时间字符串。格式为： "年-月-日 时:分:秒.毫秒 星期几"
	*@note		0代表星期日，1代表星期一，..
    */
    static std::string SystemTimeFormat(const SYSTEMTIME_WIN& sysTime)
    {
        char szFormatted[MAX_PATH] = {0};
        snprintf(szFormatted, sizeof(szFormatted)-1,"%04u-%02u-%02u %02u:%02u:%02u.%03u %u", 
            (unsigned int)sysTime.wYear,
            (unsigned int)sysTime.wMonth, 
            (unsigned int)sysTime.wDay, 
            (unsigned int)sysTime.wHour, 
            (unsigned int)sysTime.wMinute, 
            (unsigned int)sysTime.wSecond, 
            (unsigned int)sysTime.wMilliseconds,
            (unsigned int)sysTime.wDayOfWeek);
        return szFormatted;
    }

    /**
    *@brief     将时间字符串转换为SYSTEMTIME_WIN结构体(例:2016-05-08 12:34:56.323 7)
    *           时间字符串格式为： "年-月-日 时:分:秒.毫秒 星期几"
    */
    static CHNResult<> Convert(const char * pszTime, SYSTEMTIME_WIN& sysTime)
    {
        CHNResult<> result;
        if (pszTime == NULL || strlen(pszTime) != 25)
        {
            return result.SetFail("param error!pszTime:%s", pszTime);
        }

        char szYear[5] = { 0 }, szMonth[3] = { 0 }, szDay[3] = { 0 },
            szHour[3] = { 0 }, szMinute[3] = { 0 }, szSecond[3] = { 0 },
            szMilliseconds[4] = { 0 }, szDayOfWeek[2] = { 0 };
        memcpy(szYear, pszTime, 4);
        memcpy(szMonth, pszTime + 5, 2);
        memcpy(szDay, pszTime + 8, 2);
        memcpy(szHour, pszTime + 11, 2);
        memcpy(szMinute, pszTime + 14, 2);
        memcpy(szSecond, pszTime + 17, 2);
        memcpy(szMilliseconds, pszTime + 20, 3);
        memcpy(szDayOfWeek, pszTime + 24, 1);
        sysTime.wYear = atoi(szYear);
        sysTime.wMonth = atoi(szMonth);
        sysTime.wDay = atoi(szDay);
        sysTime.wHour = atoi(szHour);
        sysTime.wMinute = atoi(szMinute);
        sysTime.wSecond = atoi(szSecond);
        sysTime.wMilliseconds = atoi(szMilliseconds);
        sysTime.wDayOfWeek = atoi(szDayOfWeek);
        return result;
    }

    /**
    *@brief     将时间字符串(例:2016-05-08 12:34:56)转换为tm
    */
    static CHNResult<> Convert(const char * pszTime, struct tm& tmTimeMsValue)
    {
        //19:标准时间戳(形式为 2014-08-12 12:08:00)字符串的长度
        CHNResult<> result;
        if (pszTime == NULL || strlen(pszTime) != 19)
        {
            return result.SetFail("param error!pszTime:%s", pszTime);
        }

        //解析原串到tm结构体
        char szYear[5] = { 0 }, szMonth[3] = { 0 }, szDay[3] = { 0 }, szHour[3] = { 0 }, szMinute[3] = { 0 }, szSecond[3] = { 0 };
        memcpy(szYear, pszTime, 4);
        memcpy(szMonth, pszTime + 5, 2);
        memcpy(szDay, pszTime + 8, 2);
        memcpy(szHour, pszTime + 11, 2);
        memcpy(szMinute, pszTime + 14, 2);
        memcpy(szSecond, pszTime + 17, 2);
        tmTimeMsValue.tm_year = atoi(szYear) - 1900;
        tmTimeMsValue.tm_mon = atoi(szMonth) - 1;
        tmTimeMsValue.tm_mday = atoi(szDay);
        tmTimeMsValue.tm_hour = atoi(szHour);
        tmTimeMsValue.tm_min = atoi(szMinute);
        tmTimeMsValue.tm_sec = atoi(szSecond);
        return result;
    }

    /**
    *@brief     在某个年月上增加N个月，得到新的年月
    *@param     year[in/out]    输入年，输出新年
    *@param     month[in/out]   输入月，输出新月
    *@param     N[in]           增加N个月，负数代表减少-N个月
    *@return    是否成功
    */
    static bool AddMonth(int& year, int& month, int N)
    {
        if(year < 0 || year > 1000000 || month < 0 || month > 12)
            return false;
        int totalmonth = year*12+month+N-1;
        year = totalmonth / 12;
        month = totalmonth % 12 + 1;
        return (year >= 0);
    }

    /**
    *@brief     在某个年月日上增加N天，得到新的年月日
    *@param     year[in/out]    输入年，输出新年
    *@param     month[in/out]   输入月，输出新月
    *@param     day[in/out]     输入日，输出新日
    *@param     N[in]           增加N天，负数代表减少-N天
    *@return    是否成功
    */
    static bool AddDay(int& year, int& month, int& day, int N)
    {
        bool bAdd = (N >= 0);
        int  leftN = N;
        while(1)
        {
            if(leftN == 0)
                break;
            int dayCount = CHNTimeValue::GetDays( year,  month);
            if(dayCount == 0 || day < 0 || day > dayCount)
                return false;

            if(bAdd)
            {
                int maxAddDay = dayCount - day;
                if( maxAddDay >= leftN )
                {
                    //在本月就处理了
                    day += leftN;
                    break;
                }
                else
                {
                    //跨月
                    if(!AddMonth(year, month, 1))
                        return false;
                    day = 0;
                    leftN -= maxAddDay;
                }
            }
            else
            {
                int maxAddDay = day ;
                if( maxAddDay > (-leftN) )
                {
                    //在本月就处理了
                    day += leftN;
                    break;
                }
                else
                {
                    //跨月
                    if(!AddMonth(year, month, -1))
                        return false;
                    day = CHNTimeValue::GetDays( year,  month);
                    leftN += maxAddDay;
                }
            }
        }

        return true;
    }

    /**
    *@brief     获取对应年月份的天数
    *@param     year    [in]    对应年份，支持范围【1800,】
    *@param     month   [in]    对应月份
    *@return    天数，不支持或参数错误返回0
    */
    static int GetDays(int year, int month)
    {
        if (year < 1800)
            return 0;
        switch (month)
        {
        case 1:case 3:case 5: case 7: case 8: case 10:case 12:
            return 31;
        case 4:case 6:case 9:case 11:
            return 30;
        case 2:
            return (CHNTimeValue::IsLeapYear(year) ? 29 : 28);
        default:
            return 0;
        }
    }

    /**
    *@brief     判断是否为闰年
    *@param     year[in]    年份
    *@return    闰年返回true，否则为false
    */
    static bool IsLeapYear(int year)
    {
        return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
    }

#if defined(HNOS_WIN)
#else
    /**
    *@brief      将超时的毫秒数转换为timespec
    */
    static void Convert(unsigned int nTimeOutMs, struct timespec &ts, bool bMONOTONIC = true)
    {
        int nRet = clock_gettime(bMONOTONIC ? CLOCK_MONOTONIC : CLOCK_REALTIME, &ts);
        if (nRet != 0)
        {
            throw ("Convert,clock_gettime fail.err");
            return;
        }
        static int64_t nanosec = 1000 * 1000 * 1000;
        int64_t msecs = nTimeOutMs%nanosec + ts.tv_nsec;
        ts.tv_sec += (msecs / nanosec + nTimeOutMs / 1000);
        ts.tv_nsec = msecs%nanosec;
    }
#endif

protected:
    //内部保存日历时间(从1970年1月1日以来的毫秒数)
    int64_t m_TimeMsValue;
};




#endif //__HELLO_NET_M_CORE_S_DATATYPES__