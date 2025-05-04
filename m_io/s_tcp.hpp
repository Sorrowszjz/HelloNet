#ifndef __HELLO_NET_M_IO_S_TCP__
#define __HELLO_NET_M_IO_S_TCP__
#include "../m_core/m_core.hpp"
// #include "s_ssl.hpp"
/**
*@brief      网络地址类，封装了sockaddr_in
*            目前仅支持ipv4协议
*/

class CHNSockAddr
{
public:
    CHNSockAddr(const char *pszHost = NULL, unsigned short nPort = 0)
    {
        memset(&m_addr,0,sizeof(m_addr));
        memset(m_addr.sin_zero,0,sizeof(m_addr.sin_zero));
        m_addr.sin_family = AF_INET;
        m_addr.sin_port = htons(nPort);

        if(pszHost == NULL)
        {
            m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else if(strcasecmp(pszHost,"localhost") == 0)
        {
            static unsigned long localaddr = inet_addr("127.0.0.1");
            m_addr.sin_addr.s_addr = localaddr;
        }
        else
        {
            m_addr.sin_addr.s_addr = inet_addr(pszHost);
        }
    }

    CHNSockAddr(const sockaddr_in &addr)
    {
        memcpy(&m_addr,&addr,sizeof(m_addr));
    }

    /**
    *@brief     线程安全的域名解析，但是有可能阻塞，建议在线程池执行
    *@param     pszHost  [in]    主机名
    *@param     nPort    [in]    端口
    *@return    是否成功
    */
    CHNResult<> GetHostByName(const char* pszHost, unsigned short nPort = 0)
    {
        CHNResult<> result;
        struct addrinfo addrHint;
        memset(&addrHint, 0, sizeof(addrHint));
        addrHint.ai_family = m_addr.sin_family;
        addrHint.ai_socktype = SOCK_STREAM;
        addrHint.ai_protocol = IPPROTO_TCP;
        struct addrinfo* addrServer = NULL;

        char szService[16] = {0};
        sprintf(szService, "%hu", nPort);

        int nRet = getaddrinfo(pszHost, szService, &addrHint, &addrServer);
        if(nRet != 0 || addrServer == NULL)
        {
            result.SetSystemFail(nRet);
            return result;
        }
        m_addr.sin_addr.s_addr = ((struct sockaddr_in*)(addrServer->ai_addr))->sin_addr.s_addr;
        m_addr.sin_port = htons(nPort);
        freeaddrinfo(addrServer);
        return result;
    }

    /**
    *@brief     从ftp字符串解析出地址
    *@param     ftpstring    [in]    ftp字符串，形如字符串："Entering Passive Mode (127,0,0,1,207,175)"
    *                                算法是，从第一个数字开始计算，f1,f2,f3,f4,f5,f6，其中：
    *                                ip：f1,f2,f3,f4    端口：f5*256+f6
    *@return    是否成功
    */
    CHNResult<> FromFtpString(const char *ftpstring)
    {
        CHNResult<> result;
        if(ftpstring == NULL || ftpstring[0] == 0)
        {
            return result.SetFail("ftpstring is NULL || ftpstring[0] == 0");
        }
        int first = -1 , last = -1;
        for(int i = 0 ; i < strlen(ftpstring) ; i++)
        {
            if(first == -1 && isdigit(ftpstring[i]))
            {
                first = (int)i;
                continue;
            }
            if(first != -1 && !isdigit(ftpstring[i]) && ftpstring[i] != ',')
            {
                last = (int)i;
                break;
            }
        }
        if(first == -1 || last == -1 || last - first < 10)
        {
            return result.SetFail("ftpstring format error : %s" , ftpstring);
        }
        std::vector<std::string> ret = CHNStrUtil::HNStrSplit(std::string(ftpstring + first , last - first).c_str(), ",");
        if(ret.size() != 6)
        {
            return result.SetFail("ret.size(%d) != 6:%s", ret.size(), ftpstring);
        }

        char szHost[64] = {0};
        snprintf(szHost, sizeof(szHost), "%s.%s.%s.%s", ret[0].c_str(), ret[1].c_str(), ret[2].c_str(), ret[3].c_str());

        memset(&m_addr,0,sizeof(m_addr));
        memset(m_addr.sin_zero,0,sizeof(m_addr.sin_zero));
        m_addr.sin_family = AF_INET;

        if(HN_INET_PTON(AF_INET, szHost, (void *)(&(m_addr.sin_addr))) <= 0)
        {
            return result.SetSystemFail();
        }

        int nPort = atoi(ret[4].c_str()) * 256 + atoi(ret[5].c_str());
        if(nPort <= 0 || nPort > 65535)
        {
            return result.SetFail("port error : %d", nPort);
        }
        m_addr.sin_port = htons((unsigned short)nPort);
        return result;
    }

    /**
    *@brief     获取ftp地址串，形如字符串 "127,0,0,1,207,175" 其中：
    *           ip：f1,f2,f3,f4    端口：f5*256+f6
    *@return    结果串
    */
    std::string GetFtpString()
    {
        std::string ip = this->GetIPString();
        CHNStrUtil::HNReplaceAll(ip, ".", ",");

        unsigned short port = this->GetHostPort();
        int f5 = port / 256;
        int f6 = port % 256;

        char szFtpString[MAX_PATH] = {0};
        snprintf(szFtpString, sizeof(szFtpString), "%s,%d,%d", ip.c_str(), f5, f6);
        return szFtpString;
    }

    /**
    *@brief        获取IP地址字符串
    */
    std::string GetIPString() const
    {
        char buf[MAX_PATH] = {0};
        memset(buf,0,sizeof(buf));
        if(NULL == HN_INET_NTOP(AF_INET, (void *)(&(m_addr.sin_addr)), buf, static_cast<socklen_t>(sizeof(buf))))
        {
            return "0.0.0.0";
        }
        return buf;
    }
    
    /**
    *@brief        主机字节序的ip地址
    */
    unsigned int GetHostIP() const
    {
        return ntohl(m_addr.sin_addr.s_addr);
    }


    /**
    *@brief        主机字节序的端口
    */
   unsigned short GetHostPort() const
   {
       return ntohs(m_addr.sin_port);
   }

public:
    sockaddr_in m_addr;
};


/**
*@brief      SOCKET类的操作封装
*            目前仅支持tcp协议
*/

class CHNSocket
{
public:
    explicit CHNSocket(int fd = (int)INVALID_SOCKET) : m_socket(fd) { }
    ~CHNSocket() { }

    /**
    *@brief     socket是否有效
    *@return    是否有效
    */
    bool IsValid() const { return m_socket != INVALID_SOCKET; }

