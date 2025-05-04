#ifndef __HELLONET_M_CORE_S_THREAD__
#define __HELLONET_M_CORE_S_THREAD__
#include "s_base.hpp"

//无参函数
typedef std::function<void()> HNTaskFunc;

/**
*@brief      互斥量，用于线程之间的互斥
*            这里的互斥量是可递归的，也就是同一个线程可加锁多次，相对应的也可以解锁多次
*            (为了设计的简单性，未支持进程之间的互斥，多进程下互斥请使用CHNNamedMutex)。
*/
class CHNRecMutex :CHNNoCopyable
{
public:
    explicit CHNRecMutex()
    {
#if defined(_MSC_VER) && _MSC_VER < 1800
        InitializeCriticalSection(&m_RecMutex);
#endif
    }
    ~CHNRecMutex()
    {
#if defined(_MSC_VER) && _MSC_VER < 1800
        DeleteCriticalSection(&m_RecMutex);
#endif
    }

    /**
    *@brief        互斥量加锁
    */
    void lock()
    {
#if defined(_MSC_VER) && _MSC_VER < 1800
        EnterCriticalSection(&m_RecMutex);
#else
        m_RecMutex.lock();
#endif
    }

    /**
    *@brief        互斥量解锁
    */
    void unlock()
    {
#if defined(_MSC_VER) && _MSC_VER < 1800
        LeaveCriticalSection(&m_RecMutex);
#else
        m_RecMutex.unlock();
#endif
    }

    /**
    *@brief        互斥量尝试锁
    */
    bool try_lock()
    {
#if defined(_MSC_VER) && _MSC_VER < 1800
        return !!TryEnterCriticalSection(&m_RecMutex);
#else
        return m_RecMutex.try_lock();
#endif
    }

public:
    //重载*，主要是给条件变量用
#if defined(_MSC_VER) && _MSC_VER < 1800
    CRITICAL_SECTION&       operator*(void)       { return m_RecMutex; }
    const CRITICAL_SECTION& operator*(void) const { return m_RecMutex; }

    CRITICAL_SECTION     m_RecMutex;
#else
    std::recursive_mutex&       operator*(void)       { return m_RecMutex; }
    const std::recursive_mutex& operator*(void) const { return m_RecMutex; }

    std::recursive_mutex m_RecMutex;
#endif
};

/**
*@brief      守卫类，可以用CHNGuard的栈上对象自动加解锁。
*            可以使用任意锁，只需要锁定义了lock和unlock函数。
*/
template<typename T = CHNRecMutex>
class CHNGuard :CHNNoCopyable
{
public:
    explicit CHNGuard(T& lock) : m_lock(lock)
    {
        m_lock.lock();
    }
    ~CHNGuard() throw ()
    {
        m_lock.unlock();
    }
private:
    T& m_lock;
};

/**
*@brief      递增型ID产生器。
*            可以线程安全地产生不重复的id。
*/
template<typename T>
class CHNIDPool :CHNNoCopyable
{
public:
    explicit CHNIDPool()
        :m_currentId((T)0)
    {}

    /**
    *@brief 默认实例，你也可以生成自己的实例
    *       需要注意的是，由于模板的原因，以下两个默认示例是不同的示例：
    *       1. CHNIDPool<int>::Default()
    *       2. CHNIDPool<int64_t>::Default()
    */
    static CHNIDPool& Default()
    {
        static CHNIDPool o;
        return o;
    }

    /**
    *@brief 产生一个新的ID
    */
    T NewId()
    {
        CHNGuard<CHNRecMutex> guard(m_lock);
        m_currentId++;
        return m_currentId;
    }

    /**
    *@brief 跳转到一个指定位置。
    *       注意，此函数可能破坏产生id的唯一性，所以一般只在初始化时指定初值之用。
    *@param pos [in]    指定一个位置，将id跳转到此位置
    */
    void SkipTo(T pos)
    {
        CHNGuard<CHNRecMutex> guard(m_lock);
        m_currentId = pos;
    }
protected:
    T m_currentId;
    CHNRecMutex m_lock;
};

/**
*@brief      pthread_once工具类。多个线程运行同一个函数，要求函数只执行一次。
*            通常用于库初始化或线程安全的单例模式
*/
class CHNRunOnce :CHNNoCopyable
{
public:
    explicit CHNRunOnce()
    {
#if defined(_MSC_VER) && _MSC_VER < 1800
        m_ran = 0;
        m_event = NULL;
#endif
    }
    ~CHNRunOnce()
    {
#if defined(_MSC_VER) && _MSC_VER < 1800
        HNCloseHandle(m_event);
#endif
    }

