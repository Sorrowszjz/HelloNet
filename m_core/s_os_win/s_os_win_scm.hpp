#ifndef __HELLONET_M_CORE_S_OS_WIN_SCM__
#define __HELLONET_M_CORE_S_OS_WIN_SCM__
#include "../s_base.hpp"



/**
*@brief		windows服务操作封装类，需要管理员权限
*/
class CHNWinSCM : CHNNoCopyable
{
public:
	explicit CHNWinSCM(DWORD dwDesiredAccess = SC_MANAGER_ALL_ACCESS)
        :m_hSCManager(NULL)
    {
	    m_hSCManager = OpenSCManagerW(NULL, NULL, dwDesiredAccess);
    }
	virtual ~CHNWinSCM()
    {
        if (m_hSCManager)
	    {
		    CloseServiceHandle(m_hSCManager);
		    m_hSCManager = NULL;
	    }
    }

    /**
	*@brief		服务创建
    *@param		pszSvcName[in]      服务名，操作服务的关键
	*@param		pszBinaryPath[in]   服务文件路径
    *@param		pszDisplayName[in]  服务描述
    *@param		dwDesiredAccess[in] 服务权限
    *@param		dwServiceType[in]   服务类型
    *@param		dwStartType[in]     服务启动类型 SERVICE_AUTO_START自动，SERVICE_DEMAND_START手动，SERVICE_DISABLED禁用
	*@return	是否成功
	*/
    CHNResult<>  CreateSvc(const char* pszSvcName, 
        const char*  pszBinaryPath, 
        const char*  pszDisplayName = NULL, 
        DWORD dwDesiredAccess = SERVICE_ALL_ACCESS,
        DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, 
        DWORD dwStartType = SERVICE_AUTO_START)
    {
        CHNResult<> result;
        if(m_hSCManager == NULL)
        {
            return result.SetFail("open sc manager fail");
        }
	    if (pszSvcName == NULL || pszBinaryPath == NULL)
	    {
            return result.SetFail("pszSvcName == NULL || pszBinaryPath == NULL");
	    }
        if(access(pszBinaryPath, F_OK) != 0)
        {
            return result.SetSystemFail();
        }
	    
	    std::string strDisplayName = pszSvcName;
	    if (pszDisplayName)
	    {
		    strDisplayName = pszDisplayName;
	    }

	    SC_HANDLE hService = CreateServiceA(
		    m_hSCManager, 							/* SCManager database */
		    pszSvcName, 						    /* name of service */
		    strDisplayName.c_str(), 				/* service name to display */
		    dwDesiredAccess,                 	    /* desired access */
		    dwServiceType,							/* service type */
		    dwStartType,							/* start type */
		    SERVICE_ERROR_NORMAL,					/* error control type */
		    pszBinaryPath,							/* service's binary */
		    NULL,									/* no load ordering group */
		    NULL,									/* no tag identifier */
		    NULL,									/* no dependencies */
		    NULL,									/* LocalSystem account */
		    NULL);									/* no password */
	    if (NULL == hService)
	    {
		    return result.SetSystemFail();
	    }

	    CloseServiceHandle(hService);
	    return result;
    }

    /**
	*@brief		服务删除
    *@param		pszSvcName[in]      服务名，操作服务的关键
	*@return	是否成功
	*/
	CHNResult<>  RemoveSvc(const char* pszSvcName)
    {
        CHNResult<> result;
        if(m_hSCManager == NULL)
        {
            return result.SetFail("open sc manager fail");
        }
        if (pszSvcName == NULL)
	    {
            return result.SetFail("pszSvcName == NULL");
	    }
        SC_HANDLE hService = OpenServiceA(m_hSCManager, pszSvcName, STANDARD_RIGHTS_REQUIRED | SERVICE_STOP);
	    if (hService == NULL)
	    {
		    return result.SetSystemFail();
	    }
        HN_DEFER([&hService](){CloseServiceHandle(hService);});

        //先停止
	    SERVICE_STATUS serviceStatus; 
	    if(!ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus))
        {
            //停止失败，忽略，继续删除
            std::string desc = HNGetSysErrorMsg(GetLastError());
            printf("stop service(%s) fail: %s\n", pszSvcName, desc.c_str());
        }

