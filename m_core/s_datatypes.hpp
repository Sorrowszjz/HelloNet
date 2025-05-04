#ifndef __HELLO_NET_M_CORE_S_DATA_TYPES__
#define __HELLO_NET_M_CORE_S_DATA_TYPES__
#include "s_base.hpp"

/**
*@brief      可变数据类型
*/
template <typename Ch> 
class CHNGenericVariant
{
public:
    ///////////////////////////////////////////////////////
    //类型
    enum HNVarType 
    {
        eNullType = 0,      //!< null
        eFalseType = 1,     //!< false
        eTrueType = 2,      //!< true
        eStringType = 3,    //!< string
        eIntType = 4,       //!< int/int64
        eDoubleType = 5     //!< float
    };

    ///////////////////////////////////////////////////////
    //数据结构定义
    HNVarType   m_type;
    union 
    {
        struct
        {
            size_t length;
            const Ch* str;
        } s;
        int64_t  i;
        double   d;
    }m_data;

public:
    ///////////////////////////////////////////////////////
    //构造
    CHNGenericVariant()             {m_type = eNullType;}
    CHNGenericVariant(bool b)       {m_type = (b?eTrueType:eFalseType);}
    CHNGenericVariant(int num)      {m_type = eIntType;     m_data.i = (int64_t)num;}
    CHNGenericVariant(unsigned num) {m_type = eIntType;     m_data.i = (int64_t)num;}
    CHNGenericVariant(int64_t num)  {m_type = eIntType;     m_data.i = (int64_t)num;}
    CHNGenericVariant(uint64_t num) {m_type = eIntType;     m_data.i = (int64_t)num;}
    CHNGenericVariant(float num)    {m_type = eDoubleType;  m_data.d = (double)num;}
    CHNGenericVariant(double num)   {m_type = eDoubleType;  m_data.d = (double)num;}
    CHNGenericVariant(const Ch* str, size_t len)
    {
        if(str == nullptr)
        {
            m_type = eNullType;
            return;
        }
        m_type = eStringType;
        m_data.s.length = len;
        m_data.s.str = new Ch[m_data.s.length+1];
        memcpy((void*)m_data.s.str,(void*)str,m_data.s.length);
        memcpy((void*)((char*)m_data.s.str+m_data.s.length),(void*)"\0",1);
    }
    CHNGenericVariant(const Ch* str)
    {
        //placement new
        new (this)CHNGenericVariant(str,str?strlen(str):0);
    }
    CHNGenericVariant(const std::string& str)
    {
        //placement new
        new (this)CHNGenericVariant(str.data(),str.length());
    }
    CHNGenericVariant(const CHNGenericVariant& rhs)
    {
        if (this == &rhs)
            return ;
        switch(rhs.m_type)
        {
        case eNullType:
        case eFalseType:
        case eTrueType:
            break;
        case eStringType:
            m_data.s.str = new Ch[rhs.m_data.s.length+1];
            m_data.s.length = rhs.m_data.s.length;
            memcpy((void*)m_data.s.str,(void*)rhs.m_data.s.str,m_data.s.length);
            memcpy((void*)((char*)m_data.s.str+m_data.s.length),(void*)"\0",1);
            break;
        case eIntType:
            m_data.i = rhs.m_data.i;
            break;
        case eDoubleType:
            m_data.d = rhs.m_data.d;
            break;
        default:
            throw ("unknown type operator=.");
        }
        m_type = rhs.m_type;
    }
    CHNGenericVariant(CHNGenericVariant && rhs)
    {
        if (this == &rhs)
            return ;
        switch(rhs.m_type)
        {
        case eNullType:
        case eFalseType:
        case eTrueType:
            break;
        case eStringType:
            m_data.s.str = rhs.m_data.s.str;
            m_data.s.length = rhs.m_data.s.length;
            rhs.m_data.s.str = NULL;
            rhs.m_data.s.length = 0;
            break;
        case eIntType:
            m_data.i = rhs.m_data.i;
            break;
        case eDoubleType:
            m_data.d = rhs.m_data.d;
            break;
        default:
            throw ("unknown type operator=.");
        }
        m_type = rhs.m_type;
        rhs.m_type = eNullType;
    }
    ///////////////////////////////////////////////////////
    //赋值
    CHNGenericVariant& operator=(bool b)
    {if(m_type == eStringType)HNDeleteArr(m_data.s.str);m_type = b?eTrueType:eFalseType;return *this;}
    CHNGenericVariant& operator=(int num)
    {if(m_type == eStringType)HNDeleteArr(m_data.s.str);m_type = eIntType;  m_data.i = (int64_t)num;    return *this;}
    CHNGenericVariant& operator=(unsigned num)
    {if(m_type == eStringType)HNDeleteArr(m_data.s.str);m_type = eIntType;  m_data.i = (int64_t)num;    return *this;}
    CHNGenericVariant& operator=(int64_t num)
    {if(m_type == eStringType)HNDeleteArr(m_data.s.str);m_type = eIntType;  m_data.i = (int64_t)num;    return *this;}
    CHNGenericVariant& operator=(uint64_t num)
    {if(m_type == eStringType)HNDeleteArr(m_data.s.str);m_type = eIntType;  m_data.i = (int64_t)num;    return *this;}
    CHNGenericVariant& operator=(float num)
    {if(m_type == eStringType)HNDeleteArr(m_data.s.str);m_type = eDoubleType;   m_data.d = (double)num; return *this;}
    CHNGenericVariant& operator=(double num)
    {if(m_type == eStringType)HNDeleteArr(m_data.s.str);m_type = eDoubleType;   m_data.d = (double)num; return *this;}
    CHNGenericVariant& operator=(const Ch* str)
    {
        if(str == nullptr)
        {
            if(m_type == eStringType)HNDeleteArr(m_data.s.str);
            m_type = eNullType;
            return *this;
        }
        //如果原来就是string而且长度也够，就不需要重复new空间
        size_t len = (str?strlen(str):0);
        if(m_type != eStringType || len+1 > m_data.s.length)
        {
            if(m_type == eStringType)
                HNDeleteArr(m_data.s.str);
            m_data.s.str = new Ch[len+1];
        }
        m_type = eStringType;
        m_data.s.length = len;
        memcpy((void*)m_data.s.str,(void*)str,m_data.s.length);
        memcpy((void*)((char*)m_data.s.str+m_data.s.length),(void*)"\0",1);
        return *this;
    }
    CHNGenericVariant& operator=(const std::string& str)
    {
        //如果原来就是string而且长度也够，就不需要重复new空间
        size_t len = str.length();
        if(m_type != eStringType || len+1 > m_data.s.length)
        {
            if(m_type == eStringType)
                HNDeleteArr(m_data.s.str);
            m_data.s.str = new Ch[len+1];
        }
        m_type = eStringType;
        m_data.s.length = len;
        memcpy((void*)m_data.s.str,(void*)str.data(),m_data.s.length);
        memcpy((void*)((char*)m_data.s.str+m_data.s.length),(void*)"\0",1);
        return *this;
    }
    CHNGenericVariant& operator=(CHNGenericVariant& rhs)
    {
        if (this == &rhs)
            return *this;
        switch(rhs.m_type)
        {
        case eNullType:
        case eFalseType:
        case eTrueType:
            if(m_type == eStringType)HNDeleteArr(m_data.s.str);
            break;
        case eStringType:
            if(m_type != eStringType || rhs.m_data.s.length+1 > m_data.s.length)
            {
                if(m_type == eStringType)
                    HNDeleteArr(m_data.s.str);
                m_data.s.str = new Ch[rhs.m_data.s.length+1];
            }
            m_data.s.length = rhs.m_data.s.length;
            memcpy((void*)m_data.s.str,(void*)rhs.m_data.s.str,m_data.s.length);
            memcpy((void*)((char*)m_data.s.str+m_data.s.length),(void*)"\0",1);
            break;
        case eIntType:
            if(m_type == eStringType)HNDeleteArr(m_data.s.str);
            m_data.i = rhs.m_data.i;
            break;
        case eDoubleType:
            if(m_type == eStringType)HNDeleteArr(m_data.s.str);
            m_data.d = rhs.m_data.d;
            break;
        default:
            throw ("unknown type operator=.");
        }
        m_type = rhs.m_type;
        return *this;
    }
    CHNGenericVariant& operator=(CHNGenericVariant &&rhs)
    {
        if (this == &rhs)
            return *this;
        if(m_type == eStringType)HNDeleteArr(m_data.s.str);
        switch(rhs.m_type)
        {
        case eNullType:
        case eFalseType:
        case eTrueType:
            break;
        case eStringType:
            m_data.s.str = rhs.m_data.s.str;
            m_data.s.length = rhs.m_data.s.length;
            rhs.m_data.s.str = NULL;
            rhs.m_data.s.length = 0;
            break;
        case eIntType:
            m_data.i = rhs.m_data.i;
            break;
        case eDoubleType:
            m_data.d = rhs.m_data.d;
            break;
        default:
            throw ("unknown type operator=.");
        }
        m_type = rhs.m_type;
        rhs.m_type = eNullType;
        return *this;
    }
    ///////////////////////////////////////////////////////
    //析构
    ~CHNGenericVariant()
    {
        if(m_type == eStringType)
            HNDeleteArr(m_data.s.str);
    }

public:
    ///////////////////////////////////////////////////////
    //根据类型获取数据，一定要确保类型正确（整数、浮点、布尔、字符串）。类型不正确会抛出异常
    operator bool()     {if(m_type !=  eTrueType && m_type !=  eFalseType) throw ("type err"); return m_type == eTrueType; }
    operator int()      {if(m_type !=  eIntType)throw ("type err");   return (int)m_data.i;       }
    operator unsigned() {if(m_type !=  eIntType)throw ("type err");   return (unsigned)m_data.i;  }
    operator int64_t()  {if(m_type !=  eIntType)throw ("type err");   return (int64_t)m_data.i;   }
    operator uint64_t() {if(m_type !=  eIntType)throw ("type err");   return (uint64_t)m_data.i;  }
    operator float()    {if(m_type !=  eDoubleType)throw ("type err");return (float)m_data.d;     }
    operator double()   {if(m_type !=  eDoubleType)throw ("type err");return (double)m_data.d;    }
    operator char*()        {if(m_type !=  eStringType)throw ("type err");  return (char*)m_data.s.str;       }
    operator const char*()  {if(m_type !=  eStringType)throw ("type err");  return (const char*)m_data.s.str; }
    operator std::string()  {if(m_type !=  eStringType)throw ("type err");  return std::string(m_data.s.str,m_data.s.length); }