    /**
    *@brief     开始运行函数，可以多线程运行，最终只有一个线程会执行，其他线程会等待这个线程执行完毕才返回
    *@param     callback    [in]    要执行的函数
    *@return    是否成功
    */
    bool RunOnce(HNTaskFunc callback)
    {
#if defined(_MSC_VER) && _MSC_VER < 1800
        if (m_ran)
        {
            return true;
        }
        HANDLE created_event = CreateEvent(NULL, 1, 0, NULL);
        if (created_event == 0)
        {
            return false;
        }
        HANDLE existing_event = InterlockedCompareExchangePointer(&m_event, created_event, NULL);
        if (existing_event == NULL)
        {
            if (callback)
                callback();
            DWORD result = SetEvent(created_event);
            assert(result);
            m_ran = 1;
        }
        else
        {
            HNCloseHandle(created_event);
            DWORD result = WaitForSingleObject(existing_event, INFINITE);
            assert(result == WAIT_OBJECT_0);
        }
        return true;
#else
        std::call_once(m_once_t, callback);
        return true;
#endif
    }

private:
#if defined(_MSC_VER) && _MSC_VER < 1800
    unsigned char  m_ran;
    HANDLE         m_event;
#else
    std::once_flag m_once_t;
#endif
};

/**
*@brief      条件变量类(支持xp但是需要vs2013及以上)。
*/
class CHNCondition :CHNNoCopyable
{
public:
    explicit CHNCondition(CHNRecMutex& Mutex)
        : m_Mutex(Mutex)
    {
#if defined(_MSC_VER) && _MSC_VER < 1800
        InitializeConditionVariable(&m_Cond);
#endif
    }

    ~CHNCondition(void)
    {
        NotifyAll();
    }

public:
    /**
    *@brief     等待nWaitMs毫秒
    *@param     nWaitMs    [in]    等待多少毫秒后超时。<0无限等待 0马上返回 >0为超时时间。
    *@return    成功等到返回true，等待超时或失败返回false
    */
    bool Wait(int nWaitMs = -1)
    {
#if defined(_MSC_VER) && _MSC_VER < 1800
        return !!SleepConditionVariableCS(&m_Cond, &(*m_Mutex), (nWaitMs < 0) ? INFINITE : nWaitMs);
#else
        if (nWaitMs < 0)
        {
            m_Cond.wait(m_Mutex);
            return true;
        }
        else
        {
            return (m_Cond.wait_for(m_Mutex, std::chrono::milliseconds(nWaitMs)) != std::cv_status::timeout);
        }
#endif
    }

    /**
    *@brief        唤醒某个等待的线程
    */
    void Notify(void)
    {
#if defined(_MSC_VER) && _MSC_VER < 1800
        WakeConditionVariable(&m_Cond);
#else
        m_Cond.notify_one();
#endif
    }

    /**
    *@brief        唤醒所有等待的线程
    */
    void NotifyAll(void)
    {
#if defined(_MSC_VER) && _MSC_VER < 1800
        WakeAllConditionVariable(&m_Cond);
#else
        m_Cond.notify_all();
#endif
    }

public:
#if defined(_MSC_VER) && _MSC_VER < 1800
    CONDITION_VARIABLE            m_Cond;
#else
    std::condition_variable_any    m_Cond;
#endif
    CHNRecMutex& m_Mutex;
};

/**
*@brief      JAVA中的CountDownLatch。是同步计数器(类)。
*            基于条件变量。演示了条件变量的唯一正确用法
*            递减型的同步工具。【增加至非0则阻塞；减少至0则通知。】
*/
class CHNCountDownLatch :CHNNoCopyable
{
public:
    /**
    *@brief     初始设定一个大于0的数值，所有线程可对其进行做减法。当减至0时，唤醒等待线程。
    *@param     count        [in]    初始值，建议大于0
    */
    explicit CHNCountDownLatch(int count)
        : m_Mutex()    //mutex要比cond先构造
        , m_Cond(m_Mutex)
        , m_Count(count)
    {
    }

    /**
    *@brief     等待条件成立(数值减为小于或等于0时条件成立)
    *          【！注意：尽量不要在有多个线程等待的时候使用超时功能】
    *@param     nWaitMs    [in]    等待多少毫秒后超时。<0无限等待 0马上返回 >0为超时时间。
    *@return    成功等到返回true，等待超时或失败返回false
    */
    bool Wait(int nWaitMs = -1)
    {
        CHNGuard<CHNRecMutex> lock(m_Mutex);
        while (m_Count > 0)
        {
            bool bWaited = m_Cond.Wait(nWaitMs);
            if (bWaited || nWaitMs < 0)
            {
                continue;
            }
            return bWaited;
        }
        return true;
    }

    /**
    *@brief        计数值减1
    */
    void CountDown()
    {
        CHNGuard<CHNRecMutex> lock(m_Mutex);
        --m_Count;
        if (m_Count <= 0)
        {
            m_Cond.NotifyAll();
        }
    }

    /**
    *@brief        计数值加1
    */
    void CountUp()
    {
        CHNGuard<CHNRecMutex> lock(m_Mutex);
        ++m_Count;
    }

    /**
    *@brief        计数值直接设置为一个数值
    *@param        count    [in]    要设置的数值
    */
    void SetCount(int count)
    {
        CHNGuard<CHNRecMutex> lock(m_Mutex);
        m_Count = count;
        if (m_Count <= 0)
        {
            m_Cond.NotifyAll();
        }
    }

    /**
    *@brief        获取当前计数值
    */
    int GetCount()
    {
        CHNGuard<CHNRecMutex> lock(m_Mutex);
        return m_Count;
    }

    /**
    *@brief        加锁
    */
    void lock()
    {
        m_Mutex.lock();
    }