    /**
    *@brief     绑定地址
    *@param     addr    [in]    要绑定的地址，一般来说，只有监听套接字才需要指定地址
    *@param     reuse   [in]    是否重用端口
    *@return    是否成功
    */
    CHNResult<> CreateAndBind(const CHNSockAddr& addr = CHNSockAddr() , bool reuse = false)
    {
        CHNResult<> result;
        HNCloseSocket(m_socket);
#if defined (HNOS_WIN)
        m_socket = (int)::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
        m_socket = (int)::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
#endif
        if(!IsValid())
        {
            return result.SetSystemFail();
        }

        if(reuse)
        {
            result = this->SetReuseable(true);
            if(!result)
            {
                return result;
            }
        }

        if(::bind(m_socket, (const sockaddr*)&addr.m_addr, sizeof(addr.m_addr)) != 0)
        {
            return result.SetSystemFail();
        }
        return result;
    }
    /**
    *@brief     优雅关闭一个已连接的SOCKET
    */
    void ShutdownSocket()
    {
        if(m_socket == INVALID_SOCKET)
        {
            return;
        }
        int tmpsocket = m_socket;
        m_socket = INVALID_SOCKET;
        ::shutdown(tmpsocket, SHUT_RDWR);
        HNCloseSocket(tmpsocket);
        return ;
    }

    /**
    *@brief     判断SOCKET连接状态，
    *@return    结果存在CHNResult.IntResult或CHNResult.ErrCode
    */

    CHNResult<int> SockState()
    {
        CHNResult<int> result;
        int opt = -1;
        socklen_t len = sizeof(int);
        if(::getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&opt, &len) < 0)
        {
            return result.SetSystemFail();
        }
        return result.SetSucc(opt);
    }

    /**
    *@brief     【组合接口】创建阻塞模式监听socket，你可以调用SetNonBlocking设置为非阻塞的
    *@param     addr    [in]    要绑定的地址，一般来说，只有监听套接字才需要指定地址
    *@param     reuse   [in]    是否重用端口
    *@return    是否成功
    */
    CHNResult<> TcpListen(const CHNSockAddr& addr, bool reuse = false)
    {
        auto result = this->CreateAndBind(addr, reuse);
        if(!result)
        {
            return result;
        }
        if(::listen(m_socket, SOMAXCONN) != 0)
        {
            int nSystemError = HNGetLastError();
            HNCloseSocket(m_socket);
            return result.SetSystemFail(nSystemError);
        }
        return result;
    }

    /**
    *@brief     接收连接，仅用于监听socket
    *@param     addr        [out]   输出接收的socket地址
    *@param     nWaitMs     [in]    超时时间。如果nTimeoutMs < 0 代表永不超时(需阻塞监听的socket)； >0 代表超时毫秒数(需非阻塞监听的socket)
    *@return    返回接收的socket
    */

    int Accept(CHNSockAddr& addr , int nWaitsMs = -1)
    {
        if(nWaitsMs < 0)
        {
            int addrLen = sizeof(addr.m_addr);
            return (int)::accept(m_socket, (sockaddr*)&addr.m_addr, (socklen_t*)&addrLen);
        }

        struct timeval tv;
        tv.tv_sec = nWaitsMs / 1000;
        tv.tv_usec = (nWaitsMs % 1000) * 1000;

        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(m_socket, &readset);
        int ret = (int)::select(m_socket + 1, &readset, NULL, NULL, (nWaitsMs < 0) ? NULL : (&tv));
        if(ret < 0)
        {
            printf("::select m_socket(%d) ret(%d) < 0, err:%d", m_socket, ret, HNGetLastError());
            return (int)INVALID_SOCKET;
        }
        else if(ret == 0)
        {
            return (int)INVALID_SOCKET;
        }
        if(FD_ISSET(m_socket, &readset))
        {
            int addrlen = sizeof(addr.m_addr);
            return (int)::accept(m_socket, (sockaddr*)&addr.m_addr, (socklen_t*)&addrlen);
        }
        return (int)INVALID_SOCKET;
    }
    /**
    *@brief     非阻塞socket发起连接，仅用于客户端socket
    *@param     addr        [in]    要连接的socket地址
    *@param     nWaitMs     [in]    超时时间。如果nTimeoutMs < 0 代表永不超时 >=0 代表超时毫秒数
    *@return    是否成功
    */
    CHNResult<> TcpConnect(const CHNSockAddr& addr , int nWaitsMs = -1)
    {
        CHNResult<> result;
        int ret = ::connect(m_socket, (const struct sockaddr*)&addr.m_addr, sizeof(addr.m_addr));
        if(ret == 0)
        {
            return result;
        }
        else
        {
            int nErrorCode = HNGetLastError();
#if defined(HCOS_WIN)
            if(nErrorCode != WSAEWOULDBLOCK))
#else
            if (nErrorCode != EINPROGRESS && nErrorCode != EAGAIN && nErrorCode != EWOULDBLOCK)
#endif
            {
                return result.SetSystemFail();
            }
        }

        struct timeval tv;
        tv.tv_sec = nWaitsMs / 1000;
        tv.tv_usec = (nWaitsMs % 1000) * 1000;

        fd_set rfds,wfds;
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);

        FD_SET(m_socket, &rfds);
        FD_SET(m_socket, &wfds);

        ret = ::select(m_socket + 1, &rfds, &wfds, NULL, (nWaitsMs < 0) ? NULL : (&tv));
        if(ret < 0)
        {
            return result.SetSystemFail();
        }
        else if(ret == 0)
        {
            return result.SetFail("connect timeout");
        }
        if(FD_ISSET(m_socket, &rfds) || FD_ISSET(m_socket, &wfds))
        {
            //这里可能有两种情况：1连接失败；2连接成功且服务器正好发了一个数据过来。所以要做区分
            ret = ::connect(m_socket, (const struct sockaddr*)&addr.m_addr, sizeof(addr.m_addr));
            if(ret == 0)
            {
                return result;
            }
            int nErrorCode = HNGetLastError();
#if defined(HNOS_WIN)
            if(nErrorCode != WSAEISCONN)
#else
            if(nErrorCode != EISCONN)       
#endif
            {
                //already connected
                return result;
            }
            else
            {
                return result.SetSystemFail(nErrorCode);
            }
        }
        return result.SetFail("not FD_ISSET succ.");
    }


    /**
    *@brief     连接成功后获取自身连接对应的ip和端口
    *@return    是否成功。成功后std::string为自身ip地址；int为自身端口
    */

    CHNResult<std::pair<std::string, int>> GetSelfIpv4Addr()
    {
        CHNResult<std::pair<std::string, int>> result;
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        if(::getsockname(m_socket, (struct sockaddr*)&addr, &len) < 0)
        {
            return result.SetSystemFail();
        }
        char buf[MAX_PATH] = {0};
        memset(buf,0,sizeof(buf));
        if(NULL == HN_INET_NTOP(AF_INET, (void *)(&(addr.sin_addr)), buf, static_cast<socklen_t>(sizeof(buf))))
        {
            return result.SetSystemFail();
        }
        unsigned short port = ntohs(addr.sin_port);
        return result.SetSucc(std::make_pair(buf, port));
    }
