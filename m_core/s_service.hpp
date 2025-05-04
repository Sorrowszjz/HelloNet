#ifndef __HELLO_NET_M_CORE_S_SERVICE__
#define __HELLO_NET_M_CORE_S_SERVICE__
#include "m_core.hpp"

/**
*@brief		服务操作封装类，
*@note      对于windows： 以NT服务的形式部署(安装和卸载服务需要管理员权限)。支持的命令行参数(这些参数是可以设置的)：
*               其他：服务启动入口
*               -i：安装服务
*               -u：卸载服务
*               -s：启动服务
*               -t：停止服务
*               -p：暂停服务
*               -r：恢复服务
*               -e：直接在命令行调试运行此程序
*               -h：打印帮助    
*           对于其他系统：以daemon形式启动。支持的命令行参数：
*               其他：让程序以demon形式在后台运行
*               -e：直接在命令行调试运行此程序
*               -h：打印帮助
*@note      调用者只需要调用SetStart和SetStop指定服务的启动和停止入口，则程序会自动封装为服务。
*           示例(在main函数中)：
*               CHNService::Default().SetStart([]()
*               {
*                   printf("start！！\n");
*               });
*               CHNService::Default().SetStop([]()
*               {
*                   printf("stop！！\n");
*               });
*	            CHNService::Default().ExecSvc(argc,argv, "abc_service");
*               return 0;
*/
class CHNService : CHNNoCopyable
{
private:
    CHNService()
        :m_startfunc(nullptr)
        ,m_stopfunc(nullptr)
        ,m_pausefunc(nullptr)
        ,m_resumefunc(nullptr)
    {}

public:
    /**
    *@brief  单例模式
    *@return 单例对象
    */
    static CHNService& Default()
    {
        static CHNService o;
        return o;
    }

    /**
    *@brief 设置启动函数，注意：启动函数需要自己保证不退出！
    *@param task[in]    函数
    *@return 无
    */
    void SetStart(HNTaskFunc task)
    {
        m_startfunc = task;
    }

    /**
    *@brief 设置停止函数
    *@param task[in]    函数
    *@return 无
    */
    void SetStop(HNTaskFunc task)
    {
        m_stopfunc = task;
    }

    /**
    *@brief 设置暂停函数【暂不支持】
    *@param task[in]    函数
    *@return 无
    */
    void SetPause(HNTaskFunc task)
    {
        m_pausefunc = task;
    }

    /**
    *@brief 设置恢复函数【暂不支持】
    *@param task[in]    函数
    *@return 无
    */
    void SetResume(HNTaskFunc task)
    {
        m_resumefunc = task;
    }

