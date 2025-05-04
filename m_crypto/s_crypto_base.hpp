#ifndef __HNET_M_CRYPTO_S_CRYPTO_BASE_HPP__
#define __HNET_M_CRYPTO_S_CRYPTO_BASE_HPP__

#include "../m_core/m_core.hpp"

/**
*@brief     获取文件hash的函数
*@param     pszPath    [in]  文件路径
*@return    是否成功。成功时std::string为hash算法的十六进制可见字符串
*@note      传入的HASHBASE必须是哈希算法，需满足包含Init/Update/Finish三个方法
*           示例：auto md5 = PCGetFileHashHex<CPCMD5>(pszPath);
*/
template<typename HASHBASE>
static CHNResult<std::string> HNGetFileHashHex(const char* pszPath)
{

}



/**
*@brief     直接获取字节或字符串的 hmac
*@param     src[in]         输入的数据
*@param     srclen[in]      数据长度
*@param     keybytes[in]    密钥
*@param     keylen[in]      密钥长度
*@param     blockSize[in]   分块长度
*@param     digitSize[in]   算法结果长度
*@return    hmac
*@note      传入的HASHBASE必须是哈希算法，需满足包含Init/Update/Finish三个方法
*           示例：auto sha1hmac = PCGetBytesHMAC<CPCSHA1>(src,srclen,keybytes,keylen,64,PCSHA1_LEN);
*/

template <typename HASHBASE>
static std::string HNGetBytesHMAC(const void *src, size_t srclen,
    const void *keybytes, size_t keylen, 
    size_t blockSize, size_t digitSize)
{
    
}


/**
*@brief     填充模式
*/
enum HNPaddingMode
{
    eHNPaddingModeNone = 0, //0x00填充，此时无法智能移除填充
    eHNPaddingModePKCS7,    //PKCS7/PKCS5模式填充：填充字符串由一个字节序列组成，每个字节填充该字节序列的长度。
    eHNPaddingModePBOC,     //PBOC模式：将待加密字符串强制填充0x80，如果此时长度还未达到BLOCK(DES为8)的最小整数倍则填充0x00到BLOCK的最小整数倍。
};

/**
*@brief		根据填充模式获取最后一个分组的数据和原始数据分组后的大小。
*@param		pszPaddingMode	[IN]	填充模式。详细见PCSymEncypt函数注释。
*@param		pszSrc			[IN]	输入的数据
*@param		nSrcLen			[IN]	输入的数据长度
*@param		nBlockLen		[IN]	分组的长度，如DES/3DES为8字节，AES为16字节
*@param		pszDest			[OUT]	最后一个分组的数据缓冲区，缓冲区长度必须大/等于nBlockLen，返回后的数据长度为nBlockLen字节。
*@return	返回原始数据分组后的大小，失败返回-1
*/
static int HNCryptPadding(HNPaddingMode eMode, const unsigned char *pszSrc, unsigned int nSrcLen, unsigned int nBlockLen, unsigned char *pszDest)
{

}


/**
*@brief		根据填充模式获取最后一个分组的数据和原始数据分组后的大小。
*@param		pszPaddingMode	[IN]	填充模式。当pad_mode=""或NULL时，认为是指定字节0x00填充。填充模式见PCGetPaddingLastBlock函数。
*@param		pszSrc			[IN]	要解除填充的数据
*@param		nSrcLen			[IN]	要解除填充的数据长度
*@param		nBlockLen		[IN]	分组的长度，如DES/3DES为8字节，AES为16字节
*@return	成功时解除填充后的数据长度。指定字节填充不能被去除但是仍然会成功。失败时返回-1
*/
static int  HNCryptUnPadding(HNPaddingMode eMode, const unsigned char *pszSrc, unsigned int nSrcLen, unsigned int nBlockLen)
{

}

//old code


/***********************************************************************************************************************
* Routine Description:
*    填充函数。将输入数据(可选强制填充一字节然后)按照8字节分组，如果分组后最后一组数据不为8字节，采用指定字节填充。
* Arguments:
*    szSrc            [in]    需做填充的数据
*    nSrcLen          [in]    需做填充的数据长度
*    szDest           [out]   填充后的数据缓冲区，外部分配空间，缓冲区长度必须大于nSrcLen+16
*    nDestLen         [out]   输出填充后的数据长度
*    nArrayLen        [in]    分组长度,在DES中是8字节，SM4中是16字节
*    bForcePadding    [in]    是否需要进行强制填充，true需要，false不需要。
*    chForcePadding   [in]    强制在明文后填充的数据。
*    chPadding        [in]    分组后最后一组不足(nArrayLen)字节时的填充数据。
* Return Value:
*    无
************************************************************************************************************************/