    ///////////////////////////////////////////////////////
    //尽可能地安全转化为需要的类型，如果不能安全转换，会抛出异常
    //template<typename T> T GetAs(){throw ("can not support this type.");return nullptr;}
    //template<>  int      GetAs<int>()      {return (int)m_data.i;}
    //template<>  unsigned GetAs<unsigned>() {return (unsigned)m_data.i;}
    //template<>  bool     GetAs<bool>()     {return m_type == eTrueType ;}


};

//! 可变Value
typedef CHNGenericVariant<char> CHNValue;

/**
*@brief      值语义的缓冲区类，直接认为它是std::string即可
*            与std::string的区别是这个类保证内部数据的连续性
*/
class CHNBuffer
{
public:
    CHNBuffer(size_t nSpaceSize = 0)
        :m_Buffer(nSpaceSize)
        , m_Size(0)
    {}
    CHNBuffer(const char* str)
    {
        m_Size = 0;
        append(str);
    }
    CHNBuffer(const CHNBuffer& rhs)
        :m_Buffer(rhs.m_Buffer)
        , m_Size(rhs.m_Size)
    {}
    CHNBuffer& operator=(const CHNBuffer& rhs)
    {
        if (this == &rhs)
            return *this;
        m_Buffer = (rhs.m_Buffer);
        m_Size = (rhs.m_Size);
        return *this;
    }
    CHNBuffer(CHNBuffer && rhs)
        :m_Buffer(std::move(rhs.m_Buffer))
        , m_Size(rhs.m_Size)
    {}
    CHNBuffer& operator=(CHNBuffer &&rhs)
    {
        if (this == &rhs)
            return *this;
        m_Buffer = std::move(rhs.m_Buffer);
        m_Size = rhs.m_Size;
        return *this;
    }
    CHNBuffer& operator += (const CHNBuffer& rhs)
    {
        this->append(rhs.data(), rhs.length());
        return *this;
    }

