#ifndef __HELLO_NET_M_IO_S_SSL__
#define __HELLO_NET_M_IO_S_SSL__

#include "../m_core/m_core.hpp"

#ifdef HNHAVE_OPENSSL
#include "openssl/ssl.h"
#include "openssl/bio.h"
#include "openssl/err.h"
#endif    //HNHAVE_OPENSSL 
#if defined (HNHAVE_OPENSSL) && (defined  HNHAVE_LIBSSH2)
#ifndef LIBSSH2_OPENSSL
#define LIBSSH2_OPENSSL
#endif
#include "libssh2.h"
#endif

/**
*@brief    SSL连接辅助类
*
*/


class CHNSSLUtil : CHNNoCopyable
{
public:
    /**
    *@brief     获得单例对象
    */
    static CHNSSLUtil& obj()
    {
        static CHNSSLUtil o;
        return o;
    }

    /**
    *@brief     获取错误
    */
    static CHNResult<> GetSSLError()
    {
        CHNResult<> result;
#if defined (HNHAVE_OPENSSL)
        unsigned long ulErr = ERR_get_error();
        char szError[MAX_PATH] = { 0 };
        ERR_error_string_n(ulErr, szError, sizeof(szError));
        return result.SetFail((int)ulErr,"%s",szError);
#else
        return result;
#endif
    }
    /**
    *@brief     初始化
    *@param     bSSLThreadSafe    [in]    是否要启用线程安全的openssl，如果加载的第三方库如libcurl已经做了这个事，就不需要了
    */
    void Init(bool bSSLThreadSafe)
    {
#if defined (HNHAVE_OPENSSL)
#if defined  (HNHAVE_LIBSSH2)
        libssh2_init(0);
#endif
        m_bSSLThreadSafe = bSSLThreadSafe;
#if OPENSSL_VERSION_NUMBER >= 0x10100003L
        OPENSSL_init_ssl(OPENSSL_INIT_LOAD_CONFIG, NULL);
#else
        SSL_library_init();
#endif
        SSL_load_error_strings();
        SSLeay_add_ssl_algorithms();
        ERR_clear_error();
        m_locks.clear();
        if (bSSLThreadSafe)
        {
            //初始化openssl线程安全设施
            for (int i = 0; i < CRYPTO_num_locks(); i++)
            {
                m_locks.emplace_back(new CHNRecMutex());
            }
#if OPENSSL_VERSION_NUMBER >= OPENSSL_VERSION_1_0_0
            CRYPTO_THREADID_set_callback(SSLThreadId_CallBack);
#else
            CRYPTO_set_id_callback(SSLThreadId_CallBack);
#endif
            CRYPTO_set_locking_callback(SSLocking_CallBack);
        }

        //初始化客户端ctx(所有客户端公用一个ctx)，对于客户端，一般不需要提供证书
        ERR_clear_error();
        m_sslClientCtx = SSL_CTX_new(SSLv23_client_method());
#endif
    }

    /**
    *@brief        释放
    */
    void Release()
    {
#if defined (HNHAVE_OPENSSL)
#if (defined  HNHAVE_LIBSSH2)
        libssh2_exit();
#endif
        if (NULL != m_sslClientCtx)
        {
            SSL_CTX_free(m_sslClientCtx);
            m_sslClientCtx = NULL;
        }
        if (m_bSSLThreadSafe)
        {
#if OPENSSL_VERSION_NUMBER >= OPENSSL_VERSION_1_0_0
            CRYPTO_THREADID_set_callback(NULL);
#else
            CRYPTO_set_id_callback(NULL);
#endif
            CRYPTO_set_locking_callback(NULL);
            m_locks.clear();
        }
        ERR_free_strings();
        EVP_cleanup();
        CRYPTO_cleanup_all_ex_data();
#endif
    }


public:
#if defined(HNHAVE_OPENSSL)
#if (OPENSSL_VERSION_NUMBER >= OPENSSL_VERSION_1_0_0)
    static void SSLThreadId_CallBack(CRYPTO_THREADID * id)
    {
#if defined(HNOS_WIN)
        unsigned long val = ::GetCurrentThreadId();
#else
        unsigned long val = ::pthread_self();
#endif
        CRYPTO_THREADID_set_numeric(id, val);
        CRYPTO_THREADID_set_pointer(id, NULL);
    }
#else
    static unsigned long SSLThreadId_CallBack(void)
    {
#if defined(HNOS_WIN)
        return ::GetCurrentThreadId();
#else
        return ::pthread_self();
#endif
    }
#endif
    static void SSLocking_CallBack(int mode, int type, const char *file, int line)
    {
        if (mode & CRYPTO_LOCK)
        {
            CHNSSLUtil::obj().m_locks[type]->lock();
        }
        else
        {
            CHNSSLUtil::obj().m_locks[type]->unlock();
        }
    }
    std::vector<std::unique_ptr<CHNRecMutex> > m_locks;
    bool m_bSSLThreadSafe;
    SSL_CTX *m_sslClientCtx;
#endif

};