public:
    /**
    *@brief     设置SOCKET连接为非阻塞模式
    */
    CHNResult<> SetNonBlocking()
    {
        CHNResult<> result;
#if defined(HNOS_WIN)
        unsigned long ulArgp = 1;
        int nRet = ::ioctlsocket(m_socket, FIONBIO, &ulArgp);
        if(nRet != 0)
        {
            return result.SetSystemFail();
        }
#else
        int opts = ::fcntl(m_socket, F_GETFL);
        if(opts < 0)
        {
            return result.SetSystemFail();
        }
        opts = opts | O_NONBLOCK;
        if(::fcntl(m_socket, F_SETFL, opts) < 0)
        {
            return result.SetSystemFail();
        }
#endif
        return result;
    }

    /**
    *@brief     设置为可重用端口
    *@param     on    [in]    是否可重用端口
    *@return    是否成功
    */
    CHNResult<> SetReuseable(bool on)
    {
        CHNResult<> result;
        int opt = on ? 1 : 0;
        if(::setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0)
        {
            return result.SetSystemFail();
        }
        return result;
    }
    /**
    *@brief     是否关闭Nagle算法。
    *@param     on    [in]    true关闭Nagle算法。 false开启Nagle算法[默认值]。
    *@return    是否成功
    */

    CHNResult<> SetNoDelay(bool on)
    {
        CHNResult<> result;
        int opt = on ? 1 : 0;
        if(::setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(opt)) < 0)
        {
            return result.SetSystemFail();
        }
        return result;
    }
    /**
    *@brief     设置keepalive属性，完成端口可能会忽略，这个接口最好不要使用
    *@param     on    [in]    是否打开
    *@return    是否成功
    */    

    CHNResult<> SetKeepalive(bool on)
    {
        CHNResult<> result;
        int opt = on ? 1 : 0;
        if(::setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, (const char*)&opt, sizeof(opt)) < 0)
        {
            return result.SetSystemFail();
        }
        return result;
    }


public:
    int m_socket;
};


//////////////////////////[网络框架层]////////////////////////////////////////////////////

class CHNSocketHandle;
typedef std::shared_ptr<CHNSocketHandle> CHNSocketHandlePtr;
typedef std::weak_ptr<CHNSocketHandle> CHNSocketHandleWeakPtr;

/**
*@brief     CHNPoller的基类,抽象了跨平台的事件框架
*           实现了Reactor模式的高性能事件处理器，用IOCP模拟EPOLL仅做事件通知之用
*           【windows】基于IOCP
*           【 linux 】基于EPOLL
*           【 macos 】基于KQUEUE
*           另外一个目的是可以一个头文件里面使CHNPoller和CHNSocketHandle能相互引用
*/

class CHNPollerBase : CHNNoCopyable
{
public:
    /**
    *@brief     基类接口定义部分
    */
    virtual CHNResult<> AssociateSocket(CHNSocketHandleWeakPtr sockptr) = 0;
    virtual CHNResult<> RegisterConnect(CHNSocketHandle* sockptr, const CHNSockAddr& addr) = 0;
    virtual CHNResult<> RegisterRead(CHNSocketHandle* sockptr) = 0;
    virtual void UnAssociateSocket(CHNSocketHandle* sockptr) = 0;

protected:
    /**
    *@brief     事件抽象部分
    */
    CHNPollerBase() 
#if defined(HNOS_WIN)
    : m_hEvHandle(NULL)
    , m_lpfnConnectEx(NULL)
    , m_lpfnAcceptEx(NULL)
    , m_lpfnGetAcceptExSockAddrs(NULL)
#else
    : m_hEvHandle(-1)
#endif
    {

    }

    /**
    *@brief     事件机制是否可用
    */
   bool EventValid()
   {
#if defined(HNOS_WIN)
    return (NULL != m_hEvHandle);
#else
    return (-1 != m_hEvHandle);
#endif
   }

    /**
    *@brief     事件机制初始化
    */
    CHNResult<> EventInit()
    {
        CHNResult<> result; 
#if defined(HNOS_WIN)
        //初始化WinSock2环境
        WSADATA wsaData = { 0 };
        int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (nRet != 0 || HIBYTE(wsaData.wVersion) != 2 || LOBYTE(wsaData.wVersion) != 2)
        {
            return result.SetFail("WSAStartup version not supported");
        }
        //获取ConnectEx函数地址
        DWORD dwBytes;
        int sfdTmp = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sfdTmp == SOCKET_ERROR)
        {
            return result.SetSystemFail();
        }
        GUID  GuidConnectEx = WSAID_CONNECTEX;
        int dwErr = WSAIoctl(sfdTmp, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx), &m_lpfnConnectEx, sizeof(m_lpfnConnectEx), &dwBytes, NULL, NULL);
        if (dwErr == SOCKET_ERROR)
        {
            return result.SetSystemFail();
        }
        //获取AcceptEx函数地址
        GUID  GuidAcceptEx = WSAID_ACCEPTEX;
        dwErr = WSAIoctl(sfdTmp, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &m_lpfnAcceptEx, sizeof(m_lpfnAcceptEx), &dwBytes, NULL, NULL);
        if (dwErr == SOCKET_ERROR)
        {
            return result.SetSystemFail();
        }
        //获取GetAcceptExSockAddrs函数地址
        GUID  GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
        dwErr = WSAIoctl(sfdTmp, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockAddrs, sizeof(GuidGetAcceptExSockAddrs), &m_lpfnGetAcceptExSockAddrs, sizeof(m_lpfnGetAcceptExSockAddrs), &dwBytes, NULL, NULL);
        if (dwErr == SOCKET_ERROR)
        {
            return result.SetSystemFail();
        }
        HNCloseSocket(sfdTmp);

        m_hEvHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
        if (NULL == m_hEvHandle)
        {
            return result.SetSystemFail();
        }
#elif defined(HNOS_LINUX)
        //防止网络通信时write函数产生的SIGPIPE信号导致程序退出
        signal(SIGPIPE, SIG_IGN);
        m_hEvHandle = epoll_create1(O_CLOEXEC);
        if(m_hEvHandle == -1 || (errno == ENOSYS || errno == EINVAL))
        {
            //epoll_create1可能失败，原因是老的kernel没有实现此调用/或无法理解O_CLOEXEC flag
            m_hEvHandle = epoll_create(256);
            if(m_hEvHandle != -1)
            {
                int r = 0;
                do
                {
                    r = ioctl(m_hEvHandle, FIOCLEX);
                } 
                while (r == -1 && errno == EINTR);
                
            }
        }
        if(m_hEvHandle == -1)
        {
            return result.SetSystemFail();
        }
#else
        //防止网络通信时write函数产生的SIGPIPE信号导致程序退出
        signal(SIGPIPE, SIG_IGN);
        m_hEvHandle = kqueue();
        if (m_hEvHandle < 0)
        {
            return result.SetSystemFail();
        }