    CHNBuffer& append(const CHNBuffer& buffer)
    {
        return this->append(buffer.data(), buffer.length());
    }
    CHNBuffer& append(const char* str)
    {
        return this->append(str, strlen(str));
    }
    CHNBuffer& append(const char c)
    {
        m_Buffer.push_back(c);
        m_Size += 1;
        return *this;
    }
    CHNBuffer& append(const char* data, size_t len)
    {
        if (data == NULL || len == 0)
            return *this;
        if (m_Buffer.size() < m_Size + len)
            m_Buffer.resize(m_Size + len + ((m_Size + 1) * 2));
        memcpy(this->data() + m_Size, data, len);
        m_Size += len;
        return *this;
    }
    const char* data() const
    {
        if (m_Buffer.empty())
            return "";
        return &*m_Buffer.begin();
    }
    char* data()
    {
        if (m_Buffer.empty())
            return NULL;
        return &*m_Buffer.begin();
    }
    size_t size() const
    {
        return m_Size;
    }
    size_t length() const
    {
        return m_Size;
    }
    void clear()
    {
        m_Buffer.clear();
        m_Size = 0;
    }
    const char* c_str()
    {
        if (m_Buffer.size() > m_Size)
        {
            m_Buffer[m_Size] = 0;
        }
        else
        {
            m_Buffer.push_back('\0');
        }
        return &*m_Buffer.begin();
    }
    bool empty() const
    {
        return (m_Size == 0);
    }
    void reserve(size_t len)
    {
        if (m_Buffer.size() >= len)
            return;
        m_Buffer.resize(len);
    }
    size_t capacity()
    {
        return m_Buffer.size();
    }
    char& operator[](size_t n)
    {
        return m_Buffer[n];
    }