    /**
    *@brief        加锁
    */
    void unlock()
    {
        m_Mutex.unlock();
    }

protected:
    //mutex和cond的声明顺序要和初始化顺序一致
    CHNRecMutex     m_Mutex;
    CHNCondition    m_Cond;
    int             m_Count;
};


/**
*@brief      和CountDownLatch相对。是同步计数器(类)。尽量用CHNCountUpLatch替代信号量。
*            基于条件变量。演示了条件变量的唯一正确用法
*            递增型的同步工具。【增加至非0则通知；减少至0则阻塞。】
*/
class CHNCountUpLatch :CHNNoCopyable
{
public:
    /**
    *@brief     初始设定一个值。当减至0时，等待
    *@param     count        [in]    初始值
    */
    explicit CHNCountUpLatch(int count)
        : m_Mutex()    //mutex要比cond先构造
        , m_Cond(m_Mutex)
        , m_Count(count)
    {
    }

    /**
    *@brief     等待条件成立(数值增加到>0时条件成立，等待成功返回)
    *          【！注意：尽量不要在有多个线程等待的时候使用超时功能】
    *@param     nWaitMs    [in]    等待多少毫秒后超时。<0无限等待 0马上返回 >0为超时时间。
    *@return    成功等到返回true，等待超时或失败返回false
    */
    bool Wait(int nWaitMs = -1)
    {
        CHNGuard<CHNRecMutex> lock(m_Mutex);
        while (m_Count <= 0)
        {
            bool bWaited = m_Cond.Wait(nWaitMs);
            if (bWaited || nWaitMs < 0)
            {
                continue;
            }
            return bWaited;
        }
        --m_Count;
        return true;
    }

    /**
    *@brief        计数值加1
    */
    void CountUp()
    {
        CHNGuard<CHNRecMutex> lock(m_Mutex);
        ++m_Count;
        if (m_Count > 0)
        {
            m_Cond.NotifyAll();
        }
    }

    /**
    *@brief        计数值直接设置为一个数值
    *@param        count    [in]    要设置的数值
    */
    void SetCount(int count)
    {
        CHNGuard<CHNRecMutex> lock(m_Mutex);
        m_Count = count;
        if (m_Count > 0)
        {
            m_Cond.NotifyAll();
        }
    }

    /**
    *@brief        获取当前计数值
    */
    int GetCount()
    {
        CHNGuard<CHNRecMutex> lock(m_Mutex);
        return m_Count;
    }

    /**
    *@brief        加锁
    */
    void lock()
    {
        m_Mutex.lock();
    }

    /**
    *@brief        加锁
    */
    void unlock()
    {
        m_Mutex.unlock();
    }

protected:
    //mutex和cond的声明顺序要和初始化顺序一致
    CHNRecMutex     m_Mutex;
    CHNCondition    m_Cond;
    int             m_Count;
};

/**
*@brief      JAVA中的BlockingQueue。这里是容量不限且线程安全的阻塞队列。
*            基于条件变量。演示了条件变量的唯一正确用法
*            递增型的同步工具。【减少至0则阻塞；增加至非0则通知。】
*/
template<typename T>
class CHNBlockingQueue : CHNNoCopyable
{
public:
    CHNBlockingQueue()
        : m_Mutex()    //mutex要比cond先构造
        , m_CondNotEmpty(m_Mutex)
        , m_Deque()
    {
    }

    /**
    *@brief      将元素放入阻塞队列，并通知等待线程
    */
    void Put(const T& x, bool bPushToFront = false)
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);
        if (bPushToFront)
            m_Deque.push_front(x);
        else
            m_Deque.push_back(x);
        m_CondNotEmpty.Notify();
    }

    /**
    *@brief      将元素放入阻塞队列，并通知等待线程
    */
    void Put(T&& x, bool bPushToFront = false)
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);
        if (bPushToFront)
            m_Deque.push_front(std::move(x));
        else
            m_Deque.push_back(std::move(x));
        m_CondNotEmpty.Notify();
    }

    /**
    *@brief      从阻塞队列中(从队头)拿取一个元素，如果队列为空，在此进行等待。
    */
    T Take()
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);

        //避免虚假唤醒
        while (m_Deque.empty())
        {
            m_CondNotEmpty.Wait();
        }
        assert(!m_Deque.empty());
        T front(std::move(m_Deque.front()));
        m_Deque.pop_front();
        return std::move(front);
    }

    /**
    *@brief     从阻塞队列中(从队头)拿取一个元素，支持超时设置。
    *@param     nWaitMs    [in]    等待多少毫秒后超时。<0无限等待 0马上返回 >0为超时时间。
    *@param     element    [out]    如果成功，返回拿到的数据。否则无意义
    *@return    是否成功拿到数据
    */
    bool Take(int nWaitMs, T& element)
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);

        //避免虚假唤醒
        while (m_Deque.empty())
        {
            bool bWaited = m_CondNotEmpty.Wait(nWaitMs);
            if (bWaited || nWaitMs < 0)
            {
                continue;
            }
            return false;
        }
        assert(!m_Deque.empty());
        T front(std::move(m_Deque.front()));
        m_Deque.pop_front();
        element = std::move(front);
        return true;
    }

    /**
    *@brief        取阻塞队列中的元素数量
    */
    size_t Size()
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);
        return m_Deque.size();
    }

    /**
    *@brief        清空
    */
    void Clear()
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);
        m_Deque.clear();
    }

    /**
    *@brief        判断是否为空
    */
    bool Empty()
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);
        return m_Deque.empty();
    }