#endif
        return result;
    }  

    /**
    *@brief     事件机制清理并关闭
    */
    void EventCleanup()
    {
#if defined(HNOS_WIN)
        if(m_hEvHandle)
        {
            PostQueuedCompletionStatus(m_hEvHandle, 0, (DWORD)0, NULL);
            HNSleep(1);
        }
        HNCloseHandle(m_hEvHandle);
        WSACleanup();
#else
        HNCloseSocket(m_hEvHandle);
#endif
    }

    /**
    *@brief     事件机制绑定一个socket
    *@param     sock        [in]    socket描述符
    *@param     userdata    [in]    用户数据，可以是自定义的socket id
    *@return    是否成功。
    */
    bool EventBindSocket(SOCKET sock, void* userdata)
    {
#if defined(HNOS_WIN)
        //和完成端口绑定
        HANDLE hCompletionPort = m_hEvHandle;
        HANDLE h = CreateIoCompletionPort((HANDLE)sock, hCompletionPort, (ULONG_PTR)userdata, 0);
        if (h != hCompletionPort)
        {
            return false;
        }
#endif
    }

    /**
    *@brief     事件机制给一个socket添加一个发起连接的监听任务
    *@param     sock        [in]    socket描述符
    *@param     userdata    [in]    用户数据，可以是自定义的socket id
    *@param     addr        [in]    是否为读请求，否则为写请求
    */
    CHNResult<> EventAddConnect(SOCKET sock, void* userdata, const CHNSockAddr& addr)
    {
        CHNResult<> result;
        int lastErrcode = 0;
#if defined(HNOS_WIN)
        if (m_lpfnConnectEx == NULL)
        {
            return result.SetFail("m_lpfnConnectEx == NULL");
        }
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        BOOL bRet = ((LPFN_CONNECTEX)m_lpfnConnectEx)(sock, (const struct sockaddr *)&addr.m_addr, sizeof(addr.m_addr), NULL, 0, NULL, &m_overlapped);
        if (!bRet && (lastErrcode = WSAGetLastError()) != ERROR_IO_PENDING)
        {
            return result.SetSystemFail(lastErrcode);
        }
#elif defined(HNOS_LINUX)
        int nRet = ::connect(sock, (const struct sockaddr *)&addr.m_addr, sizeof(addr.m_addr));
        if (nRet != 0 && (lastErrcode = HNGetLastError()) != EINPROGRESS)
        {
            return result.SetSystemFail(lastErrcode);
        }
        struct epoll_event epv = { 0, { 0 } };
        epv.data.ptr = (void*)userdata;
        epv.events = EPOLLOUT | EPOLLET;
        if (0 != epoll_ctl(m_hEvHandle, EPOLL_CTL_ADD, sock, &epv))
        {
            return result.SetSystemFail();
        }
#else
        int nRet = ::connect(sock, (const struct sockaddr *)&addr.m_addr, sizeof(addr.m_addr));
        if (nRet != 0 && (lastErrcode = HNGetLastError()) != EINPROGRESS)
        {
            return result.SetSystemFail(lastErrcode);
        }
        struct kevent epv = { 0, 0, 0, 0, 0, 0 };
        EV_SET(&epv, sock, EVFILT_WRITE, EV_ADD | EV_CLEAR | EV_ENABLE | EV_ONESHOT, 0, 0, (void*)userdata);
        if (-1 == kevent(m_hEvHandle, &epv, 1, nullptr, 0, nullptr))
        {
            return result.SetSystemFail();
        }
#endif
        return result;
    }

    /**
    *@brief     事件机制给一个socket添加一个读取数据的监听任务
    *@param     sock        [in]    socket描述符
    *@param     userdata    [in]    用户数据，可以是自定义的socket id
    */

    CHNResult<> EventAddRead(SOCKET sock, void* userdata)
    {
        CHNResult<> result;
#if defined(HNOS_WIN)
        WSABUF   wsbuf;
        wsbuf.buf = NULL;
        wsbuf.len = 0;
        DWORD dwFlags = 0;
        memset(&m_overlapped, 0, sizeof(m_overlapped));
        int ret = WSARecv(sock, &wsbuf, 1, NULL, &dwFlags, &m_overlapped, NULL);
        int errcode = WSAGetLastError();
        if (ret == SOCKET_ERROR && errcode != ERROR_IO_PENDING)
        {
            return result.SetSystemFail(errcode);
        }
#elif defined(HNOS_LINUX)
        struct epoll_event epv = { 0, { 0 } };
        epv.data.ptr = (void*)userdata;
        epv.events = EPOLLIN | EPOLLET;
        if (0 != epoll_ctl(m_hEvHandle, EPOLL_CTL_ADD, sock, &epv))
        {
            return result.SetSystemFail();
        }
#else
        struct kevent epv = { 0, 0, 0, 0, 0, 0 };
        EV_SET(&epv, sock, EVFILT_READ, EV_ADD | EV_CLEAR | EV_ENABLE | EV_ONESHOT, 0, 0, (void*)userdata);
        if (-1 == kevent(m_hEvHandle, &epv, 1, nullptr, 0, nullptr))
        {
            return result.SetSystemFail();
        }
#endif
    }

    /**
    *@brief     事件机制移除一个socket上所有的监听任务
    *@param     sock        [in]    socket描述符
    *@param     userdata    [in]    用户数据，可以是自定义的socket id
    */
    void EventDelAll(SOCKET sock, void* userdata)
    {
        if(sock == INVALID_SOCKET)
        {
            return;
        }
#if defined(HNOS_WIN)
        //nothing to do.
#elif defined(HNOS_LINUX)
        struct epoll_event epvdel = { 0, { 0 } };
        epvdel.data.ptr = (void*)userdata;
        epvdel.events = EPOLLIN | EPOLLOUT | EPOLLET;
        epoll_ctl(m_hEvHandle, EPOLL_CTL_DEL, sock, &epvdel);
#else 
        //nothing to do.
#endif
    }

    /**
    *@brief     事件机制处理事件
    *@param     func        [in]    事件处理函数，如果获取到了多个事件可能会回调多次
    *                               func函数对应的参数说明：
    *                               userdata    用户数据，可以是自定义的socket id
    *                               code        0代表数据到来，否则代表错误到来
    *@return    一般会返回true代表事件处理成功。否则代表出现了严重错误或事件机制被关闭了需要退出
    */
    bool EventPoll(std::function<void(void* userdata, int code)> func)
    {
        if(!this->EventValid())
        {
            return false;
        }
#if defined(HNOS_WIN)
        ULONG_PTR       socketidkey = 0;
        OVERLAPPED*     lpOverlapped = NULL;
        unsigned long   dwBytes = 0;
        BOOL bRet = GetQueuedCompletionStatus(m_hEvHandle, &dwBytes, &socketidkey, (LPOVERLAPPED*)&lpOverlapped, INFINITE);
        if (!bRet)
        {
            if (!this->EventValid())
                return false;
            //如果* lpOverlapped不为NULL且函数从完成端口使失败的I / O操作的完成数据包出列，
            //则该函数将有关失败操作的信息存储在lpNumberOfBytes，lpCompletionKey和lpOverlapped指向的变量中。要获取扩展错误信息，请调用GetLastError
            DWORD dwLastErr = HNGetLastError();
            if (dwLastErr == 0 || dwLastErr == WAIT_TIMEOUT || lpOverlapped == NULL)
            {
                return true;
            }
            //如果* lpOverlapped为NULL，则该函数不会从完成端口使完成数据包出列。
            //在这种情况下，该函数不会将信息存储在lpNumberOfBytes和lpCompletionKey参数指向的变量中，并且它们的值是不确定的
            func((void*)socketidkey, dwLastErr);
        }
        else
        {
            if (lpOverlapped == NULL)
                return false;
            func((void*)socketidkey, 0);
        }
#elif defined(HNOS_LINUX)
        struct epoll_event epollEvents[MAX_PATH];
        int nWaitFds = epoll_wait(m_hEvHandle, epollEvents, MAX_PATH, 100);
        if (nWaitFds <= 0)
        {
            return (this->EventValid());
        }
        for (int i = 0; i < nWaitFds; i++)
        {
            void* socketid = epollEvents[i].data.ptr;
            if (epollEvents[i].events & EPOLLIN)
            {
                func((void*)socketid, 0);
            }
            if (epollEvents[i].events & EPOLLERR)
            {
                func((void*)socketid, HNERR_EVENTQUEUE);
            }
            else if (epollEvents[i].events & EPOLLOUT)
            {
                func((void*)socketid, 0);
            }
        }
#else
    struct kevent kEvents[MAX_PATH];
    int nWaitFds = kevent(m_hEvHandle, nullptr, 0, kEvents, MAX_PATH, nullptr);
    if (nWaitFds <= 0)
    {
        return (this->EventValid());
    }
    for (int i = 0; i < nWaitFds; i++)
    {
        void* socketid = kEvents[i].udata;
        if (kEvents[i].filter == EVFILT_READ)
        {
            func((void*)socketid, 0);
        }
        if (kEvents[i].flags & EV_EOF || kEvents[i].flags & EV_ERROR)
        {
            func((void*)socketid, HNERR_EVENTQUEUE);
        }
        else if (kEvents[i].filter == EVFILT_WRITE)
        {
            func((void*)socketid, 0);
        }
    }
#endif
    }

