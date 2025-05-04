#ifndef __HELLO_NET_M_CORE_S_STRINGUTIL__
#define __HELLO_NET_M_CORE_S_STRINGUTIL__

#include "s_base.hpp"

/*********************************************************************/
//seg1 字符串处理
/*********************************************************************/

/**
*@brief     字符串工具类
*/

class CHNStrUtil : CHNNoCopyable
{
public:
        /**
    *@brief     数字转字符串
    */
   static std::string IntToString(int n)
   {
       char szNumber[12] = { 0 };
       sprintf(szNumber, "%d", n);
       return szNumber;
   }
   static std::string UIntToString(unsigned int n)
   {
       char szNumber[12] = { 0 };
       sprintf(szNumber, "%u", n);
       return szNumber;
   }
   static std::string Int64ToString(int64_t n)
   {
       char szNumber[24] = { 0 };
#if defined(HNOS_WIN)
       sprintf(szNumber, "%I64d", n);
#else
       sprintf(szNumber, "%lld", n);
#endif
       return szNumber;
   }
   static std::string UInt64ToString(uint64_t n)
   {
       char szNumber[24] = { 0 };
#if defined(HNOS_WIN)
       sprintf(szNumber, "%I64u", n);
#else
       sprintf(szNumber, "%llu", n);
#endif
       return szNumber;
   }
   static std::string SizeToString(size_t n)
   {
#if HN_VAR_64BIT == 0
       return CHNStrUtil::UIntToString(n);
#else
       return CHNStrUtil::UInt64ToString(n);
#endif
   }
   static std::string SSizeToString(ssize_t n)
   {
#if HN_VAR_64BIT == 0
       return CHNStrUtil::IntToString(n);
#else
       return CHNStrUtil::Int64ToString(n);
#endif
   }
   static std::string FloatToString(double n, const char* fmtstr = "%f")
   {
       char szNumber[64] = { 0 };
       sprintf(szNumber, fmtstr, n);
       return szNumber;
   }
   static std::string BoolToString(bool n)
   {
       return n ? "true" : "false";
   }

    /**
    *@brief     字符串转数字
    */
   static int64_t StringToInt64(const char* src)
   {
       if (src == NULL || src[0] == 0)
           return 0;
#if defined(HNOS_ANDROID) && defined(__NDK_MAJOR__) && (__NDK_MAJOR__<20)
       return strtoll(src, NULL, 10);
#else
       return std::stoll(src);
#endif
   }
   static uint64_t StringToUInt64(const char* src)
   {
       if (src == NULL || src[0] == 0)
           return 0;
#if defined(HNOS_ANDROID) && defined(__NDK_MAJOR__) && (__NDK_MAJOR__<20)
       return strtoull(src, NULL, 10);
#else
       return std::stoull(src);
#endif
   }
   static long StringToUInt32(const char* src)
   {
       if(src == NULL || src[0] == 0)
         return 0;
   #if defined(HNOS_ANDROID) && defined(__NDK_MAJOR__) && (__NDK_MAJOR__<20)
       return strtoul(src,NULL,10); 
   #else
       if(src[0]== '0'){
           if(src[1]=='x' || src[1]=='X'){
               return std::stoul(src,NULL,16);
           }
           else{
               return std::stoul(src,NULL,8);
           }
       }
       return std::stoul(src);  
   #endif
   }

    /**
    *@brief     字符串格式化
    */
   static std::string Format(const char* const szFmt, ...)
   {
       if (szFmt == NULL)
       {
           return "";
       }
       HN_FORMAT_PROCESS(szFmt,
       {
           return std::string(opBuffer,nOpBufferSize);
       });
   }

    /**
    *@brief     字符串格式化成char*，需要外部释放
    *@return    字符串。需要调用者释放：delete[] ret;
    */
   static char* FormatNewCharArray(const char* const fmt, ...)
   {
       HN_FORMAT_PROCESS(fmt,
       {
           char * ret = new char[nOpBufferSize+1];
           memcpy(ret, opBuffer, nOpBufferSize);
           ret[nOpBufferSize] = 0;
           return ret;
       });
   }

       /**
    *@brief     短字符串格式化
    */
   static std::string ShortFormat(const char* const szFmt, ...)
   {
       if (szFmt == NULL)
       {
           return "";
       }
       char szContent[MAX_PATH] = { 0 };
       va_list ap;
       va_start(ap, szFmt);
       vsnprintf(szContent, sizeof(szContent) - 1, szFmt, ap);
       va_end(ap);
       return szContent;
   }

    /**
    *@brief     移除前面的空格后跳过非空格字符直到下一个空格
    */
   static char* StrSkipToken(char *p)
   {
       if (!p) return p;
       while (isspace(*p)) p++;
       while (*p && !isspace(*p)) p++;
       return p;
   }

