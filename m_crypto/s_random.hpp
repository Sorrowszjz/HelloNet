#ifndef __HNET_M_CRYPTO_S_RANDOM_HPP__
#define __HNET_M_CRYPTO_S_RANDOM_HPP__

#include "../m_core/m_core.hpp"

/**
*@brief        基于MT19937算法的随机数类
*/

class CHNRandom : CHNNoCopyable
{
public:
    /**
    *@brief     随机生成一个[start,end]范围内的无符号整数。
    *@param     start    [in]    起始值(包括)
    *@param     end      [in]    结束值(包括)
    *@return    结果。
    */
    static unsigned int HNGenRangeAt(unsigned int start = 0, unsigned int end = 0xFFFFFFFF)
    {

    }

    /**
    *@brief     随机生成一个[start,end]范围内的浮点数。
    *@param     start    [in]    起始值(包括)
    *@param     end      [in]    结束值(包括)
    *@return    结果。
    */
    static double HNGenDoubleRangeAt(double start, double end)
    {

    }

    /**
    *@brief     随机生成一个结果的每个字节都在scope范围内的字符串。
    *@param     len          [in]    要生成的字符串长度
    *@param     scope        [in]    范围，如果为空，代表不限制结果的范围(用来生成字节数组)。
    *@return    结果
    */
    static std::string HNGenString(size_t len, const std::string& scope = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789")
    {

    }

    /**
    *@brief     洗牌算法，打乱数据。
    *@param     src         [in]    要打乱数据。
    *@return    结果
    */
    static std::string HNShuffle(const std::string& src)
    {

    }

};

#endif // __HNET_M_CRYPTO_S_RANDOM_HPP__