/**
*@brief    服务器ssl上下文类，每个服务器持有一个对象
*
*/

class CHNSSLServerCtx : CHNNoCopyable
{
public:
    explicit CHNSSLServerCtx()
#if defined (HNHAVE_OPENSSL)
        : m_sslServerCtx(NULL)
#endif
    {}

    ~CHNSSLServerCtx()
    {
#if defined (HNHAVE_OPENSSL)
        if (NULL != m_sslServerCtx)
        {
            SSL_CTX_free(m_sslServerCtx);
            m_sslServerCtx = NULL;
        }
#endif
    }

    bool InitComplete()
    {
#if defined (HNHAVE_OPENSSL)
        return (NULL != m_sslServerCtx);
#else    
        return false;
#endif
    }

    /**
    *@brief     初始化，提供证书和私钥
    *@param     certfilepath    [in]    证书文件位置，PEM文件，base64格式
    *@param     keyfilepath     [in]    私钥文件位置，PEM文件，base64格式
    *@return    是否成功
    */

    CHNResult<> InitContext(const char* certfilepath, const char* keyfilepath)
    {
        CHNResult<> result;
#if defined (HNHAVE_OPENSSL)
        if (NULL != m_sslServerCtx)
        {
            SSL_CTX_free(m_sslServerCtx);
            m_sslServerCtx = NULL;
        }
        ERR_clear_error();
        m_sslServerCtx = SSL_CTX_new(SSLv23_server_method());
        if (NULL == m_sslServerCtx)
        {
            return CHNSSLUtil::GetSSLError();
        }
        //为服务端指定SSL连接所用公钥证书
        //参数 m_sslServerCtx，服务端SSL会话环境
        //参数 pCertPath，你存放公钥证书的路径
        //参数 SSL_FILETYPE_PEM，指定你所要加载的公钥证书的文件编码类型为 Base64
        if (SSL_CTX_use_certificate_file(m_sslServerCtx, certfilepath, SSL_FILETYPE_PEM) != 1)
        {
            SSL_CTX_free(m_sslServerCtx);
            m_sslServerCtx = NULL;
            return CHNSSLUtil::GetSSLError();
        }
        // 为服务端指定SSL连接所用私钥
        //参数 m_sslServerCtx，服务端SSL会话环境
        //参数 pKeyPath，你存放对应私钥文件的路径
        //参数 SSL_FILETYPE_PEM，指定你所要加载的私钥文件的文件编码类型为 Base64
        if (SSL_CTX_use_PrivateKey_file(m_sslServerCtx, keyfilepath, SSL_FILETYPE_PEM) != 1)
        {
            SSL_CTX_free(m_sslServerCtx);
            m_sslServerCtx = NULL;
            return CHNSSLUtil::GetSSLError();
        }
        // 检查SSL连接 所用的私钥与证书是否匹配【所以你仅有公钥证书是不够的】
        if (!SSL_CTX_check_private_key(m_sslServerCtx))
        {
            SSL_CTX_free(m_sslServerCtx);
            m_sslServerCtx = NULL;
            return CHNSSLUtil::GetSSLError();
        }
        return result;
#else
        return result.SetFail("not enable openssl macro.");
#endif
    }

public:
#if defined (HNHAVE_OPENSSL)
    SSL_CTX *m_sslServerCtx;
#endif
};