    /**
    *@brief 启动服务运行
    *@param argc[in]            main函数的argc
    *@param argv[in]            main函数的argv
    *@param servicename[in]     服务名称，必填，且必须和其他服务名称不重名
    *@param desc[in]            服务描述
    *@param helpinfo[in]        帮助信息，为NULL则打印默认的帮助信息
    *@param installcmd[in]      安装服务命令，为NULL则使用默认值-i
    *@param uninstallcmd[in]    卸载服务命令，为NULL则使用默认值-u
    *@param startcmd[in]        启动服务命令，为NULL则使用默认值-s
    *@param stopcmd[in]         停止服务命令，为NULL则使用默认值-t
    *@param pausecmd[in]        暂停服务命令，为NULL则使用默认值-p
    *@param resumecmd[in]       恢复服务命令，为NULL则使用默认值-r
    *@param execcmd[in]         直接运行命令，为NULL则使用默认值-e
    *@param helpcmd[in]         打印帮助命令，为NULL则使用默认值-h
    *@return 是否成功
    */
    CHNResult<> ExecSvc(int argc, char** argv, const char* servicename, const char* desc = NULL, const char* helpinfo = NULL,
                    const char* installcmd = NULL, 
                    const char* uninstallcmd = NULL,
                    const char* startcmd = NULL,
                    const char* stopcmd = NULL,
                    const char* pausecmd = NULL,
                    const char* resumecmd = NULL,
                    const char* execcmd = NULL,
                    const char* helpcmd = NULL)
    {
        //参数合法性判断
        CHNResult<> result;
        if(servicename == NULL || servicename[0] == 0)
        {
            return result.SetFail("service name is emtpy!");
        }
        m_svcname = servicename;

        //命令行运行
        if(argc > 1 && CheckArgsEqual(argv[1], execcmd , "-e"))
        {
            if(m_startfunc)
                m_startfunc();

            //这里，用户自己的start函数需要负责保证不退出

            if(m_stopfunc)
                m_stopfunc();
        }
        //安装服务
        else if(argc > 1 && CheckArgsEqual(argv[1], installcmd , "-i"))
        {
#if defined(HNOS_WIN)
            auto ret = CHNFileUtil::HNGetExecuteFileDir();
            if(!ret)
            {
                return result.SetFail("%s", ret.ErrDesc());
            }
            std::string strBinPath = ret.Get().first + ret.Get().second;

            CHNWinSCM svcManager;
            return svcManager.CreateSvc(servicename, strBinPath.c_str(), desc);
#else
            return result.SetFail("not supported install cmd");
#endif
        }
        //卸载服务
        else if(argc > 1 && CheckArgsEqual(argv[1], uninstallcmd , "-u"))
        {
#if defined(HNOS_WIN)
            CHNWinSCM svcManager;
            return svcManager.RemoveSvc(servicename);
#else
            return result.SetFail("not supported uninstall cmd");
#endif
        }
        //启动服务
        else if(argc > 1 && CheckArgsEqual(argv[1], startcmd , "-s"))
        {
#if defined(HNOS_WIN)
            CHNWinSCM svcManager;
            return svcManager.StartSvc(servicename);
#else
            return result.SetFail("not supported start cmd");
#endif
        }
        //停止服务
        else if(argc > 1 && CheckArgsEqual(argv[1], stopcmd , "-t"))
        {
#if defined(HNOS_WIN)
            CHNWinSCM svcManager;
            return svcManager.StopSvc(servicename);
#else
            return result.SetFail("not supported stop cmd");
#endif
        }
        //暂停服务
        else if(argc > 1 && CheckArgsEqual(argv[1], pausecmd , "-p"))
        {
#if defined(HNOS_WIN)
            CHNWinSCM svcManager;
            return svcManager.PauseSvc(servicename);
#else
            return result.SetFail("not supported pause cmd");
#endif
        }
        //恢复服务
        else if(argc > 1 && CheckArgsEqual(argv[1], resumecmd , "-r"))
        {
#if defined(HNOS_WIN)
            CHNWinSCM svcManager;
            return svcManager.ResumeSvc(servicename);
#else
            return result.SetFail("not supported resume cmd");
#endif
        }
        //打印帮助
        else if(argc > 1 && CheckArgsEqual(argv[1], helpcmd , "-h"))
        {
            if(helpinfo == NULL || helpinfo[0] == 0)
            {
#if defined(HNOS_WIN)
            printf("service(%s) help info:\n"
                "%s: install service\n"
                "%s: uninstall service\n"
                "%s: start service\n"
                "%s: stop service\n"
                "%s: pause service\n"
                "%s: resume service\n"
                "%s: rum as command line\n"
                "%s: print help info.\n",
                servicename,
                (installcmd&&installcmd[0]!=0)?installcmd:"-i",
                (uninstallcmd&&uninstallcmd[0]!=0)?uninstallcmd:"-u",
                (startcmd&&startcmd[0]!=0)?startcmd:"-s",
                (stopcmd&&stopcmd[0]!=0)?stopcmd:"-t",
                (pausecmd&&pausecmd[0]!=0)?pausecmd:"-p",
                (resumecmd&&resumecmd[0]!=0)?resumecmd:"-r",
                (execcmd&&execcmd[0]!=0)?execcmd:"-e",
                (helpcmd&&helpcmd[0]!=0)?helpcmd:"-h");
#else
            printf("service(%s) help info:\n"
                "no arg: run as daemon\n"
                "%s: rum as command line\n",
                servicename,
                (execcmd&&execcmd[0]!=0)?execcmd:"-e");

#endif
            }
            else
            {
                printf("%s\n",helpinfo);
            }
        }
        //其他，服务启动入口
        else 
        {
#if defined(HNOS_WIN)
            //服务入口
            SERVICE_TABLE_ENTRYA stSrvEntry[] =
	        {
		        { (char*)servicename, (LPSERVICE_MAIN_FUNCTIONA)CHNService::HNWindowsNTServiceMain },
		        { NULL, NULL }
	        };

            //启动服务入口
            if (!StartServiceCtrlDispatcherA(stSrvEntry))
	        {
                return result.SetSystemFail();
	        }
#else
            //demon入口
            if(::daemon(0,0) == -1)
                return result.SetSystemFail();
            if(m_startfunc)
                m_startfunc();
            if(m_stopfunc)
                m_stopfunc();
#endif
        }
        return result;
    }

#if defined(HNOS_WIN)
    /**
    *@brief windows服务入口
    *@return 无
    */
    static void WINAPI HNWindowsNTServiceMain(DWORD dwNumServicesArgs,LPSTR *lpServiceArgVectors)
    {
        try
	    {
            //注册服务控制
            auto& svcObj = CHNService::Default();
            svcObj.m_SvcStatusHandle = RegisterServiceCtrlHandlerA(svcObj.m_svcname.c_str(), CHNService::HNWindowsNTServiceControlHander);
		    if (svcObj.m_SvcStatusHandle == NULL)
		    {
                std::string desc = HNGetSysErrorMsg(GetLastError());
                printf("RegisterServiceCtrlHandlerA fail:%s\n", desc.c_str() ) ;
			    return;
		    }
            svcObj.m_SvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
            svcObj.m_SvcStatus.dwServiceSpecificExitCode = 0;  

            //服务准备启动
            CHNService::HNWindowsNTServiceReport( SERVICE_START_PENDING, 3000 );
            CHNService::HNWindowsNTServiceReport( SERVICE_RUNNING, 0 );

            //运行程序
            if(svcObj.m_startfunc)
                svcObj.m_startfunc();

            //程序返回，服务停止
            CHNService::HNWindowsNTServiceReport( SERVICE_STOPPED, 0 );
        }
        catch(...)
	    {
            std::string desc = HNGetSysErrorMsg(GetLastError());
            printf("HNWindowsNTServiceMain fail:%s\n", desc.c_str() ) ;
	    }
    }