    /**
    *@brief     StrSkipLine
    */
   static char* StrSkipLine(char *buffer, int buflen)
   {
       char *ptr = buflen ?
           (char *)memchr(buffer, '\n', buflen) : /* bleh */
           strchr(buffer, '\n');
       return ++ptr;
   }

    /**
    *@brief     移除字符串的最后n个字符
    */
   static void HNRemoveLast(char* szStr, unsigned int n)
   {
       if (szStr == NULL || szStr[0] == 0 || n == 0)
           return;
       size_t nLen = strlen(szStr);
       if (n >  nLen)
       {
           szStr[0] = 0;
           return;
       }
       szStr[nLen - n] = 0;
   }

    /**
    *@brief        从函数全名称里面提取出函数实际名称,仅供参考
    */
   static std::string HNGetFuncNameShort(const char* szFunction)
   {
       if (szFunction == NULL) return "";
       int nFuncLen = (int)strlen(szFunction);

       int left = -1;
       int right = -1;
       for (int i = nFuncLen - 1; i >= 0; i--)
       {
           if (left != -1 && right != -1)
               break;
           if (right == -1 && szFunction[i] == '(')
           {
               right = i;
               i--;
               continue;
           }
           if (right != -1 && left == -1 && (szFunction[i] == ' ' || szFunction[i] == ':'))
           {
               left = i;
           }
       }
       //if(left == -1)    left = 0;
       if (right == -1)   right = nFuncLen;
       if (left + 1 >= nFuncLen || left >= right)
           return szFunction;

       return std::string(szFunction + left + 1, right - left - 1);
   }

       /**
    *@brief        从文件全名称里面提取实际文件名称，仅供参考
    */
   static const char* HNGetFileNameShort(const char* szFilePath)
   {
       if (szFilePath == NULL) return "";

       //从后到前找分隔符
       int nPathLen = (int)strlen(szFilePath);
       for (int i = nPathLen - 1; i >= 0; i--)
       {
           if (szFilePath[i] == '\\' || szFilePath[i] == '/')
           {
               if (i >= nPathLen - 1)
                   return szFilePath;
               return szFilePath + i + 1;
           }
           if (i == nPathLen - 1 && szFilePath[i] == ':')
           {
               return szFilePath;
           }
       }
       return szFilePath;
   }

       /**
    *@brief     将字符串去掉前/后的特定字符(一般用于去空格 )
    *@param     src        `[in]    原始串
    *@param     what        [in]    移除的字符 如" \n\t\r"则表示前后移除这四个字符
    *@param     nSrcLen     [in]    原始串长度。nSrcLen < 0时，认为src为c字符串
    *@return    结果字符串
    */
   static std::string HNStrTrim(const char *src, const char *what, int nSrcLen = -1)
   {
       if (NULL == src || src[0] == 0)
       {
           return "";
       }
       std::string result;
       if (nSrcLen < 0)
           result = src;
       else
           result = result.assign(src, nSrcLen);
       HNStrTrimInplace(result, what);
       return result;
   }

    /**
    *@brief     将字符串去掉前/后的特定字符(一般用于去空格 ) 原地替换
    *@param     src         [in/out]    原始串
    *@param     what        [in]        移除的字符 如" \n\t\r"则表示前后移除这四个字符
    *@return    结果字符串
    */
   static void HNStrTrimInplace(std::string& src, const char* what)
   {
       if (NULL == what || what[0] == 0)
       {
           return;
       }
       src.erase(0, src.find_first_not_of(what));
       src.erase(src.find_last_not_of(what) + 1);
   }

       /**
    *@brief     替换全部子串
    *@param     src       [IN/OUT]    原始串输入，替换后的输出
    *@param     strOld    [in]        待替换的子串
    *@param     strNew    [in]        替换后的子串
    *@return    无
    */
   static void HNReplaceAll(std::string& src, const std::string& strOld, const std::string& strNew)
   {
       if (src.empty() || strOld.empty())
           return;
       std::string::size_type nPos = 0;
       std::string::size_type nNewLen = strNew.length();
       while ((nPos = src.find(strOld, nPos)) != src.npos)
       {
           src.replace(nPos, strOld.length(), strNew);
           nPos += nNewLen;
       }
   }

    /**
   *@brief     替换全部子串
   *@param     src[in]           原始串输入
   *@param     srcLen[in]        原始串输入长度
   *@param     strOld[in]        待替换的子串
   *@param     strNew[in]        替换后的子串
   *@return    替换后的输出
   */
   static std::string HNReplaceAllLen(const char* src, size_t srcLen, const std::string& strOld, const std::string& strNew)
   {
       if (src == NULL || srcLen == 0)
           return "";
       
       std::string ret(src,srcLen);
       std::string::size_type nPos = 0;
       std::string::size_type nNewLen = strNew.length();
       while ((nPos = ret.find(strOld, nPos)) != ret.npos)
       {
           ret.replace(nPos, strOld.length(), strNew);
           nPos += nNewLen;
       }
       return ret;
   }

