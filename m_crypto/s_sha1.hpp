#ifndef __HNET_M_CRYPTO_S_SHA1_HPP__
#define __HNET_M_CRYPTO_S_SHA1_HPP__

#include "../m_core/m_core.hpp"
#include "s_crypto_base.hpp"

#define  HNSHA1_LEN        (20)


/**
*@brief        SHA1计算
*/

class CHNSHA1 : CHNNoCopyable
{
public:

    /**
    *@brief     直接获取字节或字符串的sha1
    *@param     src    [in]    输入的数据
    *@param     len    [in]    数据长度
    *@return    20字节的sha1
    */
    static std::string GetBytesSHA1(const void * src, size_t len)
    {

    }

    /**
    *@brief     直接获取字节或字符串的sha1 hmac
    *@param     src[in]         输入的数据
    *@param     srclen[in]      数据长度
    *@param     keybytes[in]    密钥
    *@param     keylen[in]      密钥长度
    *@return    20字节的sha1 hmac
    */
    static std::string GetBytesSHA1HMAC(const void * src , size_t srclen , const void * keybytes, size_t keylen)
    {

    }

public:
    /**
    *@brief Initialize new context
    *
    */
    void Init()
    {

    }

    /**
    * @brief Run your data through this
    *
    * @param p       Buffer to run SHA1 on
    * @param len     Number of bytes
    */
    void Update(const void *p , size_t len)
    {

    }

    /**
    * @brief Add padding and return the message digest
    * @return Generated message digest
    */
    std::string Finish()
    {

    }
private:
    /** Hash a single 512-bit block. This is the core of the algorithm.
    */
    void HNSHA1_transform(const uint8_t buffer[64])
    {

    }
private:
    uint32_t h[5];
    /**< Context state */
    uint32_t count[2];
    /**< Counter       */
    uint8_t buffer[64];
    /**< SHA-1 buffer  */
};

#endif // __HNET_M_CRYPTO_S_SHA1_HPP__