protected:
    /**
    *@brief     事件底层句柄
    */
#if defined(HNOS_WIN)
    HANDLE      m_hEvHandle;    //完成端口句柄
    OVERLAPPED  m_overlapped;   //无用，仅仅作为参数传入
    void*       m_lpfnConnectEx;//LPFN_CONNECTEX
    void*       m_lpfnAcceptEx; //LPFN_ACCEPTEX
    void*       m_lpfnGetAcceptExSockAddrs;//LPFN_GETACCEPTEXSOCKADDRS
#else
    int         m_hEvHandle;    //epoll/kqueue句柄
#endif
};

/**
*@brief     【注意：CHNSocketHandle对象只能被智能指针管理，否则崩溃】
*            Reactor模式的socket处理句柄，一个句柄代表一个连接
*            用于和CHNPoller配合使用。
*            支持两种连接:1时作为客户端进行主动连接(XConnect)
*                       2是作为服务端的处理器处理到来的连接(ProcessAttach)
*/

class CHNSocketHandle
    : CHNNoCopyable
    , public std::enable_shared_from_this<CHNSocketHandle>
{
public:
    /**
    *@brief        构造CHNSocketHandle，此时并不新建socket
    *@param        bClient      [in]    true为客户端，false为服务端的处理连接。
    *@param        pollerptr    [in]    实际的poller，传入&CHNPoller::obj()即可，在这里写默认参数会编译不过，因为CHNPoller是在下面定义的
    */
    explicit CHNSocketHandle(bool bClient, CHNPollerBase* pollptr /* = &CHNPoller::obj() */)
    :m_socketid(0)
    , m_bClient(bClient)
    , m_bUseSSL(false)
    , m_PollerBase(pollptr)
    {

    }

    /**
    *@brief     判断是否关闭
    *@return    是否关闭
    */
    bool Closed(){return (!m_hnsocket.IsValid());}

    /**
    *@brief     获取原始socket对象
    *@return    原始socket对象
    */
    CHNSocket& GetSocket(){return (m_hnsocket);}

    /**
    *@brief     获取socket对象内部id
    *@return    内部id
    */
    int GetInternalSocketId(){return m_socketid;}

    /**
    *@brief      主动执行关闭的动作
    *            执行这个动作会触发OnClosed事件
    */
    void XClose()
    {
        if(m_bUseSSL)
        {
            m_SslConnection.FreeConnection();
        }
        if(m_hnsocket.IsValid())
        {
            m_PollerBase->UnAssociateSocket(this);
            m_hnsocket.ShutdownSocket();
            m_NotifyQueue.Put(HNERR_CLOSED);
            OnClosed();
        }
    }

    /**
    *@brief     作为客户端执行连接动作，和ProcessAttach互不兼容
    *@param     addr        [in]    远端地址
    *@param     nTimeOut    [in]    超时时间。如果nTimeoutMs < 0 代表永不超时 >=0 代表超时毫秒数
    *@param     bUseSSL     [in]    是否使用ssl连接，ssl和非ssl不能混用
    *@return    是否成功。成功后StrResult1()为自身ip地址；IntResult()为自身端口
    */
    CHNResult<std::pair<std::string, int> > XConnect (const CHNSockAddr& addr, int nTimeoutMs, bool bUseSSL = false)
    {
        m_bUseSSL = bUseSSL;
        CHNResult<std::pair<std::string, int> > result;
        if(!m_bClient || m_PollerBase == NULL)
        {
            return result.SetFail("socket type is server || m_PollerBase == NULL.");
        }
        XClose();
        m_NotifyQueue.Clear();
        auto result2 = m_hnsocket.CreateAndBind();
        if (!result2)
        {
            return result.SetFail("CreateAndBind fail:%s",result2.ErrDesc());
        }
        result2 = m_hnsocket.SetNonBlocking();
        if (!result2)
        {
            XClose();
            return result.SetFail("SetNonBlocking fail:%s",result2.ErrDesc());
        }
        result2 = m_PollerBase->AssociateSocket((CHNSocketHandleWeakPtr)(shared_from_this()));
        if (!result2)
        {
            XClose();
            return result.SetFail("AssociateSocket fail:%s",result2.ErrDesc());
        }
        result2 = m_PollerBase->RegisterConnect(this, addr);
        if (!result2)
        {
            XClose();
            return result.SetFail("RegisterConnect fail:%s",result2.ErrDesc());
        }
        int nNotifyCode = 0;
        if (!m_NotifyQueue.Take(nTimeoutMs, nNotifyCode))
        {
            XClose();
            return result.SetFail("connection timeout.");
        }
        if (nNotifyCode != 0)
        {
            XClose();
            return result.SetSystemFail(nNotifyCode);
        }
        if (m_bUseSSL)
        {
            result2 = m_SslConnection.SslInitConnect(m_hnsocket.m_socket, nTimeoutMs);
            if (!result2)
            {
                XClose();
                return result.SetFail("SslInitConnect fail:%s",result2.ErrDesc());
            }
        }
        return m_hnsocket.GetSelfIpv4Addr();
    }

    /**
    *@brief     发送数据动作
    *@param     data        [in]    数据
    *@param     length      [in]    数据长度
    *@param     nTimeoutMs  [in]    超时时间毫秒，<0代表永不超时
    *@return    是否成功
    */

    CHNResult<> XSend(const char* data, size_t length, int nTimeoutMs = -1)
    {
        CHNResult<> result;
        if(!m_hnsocket.IsValid())
        {
            XClose();
            return result.SetFail("socket is invalid.");
        }
        if(data == NULL)
        {
            return result.SetFail("data is NULL");
        }

        if (length > 0xFFFFFFFF)
        {
            return result.SetFail("data length too long");
        }
        if (m_bUseSSL)
        {
            result = m_SslConnection.SslSend(data, length);
            if (!result)
            {
                XClose();
            }
            return result;
        }
        CHNTimeValue tvStart = CHNTimeValue::TickCount();
        size_t nSendBytes = 0;
        while(1)
        {
            int nRetLen = (int)::send(m_hnsocket.m_socket, data + nSendBytes, (int)(length - nSendBytes), 0);
            if (nRetLen > 0 && length > 0)
            {
                nSendBytes += nRetLen;

                //发完了
                if (nSendBytes >= length)
                {
                    break;
                }
            }
            else if (nRetLen == 0 && length == 0)
            {
                //写入了0字节，只有主动发送0字节才会有这种情况。其他情况要报错
                break;
            }
            else
            {
                int nErrorCode = HNGetLastError();
#if defined(HNOS_WIN)
                if (nErrorCode == WSAEWOULDBLOCK)
#else
                if (nErrorCode == EAGAIN || nErrorCode == EWOULDBLOCK)
#endif
                {
                    //内核缓冲区写满了，稍等下，继续发
                    HNSleep(100);
                }
                else
                {
                    //出现错误，直接断开连接
                    XClose();
                    return result.SetSystemFail(nErrorCode);
                }
            }
        }
        return result;
    }

    /**
    *@brief     从接收数据流中读取数据，超时则直接关闭
    *@param     nWaitMs       [in]    等待多少毫秒后超时。<0无限等待 0马上返回 >0为超时时间。
    *@param     buf           [in]    用户提供的数据缓冲区，例如用户提供2048字节缓冲区：CHNBuffer buf(2048);缓冲区中如果已有数据，将会被清空
    *@param     bForceBufFull [in]    bForceBufFull = false   一般情况：超时/出错/用户缓冲区读满/网卡缓冲区读完则返回
    *                                 bForceBufFull = true    例外情况：强制用户缓冲区读满则才返回，或者出现出错/超时。
    *@return    是否成功
    */
    CHNResult<> XRecv(int nWaitMs, CHNBuffer& buf, bool bForceBufFull = false)
    {
        CHNResult<> result;
        unsigned int nBufCapacity = (unsigned int)buf.capacity();
        if(!m_hnsocket.IsValid() || m_PollerBase == NULL || nBufCapacity == 0)
        {
            return result.SetFail("params invalid or not socket invalid.");
        }

        buf.updatesize(0);

        CHNTimeValue startTime = CHNTimeValue::TickCount();
        while(1)
        {
            result = m_PollerBase->RegisterRead(this);
            if(!result)
            {
                XClose();
                return ((buf.empty()) ? (result) : (result.SetSucc()));
            }
            int notifyCode = 0;
            bool bSucc = m_NotifyQueue.Take(nWaitMs, notifyCode);
            if(!bSucc)
            {
                return ((buf.empty()) ? (result.SetFail("recv timeout.")) : (result));
            }
            if(notifyCode != 0)
            {
                XClose();
                return ((buf.empty()) ? (result.SetSystemFail(notifyCode)) : (result));
            }

            //接收数据，直到buffer收满
            while(1)
            {
                if(buf.size() >= nBufCapacity)
                {
                    return result;
                }
                int nRetLen = 0;
                if(m_bUseSSL)
                {
                    nRetLen = m_SslConnection.SslRecv(buf.data() + buf.size(), nBufCapacity - (int)buf.size());
                }
                else
                {
                    nRetLen = ::recv(m_hnsocket.m_socket, buf.data() + buf.size(), nBufCapacity - (int)buf.size(), 0);
                }

                if(nRetLen > 0)
                {
                    buf.addsize(nRetLen);
                    continue;
                }
                else if(nRetLen == 0)
                {
                    XClose();
                    return ((buf.empty()) ? (result.SetFail("remote host closed connection.")) : (result));
                }
                else
                {
                    if(m_bUseSSL)
                    {
                        if(m_SslConnection.CanContinueRead(nRetLen))
                        {
                            break;
                        }
                        else
                        {
                            XClose();
                            return ((buf.empty()) ? (result.SetFail( "ssl recv err.")) : (result));
                        }
                    }
                    else
                    {
                        int nErrorCode = HNGetLastError();
#if defined(HNOS_WIN)
                        if(nErrorCode == WSAEWOULDBLOCK)
#else 
                        if(nErrorCode == EAGAIN || nErrorCode == EWOULDBLOCK)
#endif
                        {
                            break;
                        }
                        else
                        {
                            XClose();
                            return ((buf.empty()) ? (result.SetSystemFail(nErrorCode)) : (result));
                        }
                    }
                }
            }
            //没有出现错误&&没有超时&&指定了一定要接收的长度&&当前接收长度小于指定长度||接收长度为0，还需要继续收
            if(m_hnsocket.IsValid() && !startTime.IsTimeOut(nWaitMs) && 
                (buf.size() == 0 || (bForceBufFull && buf.size() < nBufCapacity)))
                {
                    HNSleep(1);
                    continue;
                }
            else
            {
                break;
            }
        }
        return result;
    }
protected:
    //用户实现这些部分虚函数来扩展功能。注意：
    //1.所有虚函数用户只需要实现，而不允许调用！
    //2.这些函数里面不允许做耗时操作！

    /**
    *@brief       用户处理关闭事件，这个时候连接已经关闭，用户只需要处理自己的业务事件
    *            【注意：在OnClose函数里面禁止再调用XClose！】
    *            【注意：如果用户在处理业务半途收到关闭事件，忽略即可，收到关闭事件后并不会删除收发缓冲区数据】
    */
    virtual void OnClosed() {
    //        printf("[%p]socket closed\n", this);
    }

    /**
    *@brief       用户处理连接到来事件，这个时候连接已经绑定好，用户只需要处理自己的业务事件
    *            【注意：仅用于服务端的处理连接】
    *@param       addr    [in]    传入的连接地址
    */
   virtual void OnAccepted(const CHNSockAddr& addr) {}

    /**
    *@brief       用于服务端处理数据队列中的数据
    *             一般的做法是在这里循环调用XGetPack拿取数据进行处理
    *            【注意：仅用于服务端的处理连接】
    *            【注意：重写此方法时，需保证此方法为public的】
    */
   virtual void OnService() {}
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //内部处理，无需关注
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

public:
    /**
    *@brief     连接到来事件，禁止用户调用，和XConnect互不兼容
    *@param     sock        [in]    传入的连接
    *@param     addr        [in]    传入的连接地址
    *@param     bUseSSL     [in]    是否使用ssl连接，ssl和非ssl不能混用
    *@param     sslServerCtx[in]    仅bUseSSL时有效，服务器的ctx
    *@return    是否成功
    */
    CHNResult<> ProcessAttach(int sock, const CHNSockAddr& addr, bool bUseSSL, const CHNSSLServerCtx &sslServerCtx = CHNSSLServerCtx())
    {
        CHNResult<> result;
        m_bUseSSL = bUseSSL;
        XClose();
        m_NotifyQueue.Clear();
        if(m_bClient || m_PollerBase == NULL)
        {
            return result.SetFail("socket type is client || m_PollerBase == NULL.");
        }
        m_hnsocket.m_socket = sock;
        if(m_bUseSSL)
        {
            result = m_SslConnection.SslInitAttach(m_hnsocket.m_socket, sslServerCtx);
            if (!result)
            {
                XClose();
                return result;
            }
        }
        result = m_hnsocket.SetNonBlocking();
        if (!result)
        {
            XClose();
            return result;
        }
        result = m_PollerBase->AssociateSocket(shared_from_this());
        if (!result)
        {
            XClose();
            return result;
        }
        OnAccepted(addr);
        return result;
    }
    /**
    *@brief     库内部处理通知事件
    *@param     code    [in]    见m_NotifyQueue变量说明
    */
   void NotifyEvent(int code)
   {
       m_NotifyQueue.Put(code);
   }
protected:
    int  m_socketid;   //连接唯一id
    bool m_bClient;    //是否为客户端
    bool m_bUseSSL;    //是否为ssl连接
    CHNSSLConnection       m_SslConnection; //ssl连接对象
    CHNBlockingQueue<int>  m_NotifyQueue;   //接收通知队列，0代表数据到来，否则代表错误到来
    CHNPollerBase*         m_PollerBase;    //CHNPoller指针
    CHNSocket              m_hnsocket;      //原始套接字
    friend class CHNPoller;
};