   /**
   *@brief        将字符串转换为小写
   */
   static std::string  HNStrToLower(const char* src)
   {
       if (src == NULL)
       {
           return "";
       }
       std::string result = src;
       std::transform(result.begin(), result.end(), result.begin(), ::tolower);
       return result;
   }

   /**
   *@brief        将字符串转换为大写
   */
   static std::string  HNStrToUpper(const char* src)
   {
       if (src == NULL)
       {
           return "";
       }
       std::string result = src;
       std::transform(result.begin(), result.end(), result.begin(), ::toupper);
       return result;
   }

   /**
   *@brief        不区分大小写的strstr函数
   */
   static const char* HNStrStri(const char* str, const char* subStr)
   {
       if (str == NULL || subStr == NULL ||
           str[0] == 0 || subStr[0] == 0)
       {
           return NULL;
       }
       size_t lenSub = strlen(subStr);
       while (*str)
       {
           if (strncasecmp(str, subStr, lenSub) == 0)
           {
               return str;
           }
           ++str;
       }
       return NULL;
   }

   /**
   *@brief     获取字符串的子串flag后面的字符串并移除空白，如src="hello: world3 ", falg=":", 结果为 "world3"
   *@return    结果串，如果未找到，返回空串
   */
   static std::string HNStrGetFlagAfter(const char* src, const char* flag)
   {
       if (src == NULL || flag == NULL || src[0] == 0)
           return "";
       if (flag[0] == 0)
           return src;
       const char* pFlag = strstr(src, flag);
       if (pFlag == NULL)
           return "";
       return CHNStrUtil::HNStrTrim(pFlag + strlen(flag), "\t \n\r");
   }

   /**
   *@brief     字符串表示的可读字节数转换为整数，如"4 kb" => 4096
   *@return    字节数，失败返回0
   */
   static int64_t HNSizeDescToBytes(const char* src)
   {
       if (src == NULL || src[0] == 0)
           return 0;

       int64_t nTimes = 1;
       if (CHNStrUtil::HNStrStri(src, "kb"))
       {
           nTimes = 1024;
       }
       else if (CHNStrUtil::HNStrStri(src, "mb"))
       {
           nTimes = 1024 * 1024;
       }
       else if (CHNStrUtil::HNStrStri(src, "gb"))
       {
           nTimes = 1024 * 1024 * 1024;
       }
       int64_t srcBytes = CHNStrUtil::StringToInt64(src);
       return srcBytes * nTimes;
   }

   /**
   *@brief        判断字符串是不是以特定的字符串开头
   */
   static bool HNStrIsStartWith(const char* src, const char* startstr, bool bIgnoreCase)
   {
       if (src == NULL || startstr == NULL) return false;
       if (startstr[0] == 0) return true;
       if (src[0] == 0) return false;

       int nStartStrLen = (int)strlen(startstr);
       if (bIgnoreCase)
       {
           if (strncasecmp(src, startstr, nStartStrLen) == 0)
               return true;
       }
       else
       {
           if (strncmp(src, startstr, nStartStrLen) == 0)
               return true;
       }
       return false;
   }

   /**
   *@brief        判断字符串是不是以特定的字符串结尾
   */
   static bool HNStrIsEndWith(const char* src, const char* endstr, bool bIgnoreCase)
   {
       if (src == NULL || endstr == NULL) return false;
       if (endstr[0] == 0) return true;
       if (src[0] == 0) return false;

       int nSrcLen = (int)strlen(src);
       int nEndLen = (int)strlen(endstr);
       if (nSrcLen < nEndLen) return false;

       const char* pSrc = src + nSrcLen - 1;
       const char* pEnd = endstr + nEndLen - 1;
       for (; pEnd >= endstr; pSrc--, pEnd--)
       {
           if (bIgnoreCase)
           {
               if (::toupper(*pSrc) != ::toupper(*pEnd)) return false;
           }
           else
           {
               if (*pSrc != *pEnd) return false;
           }
       }
       return true;
   }

   /**
   *@brief     将字符串按照delim分割
   *@param     src      [in]    原始串
   *@param     delim    [in]    以指定多个字符 如" \n"则表示按照空格或换行分割
   *@return    分割后的列表
   */
   static std::vector<std::string> HNStrSplit(const char *src, const char *delim)
   {
       std::vector<std::string> ret;
       if (src == NULL)
       {
           return ret;
       }
       if (delim == NULL || delim[0] == 0)
       {
           ret.push_back(src);
           return ret;
       }
       std::string s = src;
       std::string::size_type lastPos = s.find_first_not_of(delim, 0);
       std::string::size_type pos = s.find_first_of(delim, lastPos);
       while (std::string::npos != pos || std::string::npos != lastPos)
       {
           ret.emplace_back(s.substr(lastPos, pos - lastPos));
           lastPos = s.find_first_not_of(delim, pos);
           pos = s.find_first_of(delim, lastPos);
       }
       return ret;
   }

