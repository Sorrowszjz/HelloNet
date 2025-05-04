#ifndef __HELLO_NET_M_CORE_S_UNITTTEST__
#define __HELLO_NET_M_CORE_S_UNITTTEST__
#include "m_core.hpp"
#include "s_base.hpp"
#include "s_stringutil.hpp"


/**
*@brief  单元测试功能
*
*/
class CHNUnitTest : CHNNoCopyable
{
public:
    CHNUnitTest(const char* pszProject, bool bVerbose = true)
        : m_strProject(pszProject)
        , m_nAllCount(0)
        , m_nFailCount(0)
        , m_bVerbose(bVerbose)
    {}

    /**
    *@brief  开始测试
    *
    */
    void Start()
    {
        m_tvStart = CHNTimeValue::Now();
        m_nAllCount = 0;
        m_nFailCount = 0;
        CHNAutoConsoleColor autoColor(true);
        printf("[%s][%s] 项目，测试开始 !!! \n", m_tvStart.Format().Get().c_str(), m_strProject.c_str() );
        
    }

    /**
    *@brief  完成测试并报告测试情况
    *
    */
    bool EndAndReport()
    {
        CHNTimeValue tvEnd = CHNTimeValue::Now();
        auto tvAll = tvEnd - m_tvStart;
        auto tvAllStr = CHNStrUtil::Int64ToString(tvAll);

        CHNAutoConsoleColor autoColor(m_nFailCount == 0);
        printf("[%s][%s] 项目，总共执行测试案例[%d]个，失败[%d]个，测试耗时[%s]毫秒, 测试[%s] !!! \n", 
            tvEnd.Format().Get().c_str(),m_strProject.c_str(), m_nAllCount, m_nFailCount, tvAllStr.c_str(),
            m_nFailCount == 0 ? "通过" : "不通过");
        return (m_nFailCount == 0);
    }

    //检测一段内存是否相等
    bool CheckMemEqual(const char* item, void* src, void* dst, size_t len)
    {
        m_nAllCount++;
        if (src == NULL && dst == NULL)
        {
            if (m_bVerbose)
            {
                CHNAutoConsoleColor autoColor(true);
                printf("[%s] 案例，测试 [通过]\n", item);
            }
            return true;
        }
        else if (src == NULL || dst == NULL)
        {
            if (m_bVerbose)
            {
                CHNAutoConsoleColor autoColor(false);
                printf("[%s] 案例，测试 [不通过]，其中一个为NULL，另外一个不为NULL\n", item);
            }
            return false;
        }
        else if (memcmp(src, dst, len) != 0)
        {
            m_nFailCount++;
            if (m_bVerbose)
            {
                auto srchex = CHNStrUtil::HNBytes2Dsp(src, len);
                auto dsthex = CHNStrUtil::HNBytes2Dsp(dst, len);
                CHNAutoConsoleColor autoColor(false);
                printf("[%s] 案例，测试 [不通过]， 内存数据不相等。src=[%s],dst=[%s]\n", item, srchex.c_str(), dsthex.c_str());
            }
            return false;
        }
        else
        {
            if (m_bVerbose)
            {
                CHNAutoConsoleColor autoColor(true);
                printf("[%s] 案例，测试 [通过]\n", item);
            }
            return true;
        }
    }

    //检测两个值相等，可以自定义比较函数
    template< typename T>
    bool CheckEqual(const char* item, T src, T dst, std::function<bool(T a, T b)> compareFunc = IsFullSame<T>)
    {
        m_nAllCount++;
        if (!compareFunc(src,dst))
        {
            m_nFailCount++;
            if (m_bVerbose)
            {
                CHNAutoConsoleColor autoColor(false);
                printf("[%s] 案例，测试 [不通过]， 两个数据值不相等\n", item);
            }
            return false;
        }
        else
        {
            if (m_bVerbose)
            {
                CHNAutoConsoleColor autoColor(true);
                printf("[%s] 案例，测试 [通过]\n", item);
            }
            return true;
        }
    }

    //自定义函数检测
    bool Check(const char* item, std::function<bool()> compareFunc)
    {
        m_nAllCount++;
        bool bSucc = compareFunc();
        if (!bSucc)
            m_nFailCount++;
        if (m_bVerbose)
        {
            CHNAutoConsoleColor autoColor(bSucc);
            printf("[%s] 案例，测试 [%s]\n", item, (bSucc ? "通过" : "不通过"));
        }
        return bSucc;
    }

public:
    //自定义比较函数
    template< typename T>
    static bool IsFullSame(T a, T b)
    {
        return a == b;
    }

private:
    std::string m_strProject;
    int m_nAllCount;
    int m_nFailCount;
    bool m_bVerbose;
    CHNTimeValue m_tvStart;
};

#endif //__HELLO_NET_M_CORE_S_UNITTTEST__