/**
*@brief 【单例】网络框架的事件基础，如果要使用网络功能，需要在使用之前初始化它,如：
*       int main(int argc, char** argv)
*       {
*           //init.
*           CHNResult result = CHNPoller::obj().Start();
*           if(!result)
*           {
*               printf("init fail. %s\n", result.ErrDesc());
*               return 0;
*           }
*
*           //your code.
*           //....
*
*           //clean up.
*           CHNPoller::obj().Stop();
*           return 0;
*       }
*/

class CHNPoller : public CHNPollerBase
{   
public:
    static CHNPoller& obj()
    {
        static CHNPoller o;
        return o;
    }
    /**
    *@brief     事件处理器启动
    *           【注意！要使用hnlib的网络功能，必须在程序启动时调用这个函数】
    *@param     bSSLThreadSafe    [in]    是否要实现openssl的线程安全，如果加载了其他库如libcurl已经实现了线程安全，这里就不需要了。
    *@return    是否成功
    */
    CHNResult<> Start(bool bSSLThreadSafe = true)
    {
        CHNResult<> result;
        if(this->EventValid())
        {
            return result;
        }
        hnlib_internal::CHNLocalErrInfo::obj();
        CHNStaticDeclare::obj();
        CHNThreadPool::Default();
        CHNLog::Default();

        //初始化事件机制
        result = this->EventInit();
        if (!result)
        {
            return result;
        }

        //初始化ssl
        CHNSSLUtil::obj().Init(bSSLThreadSafe);

        return CHNThreadPool::Default().AddTask(std::bind(&CHNPoller::LoopPollerInThread, this));
    }

