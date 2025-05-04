#ifndef __HELLO_NET_M_IO_S_WEBSOCKET__
#define __HELLO_NET_M_IO_S_WEBSOCKET__


#include "../m_core/m_core.hpp"

/**
*@brief    websocket协议解析类
*       +-+-+-+-+-------+-+-------------+-------------------------------+
*        0                   1                   2                   3
*        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
*       +-+-+-+-+-------+-+-------------+-------------------------------+
*       |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
*       |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
*       |N|V|V|V|       |S|             |   (if payload len==126/127)   |
*       | |1|2|3|       |K|             |                               |
*       +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
*       |     Extended payload length continued, if payload len == 127  |
*       + - - - - - - - - - - - - - - - +-------------------------------+
*       |                     Payload Data continued ...                |
*       +---------------------------------------------------------------+
*/

class CHNWebSocketUnPack : CHNNoCopyable
{
public:
    /**
    *@brief        帧头1字节
    */
    enum HNWSFrameType
    {
        HNWS_TEXT_FRAME = 0x81,    //文本数据帧
        HNWS_BIN_FRAME = 0x82,    //二进制数据帧
        HNWS_CLOSE_FRAME = 0x88,    //关闭数据帧
        HNWS_PING_FRAME = 0x89,    //心跳帧PING
        HNWS_PONG_FRAME = 0x8A     //心跳帧PONG
    };

    /**
    *@brief        构造方法
    *@param        bClient [in]    是否为websocket客户端，否则为服务端，客户端和服务端区别如下：
    *                            1.握手时客户端发起认证，服务端要根据客户端发起的认证码计算认证结果(公开算法)
    *                            2.客户端数据必须经过mask，服务端数据不需要
    */
    explicit CHNWebSocketUnPack(bool bClient)
    :m_bClient(bClient)
    {}

    /**
    *@brief     客户端生成Sec-WebSocket-Key
    *@return    结果
    */
    static std::string GenerateSecWebSocketKey()
    {
        std::string binkey = CHNRandom::HNGenString(16,"");
        return CHNStrUtil::HNBase64Encode((const unsigned char*)binkey.data(),binkey.length());
    }

    /**
    *@brief     服务端根据请求的Sec-WebSocket-Key生成Sec-WebSocket-Accept数据
    *@param     secWebSocketKey [in]    客户端给的密钥
    *@return    回应的校验值
    */
    static std::string GetValidateCode(const std::string& secWebSocketKey)
    {
        // 使用请求传过来的KEY+协议字符串，先用SHA1加密然后使用base64编码算出一个应答的KEY
        std::string serverKey  = secWebSocketKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        //sha1 & base64
        std::string digest = CHNSHA1::HNGetBytesSHA1(serverKey.data(), serverKey.length());
        return CHNStrUtil::HNBase64Encode((const unsigned char*)digest.data(), HNSHA1_LEN);
    }


    /**
    *@brief     打包心跳帧
    *@param     bPing [in]    true为PING，false为PONG
    *@param     data  [in]    心跳帧附带的数据，一般使用时不应该附带数据（但是协议是支持的）
    *@return    打包结果数据。
    *@note      1.任意一方可主动发送PING(可附带数据)，收到PING必须回应相同的PONG（包括数据）
    *           2.没收到PING而主动发送PONG被认为是单向心跳，单向心跳不应该被回应
    *           3.如果收到PING没及时回应时又收到了第二个PING，可直接回应第二个PING，忽略第一个
    *           总结：收到PING必须回应相同数据的PONG，收到PONG什么都不要做
    */
    std::string PackHeartBeatFrame(bool bPing, const std::string& data = std::string())
    {
        if(!bPing)
        {
            return PackFrame(data, HNWS_PONG_FRAME);
        }
        else
        {
            return PackFrame(data, HNWS_PING_FRAME);
        }
    }

    /**
    *@brief     打包断开帧
    *@param     code   [in]    断开错误码，参考websocket标准错误码，如1000代表正常关闭
    *@param     reason [in]    断开原因（可选，必须传入UTF8编码）
    *@return    打包结果数据。
    */
    std::string PackCloseFrame(unsigned short code = 1000, const char* reason = NULL)
    {
        
    }