static void  Pub_PaddingData(unsigned char *szSrc, unsigned int nSrcLen, unsigned char *szDest, unsigned int *nDestLen, int nArrayLen, bool bForcePadding, unsigned char chForcePadding, unsigned char chPadding)
{

}

/*ECB和CBC模式算法回调函数定义*/
typedef CHNResult<>(*_lpCallbackAlgorithm)(bool bEncrypt, const  unsigned char* szData, unsigned char* szResult, const unsigned char* szKey, unsigned int nKeyLen);

/***********************************************************************************************************************
* Routine Description:
*    对任意长度的输入使用(回调函数)算法采用ECB模式对szInput数据进行加/解密，输出放在szOutput中，如果在加密时有填充，则解密后不会自动去掉填充数据。
*    明文进行(自定义)字节分组，如果输入数据长度不是(自定义)的整数倍，则使用指定字符填充(解密模式下会报错)
* Arguments:
*    bEncrypt    [in]        是否是加密。true加密，false解密
*    szInput        [in]        输入数据buffer的指针
*    nInputLen    [in]        输入数据的长度
*    szOutput    [IN/OUT]    输出数据的buffer指针，需要调用者分配空间！
*    nOutputLen    [IN/OUT]    输出数据的长度指针。[in]传入szOutput分配空间长度，[out]传出szOutput的内容长度。建议：分配空间长度 > 输入数据长度+nAlgArrayLen字节
*    szkey        [in]        用于加密的密钥
*    nKeyLen        [in]        输入密钥key的长度
*    chPadding    [in]        输入数据不足8字节的整数倍时，填充的数据，一般为0x00
*    nAlgArrayLen[in]        算法分组长度
*    lpAlg        [in]        算法回调函数
* Return Value:
*    void
************************************************************************************************************************/

static void Pub_ECB(bool bEncrypt, unsigned char* szInput, unsigned int nInputLen, unsigned char* szOutput, unsigned int* nOutputLen, unsigned char* szKey, unsigned int nKeyLen,
    unsigned char chPadding, unsigned int nAlgArrayLen, _lpCallbackAlgorithm lpAlg)
{
    
}

/***********************************************************************************************************************
* Routine Description:
*    对任意长度的输入使用(回调函数)算法采用CBC模式对szInput数据进行加/解密，输出放在szOutput中，如果在加密时有填充，则解密后不会自动去掉填充数据。
*    明文进行(自定义)字节分组，如果输入数据长度不是(自定义)的整数倍，则使用指定字符填充(解密模式下会报错)
* Arguments:
*    bEncrypt    [in]        是否是加密。true加密，false解密
*    szInput        [in]        输入数据buffer的指针
*    nInputLen    [in]        输入数据的长度
*    szOutput    [IN/OUT]    输出数据的buffer指针，需要调用者分配空间！
*    nOutputLen    [IN/OUT]    输出数据的长度指针。[in]传入szOutput分配空间长度，[out]传出szOutput的内容长度。建议：分配空间长度 > 输入数据长度+nAlgArrayLen字节
*    szkey        [in]        用于加密的密钥
*    nKeyLen        [in]        输入密钥key的长度
*    szIV        [in]        CBC模式初始向量的指针，默认长度为nAlgArrayLen字节。如果传入空指针NULL，则默认使用nAlgArrayLen个字节0作为初始向量。
*    chPadding    [in]        输入数据不足nAlgArrayLen字节的整数倍时，填充的数据，一般为0x00
*    nAlgArrayLen[in]        算法分组长度
*    lpAlg        [in]        算法回调函数
* Return Value:
*    void
************************************************************************************************************************/

static void Pub_CBC(bool bEncrypt, unsigned char* szInput, unsigned int nInputLen, unsigned char* szOutput, unsigned int* nOutputLen, unsigned char* szKey, unsigned int nKeyLen,
    unsigned char* szIV, unsigned char chPadding, unsigned int nAlgArrayLen, _lpCallbackAlgorithm lpAlg)
{
    
}
#endif // __HNET_M_CRYPTO_S_CRYPTO_BASE_HPP__