   /**
   *@brief     获取字符串中被两个子字符串夹在中间的子字符串。(函数先从原串从左到右找到第一个左边串，
   然后从第一个左边串开始从左到右找到第一个右边串)如
   pszSrc：        "TABLENAME=ct_card_transfer_trade&CIPERFIELD=&KEYFIELDNAME=trd_sn"
   pszLeft：       "TABLENAME="
   pszRight：      "&"
   strResult为:    "ct_card_transfer_trade"
   *@param     pszSrc      [in]    原串，不可为空
   *@param     pszLeft     [in]    左边串，如果为空或NULL，则从最左边算起
   *@param     pszRight    [in]    边串，如果为空或NULL，则计算到最右边
   *@param     strResult   [out]   结果保存的缓冲区
   *@return    失败返回NULL，成功返回指向pszRight在从pszSrc找到第一个pszLeft后第一次出现的位置的指针。
   */
   static const char* HNGetMiddleFiled(const char * pszSrc, const char * pszLeft, const char * pszRight, std::string& strResult)
   {
       if (pszSrc == NULL || pszSrc[0] == 0)
       {
           return NULL;
       }
       if ((pszLeft == NULL || pszLeft[0] == 0) && (pszRight == NULL || pszRight[0] == 0))
       {
           strResult = pszSrc;
           return pszSrc + strlen(pszSrc);
       }

       const char * pStart;
       const char * pEnd;
       if (pszLeft == NULL || pszLeft[0] == 0)
       {
           pStart = pszSrc;
       }
       else
       {
           pStart = strstr(pszSrc, pszLeft);
           if (pStart == NULL)
           {
               return NULL;
           }
       }

       size_t nLenPos = (pszLeft == NULL) ? 0 : strlen(pszLeft);
       if (pszRight == NULL || pszRight[0] == 0)
       {
           pEnd = pszSrc + strlen(pszSrc);
       }
       else
       {
           pEnd = strstr(pStart + nLenPos, pszRight);
           if (pEnd == NULL)
           {
               return NULL;
           }
       }
       ssize_t nRetLen = pEnd - pStart - nLenPos;
       if (nRetLen < 0)
           return NULL;
       strResult.assign(pEnd - nRetLen, nRetLen);
       return pEnd;
   }

   /**
   *@brief     将真实字节数组转换为十六进制显示的字符串，如"123"转换为"313233"
   *@param     src[in]            原始串
   *@param     src_len[in]        原始串长度
   *@param     delim[in]        转换后每个字节的分隔符
   *@param     bLowerCase[in]    转换后的十六进制数据的ABCDEF是大写还是小写
   *@param     nTypeLen[in]    src的数据类型长度，默认为byte（1），可选short（2）或int（4）
   *@return    转换后的结果串
   */
   static std::string  HNBytes2Dsp(const void* src, size_t src_len, const char* delim = "", bool bLowerCase = false, int nTypeLen = 1)
   {
       size_t delim_len = delim ? strlen(delim) : 0;
       if (src == NULL || src_len == 0 || delim == NULL || delim_len > 8)
       {
           return "";
       }
       if (nTypeLen != sizeof(unsigned char) && nTypeLen != sizeof(unsigned short) && nTypeLen != sizeof(unsigned int))
       {
           nTypeLen = sizeof(unsigned char);
       }

       std::string result;
       result.reserve(src_len * (2 + delim_len) + 2);
       char pszTmpBuf[12];
       for (size_t i = 0; i < src_len / nTypeLen; i++)
       {
           if (nTypeLen == sizeof(unsigned char))
           {
               sprintf(pszTmpBuf, bLowerCase ? "%02x" : "%02X", ((unsigned char*)src)[i]);
           }
           else if (nTypeLen == sizeof(unsigned short))
           {
               sprintf(pszTmpBuf, bLowerCase ? "%04x" : "%04X", ((unsigned short*)src)[i]);
           }
           else if (nTypeLen == sizeof(unsigned int))
           {
               sprintf(pszTmpBuf, bLowerCase ? "%08x" : "%08X", ((unsigned int*)src)[i]);
           }

           result += pszTmpBuf;
           result += delim;
       }
       return result;
   }

   /**
   *@brief     将十六进制显示的字符串转换为真实字节数组，会忽略掉非十六进制字符。如"31 32 33"转换为"123"
   *@param     pszSrc        [in]    原始串
   *@return    转换后的字节数组
   */
   static std::string  HNDsp2Bytes(const char *pszSrc)
   {
       std::string ret;
       if (pszSrc == NULL)
       {
           return ret;
       }
       int sTmpVal = 0;
       unsigned char buf[3];
       for (; *pszSrc; pszSrc++)
       {
           if (!isxdigit(*pszSrc))
               continue;

           buf[0] = *pszSrc;
           pszSrc++;
           for (; *pszSrc; pszSrc++)
           {
               if (!isxdigit(*pszSrc))
                   continue;

               buf[1] = *pszSrc;
               buf[2] = 0;

               sscanf((char *)buf, "%02X", &sTmpVal);
               ret.push_back((unsigned char)sTmpVal);
               break;
           }
       }
       return ret;
   }