    /**
    *@brief     事件处理器停止
    */

    void Stop()
    {
        if(!this->EventValid())
        {
            return ;
        }

        {
            CHNGuard<CHNCountDownLatch> guard(m_condStoped);
            for(auto it = m_sockethadles.begin() ; it != m_sockethadles.end() ; ++it)
            {
                CHNSocketHandlePtr sharedptr((it->second).lock());
                if(sharedptr)
                {
                    sharedptr->NotifyEvent(HNERR_CLOSED);
                }
                m_sockethadles.clear();
            }

            this->EventCleanup();
            m_condStoped.Wait();
            CHNSSLUtil::obj().Release();
        }
    }


private:

    /**
    *@brief     将一个socket和事件处理器进行关联
    *@param     sockptr    [in]    附加指针，为CHNSocketHandle指针
    *@return    是否成功
    */
    CHNResult<> AssociateSocket(CHNSocketHandleWeakPtr sockptr) override
    {
        CHNResult<> result;
        CHNSocketHandlePtr sharedptr(sockptr.lock());

        if (!sharedptr)
        {
            return result.SetFail( "sockptr destroyed.");
        }
        if (!sharedptr->m_hnsocket.IsValid())
        {
            return result.SetFail( "sock == INVALID_SOCKET || sockptr == NULL");
        }
        if (!this->EventValid())
        {
            return result.SetFail( "you must call CHNPoller::obj().Start() before use network framework.");
        }

        {
            CHNGuard<CHNCountDownLatch> guard(m_condStoped);
            m_nGlobalSocketId++;
            sharedptr->m_socketid = m_nGlobalSocketId;

            m_sockethadles.insert(std::make_pair(m_nGlobalSocketId, sockptr));
        }
        //和完成端口绑定
        if (!this->EventBindSocket(sharedptr->m_hnsocket.m_socket, (void*)sharedptr->m_socketid))
        {
            return result.SetSystemFail();
        }
        return result;
    }
    /**
    *@brief     注册一个连接事件，注册后，当连接成功时，会通知userptr处理连接可写事件
    *@param     sockptr     [in]    socket
    *@param     addr        [in]    要连接的地址
    *@return    是否成功
    */
    CHNResult<> RegisterConnect(CHNSocketHandle* sockptr, const CHNSockAddr& addr) override
    {
        CHNResult<> result;
        if(!sockptr->m_hnsocket.IsValid())
        {
            return result.SetFail("sock == INVALID_SOCKET");
        }

        if(!this->EventValid())
        {
            return result.SetFail("you must call CHNPoller::obj().Start() before use network framework.");
        }

        this->EventDelAll(sockptr->m_hnsocket.m_socket, (void*)sockptr->m_socketid);
        return this->EventAddConnect(sockptr->m_hnsocket.m_socket, (void*)sockptr->m_socketid, addr);
    }

    /**
    *@brief     注册一个数据可读事件，注册后，当数据可读时，会通知处理连接可读事件
    *@param     sockptr        [in]    socket
    *@return    是否成功
    */
    CHNResult<> RegisterRead(CHNSocketHandle* sockptr) override
    {
        CHNResult<> result;
        if (!sockptr->m_hnsocket.IsValid())
        {
            return result.SetFail( "sock == INVALID_SOCKET");
        }
        if (!this->EventValid())
        {
            return result.SetFail( "you must call CHNPoller::obj().Start() before use network framework.");
        }
        this->EventDelAll(sockptr->m_hnsocket.m_socket, (void*)sockptr->m_socketid);
        return this->EventAddRead(sockptr->m_hnsocket.m_socket, (void*)sockptr->m_socketid);
    }