/**
*@brief    SSL连接类，每个客户端连接或服务端接收连接拥有一个对象
*
*/

class CHNSSLConnection : CHNNoCopyable
{
    public:
    explicit CHNSSLConnection()
#if defined (HNHAVE_OPENSSL)
        : m_ssl(NULL)
#endif
    {}
    ~CHNSSLConnection()
    {}

    /**
    *@brief     作为客户端初始化并连接
    *@param     sock    [in]    原始socket
    *@param     nWaitMs [in]    连接超时时间(毫秒，<0代表不超时)
    *@return    是否成功
    */
    CHNResult<> SslInitConnect(int sock, int nWaitMs)
    {
        CHNResult<> result;
#if defined (HNHAVE_OPENSSL)
        //初始化ctx，对于客户端，一般不需要提供证书
        FreeConnection();
        ERR_clear_error();
        m_ssl = SSL_new(CHNSSLUtil::obj().m_sslClientCtx);
        if (NULL == m_ssl)
        {
            return CHNSSLUtil::GetSSLError();
        }

        //SSL绑定原生socket,并设置为客户端模式
        int ret = SSL_set_fd(m_ssl, sock);
        if (1 != ret)
        {
            return CHNSSLUtil::GetSSLError();
        }
        SSL_set_connect_state(m_ssl);

        //select模式连接
        struct timeval tv;
        tv.tv_sec = nWaitMs / 1000;
        tv.tv_usec = 1000 * (nWaitMs % 1000);
        fd_set fdRead, fdWrite;
        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        while ((ret = SSL_do_handshake(m_ssl)) != 1)
        {
            int err = SSL_get_error(m_ssl, ret);
            if (err == SSL_ERROR_WANT_WRITE)
            {
                FD_SET(sock, &fdWrite);
            }
            else if (err == SSL_ERROR_WANT_READ)
            {
                FD_SET(sock, &fdRead);
            }
            else
            {
                int SysError = HNGetLastError();
                if (SysError != 0)
                    return result.SetSystemFail(SysError);
                else
                    return CHNSSLUtil::GetSSLError();
            }

            ret = select(sock + 1, &fdRead, &fdWrite, NULL, (nWaitMs < 0) ? NULL : (&tv));
            if (ret < 0)
            {
                return result.SetSystemFail();
            }
            else if (ret == 0)
            {
                return result.SetFail("connect timeout.");
            }

            //当select到感兴趣的事件发生时，可能ssl握手还未完成(ssl需要六次握手)，此时不break，而是需要等到握手完成才算ssl连接完毕。
            ERR_clear_error();
        }

        //客户端要求服务的的证书，一般可以不做
        /*X509* serverCert = SSL_get_peer_certificate(m_ssl);
        if (NULL == serverCert)
        {
        return CHNSSLUtil::GetSSLError();
        }
        char* SslSubject = X509_NAME_oneline(X509_get_subject_name(serverCert), 0, 0);
        char* SslIssuer = X509_NAME_oneline(X509_get_issuer_name(serverCert), 0, 0);
        SSL_CTX_set_verify();                //TODO:指定证书验证方式
        SSL_CTX_load_verify_location();      //TODO:为SSL会话环境加载本应用所信任的CA证书列表
        OPENSSL_free(SslSubject);
        OPENSSL_free(SslIssuer);
        X509_free(serverCert);
        */
        return result;
#else
        return result.SetFail( "not enable openssl macro.");
#endif
    }

    /**
    *@brief     作为服务端的接收连接初始化
    *@param     sock            [in]    accept到的原始套接字
    *@param     sslServerCtx    [in]    服务器的上下文，通过CHNSSLServerCtx得到
    *@return    是否成功
    */
    CHNResult<> SslInitAttach(int sock, const CHNSSLServerCtx &sslServerCtx)
    {
        CHNResult<> result;
#if defined (HNHAVE_OPENSSL)
        FreeConnection();
        ERR_clear_error();
        if (NULL == sslServerCtx.m_sslServerCtx)
        {
            return result.SetFail("not init sslServerCtx succ.");
        }
        m_ssl = SSL_new(sslServerCtx.m_sslServerCtx);
        if (NULL == m_ssl)
        {
            return CHNSSLUtil::GetSSLError();
        }

        //SSL绑定原生socket
        int ret = SSL_set_fd(m_ssl, sock);
        if (1 != ret)
        {
            return CHNSSLUtil::GetSSLError();
        }
        SSL_set_accept_state(m_ssl);
        ret = SSL_accept(m_ssl);
        if (ret <= 0)
        {
            return CHNSSLUtil::GetSSLError();
        }

        //验证客户端证书，一般不需要
        return result;
#else
        return result.SetFail("not enable openssl macro.");
#endif
    }