    /**
    *@brief     打包帧
    *@param     data [in]    用户数据，不能超过4GB长度
    *@param     type [in]    帧类型。注意：文本帧（包括错误原因数据）需要使用UTF-8编码
    *@return    帧数据。
    */
    std::string PackFrame(const std::string& data, HNWSFrameType type = HNWS_TEXT_FRAME)
    {

    }

    /**
    *@brief     从流中读取并解包一帧，自动处理心跳和关闭帧
    *@param     socketptr  [in]    接收数据的socket
    *@param     nWaitMs    [in]    接收数据超时时间
    *@return    是否成功，成功则一帧数据保存在StrResult1，其他信息在成员变量中
    */
    CHNResult<std::string> UnPackFrame(CHNSocketHandle* socketptr, int nWaitMs)
    {

    }


protected:

    /**
    *@brief     解码payload并附加到输入的payloadbuf
    *@param     payloaddata  [in]     输入的数据
    *@param     payloadbuf   [out]    输出的缓冲区
    */
    inline void AppendDecryptPayLoad(const char* payloaddata, std::string& payloadbuf)
    {

    }

public:
    //解码器是否为websocket客户端进行解码
    bool m_bClient;

    unsigned char m_fin;    //0后续还有数据 否则结束
    unsigned char m_opcode; //0x0表示附加数据帧 0x1表示文本数据帧 0x2表示二进制数据帧 0x3-7暂时无定义，为以后的非控制帧保留 0x8表示连接关闭 0x9表示ping 0xA表示pong 0xB-F暂时无定义，为以后的控制帧保留
    unsigned char m_mask;   //用于标识PayloadData是否经过掩码处理，客户端发出的数据帧需要进行掩码处理为1
    uint64_t m_payloadLen;  //附带的数据长度，按7位，7+16位，7+64位递推
};



/**
*@brief        WebSocket请求类
*/
class CHNWebSocketClient : public CHNNoCopyable
{
public:
    explicit CHNWebSocketClient()
        :m_wsUnPack(true)
        , m_hSocketPtr(std::make_shared<CHNSocketHandle>(true, &CHNPoller::obj()))
    {
    }

    ~CHNWebSocketClient()
    {
        if (m_hSocketPtr)
            m_hSocketPtr->XClose();
    }

    /**
    *@brief     握手ws/wss请求
    *@param     url         [in]    请求url
    *@param     nTimeOut    [in]    超时毫秒。如果nTimeoutMs < 0 代表永不超时
    *@return    是否成功。 成功后用下面的取结果函数取结果
    */
    CHNResult<> WSHandeShake(const char* url, int nTimeOut)
    {

    }

    /**
    *@brief     发送用户数据，发送一帧
    *@param     data        [in]    要发送的用户数据
    *@param     nTimeoutMs  [in]    超时时间毫秒，<0代表永不超时
    *@param     type        [in]    帧类型，默认为文本帧
    *@return    是否成功
    */
    CHNResult<> SendText(const std::string& data, int nTimeoutMs = -1, CHNWebSocketUnPack::HNWSFrameType type = CHNWebSocketUnPack::HNWS_TEXT_FRAME)
    {

    }

    /**
    *@brief     接收用户一帧数据帧（心跳帧会自动处理）
    *@param     nTimeOut      [in]    超时毫秒。如果nTimeoutMs < 0 代表永不超时
    *@return    是否成功(成功后帧数据：std::string，帧类型：可读取m_wsUnpack的成员变量进一步判断是文本还是二进制帧)
    */
    CHNResult<std::string> RecvText(int nTimeOut)
    {

    }
public:
    CHNWebSocketUnPack m_wsUnPack; //websocket解包器
    CHNSocketHandlePtr m_hSocketPtr; //socket对象
};



/**
*@brief      CWebSocket连接类
*            websoket有两次过程：1.http协议握手过程；2.握手后的数据帧收发过程
*            【本对象只能被智能指针管理】
*/

class CHNWebSocketConnection : public CHNSocketHandle
{

};
#endif // __HELLO_NET_M_IO_S_WEBSOCKET__