    /**
    *@brief     将一个socket和事件处理器进行解除关联
    *@param     sockptr    [in]    CHNSocketHandle指针
    *@return    是否成功
    */
    void UnAssociateSocket(CHNSocketHandle* sockptr) override
    {
        this->EventDelAll(sockptr->m_hnsocket.m_socket, (void*)sockptr->m_socketid);
        CHNGuard<CHNCountDownLatch> guard(m_condStoped);
        m_sockethadles.erase(sockptr->m_socketid);
    }
private:
    explicit CHNPoller()
        :m_nGlobalSocketId(0)
        , m_condStoped(0)
    {}
    ~CHNPoller()
    {
        Stop();
    }
    void LoopPollerInThread()
    {
        m_condStoped.SetCount(1);

        while( this->EventPoll([this](void* userdata, int code)
        {
            CHNSocketHandlePtr sharedptr;
            {
                CHNGuard<CHNCountDownLatch> guard(m_condStoped);
                auto it = m_sockethadles.find((int)(long)userdata);
                if (m_sockethadles.end() == it)
                {
                    return;
                }
                sharedptr = (it->second).lock();
            }
            if (sharedptr)
            {
                sharedptr->NotifyEvent(code);
            }
        }))
        {

        }
        m_condStoped.CountDown();
    }
private:
    int m_nGlobalSocketId; //全局socket id
    std::unordered_map<int, CHNSocketHandleWeakPtr> m_sockethadles;    //记录socketid和socket智能对象，防止请求到来进行函数调用时socket对象被释放
    CHNCountDownLatch    m_condStoped;      //是否停止标志
    friend class CHNSocketHandle;
};
/**
*@brief  监听连接管理器，用于产生监听连接和管理连接池
*        typename CON 为管理的连接类型，必须是CHNSocketHandle或继承于CHNSocketHandle
*        完成以下管理功能：
*            1.管理连接池，当连接到来时初始化接收连接CON
*/

template <typename CON>
class CHNTcpListenService : CHNNoCopyable
{
public:
    typedef std::function<void ( std::shared_ptr<CON> )> HNNewClientCallBack;
public:
    CHNTcpListenService()
    : m_condStoped(0)
    , m_bSSLMode(false)
    , m_bRunning(false)
    , m_newClientFunc(nullptr)
    {
    }
    ~CHNTcpListenService()
    {
        Stop();
    }
    /**
    *@brief     设置为ssl模式
    *@param     certfilepath    [in]    证书文件位置，PEM文件，base64格式
    *@param     keyfilepath     [in]    私钥文件位置，PEM文件，base64格式
    */
    CHNResult<> SetSSL(const char* certfilepath, const char* keyfilepath)
    {
        m_bSSLMode = true;
        return m_sslctx.InitContext(certfilepath, keyfilepath);
    }

    /**
    *@brief     设置创建新客户端时执行的回调函数
    *@param     func    [in]    回调函数
    */
    void SetCreatedClientCallBack(HNNewClientCallBack func)
    {
        m_newClientFunc = func;
    }

    /**
    *@brief     开始工作
    *@param     addr    [in]    要监听的地址和端口
    *@param     reuse   [in]    是否重用端口
    *@return    是否成功
    */
    CHNResult<> Start(const CHNSockAddr& addr, bool reuse = false)
    {
        CHNResult<> result;
        if(m_bRunning)
        {
            return result;
        }
        if (m_bSSLMode && !m_sslctx.InitComplete())
        {
            return result.SetFail("ssl mode but load ssl ctx fail.");
        }

        result = m_listensocket.TcpListen(addr, reuse);
        if (!result)
        {
            return result;
        }
        m_bRunning = true;

        return CHNThreadPool::Default().AddTask(std::bind(&CHNTcpListenService::WorkingInThread, this));
    }

    /**
    *@brief     停止工作
    */
    void Stop()
    {
        m_bRunning = false;
        HNCloseSocket(m_listensocket.m_socket);
        {
            CHNGuard<CHNCountDownLatch> guard(m_condStoped);
            m_clients.clear();
        }
        m_condStoped.Wait();
    }

    /**
    *@brief     获取客户端
    *@param     clientid    [in]    客户端内部id
    *@return    客户端指针
    */
    std::weak_ptr<CON> GetClient(int clientid)
    {
        std::weak_ptr<CON> ptr;
        {
            CHNGuard<CHNCountDownLatch> guard(m_condStoped);
            auto ret = m_clients.find(clientid);
            if(ret != m_clients.end())
            {
                ptr = ret->second;
            }
            return ptr;
        }
    }

    /**
    *@brief     广播消息
    */
    void BroadCastMsg(const char* data, size_t len)
    {
        CHNGuard<CHNCountDownLatch> guard(m_condStoped);
        for(auto it = m_clients.begin(); it != m_clients.end(); ++it)
        {
            std::shared_ptr<CON> ptr = it->second.lock();
            if(ptr)
            {
                ptr->XSend(data,len);
            }
        }
    }

    /**
    *@brief     已连上的客户端数量
    */
    size_t ClientSize()
    {
        CHNGuard<CHNCountDownLatch> guard(m_condStoped);
        return m_clients.size();
    }
protected:

    void WorkingInThread()
    {
        m_condStoped.SetCount(1);

        while(m_bRunning)
        {
            CHNSockAddr addrClient;
            int clientFd = m_listensocket.Accept(addrClient, 2000);
            if(m_bRunning && INVALID_SOCKET != clientFd)
            {
                m_condStoped.CountUp();
                CHNThreadPool::Default().AddTask(std::bind(&CHNTcpListenService::ClientProcessor, this, clientFd, addrClient));
            }
        }

        m_condStoped.SetCount(0);
    }


    //客户处理
    void ClientProcessor(int clientFd, CHNSockAddr addrClient)
    {
        //建立连接对象
        std::shared_ptr<CON> ptr(new CON());

        //将新来的client丢给连接对象
        auto result = ptr->ProcessAttach(clientFd, addrClient, m_bSSLMode, m_sslctx);
        if (!result)
        {
            return;
        }

        if(m_newClientFunc)
            m_newClientFunc(ptr);

        int clientid = ptr->GetInternalSocketId();
        {
            CHNGuard<CHNCountDownLatch> guard(m_condStoped);
            m_clients.insert(std::make_pair(clientid,ptr));
        }

        ptr->OnService();
        {
            CHNGuard<CHNCountDownLatch> guard(m_condStoped);
            m_clients.erase(clientid);
        }

        //用户退出的时候并没有关闭连接，这个时候可能是用户发送了最后一个回应，需要等一下网络把这个回应彻底发完
        //或者设置linger选项，在close的时候确保数据回应完成。
        //这里采用比较简单的做法，直接等待一个经验值时间，一般发送的时候都可以发完
        if (!ptr->Closed())
        {
            HNSleep(300);
            ptr->XClose();
        }
        m_condStoped.CountDown();
    }
    CHNSocket m_listensocket;
    bool      m_bSSLMode;
    bool      m_bRunning;
    CHNSSLServerCtx      m_sslctx;
    CHNCountDownLatch    m_condStoped;
    std::unordered_map<int, std::weak_ptr<CON> > m_clients;
    HNNewClientCallBack m_newClientFunc;
};

#endif // __HELLO_NET_M_IO_S_TCP__