    /**
    *@brief     发送数据
    *@return    是否成功
    */
    CHNResult<> SslSend(const char* data, size_t length)
    {
        CHNResult<> result;
        if (data == NULL || length == 0)
        {
            return result.SetFail("data empty or length == 0");
        }
#if defined (HNHAVE_OPENSSL)
        while(1)
        {
            ERR_clear_error();
            int ret = SSL_write(m_ssl, data, length);
            if (ret < 0 || ret != length)
            {
                int err = SSL_get_error(m_ssl, ret);
                if(err == SSL_ERROR_WANT_WRITE)
                {
                    //如果基础BIO是非阻塞的，则当基础BIO无法满足SSL_write（）继续操作的需求时，SSL_write（）也将返回。
                    //在这种情况下，使用返回值SSL_write（）调用 SSL_get_error（3）将产生 SSL_ERROR_WANT_READ或 SSL_ERROR_WANT_WRITE。
                    //由于可以随时进行重新协商，因此对SSL_write（）的调用也可能导致读取操作！
                    //然后，调用过程必须在采取适当的措施以满足SSL_write（）的需要之后重复调用。动作取决于基本的BIO。
                    //当使用非阻塞套接字时，什么也不做，但是可以使用select（）检查所需条件。
                    //当使用缓冲BIO（例如BIO对）时，必须先将数据写入BIO或从BIO中检索出数据，然后才能继续。
                    //当由于SSL_ERROR_WANT_READ或 SSL_ERROR_WANT_WRITE而必须重复SSL_write（）操作时 ，必须使用相同的参数来重复该操作。
                    HNSleep(100);
                    continue;
                }
                return CHNSSLUtil::GetSSLError();
            }
            else if(ret == 0)
            {
                //写入操作不成功。底层连接可能已关闭。调用SSL_get_error（）并返回返回值ret，以查明是否发生了错误或连接已完全关闭（SSL_ERROR_ZERO_RETURN）。
                //SSLv2（不建议使用）不支持关闭警报协议，因此只能检测到基础连接是否已关闭。无法检查关闭发生的原因。
                int err = SSL_get_error(m_ssl, ret);
                char szError[MAX_PATH] = { 0 };
                ERR_error_string_n(err, szError, sizeof(szError));
                return result.SetFail((int)err,"%s",szError);
            }
            return result.SetSucc(ret);
        }
#else
        return result.SetFail( "not enable openssl macro.");
#endif
    }
    /**
    *@brief     接收数据
    *@return    是否成功
    */
    int SslRecv(void* data, int len)
    {
#if defined (HNHAVE_OPENSSL)
        return SSL_read(m_ssl, data, len);
#else
        return HNERR_NOTLOADLIB;
#endif
    }
    /**
    *@brief     是否可以继续读数据
    *@return    是否成功
    */
    bool CanContinueRead(int errcode)
    {
#if defined (HNHAVE_OPENSSL)
        int err = SSL_get_error(m_ssl, errcode);
        ERR_clear_error();
        return (err == SSL_ERROR_WANT_READ);
#else
        return false;
#endif
    }

    /**
    *@brief     释放
    */
    void FreeConnection()
    {
#if defined (HNHAVE_OPENSSL)
        if (NULL != m_ssl)
        {
            SSL_shutdown(m_ssl);
            SSL_free(m_ssl);
            m_ssl = NULL;
        }
#endif
    }
protected:
#if defined (HNHAVE_OPENSSL)
    SSL        *m_ssl;
#endif
};

#endif // __HELLO_NET_M_IO_S_SSL__