        //再删除
	    if(!DeleteService(hService))
        {
            return result.SetSystemFail();
        }
        return result;
    }

    /**
	*@brief		服务启动
	*@param		pszSvcName[in]      服务名，操作服务的关键
	*@return	是否成功
	*/
	CHNResult<>  StartSvc(const char* pszSvcName)
    {
        CHNResult<> result;
        if(m_hSCManager == NULL)
        {
            return result.SetFail("open sc manager fail");
        }
        if (pszSvcName == NULL)
	    {
            return result.SetFail("pszSvcName == NULL");
	    }
        SC_HANDLE hService = OpenServiceA(m_hSCManager, pszSvcName, SERVICE_START);
	    if (hService == NULL)
	    {
		    return result.SetSystemFail();
	    }
        HN_DEFER([&hService](){CloseServiceHandle(hService);});

	    if(!StartServiceA(hService, 0, NULL))
        {
            return result.SetSystemFail();
        }
        return result;
    }

    /**
	*@brief		服务停止
	*@param		pszSvcName[in]      服务名，操作服务的关键
	*@return	是否成功
	*/
	CHNResult<>  StopSvc(const char* pszSvcName)
    {
        return this->ControlSvc(pszSvcName, SERVICE_CONTROL_STOP);
    }

    /**
	*@brief		服务暂停
	*@param		pszSvcName[in]      服务名，操作服务的关键
	*@return	是否成功
	*/
	CHNResult<>  PauseSvc(const char* pszSvcName)
    {
        return this->ControlSvc(pszSvcName, SERVICE_CONTROL_PAUSE);
    }

    /**
	*@brief		服务恢复
	*@param		pszSvcName[in]      服务名，操作服务的关键
	*@return	是否成功
	*/
	CHNResult<>  ResumeSvc(const char* pszSvcName)
    {
        return this->ControlSvc(pszSvcName, SERVICE_CONTROL_CONTINUE);
    }

	/**
	*@brief		服务状态查询
	*@param		pszSvcName[in]      服务名，操作服务的关键
	*@return	成功则int为如下服务状态
	*@			SERVICE_STOPPED				1
	*@			SERVICE_START_PENDING		2
	*@			SERVICE_STOP_PENDING		3
	*@			SERVICE_RUNNING				4
	*@			SERVICE_CONTINUE_PENDING	5
	*@			SERVICE_PAUSE_PENDING		6
	*@			SERVICE_PAUSED				7
	*/
	CHNResult<int>  QuerySvcStats(const char* pszSvcName)
    {
        CHNResult<int> result;
        if(m_hSCManager == NULL)
        {
            return result.SetFail("open sc manager fail");
        }
        if (pszSvcName == NULL)
	    {
            return result.SetFail("pszSvcName == NULL");
	    }
        SC_HANDLE hService = OpenServiceA(m_hSCManager, pszSvcName, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG | SERVICE_ENUMERATE_DEPENDENTS);
	    if (hService == NULL)
	    {
		    return result.SetSystemFail();
	    }
        HN_DEFER([&hService](){CloseServiceHandle(hService);});

        SERVICE_STATUS    serviceStatus;
	    if(!QueryServiceStatus(hService,  &serviceStatus))
        {
            return result.SetSystemFail();
        }
        return result.SetSucc(serviceStatus.dwCurrentState);
    }

protected:

    /**
	*@brief		控制服务
	*@param		pszSvcName[in]      服务名，操作服务的关键
    *@param		dwControl[in]       控制命令，如 SERVICE_CONTROL_STOP
	*@return	是否成功
	*/
	CHNResult<>  ControlSvc(const char* pszSvcName, DWORD dwControl)
    {
        CHNResult<> result;
        if(m_hSCManager == NULL)
        {
            return result.SetFail("open sc manager fail");
        }
        if (pszSvcName == NULL)
	    {
            return result.SetFail("pszSvcName == NULL");
	    }
        SC_HANDLE hService = OpenServiceA(m_hSCManager, pszSvcName, SERVICE_STOP | SERVICE_PAUSE_CONTINUE);
	    if (hService == NULL)
	    {
		    return result.SetSystemFail();
	    }
        HN_DEFER([&hService](){CloseServiceHandle(hService);});

	    SERVICE_STATUS    serviceStatus;
	    if(!ControlService(hService, dwControl, &serviceStatus))
        {
            return result.SetSystemFail();
        }
        return result;
    }

protected:

	//服务管理器句柄，对象构造时打开，对象析构时关闭
	SC_HANDLE	m_hSCManager;
};

#endif // __HELLONET_M_CORE_S_OS_WIN_SCM__