   /**
   *@brief     将UTF8编码的字符串进行URL编码
   *           参照RFC3986规定，URL编码时默认使用UTF8模式
   *@param     src     [in]    UTF8编码的待URL编码的原串
   *@param     mode    [in]    URL编码的模式：
   *                             nMode = 1 使用js1.5 encodeURI            82个不编码字符!#$&'()*+,/:;=?@-._~0-9a-zA-Z
   *                             nMode = 2 使用js1.5 encodeURIComponent   71个不编码字符!'()*-._~0-9a-zA-Z
   *@return    编码后的字符串
   */
   static std::string  HNUrlEncode(const char *src, int mode = 1)
   {
       //参数检查
       if (src == NULL)
       {
           return "";
       }
       if (mode < 1 || mode > 2)
       {
           return src;
       }
       std::string result;
       result.reserve((size_t)(strlen(src)*1.5f) + 1);

       //MASK没必要每次都计算，这是encodeURI计算过程示例
       /*char * what = "!#$&'()*+,/:;=?@-._~0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
       char mask[0xFF];
       memset(mask, 0, 0xFF);
       while (*what)
       {
       mask[*what] = '\1';
       what++;
       }*/
       //MASK计算结果
       static const unsigned char MASK_ENCODE_URI[0xFF] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\0\1\1\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\1\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\0\1\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
       static const unsigned char MASK_ENCODE_URLCOMPONENT[0xFF] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1\0\0\0\0\0\1\1\1\1\0\0\1\1\0\1\1\1\1\1\1\1\1\1\1\0\0\0\0\0\0\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\0\1\0\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\0\0\0\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
       static const unsigned char BASE_CHAR = 'A';    //如果要更改编码格式为小写如%cf模式,则baseChar需更改为'a'
       for (; *src; src++)
       {
           unsigned char c = *src;
           if ((mode == 1 && (!MASK_ENCODE_URI[c])) || (mode == 2 && (!MASK_ENCODE_URLCOMPONENT[c])))
           {

               result += '%';
               result += (c >= 0xA0) ? ((c >> 4) - 10 + BASE_CHAR) : ((c >> 4) + '0');
               result += ((c & 0xF) >= 0xA) ? ((c & 0xF) - 10 + BASE_CHAR) : ((c & 0xF) + '0');
           }
           else
           {
               result += c;
           }
       }
       return result;
   }

   /**
   *@brief     将URL编码的字符串进行URL解码，会将+解码为空格，解码后的字符编码依赖于URL编码时的字符编码
   *@param     src        [in]    待URL解码的原串
   *@return    解码后的字符串
   */
   static std::string  HNUrlDecode(const char *src)
   {
       //参数检查
       if (src == NULL)
       {
           return "";
       }
       std::string result;
       result.reserve((size_t)(strlen(src)*1.5f) + 1);

       for (; *src; src++)
       {
           if (*src == '+')
           {
               result += ' ';
           }
           else if (*src == '%' && isxdigit(*(src + 1)) && isxdigit(*(src + 2)))
           {
               int c = 0;
               for (int i = 1; i <= 2; i++)
               {
                   c <<= 4;
                   if (src[i] >= '0' && src[i] <= '9')
                   {
                       c |= (src[i] - '0');
                   }
                   else if (src[i] >= 'a' && src[i] <= 'f')
                   {
                       c |= (src[i] - 'a') + 10;
                   }
                   else if (src[i] >= 'A' && src[i] <= 'F')
                   {
                       c |= (src[i] - 'A') + 10;
                   }
               }
               result += ((char)(c & 0xff));
               src += 2;
           }
           else
           {
               result += (*src);
           }
       }
       return result;
   }