protected:
    //mutex和cond的声明顺序要和初始化顺序一致
    CHNRecMutex     m_Mutex;
    CHNCondition    m_CondNotEmpty;
    std::deque<T>   m_Deque;
};

/**
*@brief      线程安全的容器，可方便地从前后端插入和拿取元素
*            如果你想实现insert/remove等更多功能，继承此类即可
*@param        T    [in]    元素类型
*@param        C    [in]    容器类型
*/
template<typename T, typename C = std::deque<T> >
class CHNSafeQueue : CHNNoCopyable
{
public:
    /**
    *@brief     将元素放入队列尾部
    */
    void PushBack(const T& x)
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);
        m_Deque.push_back(x);
    }
    /**
    *@brief     将元素放入队列尾部
    */
    void PushBack(T&& x)
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);
        m_Deque.push_back(std::move(x));
    }
    /**
    *@brief     将元素放入队列尾部
    */
    void PushFront(const T& x)
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);
        m_Deque.push_front(x);
    }
    /**
    *@brief     将元素放入队列尾部
    */
    void PushFront(T&& x)
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);
        m_Deque.push_front(std::move(x));
    }

    /**
    *@brief     从队列头部中拿取一个元素。
    *@param     element    [out]    如果成功，返回拿到的数据。否则无意义
    *@return    是否成功拿到数据
    */
    bool TakeFront(T& element)
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);
        if (m_Deque.empty())
        {
            return false;
        }
        T front(std::move(m_Deque.front()));
        m_Deque.pop_front();
        element = std::move(front);
        return true;
    }
    /**
    *@brief     从队列尾部中拿取一个元素。
    *@param     element    [out]    如果成功，返回拿到的数据。否则无意义
    *@return    是否成功拿到数据
    */
    bool TakeBack(T& element)
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);
        if (m_Deque.empty())
        {
            return false;
        }
        T back(std::move(m_Deque.back()));
        m_Deque.pop_back();
        element = std::move(back);
        return true;
    }

    /**
    *@brief        取队列中的元素数量
    */
    size_t Size()
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);
        return m_Deque.size();
    }
    /**
    *@brief        清空
    */
    void Clear()
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);
        m_Deque.clear();
    }
    /**
    *@brief        判断是否为空
    */
    bool Empty()
    {
        CHNGuard<CHNRecMutex> guard(m_Mutex);
        return m_Deque.empty();
    }

protected:
    CHNRecMutex  m_Mutex;
    C            m_Deque;
};