    /**
    *@brief     选择一段转化为整数
    *@param     offset    [in]    位置
    *@param     len        [in]    长度
    *@return    是否成功，成功则IntResult为整数
    */
    CHNResult<int> toInt(size_t offset, size_t len)
    {
        CHNResult<int> result;
        if (offset + len >= m_Size || len == 0 || len > 10)
            return result.SetFail("params invalid.");
        for (size_t i = offset; i < offset + len; i++)
        {
            if (!isdigit(m_Buffer[i]))
                return result.SetFail("data [%c] is not a digit. ", m_Buffer[i]);
        }
        std::string tmp(data() + offset, len);
        return result.SetSucc(atoi(tmp.c_str()));
    }

    /**
    *@brief     从头部抹除字节(可能会影响容量，导致容量也同步减少)
    *@param     len        [in]    抹除的字节数
    *@return    自身this引用，以便链式调用
    */
    CHNBuffer& striphead(size_t len)
    {
        if (len == 0)
        {
            return *this;
        }
        if (len < m_Size)
        {
            m_Buffer.erase(m_Buffer.begin(), m_Buffer.begin() + len);
            m_Size -= len;
        }
        else
        {
            m_Size = 0;
        }
        return *this;
    }


    //这两个函数比较特殊，一般用于如下类似场景：
    /*
    CHNBuffer buf(1024);
    memcpy(buf.data(), "hello", 5);
    buf.updatesize(5);
    */
    void updatesize(size_t len)
    {
        m_Size = len;
    }
    void addsize(size_t len)
    {
        m_Size += len;
    }
protected:
    std::vector<char>    m_Buffer;
    size_t               m_Size;
};