   /**
   *@brief     base64编码
   *@param     buf            [in]    原串
   *@param     bufLen         [in]    原串长度
   *@param     base64table    [in]    自定义base64码表
   *@return    编码后的字符串
   */
   static std::string HNBase64Encode(const void* buf, size_t bufLen, const char* base64table = HN_BASE64_TABLE)
   {
       size_t i, n;
       int C1, C2, C3;
       unsigned char* p;

       if (buf == NULL || bufLen == 0 || base64table == NULL || strlen(base64table) != 64)
       {
           return("");
       }

       n = bufLen / 3 + (bufLen % 3 != 0);
       if (n > (((size_t)-1) - 1) / 4)
       {
           return("");
       }

       HN_OPTIMIZE_NEWBUF(unsigned char, (n * 4 + 1),
           {
               n = (bufLen / 3) * 3;
               const unsigned char* src = (const unsigned char*)buf;
               for (i = 0, p = opBuffer; i < n; i += 3)
               {
                   C1 = *src++;
                   C2 = *src++;
                   C3 = *src++;

                   *p++ = base64table[(C1 >> 2) & 0x3F];
                   *p++ = base64table[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];
                   *p++ = base64table[(((C2 & 15) << 2) + (C3 >> 6)) & 0x3F];
                   *p++ = base64table[C3 & 0x3F];
               }

               if (i < bufLen)
               {
                   C1 = *src++;
                   C2 = ((i + 1) < bufLen) ? *src++ : 0;

                   *p++ = base64table[(C1 >> 2) & 0x3F];
                   *p++ = base64table[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];

                   if ((i + 1) < bufLen)
                       *p++ = base64table[((C2 & 15) << 2) & 0x3F];
                   else *p++ = '=';

                   *p++ = '=';
               }
               return std::string((const char*)opBuffer, p - opBuffer);
           });
   }

   /**
   *@brief     base64解码
   *@param     encoded_string     [in]    原串
   *@param     base64table        [in]    自定义base64码表
   *@return    解码后的字节数组，失败为空
   */
   static std::string HNBase64Decode(const std::string & encoded_string, const char* base64table = HN_BASE64_TABLE)
   {
       //参数校验
       if (encoded_string.empty() || base64table == NULL || strlen(base64table) != 64)
           return "";

       //组织码表
       unsigned char base64_dec_map[128];
       memset(base64_dec_map, 127, sizeof(base64_dec_map));
       for (unsigned char i = 0; i < 64; ++i)
       {
           if (base64table[i] >= sizeof(base64_dec_map))
           {
               //码表不合法
               return "";
           }
           base64_dec_map[(unsigned char)base64table[i]] = i;
       }
       base64_dec_map[(unsigned char)'='] = 64;

       //检查有效性并获得输出长度
       size_t i, n;
       uint32_t j, x;
       for (i = n = j = 0; i < encoded_string.length(); i++)
       {
           //在检查EOL之前先跳过空格
           x = 0;
           while (i < encoded_string.length() && encoded_string[i] == ' ')
           {
               ++i;
               ++x;
           }

           //缓冲区最后的空格忽略
           if (i == encoded_string.length())
               break;

           if ((encoded_string.length() - i) >= 2 &&
               encoded_string[i] == '\r' && encoded_string[i + 1] == '\n')
               continue;

           if (encoded_string[i] == '\n')
               continue;

           //行内的空格是错误的
           if (x != 0)
               return("");

           if (encoded_string[i] == '=' && ++j > 2)
               return("");

           if (encoded_string[i] > 127 || base64_dec_map[encoded_string[i]] == 127)
               return("");

           if (base64_dec_map[encoded_string[i]] < 64 && j != 0)
               return("");

           n++;
       }
       if (n == 0)
       {
           return("");
       }

       //下面的表达式是为了计算以下公式而无需n中整数溢出的风险：
       //n =（（n * 6）+ 7）>> 3;
       n = (6 * (n >> 3)) + ((6 * (n & 0x7) + 7) >> 3);
       n -= j;

       //解码
       HN_OPTIMIZE_NEWBUF(unsigned char, (n + 1),
       {
           unsigned char *p;
           const char* src = encoded_string.data();
           for (j = 3, n = x = 0, p = opBuffer; i > 0; i--, src++)
           {
               if (*src == '\r' || *src == '\n' || *src == ' ')
                   continue;

               j -= (base64_dec_map[*src] == 64);
               x = (x << 6) | (base64_dec_map[*src] & 0x3F);

               if (++n == 4)
               {
                   n = 0;
                   if (j > 0)
                       *p++ = (unsigned char)(x >> 16);
                   if (j > 1)
                       *p++ = (unsigned char)(x >> 8);
                   if (j > 2)
                       *p++ = (unsigned char)(x);
               }
           }
           return std::string((const char*)opBuffer, p - opBuffer);
       });
   }