/**
*@brief      用于进程间的互斥量。
*              windows:使用Mutex实现，因为mutex可以很好地解决进程退出后的信号释放问题(而Semaphore/Event不行)
*              linux:  使用flock实现，因为linux下的mutex只有在共享内存下才能跨进程使用，行为和windows的mutex不太一样不好统一；信号量又不能解决进程退出后的信号释放问题
*              android:使用flock实现，未测试，可能有问题。android没有/dev/shm 使用时注意
*/
class CHNNamedMutex :CHNNoCopyable
{
public:
    /**
    *@brief     构造mutex，通过一个名称来在多进程中唯一确定互斥量。创建此内核对象需要一定耗时。
    *@param     szName        [in]    互斥量名字，传入空使用默认名字
    *           bAutoPath     [in]    当为true时，szName只需要传入名称，构造对象时会自动放入合适的路径：
    *                                【windows】根据权限自动选择放在Global\\区域（所有用户共用，但需要管理员权限）或非Global\\区域
    *                                【 linux 】会将其放在/dev/shm。因为有名信号量也存储在这里 ，这个目录是777权限的
    *                                当为false时，需要用户的szName自己传入合适的名称/路径。
    */
    explicit CHNNamedMutex(const char* szName, bool bAutoPath = true)
    {
        std::string strMutexName;
#if defined(HNOS_WIN)
        m_hMutex = NULL;
        if (szName != NULL && szName[0] != 0)
        {
            if (bAutoPath)
                strMutexName = "Global\\";
            strMutexName += szName;
        }
        else
        {
            strMutexName = "Global\\DefaultCHNNamedMutex";
        }

        //TODO 如果是global，需要SE_CREATE_GLOBAL_NAME权限
        m_hMutex = CreateMutexA(NULL, FALSE /*是否创建时即lock。name存在且互斥量已经创建时时会忽略这个参数*/, strMutexName.c_str());
        if (m_hMutex == NULL)
        {
            DWORD dwErrCode = HNGetLastError();
            if (bAutoPath && ERROR_ACCESS_DENIED == dwErrCode)
            {
                //Global没有权限，自动尝试非Global
                auto notGlobal = strMutexName.c_str() + strlen("Global\\");
                m_hMutex = CreateMutexA(NULL, FALSE, notGlobal);
                if (m_hMutex == NULL)
                {
                    throw ("CHNNamedMutex::CreateMutexA fail.err");
                }
            }
            else
            {
                throw ("CHNNamedMutex::CreateMutexA Global fail.err");
            }
        }
#else
        m_fd = -1;
        if (szName != NULL && szName[0] != 0)
        {
            if (bAutoPath)
                strMutexName = "/dev/shm/";
            strMutexName += szName;
        }
        else
        {
            strMutexName = "/dev/shm/DefaultCHNNamedMutex";
        }
        ::umask(0);
        m_fd = ::open(strMutexName.c_str(), O_CREAT | O_RDWR, S_IREAD | S_IWRITE | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        if (m_fd < 0)
        {
            throw ("CHNNamedMutex::open fail.err");
        }
#endif
    }
    ~CHNNamedMutex()
    {
#if defined(HNOS_WIN)
        HNCloseHandle(m_hMutex);
#else
        if (m_fd >= 0)
            close(m_fd);
#endif
    }

    /**
    *@brief     加锁操作，允许线程内重复加锁
    *@param     nWaitMs    [in]    【windows】等待超时时间(毫秒)。<0无限等待 0马上返回 >0为超时时间。
    *                              【 linux 】等待超时时间(毫秒)。<0无限等待 >= 马上返回【文件锁不支持超时时间】
    *@return    是否成功获取到锁
    */
    bool lock(int nWaitMs = -1)
    {
#if defined(HNOS_WIN)
        if (nWaitMs == 0) nWaitMs = 1;
        DWORD dwRet = WaitForSingleObject(m_hMutex, nWaitMs<0 ? INFINITE : nWaitMs);
        if (WAIT_OBJECT_0 == dwRet)
        {
            //成功等到信号
            return true;
        }
        else if (WAIT_TIMEOUT == dwRet)
        {
            //其他进程正在处理
            return false;
        }
        else if (WAIT_ABANDONED == dwRet)
        {
            //其他进程被意外结束而没有释放信号，这个时候可以直接当成成功
            return true;
        }
        else
        {
            return false;
        }
#else
        struct flock f_lock;
        f_lock.l_type = F_WRLCK;
        f_lock.l_whence = SEEK_SET;
        f_lock.l_start = 0;
        f_lock.l_len = 0; //0表示整个文件加锁
        if (nWaitMs < 0)
        {
            //无限等待
            int nRet = ::fcntl(m_fd, F_SETLKW, &f_lock);
            if (nRet == -1)
            {
                return false;
            }
            return true;
        }
        else
        {
            //尝试等待
            int nRet = ::fcntl(m_fd, F_SETLK, &f_lock);
            if (nRet == -1)
            {
                if (HNGetLastError() != EAGAIN && HNGetLastError() != EACCES)
                {
                    return false;
                }
                return false;
            }
            return true;
        }
#endif
    }

    /**
    *@brief     解锁操作
    *@return    是否成功
    */
    bool unlock()
    {
#if defined(HNOS_WIN)
        BOOL bRet = ReleaseMutex(m_hMutex);
        if (FALSE == bRet)
        {
            return false;
        }
#else
        struct flock f_lock;
        f_lock.l_type = F_UNLCK;
        f_lock.l_whence = SEEK_SET;
        f_lock.l_start = 0;
        f_lock.l_len = 0; //0表示整个文件加锁
        int nRet = fcntl(m_fd, F_SETLKW, &f_lock);
        if (nRet == -1)
        {
            return false;
        }
#endif
        return true;
    }

private:
#if defined(HNOS_WIN)
    HANDLE    m_hMutex;
#else
    int       m_fd;
#endif
};

#if defined(HNOS_WIN)
	#if defined(WINVER) && (WINVER < 0x0500)
	inline void  __stdcall ahn_function(ULONG) {}
	#else
	inline void  __stdcall ahn_function(ULONG_PTR) {}
	#endif
#endif

/**
*@brief      线程对象，你可以用线程对象作为你的类的成员，
*            然后在构造线程对象时用std::bind绑定你的一个类成员函数作为func来使其在线程中执行。
*            注意：    持有线程对象的类最好不要构造成静态实例或全局实例，
*                     因为该实例析构时可能导致线程退出时候等不到信号
*/
class CHNThreadHandle : CHNNoCopyable
{
public:
    /**
    *@brief     创建线程对象，必须传入线程执行体和可选的线程名字
    */
    explicit CHNThreadHandle(HNTaskFunc func, const std::string &name = "")
        :m_ThreadFunc(func)
        , m_strName(name)
        , m_bRunning(false)
    {
#if defined(_MSC_VER) && _MSC_VER < 1800
        thread_handle_ = NULL;
        exit_event_    = NULL;
        entry_event_   = NULL;
#endif
    }
    ~CHNThreadHandle()
    {
        //WaitExit();     //tcice add 2021/11/2
#if defined(_MSC_VER) && _MSC_VER < 1800
        HNCloseHandle(thread_handle_);
#endif
    }