/**
*@brief      LRU策略的map
*            算法根据数据的历史访问记录来进行淘汰数据，其核心思想是“如果数据最近被访问过，那么将来被访问的几率也更高”。
*            当缓存达到容量上限时，优先替换掉近少使用的数据。
*            CHNLRUMap 基于哈希表和双向链表实现(std::unordered_map、std::list)，内部元素是无序的。
*            整体的设计思路是，可以使用 HashMap 存储 key，这样可以做到 save 和 get key的时间都是 O(1)，
*            而 HashMap 的 Value 指向双向链表实现的 LRU 的 Node 节点
*/
template <typename K, typename V>
class CHNLRUMap
{
public:
    typedef typename std::unordered_map<K, V>::iterator hnlru_iter;
public:
    explicit CHNLRUMap(size_t capacity = 1024)
    {
        m_capacity = capacity > 0 ? capacity : 1024;
    }

    ~CHNLRUMap()
    {}

    /**
    *@brief        是否为空
    */
    bool empty() const
    {
        return m_kv.size() == 0;
    }

    /**
    *@brief        取长度
    */
    size_t size() const
    {
        return m_kv.size();
    }

    /**
    *@brief        取begin迭代器
    */
    hnlru_iter begin()
    {
        return m_kv.begin();
    }

    /**
    *@brief        取end迭代器
    */
    hnlru_iter end()
    {
        return m_kv.end();
    }

    /**
    *@brief      查找
    *            通过 HashMap 找到 LRU 链表节点，因为根据LRU 原理，这个节点是最新访问的，所以要把节点插入到队头，然后返回缓存的值。
    */
    hnlru_iter find(const K& key)
    {
        hnlru_iter it = m_kv.find(key);
        if (it != m_kv.end() && m_kl.front() != key)
        {
            auto ki = m_ki.find(key);
            m_kl.splice(m_kl.begin(), m_kl, ki->second); //移动到front
            ki->second = m_kl.begin();
        }
        return it;
    }

    /**
    *@brief  插入
    *        首先在 HashMap 找到 Key 对应的节点，如果节点存在，更新节点的值，并把这个节点移动队头。
    *        如果不存在，需要构造新的节点，并且尝试把节点塞到队头，如果LRU空间不足，则通过 tail 淘汰掉队尾的节点，同时在 HashMap 中移除 Key。
    */
    void insert(const K& key, const V& value)
    {
        auto r = m_kv.insert(std::make_pair(key, value));
        if (!r.second) return;

        m_kl.push_front(key);
        m_ki[key] = m_kl.begin();

        if (m_kv.size() > m_capacity)
        {
            K k = m_kl.back();
            m_kl.pop_back();
            m_kv.erase(k);
            m_ki.erase(k);
        }
    }

    /**
    *@brief    移除
    */
    void erase(hnlru_iter it)
    {
        if (it != m_kv.end())
        {
            auto ki = m_ki.find(it->first);
            m_kl.erase(ki->second);
            m_ki.erase(ki);
            m_kv.erase(it);
        }
    }

    /**
    *@brief    移除
    */
    void erase(const K& key)
    {
        this->erase(m_kv.find(key));
    }

    /**
    *@brief    清空
    */
    void clear()
    {
        m_kv.clear();
        m_ki.clear();
        m_kl.clear();
    }

    /**
    *@brief    交换
    */
    void swap(CHNLRUMap& x)
    {
        m_kv.swap(x.m_kv);
        m_ki.swap(x.m_ki);
        m_kl.swap(x.m_kl);
        std::swap(m_capacity, x.m_capacity);
    }

protected:
    std::unordered_map<K, V> m_kv;
    std::unordered_map<K, typename std::list<K>::iterator> m_ki;
    std::list<K> m_kl; // key列表
    size_t m_capacity; // 最大容量
};


#endif  //__HELLO_NET_M_CORE_S_CONSOLE__