   /**
   *@brief     判断是不是utf8字符串，这个函数不是特别准确，比如gbk编码的这些字可能检测错误：位、前、支、校、写、元...
   *@param     srcstr    [in]    字符串
   *@return    是否位utf8
   */
   static bool HNStrIsValidUtf8(const std::string& srcstr)
   {
       unsigned char *s = (unsigned char *)srcstr.data();
       size_t length = srcstr.length();
       for (unsigned char *e = s + length; s != e;)
       {
           if (s + 4 <= e && ((*(uint32_t *)s) & 0x80808080) == 0)
           {
               s += 4;
           }
           else
           {
               while (!(*s & 0x80))
               {
                   if (++s == e)
                   {
                       return true;
                   }
               }

               if ((s[0] & 0x60) == 0x40)
               {
                   if (s + 1 >= e || (s[1] & 0xc0) != 0x80 || (s[0] & 0xfe) == 0xc0)
                   {
                       return false;
                   }
                   s += 2;
               }
               else if ((s[0] & 0xf0) == 0xe0)
               {
                   if (s + 2 >= e || (s[1] & 0xc0) != 0x80 || (s[2] & 0xc0) != 0x80 ||
                       (s[0] == 0xe0 && (s[1] & 0xe0) == 0x80) || (s[0] == 0xed && (s[1] & 0xe0) == 0xa0))
                   {
                       return false;
                   }
                   s += 3;
               }
               else if ((s[0] & 0xf8) == 0xf0)
               {
                   if (s + 3 >= e || (s[1] & 0xc0) != 0x80 || (s[2] & 0xc0) != 0x80 || (s[3] & 0xc0) != 0x80 ||
                       (s[0] == 0xf0 && (s[1] & 0xf0) == 0x80) || (s[0] == 0xf4 && s[1] > 0x8f) || s[0] > 0xf4)
                   {
                       return false;
                   }
                   s += 4;
               }
               else
               {
                   return false;
               }
           }
       }
       return true;
   }

   /**
   *@brief     判断字符串中每个字节是否符合一定的规则
   *@param     src    [in]    字符串
   *@param     len    [in]    字符串长度
   *@param     func   [in]    匹配规则
   *@return    如果字符串中所有字符全部符合匹配规则，则返回true，否则返回false
   */
   static bool StringTypeMatch(const char* src, size_t len, std::function<bool(char)> func)
   {
       if(src == NULL || len == 0 || !func)
           return true;
       for(size_t i = 0 ; i < len; i++)
       {
           if(!func(src[i]) )
               return false;
       }
       return true;
   }

   /**
   *@brief     通配符匹配算法
   *@param     pattern     [in]    通配符
   *@param     patternLen  [in]    通配符长度
   *@param     str         [in]    待匹配数据
   *@param     stringLen   [in]    待匹配数据长度
   *@param     nocase      [in]    是否忽略大小写匹配
   *@return    是否匹配成功
   */
   static bool StringMatchLen(const char *pattern, int patternLen, const char *srcstr, int stringLen, bool nocase)
   {
       if (pattern == NULL)
       {
           return srcstr == NULL;
       }
       if (srcstr == NULL)
       {
           return pattern == NULL;
       }
       while (patternLen && stringLen)
       {
           switch (pattern[0]) {
           case '*':
               while (patternLen && pattern[1] == '*') {
                   pattern++;
                   patternLen--;
               }
               if (patternLen == 1)
                   return true; /* match */
               while (stringLen) {
                   if (StringMatchLen(pattern + 1, patternLen - 1,
                       srcstr, stringLen, nocase))
                       return true; /* match */
                   srcstr++;
                   stringLen--;
               }
               return false; /* no match */
               break;
           case '?':
               srcstr++;
               stringLen--;
               break;
           case '[':
           {
               int notmatch, match;

               pattern++;
               patternLen--;
               notmatch = pattern[0] == '^';
               if (notmatch) {
                   pattern++;
                   patternLen--;
               }
               match = 0;
               while (1) {
                   if (pattern[0] == '\\' && patternLen >= 2) {
                       pattern++;
                       patternLen--;
                       if (pattern[0] == srcstr[0])
                           match = 1;
                   }
                   else if (pattern[0] == ']') {
                       break;
                   }
                   else if (patternLen == 0) {
                       pattern--;
                       patternLen++;
                       break;
                   }
                   else if (patternLen >= 3 && pattern[1] == '-') {
                       int start = pattern[0];
                       int end = pattern[2];
                       int c = srcstr[0];
                       if (start > end) {
                           int t = start;
                           start = end;
                           end = t;
                       }
                       if (nocase) {
                           start = tolower(start);
                           end = tolower(end);
                           c = tolower(c);
                       }
                       pattern += 2;
                       patternLen -= 2;
                       if (c >= start && c <= end)
                           match = 1;
                   }
                   else {
                       if (!nocase) {
                           if (pattern[0] == srcstr[0])
                               match = 1;
                       }
                       else {
                           if (tolower((int)pattern[0]) == tolower((int)srcstr[0]))
                               match = 1;
                       }
                   }
                   pattern++;
                   patternLen--;
               }
               if (notmatch)
                   match = !match;
               if (!match)
                   return false; /* no match */
               srcstr++;
               stringLen--;
               break;
           }
           case '\\':
               if (patternLen >= 2) {
                   pattern++;
                   patternLen--;
               }
               /* fall through */
           default:
               if (!nocase) {
                   if (pattern[0] != srcstr[0])
                       return false; /* no match */
               }
               else {
                   if (tolower((int)pattern[0]) != tolower((int)srcstr[0]))
                       return false; /* no match */
               }
               srcstr++;
               stringLen--;
               break;
           }
           pattern++;
           patternLen--;
           if (stringLen == 0) {
               while (*pattern == '*') {
                   pattern++;
                   patternLen--;
               }
               break;
           }
       }
       return  (patternLen == 0 && stringLen == 0);
   }