    /**
    *@brief     启动线程(Start函数不要多次调用)
    *           线程真正启动后才返回
    */
    CHNResult<> Start()
    {
        CHNResult<> result;
        if (m_bRunning)
        {
            return result;
        }

        //创建线程
        //m_condStarted.SetCount(1);
        //m_condStopped.SetCount(1);
#if defined(_MSC_VER) && _MSC_VER < 1800
        entry_event_ = ::CreateEventA(0, true, false, 0);
        if (!entry_event_) {
            return result.SetSystemFail();
        }
        exit_event_ = ::CreateEventA(0, true, false, 0);
        if (!entry_event_) {
            return result.SetSystemFail();
        }
        uintptr_t ret = _beginthreadex(NULL, 0, HNThreadStartAddr, this, 0, NULL);
        if (ret == 0 || ret == -1)
        {
            HNCloseHandle(entry_event_);
            HNCloseHandle(exit_event_);
            return result.SetSystemFail();
        }
        thread_handle_ = reinterpret_cast<HANDLE>(ret);
        if (entry_event_)
        {
            ::WaitForSingleObject(entry_event_, INFINITE);
            HNCloseHandle(entry_event_);
        }
#else
        pthread_.reset(new std::thread(HNThreadStartAddr, this));
        //hThread.detach();
#endif
        return result;
    }

    /**
    *@brief     等待线程退出，或者超时
    *@param     nWaitMs    [in]    等待多少毫秒后超时。<0无限等待 0马上返回 >0为超时时间。
    *@return    无
    */
    void WaitExit(int nWaitMs = -1)
    {
        //if (!m_bRunning)
        //    return;
#if defined(_MSC_VER) && _MSC_VER < 1800
        //m_bRunning = false; 
        HANDLE handles[2] = { exit_event_, thread_handle_ };
        ::WaitForMultipleObjects(2, handles, FALSE, INFINITE);
        HNCloseHandle(exit_event_);
        ::QueueUserAHN(ahn_function, thread_handle_, 0);
        ::WaitForSingleObject(thread_handle_, INFINITE);
#else
        //m_bRunning = false;
        if(pthread_)
            pthread_->join();
#endif
        //m_condStopped.Wait(nWaitMs);
    }

    /**
    *@brief        线程入口点
    */
#if defined(_MSC_VER) && _MSC_VER < 1800
    static unsigned int __stdcall    HNThreadStartAddr(void* obj)
#else
    static void    HNThreadStartAddr(void* obj)
#endif
    {
        CHNThreadHandle* lpThis = static_cast<CHNThreadHandle*>(obj);
        if (lpThis)
        {
#if defined(_MSC_VER) && _MSC_VER < 1800
            ::SetEvent(lpThis->entry_event_);
#endif
            lpThis->m_bRunning = true;

            if (lpThis->m_ThreadFunc)
                lpThis->m_ThreadFunc();

            lpThis->m_bRunning = false;
#if defined(_MSC_VER) && _MSC_VER < 1800
            ::SetEvent(lpThis->exit_event_);
#endif
        }
#if defined(_MSC_VER) && _MSC_VER < 1800
        ::SleepEx(INFINITE, TRUE);
        return 0;
#else
        return;
#endif
    }

protected:
    HNTaskFunc           m_ThreadFunc;
#if defined(_MSC_VER) && _MSC_VER < 1800
    volatile HANDLE thread_handle_;
    HANDLE exit_event_;
    HANDLE entry_event_;
#else
    std::unique_ptr<std::thread>             pthread_;
#endif

public:
    std::string m_strName;
    bool        m_bRunning;
};

/**
*@brief      线程池。对线程数量最大值有限制；对任务队列长度无限制
*            如果任务队列长度大于线程数量，剩余任务将会在队列中等待
*/
class CHNThreadPool : CHNNoCopyable
{
public:
    explicit CHNThreadPool(unsigned int MaxThreadCount = HN_MAXSTACK)
        : m_ThreadPoolMutex()
        , m_TasksNotEmpty(m_ThreadPoolMutex)
        , m_ThreadPoolRunning(true)
        , m_MaxThreadCount(MaxThreadCount == 0 ? HN_MAXSTACK : MaxThreadCount)
        , m_IdleThreadCount(0)
    {}
    ~CHNThreadPool()
    {
        NotifyAndWaitExit();
    }

    /**
    *@brief     得到默认的线程池，线程池默认启动
    *@return    线程池单例对象
    */
    static CHNThreadPool& Default()
    {
        static CHNThreadPool o;
        return o;
    }

    /**
    *@brief     将任务增加至线程池。
    *           线程池退出后，不能再添加任务
    *@param     task    [in]    任务。
    *@return    是否成功
    */
    CHNResult<> AddTask(HNTaskFunc task)
    {
        //增加任务并通知处理。通知之后，及时解锁
        CHNResult<> result;
        {
            CHNGuard<CHNRecMutex> guard1(m_ThreadPoolMutex);
            if (!IsRunning())
            {
                return result.SetFail("thread pool is stoped");
            }
            m_Tasks.push_back(std::move(task));
            m_TasksNotEmpty.Notify();

            //如果线程数量已经达到上限，不再创建线程
            if (m_Threads.size() >= m_MaxThreadCount)
                return result;

            //如果存在空闲线程，不再创建线程
            if (m_IdleThreadCount > 0)
                return result;
        }

        //创建并启动线程
        CHNGuard<CHNRecMutex> guard2(m_ThreadPoolMutex);
        CHNThreadHandle* th = new CHNThreadHandle(std::bind(&CHNThreadPool::ProcessTask, this), "");
        m_Threads.emplace_back(th);
        return th->Start();
    }