    /**
    *@brief windows服务控制回调
    *@return 无
    */
    static void WINAPI HNWindowsNTServiceControlHander(DWORD dwControl)
    {
        switch (dwControl)
	    {
            //停止服务
		    case SERVICE_CONTROL_STOP:
			    {
                    CHNService::HNWindowsNTServiceReport( SERVICE_STOP_PENDING, 0 );
                    auto& svcObj = CHNService::Default();
                    if(svcObj.m_stopfunc)
                        svcObj.m_stopfunc();
                    //程序自然返回也会报告停止，这里无需再次报告
				    //CHNService::HNWindowsNTServiceReport( SERVICE_STOPPED, 0 );
			    }
			    break;
            //暂停服务
		    case SERVICE_CONTROL_PAUSE:
                {
                    CHNService::HNWindowsNTServiceReport( SERVICE_PAUSE_PENDING, 0 );
                    auto& svcObj = CHNService::Default();
                    if(svcObj.m_pausefunc)
                        svcObj.m_pausefunc();
                    CHNService::HNWindowsNTServiceReport( SERVICE_PAUSED, 0 );
                }
			    break;
            //恢复服务
		    case SERVICE_CONTROL_CONTINUE:
                {
                    CHNService::HNWindowsNTServiceReport( SERVICE_CONTINUE_PENDING, 0 );
                    auto& svcObj = CHNService::Default();
                    if(svcObj.m_resumefunc)
                        svcObj.m_resumefunc();
                    CHNService::HNWindowsNTServiceReport( SERVICE_RUNNING, 0 );
                }
			    break;
		    default:
                printf("unknown control cmd:%d\n", (int)dwControl);
			    break;
	    }
    }

    /**
    *@brief  报告状态
    *@param dwCurrentState[in]  The current state (see SERVICE_STATUS)
    *@param dwWaitHint[in]      Estimated time for pending operation, in milliseconds
    *@return 无
    */
    static void HNWindowsNTServiceReport( DWORD dwCurrentState, DWORD dwWaitHint = 0)
    {
        static DWORD dwCheckPoint = 1;
        auto& svcObj = CHNService::Default();

        // Fill in the SERVICE_STATUS structure.
        svcObj.m_SvcStatus.dwCurrentState = dwCurrentState;
        svcObj.m_SvcStatus.dwWin32ExitCode = NO_ERROR;
        svcObj.m_SvcStatus.dwWaitHint = dwWaitHint;

        if (dwCurrentState == SERVICE_START_PENDING)
            svcObj.m_SvcStatus.dwControlsAccepted = 0;
        else 
            svcObj.m_SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

        if ( (dwCurrentState == SERVICE_RUNNING) ||
               (dwCurrentState == SERVICE_STOPPED) )
            svcObj.m_SvcStatus.dwCheckPoint = 0;
        else 
            svcObj.m_SvcStatus.dwCheckPoint = dwCheckPoint++;

        // Report the status of the service to the SCM.
        if(!SetServiceStatus( svcObj.m_SvcStatusHandle, &svcObj.m_SvcStatus ))
        {
            std::string desc = HNGetSysErrorMsg(GetLastError());
            printf("SetServiceStatus fail:%s\n", desc.c_str() ) ;
        }
    }

#endif

protected:

    /**
    *@brief 检查参数是否满足条件
    *@return 无
    */
    bool CheckArgsEqual(const char* arg, const char* cmd, const char* defaultcmd)
    {
        if(cmd == NULL || cmd[0] == 0)
            return (strcmp(arg, defaultcmd) == 0);
        else
            return (strcmp(arg, cmd) == 0);
    }


protected:
    std::string m_svcname;
    HNTaskFunc  m_startfunc;
    HNTaskFunc  m_stopfunc;
    HNTaskFunc  m_pausefunc;
    HNTaskFunc  m_resumefunc;
#if defined(HNOS_WIN)
    SERVICE_STATUS          m_SvcStatus; 
    SERVICE_STATUS_HANDLE   m_SvcStatusHandle; 
#endif
};  

#endif //__HELLO_NET_M_CORE_S_SERVICE__