   /**
   *@brief     文件名的通配符匹配
   *@param     szFileName  [in]    输入的文件名称，如precache-manifest.1A2B.js
   *@param     szFilter    [in]    通配符，如precache-manifest.*.js
   *@param     bIgnoreCase [in]    是否忽略大小写匹配
   *@return    是否匹配成功
   */
   static bool FileNameFilter(const char *szFileName, const char* szFilter, bool bIgnoreCase = false)
   {
       return StringMatchLen(szFilter, (int)strlen(szFilter), szFileName, (int)strlen(szFileName), bIgnoreCase);
   }

   /**
   *@brief     检查一个字符串是否符合变量命名规则
   *                1.只能包含0~9 a~z A~Z _  这63个字符
   *                2.只能以英文字母和下划线开头
   *                3.不能以数字开头，也不能只有纯数字
   *                4.不能为空
   *@param     szSrc  [in]    待检查的字符串
   *@return    是否为合法的变量
   */
   static bool StrIsVariable(const char *szSrc)
   {
       if (szSrc == NULL || szSrc[0] == 0)
           return false;

       size_t len = strlen(szSrc);
       for (size_t i = 0; i < len; i++)
       {
           int ch = szSrc[i];
           if (i == 0)
           {
               //首字符只能是字母和下划线
               if (ch != '_' && !::isalpha(ch))
                   return false;
           }
           else
           {
               //后面的字母只能是字母/下划线/数字
               if (!::isalpha(ch) && !::isdigit(ch) && ch != '_')
                   return false;
           }
       }
       return true;
   }

   /**
   *@brief     检查一个字符串是否符合正整数规则
   *                1.只能包含0~9 X x abcdefABCDEF这24个字符
   *                2.如果有x或X，必须是十六进制数字格式，如0x123B
   *                4.不能为空
   *@param     szSrc  [in]    待检查的字符串
   *@return    是否为合法的正整数
   */
   static bool StrIsUInt(const char *szSrc)
   {
       if (szSrc == NULL || szSrc[0] == 0)
           return false;
       size_t len = strlen(szSrc);
       bool   isHex = (len > 2 && (szSrc[1] == 'x' || szSrc[1] == 'X'));
       for (size_t i = 0; i < len; i++)
       {
           int ch = szSrc[i];
           if (isHex)
           {
               if (i == 0 && ch != '0')
                   return false;
               if (i == 1)
                   continue;
               if (!::isxdigit(ch))
                   return false;
           }
           else
           {
               if (!::isdigit(ch))
                   return false;
           }
       }
       return true;
   }

   /**
   *@brief     等价于CString::Right
   *@param     src[in]     原始串
   *@param     count[in]   取原始串右边的长度
   *@return    结果字符串
   */
   static std::string HNRight(const std::string& src, int count)
   {
       if (count < 0)
           count = 0;
       auto length = src.length();
       if ((decltype(length))count >= length)
           return src;
       return std::string(src.data() + (length - count), count);
   }

   /**
   *@brief     等价于CString::Left
   *@param     src[in]     原始串
   *@param     count[in]   取原始串左边的长度
   *@return    结果字符串
   */
   static std::string HNLeft(const std::string& src, int count)
   {
       if (count < 0)
           count = 0;
       auto length = src.length();
       if ((decltype(length))count >= length)
           return src;
       return std::string(src.data(), count);
   }

   /**
   *@brief     等价于CString::Mid
   *@param     src[in]     原始串
   *@param     first[in]   起始位置
   *@param     count[in]   取原始串的长度
   *@return    结果字符串
   */
   static std::string HNMid(const std::string& src, int first, int count)
   {
       if (first < 0)
           first = 0;
       if (count < 0)
           count = 0;

       if ((size_t)(first + count) > src.length())
       {
           count = src.length() - first;
       }
       if ((size_t)first > src.length())
       {
           count = 0;
       }
       assert((count == 0) || ((size_t)(first + count) <= src.length()));

       // optimize case of returning entire string
       if ((first == 0) && ((first + count) == src.length()))
       {
           return src;
       }
       return std::string(src.data() + first, count);
   }

   /**
   *@brief     等价于CString::Mid，count = 剩下串的长度
   *@param     src[in]     原始串
   *@param     first[in]   起始位置
   *@return    结果字符串
   */
   static std::string HNMid(const std::string& src, int first)
   {
       return CHNStrUtil::HNMid(src, first, src.length() - first);
   }


};

#endif // __HELLO_NET_M_CORE_S_STRINGUTIL__