    /**
    *@brief     通知线程池退出并等待其退出，超过超时时间后，强制杀死线程(暂不支持)
    *           调用此函数后，线程池中止，不能再调用AddTask添加任务
    *           线程池中止后，队列中剩余的任务(如果有)将会丢失。
    *@param     exitfunc    [in]    通知结束之后额外的处理动作，一般用于任务本身的阻塞解除通知。
    *@param     nWaitMs     [in]    每个线程等待多少毫秒后超时。<0无限等待 0马上返回 >0为超时时间。
    *@return    无
    */
    void NotifyAndWaitExit(std::function<void(void)> exitfunc = [](){}, int nWaitMs = -1)
    {
        if (!m_ThreadPoolRunning)
            return;

        {
            CHNGuard<CHNRecMutex> guard(m_ThreadPoolMutex);
            m_ThreadPoolRunning = false;
            m_TasksNotEmpty.NotifyAll();
        }
        if (exitfunc)
            exitfunc();
        for (auto it = m_Threads.begin(); it != m_Threads.end(); ++it)
        {
            (*it)->WaitExit(nWaitMs);
        }
        m_Threads.clear();
        m_IdleThreadCount = 0;
    }

    /**
    *@brief     判断线程池是否还在运行
    *           可以给任务函数在循环中使用。
    *@return    是否运行
    */
    bool IsRunning()
    {
        return m_ThreadPoolRunning;
    }

    /**
    *@brief     取当前任务队列中的剩余任务数
    *           如果剩余任务数过高，请谨慎添加任务。
    *@return    数量
    */
    size_t TasksSize()
    {
        CHNGuard<CHNRecMutex> guard(m_ThreadPoolMutex);
        return m_Tasks.size();
    }

    /**
    *@brief     取当前线程数
    *           如果当前线程数过高，请谨慎添加任务。
    *@return    数量
    */
    size_t ThreadsSize()
    {
        CHNGuard<CHNRecMutex> guard(m_ThreadPoolMutex);
        return m_Threads.size();
    }

