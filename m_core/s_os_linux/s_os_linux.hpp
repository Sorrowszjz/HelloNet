#ifndef __HELLONET_M_CORE_S_OS_LINUX__
#define __HELLONET_M_CORE_S_OS_LINUX__
#include "../s_base.hpp"

/*********************************************************************/
//libdbus
#ifdef HNHAVE_LIBDBUS
#include <dbus/dbus.h>

class CHNDBUSClient : CHNNoCopyable
{
public:
    explicit CHNDBUSClient(const char* serviceName, const char* intfName, const char* methodName)
        :m_serviceName(serviceName ? serviceName : "")
        , m_intfName(intfName ? intfName : "")
        , m_methodName(methodName ? methodName : "")
    {}
    ~CHNDBUSClient()
    {}

    /**
    *@brief     发送消息(wparam为64位)
    *@param     hWnd         [in]    目标窗口名称
    *@param     wParam       [in]    参数1
    *@param     lParam       [in]    参数2
    *@param     wParam64     [in]    参数1是不是要以64位发送
    *@return    是否成功
    */
    CHNResult<> Send(void* hWnd, int wParam, ssize_t lParam, bool wParam64 = false)
    {
        CHNResult<> result;
        if (hWnd == NULL)
        {
            return result.SetFail("hWnd == NULL.");
        }
        const char * path = (const char *)hWnd;
        DBusError err;
        dbus_error_init(&err);

        DBusConnection* conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
        if (dbus_error_is_set(&err))
        {
            dbus_error_free(&err);
            return result.SetFail("Connect Error %s", err.message);
        }
        if (conn == NULL)
        {
            return result.SetFail("Connect %s null", path);
        }
        CHNAutoObj autoCloseConn([&conn]()
        {
            dbus_connection_unref(conn);
        });

        DBusMessage *request = dbus_message_new_method_call(m_serviceName.c_str(), path,
            m_intfName.c_str(), m_methodName.c_str());
        if (request == NULL)
        {
            return result.SetFail("request is null. szServiceName=%s", m_serviceName.c_str());
        }
        CHNAutoObj autoCloseReq([&request]()
        {
            dbus_message_unref(request);
        });

        dbus_message_set_no_reply(request, TRUE);
        DBusMessageIter iter;
        dbus_message_iter_init_append(request, &iter);

        int bRet = 0;
        if (wParam64)
        {
            uint64_t wParam_tmp = wParam;
            bRet = dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT64, &wParam_tmp);
        }
        else
        {
            bRet = dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &wParam);
        }
        if (!bRet)
        {
            return result.SetFail("dbus_message_iter_append_basic wParam Error");
        }
        if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT64, &lParam))
        {
            return result.SetFail("dbus_message_iter_append_basic lParam Error");
        }
        if (!dbus_connection_send(conn, request, NULL))
        {
            return result.SetFail("dbus_connection_send Error");
        }
        dbus_connection_flush(conn);
        return result;
    }

private:
    std::string m_serviceName;
    std::string m_intfName;
    std::string m_methodName;
};

#endif /*HNHAVE_LIBDBUS*/

#endif // __HELLONET_M_CORE_S_OS_LINUX__