    /**
    *@brief     取当前空闲线程数
    *@return    数量
    */
    size_t IdleThreadsSize()
    {
        CHNGuard<CHNRecMutex> guard(m_ThreadPoolMutex);
        return m_IdleThreadCount;
    }

private:
    /**
    *@brief     线程执行体，本质上是从一个阻塞队列里面取任务执行
    *@return    无
    */
    void ProcessTask()
    {
        while (m_ThreadPoolRunning)
        {
            HNTaskFunc task;
            {
                CHNGuard<CHNRecMutex> guard(m_ThreadPoolMutex);
                m_IdleThreadCount++;
                while (m_Tasks.empty() && m_ThreadPoolRunning)
                {
                    m_TasksNotEmpty.Wait();
                }
                if (m_IdleThreadCount > 0)
                    m_IdleThreadCount--;

                if (m_Tasks.empty())
                {
                    continue;
                }
                task = m_Tasks.front();
                m_Tasks.pop_front();
            }
            if (task)
            {
                task();
            }
        }
    }
private:
    CHNRecMutex  m_ThreadPoolMutex;
    CHNCondition m_TasksNotEmpty;
    bool         m_ThreadPoolRunning;
    unsigned int m_MaxThreadCount;
    unsigned int m_IdleThreadCount;
    std::vector<std::unique_ptr<CHNThreadHandle> > m_Threads;
    std::deque <HNTaskFunc> m_Tasks;
};

/**
*@brief     状态机类
*           必须以智能指针std::shared_ptr<CHNStateMachine>创建对象
*           这里用到了异常，但只有出现编码错误（如switch到一个不存在的状态）才会抛异常
*           也就是说，如果你肯定你的编码没有错误，就可以不必catch异常
*/
class CHNStateMachine
    : CHNNoCopyable
    , public std::enable_shared_from_this<CHNStateMachine>
{
public:
    /**
    *@brief     状态函数的定义。
    *           1.当函数进入时代表进入此状态；当函数退出时代表退出此状态
    *           2.你可以调用状态机的AddState添加任意个数的状态（>0）
    *           3.状态函数中，你可以使用machine.WaitEvent()判断是否收到了状态切换事件，如果收到了，你可以：
    *               a.最常见做法：马上return，退出你这个状态（至于后续会调度到什么状态由状态机控制）。
    *               b.如果这个时候你还有紧急任务没有完成，你可以先完成紧急任务（时间不宜过长），然后再return
    *               c.如果你拒绝切换到下一个状态，可以直接忽略，继续写后面的逻辑，然后在必要时再次调用WaitEvent
    */
    typedef std::function<void(CHNStateMachine&)> HNStateFunc;

public:
    explicit CHNStateMachine()
        : m_bRunning(false)
        , m_currentState(-1)
        , m_nextState(-1)
        , m_condStopped(0)
    {}
    ~CHNStateMachine()
    {}

    /**
    *@brief     状态切换的合法性验证，如果你需要做合法性验证，需要自己继承状态机类并重写这个方法
    *@param     from    [in]    从哪个状态切换
    *@param     to      [in]    切换到哪个状态
    *@return    是否允许
    */
    virtual bool Validate(int from, int to)
    {
        return true;
    }

    /**
    *@brief     添加一个状态到状态机，需要在启动状态机之前调用
    *@param     state   [in]    状态id，一个id只能对应一个状态函数
    *@param     func    [in]    状态函数
    *@return    无
    */
    void AddState(int state, HNStateFunc func)
    {
        if (m_bRunning)
        {
            //状态机已经启动了，不能再添加状态
            throw ("state is running,can NOT add state");
        }
        if (m_states.find(state) != m_states.end())
        {
            //状态已经存在了，不能重复添加
            throw ("state exists, can NOT add this state.");
        }
        m_states.insert(std::make_pair(state, std::move(func)));
    }

    /**
    *@brief     跳转到某个状态,这里只是把切换事件放入队列，具体切换在状态函数中做
    *@param     state   [in]    状态id
    *@param     force   [in]    是否强制跳转 是：将命令放入队列，可以保证它一定会被执行，一般用于外部进行切换
    *                                      否：当前队列中如果有其他切换请求，则放弃，一般用于状态函数中的切换
    *@return    跳转是否成功
    */
    bool SwitchTo(int state, bool force = true)
    {
        if (!m_bRunning)
        {
            //状态机还没有启动或已经退出，不能切换状态
            return false;
        }
        if (m_states.find(state) == m_states.end())
        {
            //没有这个状态
            throw ("state NOT exists, can NOT switch to this state.");
        }
        if (m_currentState == state)
        {
            //当前已经是这个状态了
            return true;
        }
        if (!this->Validate(m_currentState, state))
        {
            //不合法
            return false;
        }
        if (force)
        {
            //强制放入命令
            m_events.Put(state);
        }
        else
        {
            //不强制，要看命令队列是否为空
            if (m_events.Empty())
            {
                m_events.Put(state);
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    /**
    *@brief     启动状态机，并使其在线程中运行
    *@param     state   [in]    初始状态
    */
    void Start(int state)
    {
        if (m_bRunning)
        {
            return;
        }
        if (m_states.find(state) == m_states.end())
        {
            //没有这个状态
            throw ("state NOT exists, can NOT switch to this state.");
        }
        m_bRunning = true;

        std::weak_ptr<CHNStateMachine> weakPtr = (std::weak_ptr<CHNStateMachine>)this->shared_from_this();
        CHNThreadPool::Default().AddTask([weakPtr, state]()
        {
            auto shardPtr = weakPtr.lock();
            if (!shardPtr)
            {
                return;
            }
            shardPtr->RunForever(state);
        });
    }

    /**
    *@brief     等待事件，一般在状态函数中用
    *@param     nWaitMs   [in]    等待的超时时间。<0代表无限等待，=0马上返回，>0代表等待nWaitMs毫秒
    *@return    是否等到了事件
    */
    bool WaitEvent(int nWaitMs = -1)
    {
        //状态机退出了
        if (!m_bRunning)
        {
            return true;
        }
        int nNextState = -1;
        bool bSucc = m_events.Take(nWaitMs, nNextState);
        if (bSucc)
        {
            m_nextState = nNextState;
        }
        return bSucc;
    }

    /**
    *@brief     获取当前状态
    *@return    当前状态
    */
    int GetCurrentState() const
    {
        return m_currentState;
    }

    /**
    *@brief     获取即将要进入的下一个状态
    *           一般在状态函数中使用，在WaitEvent返回真后，如果你想知道状态机即将把你调度到哪个状态，你可以使用这个函数
    *@return    下一个状态
    */
    int GetNextState() const
    {
        return m_nextState;
    }

    /**
    *@brief     等待状态机退出，或者超时
    *@param     nWaitMs    [in]    等待多少毫秒后超时。<0无限等待 0马上返回 >0为超时时间。
    *@return    无
    */
    void WaitExit(int nWaitMs = -1)
    {
        if (!m_bRunning)
            return;
        m_bRunning = false;
        m_events.Put(-1);
        m_condStopped.Wait(nWaitMs);
    }

public:
    /**
    *@brief     状态机运行函数
    *@param     state   [in]    初始状态
    */
    void RunForever(int state)
    {
        //进入初始状态
        //printf("##################[%d]==>[%d]\n",m_currentState, state);
        m_condStopped.SetCount(1);
        m_currentState = state;
        m_nextState = state;
        m_states[state](*(this));

        while (m_bRunning)
        {
            if (m_nextState == m_currentState)
            {
                m_nextState = m_events.Take();
            }
            //printf("##################[%d]==>[%d]\n",m_currentState, m_nextState);
            m_currentState = m_nextState;
            if (!m_bRunning)
                break;
            auto func = m_states[m_nextState];
            if (func)
            {
                func(*this);
            }
        }
        m_condStopped.CountDown();
        //printf("##################[exit!]\n");
    }

protected:
    bool m_bRunning;        //状态机是否运行中
    int  m_currentState;    //当前状态id
    int  m_nextState;       //即将要切换到下一个状态id
    std::unordered_map<int, HNStateFunc> m_states;  //状态id,状态函数的键值对列表
    CHNBlockingQueue<int>   m_events;              //事件队列
    CHNCountDownLatch       m_condStopped;         //退出条件 
};

#endif // __HELLONET_M_CORE_S_THREAD__