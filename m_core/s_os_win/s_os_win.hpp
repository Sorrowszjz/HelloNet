#ifndef  __HELLONET_M_CORE_S_OS_WIN__
#define  __HELLONET_M_CORE_S_OS_WIN__
#include "../s_base.hpp"


//未导出函数的一些定义
//NTDLL
extern void WINAPI RtlGetNtVersionNumbers(DWORD*, DWORD*, DWORD*);

/**
*@brief      获取user32等系统库的函数
*            如 HNWIN_USE_API(NTDLL,NtQuerySystemInformation)
*            代表从 NTDLL.dll加载NtQuerySystemInformation函数
*            执行之后可直接使用HN_NtQuerySystemInformation函数。
*/
#define HNWIN_USE_API(NAME, API) static auto HN_##API = (decltype(&API))(GetProcAddress(GetModuleHandleW(L###NAME), #API))



/**
*@brief     开启系统的程序崩溃请求
*           Vista之后，微软加了一个特性
*           程序崩溃时，默认不交给程序崩溃处理
*           而是使用一个莫名其妙的机制，不让程序进入崩溃环节
*           这个函数可以禁止它
*/
static void HNWinEnableCrossKernelCrashing()
{
	/*
	"HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options"
		DWORD DisableUserModeCallbackFilter = 1

	"HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\TestCppCodegen.exe"
		DWORD DisableUserModeCallbackFilter = 1
	*/
	typedef BOOL (WINAPI *tGetPolicy)(LPDWORD lpFlags); 
	typedef BOOL (WINAPI *tSetPolicy)(DWORD dwFlags); 
	const DWORD EXCEPTION_SWALLOWING = 0x1;
 
	HMODULE hKernel32 = LoadLibraryW(L"kernel32.dll"); 
	tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(hKernel32, "GetProcessUserModeExceptionPolicy"); 
	tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(hKernel32, "SetProcessUserModeExceptionPolicy"); 
	if (pGetPolicy && pSetPolicy) 
	{ 
		DWORD dwFlags; 
		if (pGetPolicy(&dwFlags)) 
		{ 
			// Turn off the filter 
			pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING);
		} 
	} 
}


/**
*@brief     权限操作
*/
class CHNPrivilege : CHNNoCopyable
{
public:
    /**
    *@brief     提升权限
    *@param     lpName    [in]    权限名称：SE_SYSTEMTIME_NAME/SE_SHUTDOWN_NAME...
    *@return    是否提权成功
    */
    static CHNResult<> Adjust(const wchar_t* lpName)
    {
        CHNResult<> result;
        if (lpName == NULL || lpName[0] == 0)
        {
            return result.SetFail("lpName is empty.");
        }
        OSVERSIONINFOW osv;
        osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
        ::GetVersionExW(&osv);
        if (osv.dwPlatformId != VER_PLATFORM_WIN32_NT)
        {
            //不需要提权
            return result;
        }

        HANDLE hToken = NULL;
        TOKEN_PRIVILEGES tkp;
        if (!::OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        {
            return result.SetSystemFail();
        }
        CHNAutoObj autoToken([&hToken](){HNCloseHandle(hToken); });
        if (!::LookupPrivilegeValueW(NULL, lpName, &tkp.Privileges[0].Luid))
        {
            return result.SetSystemFail();
        }
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        if (!::AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
        {
            return result.SetSystemFail();
        }
        return result;
    }
    static CHNResult<> Adjust(const char* lpName)
    {
        std::wstring wslpName;
        auto result = CHNConvertHelper::HNA2W(wslpName, lpName);
        if (result)
        {
            result = Adjust(wslpName.c_str());
        }
        return result;
    }
};

/**
*@brief        窗口操作
*/
class CHNWind : CHNNoCopyable
{
public:
    /**
    *@brief     模糊查找窗口句柄(仅支持windows)
    *@param     lpWindowName    [in]    窗口标题名称或名称的一部分
    *@param     lpClassName     [in]    可选的窗口类名。比如qt程序的窗口类名为 Qt5QWindowIcon
    *@return    成功：窗口句柄列表。失败：列表为空
    */
    static std::map<std::string, HWND> HNFindHWND(const char* lpWindowName, const char* lpClassName = NULL)
    {
        std::map<std::string, HWND> ret;

        char titleText[MAX_PATH] = { 0 };
        HWND nHwnd = ::FindWindowA(lpClassName, NULL);
        while (nHwnd != NULL)
        {
            ::GetWindowTextA(nHwnd, titleText, MAX_PATH);
            if (lpWindowName == NULL || lpWindowName[0] == 0 ||
                strstr(titleText, lpWindowName) != NULL)
            {
                ret.insert(std::make_pair(titleText, nHwnd));
            }

            nHwnd = ::FindWindowExA(0, nHwnd, lpClassName, NULL);
        }
        return ret;
    }

    /**
    *@brief     根据进程id查找窗口句柄(仅支持windows)
    *@param     pid    [in]    进程id
    *@return    成功：窗口句柄。失败：空
    */
    static HWND HNFindHWNDByPid(int pid)
    {
        HWND h = GetTopWindow(0);
        while (h)
        {
            DWORD tmppid = 0;
            DWORD dwTheardId = GetWindowThreadProcessId(h, &tmppid);

            if (dwTheardId != 0)
            {
                if (tmppid == pid)
                {
                    return h;
                }
            }
            h = GetNextWindow(h, GW_HWNDNEXT);
        }
        return NULL;
    }
};

/**
*@brief     注册表操作类(仅windows支持)
*/
class CHNRegistry : CHNNoCopyable
{
public:
    explicit CHNRegistry()
        :m_hKey(NULL){}
    virtual ~CHNRegistry(){ CloseReg(); }

    /**
    *@brief     关闭注册表
    */
    void CloseReg() { HNRegCloseKey(m_hKey); }

    /**
    *@brief     读取注册表，只支持REG_SZ和REG_DWORD
    *@param     hKey    [in]    如 HKEY_LOCAL_MACHINE
    *@param     szPath  [in]    子路径
    *@param     szKey   [in]    key
    *@return    是否成功。成功：int[类型] std::string[值value]   
    */
    CHNResult<std::pair<int,std::string> > ReadReg(HKEY hKey, const char* szPath, const char* szKey)
    {
        CHNResult<std::pair<int,std::string> > result;
        if (szPath == NULL || szPath[0] == 0 || NULL == szKey)
        {
            return result.SetFail("params err.");
        }
        CloseReg();
        int nRet = RegOpenKeyExA(hKey, szPath, 0, KEY_QUERY_VALUE, &m_hKey);
        if (nRet != ERROR_SUCCESS)
        {
            return result.SetSystemFail(nRet);
        }

        DWORD dwType;
        unsigned char  szData[HN_MAXURL] = { 0 };
        DWORD dwDataLen = sizeof(szData);
        nRet = RegQueryValueExA(m_hKey, szKey, 0, &dwType, szData, &dwDataLen);
        if (nRet != ERROR_SUCCESS)
        {
            return result.SetSystemFail(nRet);
        }
        if (dwType == REG_SZ)
        {
            return result.SetSucc(std::make_pair( dwType,(const char*)szData));
        }
        else if (dwType == REG_DWORD)
        {
            char szValue[36] = { 0 };
            snprintf(szValue, sizeof(szValue), "%d", *((DWORD*)szData));
            return result.SetSucc(std::make_pair( dwType,(const char*)szValue));
        }
        else
        {
            return  result.SetFail(-99,"value  type is unsupported.dwType:%d", dwType);
        }
    }

    /**
    *@brief     写入注册表 ，只支持REG_SZ和REG_DWORD
    *@param     hKey    [in]    如 HKEY_LOCAL_MACHINE
    *@param     szPath  [in]    子路径
    *@param     szKey   [in]    key
    *@param     szValue [in]    value
    *@return    是否成功
    */
    CHNResult<> WriteReg(HKEY hKey, const char* szPath, const char* szKey, const char* szValue)
    {
        CHNResult<> result;
        if (szPath == NULL || szPath[0] == 0 || NULL == szKey || szKey[0] == 0 || NULL == szValue)
        {
            return result.SetFail("params err");
        }
        CloseReg();
        int nRet = RegOpenKeyExA(hKey, szPath, 0, KEY_ALL_ACCESS, &m_hKey);
        if (nRet != ERROR_SUCCESS)
        {
            nRet = RegCreateKeyA(hKey, szPath, &m_hKey);
            if (nRet != ERROR_SUCCESS)
            {
                return result.SetSystemFail(nRet);
            }
        }

        DWORD dwType;
        DWORD dwDataLen = 0;
        nRet = RegQueryValueExA(m_hKey, szKey, 0, &dwType, NULL, &dwDataLen);
        if (nRet != ERROR_SUCCESS)
        {
            nRet = RegSetValueExA(m_hKey, szKey, 0, REG_SZ, (LPBYTE)szValue, (DWORD)strlen(szValue));
            if (nRet != ERROR_SUCCESS) return result.SetSystemFail(nRet);
        }
        else
        {
            if (dwType == REG_SZ)
            {
                nRet = RegSetValueExA(m_hKey, szKey, 0, REG_SZ, (LPBYTE)szValue, (DWORD)strlen(szValue));
                if (nRet != ERROR_SUCCESS) return result.SetSystemFail();
            }
            else if (dwType == REG_DWORD)
            {
                DWORD dwValue = atoi(szValue);
                nRet = RegSetValueExA(m_hKey, szKey, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(DWORD));
                if (nRet != ERROR_SUCCESS) return result.SetSystemFail();
            }
            else
            {
                //不支持的写入类型
                return result.SetFail(-99,"type unsupported.dwType:%d", dwType);
            }
        }
        return result;
    }

    /**
    *@brief     读取注册表，只支持REG_DWORD
    *@param     hKey    [in]    如 HKEY_LOCAL_MACHINE
    *@param     szPath  [in]    子路径
    *@param     szKey   [in]    key
    *@return    成功：int[结果]
    */
    CHNResult<int> ReadRegDWORD(HKEY hKey, const char* szPath, const char* szKey)
    {
        CHNResult<int> result;
        if (szPath == NULL || szPath[0] == 0 || NULL == szKey || szKey[0] == 0)
        {
            return result.SetFail("params err.");
        }
        CloseReg();
        int nRet = RegOpenKeyExA(hKey, szPath, 0, KEY_QUERY_VALUE, &m_hKey);
        if (nRet != ERROR_SUCCESS)
        {
            return result.SetSystemFail(nRet);
        }

        DWORD dwType, dwData;
        DWORD dwDataLen = sizeof(dwData);
        nRet = RegQueryValueExA(m_hKey, szKey, 0, &dwType, (LPBYTE)&dwData, &dwDataLen);
        if (nRet != ERROR_SUCCESS)
        {
            return result.SetSystemFail(nRet);
        }
        if (dwType == REG_DWORD)
        {
            return result.SetSucc((int)dwData);
        }
        else
        {
            return  result.SetFail(-99,"value  type is unsupported.dwType:%d", dwType);
        }
    }

    /**
    *@brief     枚举注册表节点
    *@param     hKey    [in]    如 HKEY_LOCAL_MACHINE
    *@param     szPath  [in]    子路径
    *@param     vecDirs [out]   输出对应子目录的列表
    *@param     vecFiles[out]   输出对应子文件的列表
    *@return    成功：CHNResult.Succ    失败：CHNResult.ErrDesc[错误描述]
    */
    CHNResult<> ListReg(HKEY hKey, const char* szPath, std::vector<std::string>& vecDirs, std::vector<std::string>& vecFiles)
    {
        CHNResult<> result;
        vecDirs.clear();
        vecFiles.clear();
        if (szPath == NULL || szPath[0] == 0)
        {
            return result.SetFail("params err");
        }
        CloseReg();
        int nRet = RegOpenKeyExA(hKey, szPath, 0, KEY_READ, &m_hKey);
        if (nRet != ERROR_SUCCESS)
        {
            return result.SetSystemFail(nRet);
        }

        //查询节点信息
        DWORD   dwSubKeys;
        DWORD   dwbMaxSubKeyLen;
        DWORD   dwValues;
        DWORD   dwbMaxValueNameLen;
        nRet = RegQueryInfoKeyA(m_hKey, NULL, NULL, NULL,
            &dwSubKeys, &dwbMaxSubKeyLen, NULL,
            &dwValues, &dwbMaxValueNameLen, NULL, NULL, NULL);
        if (ERROR_SUCCESS != nRet)
        {
            return result.SetSystemFail(nRet);
        }
        if (dwSubKeys == 0 && dwValues == 0)
        {
            //全空
            return result;
        }

        //查询目录
        DWORD dwBufLen = (dwbMaxSubKeyLen>dwbMaxValueNameLen ? dwbMaxSubKeyLen : dwbMaxValueNameLen) + 1;
        char* szNewValue = new char[dwBufLen];
        CHNAutoObj autoDeleteDir([&szNewValue](){HNDeleteArr(szNewValue); });
        for (DWORD i = 0; i < dwSubKeys; i++)
        {
            memset(szNewValue, 0, dwBufLen);
            DWORD num = dwBufLen;
            nRet = RegEnumKeyExA(m_hKey, i, szNewValue, &num, NULL, NULL, NULL, NULL);
            if (ERROR_SUCCESS != nRet || num >= dwBufLen)
            {
                return result.SetSystemFail(nRet);
            }
            szNewValue[num] = 0;
            vecDirs.push_back(szNewValue);
        }

        //查询文件
        for (DWORD i = 0; i < dwValues; i++)
        {
            memset(szNewValue, 0, dwBufLen);
            DWORD num = dwBufLen;
            DWORD lpType;
            nRet = RegEnumValueA(m_hKey, i, szNewValue, &num, 0, &lpType, NULL, NULL);
            if (ERROR_SUCCESS != nRet || num >= dwBufLen)
            {
                return result.SetSystemFail(nRet);
            }
            szNewValue[num] = 0;
            vecFiles.push_back(szNewValue);
        }
        return result;
    }

private:
    HKEY m_hKey;
};

/**
*@brief     初始化COM的接口
*/
class CHNDCOMInit : CHNNoCopyable
{
public:
    explicit CHNDCOMInit()
    {
#if defined(HNOS_WIN)
        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (FAILED(hr))
        {
            throw ("CoInitializeEx failed");
        }
        hr = CoInitializeSecurity(NULL,                   //Security Descriptor
            -1,                          //COM authentication
            NULL,                        //Authentication services
            NULL,                        //Reserved
            RHN_C_AUTHN_LEVEL_DEFAULT,   //Default authentication
            RHN_C_IMP_LEVEL_IMPERSONATE, //Default Impersonation
            NULL,                        //Authentication info
            EOAC_NONE,                   //Additional capabilities
            NULL);                       //Reserved
        if (FAILED(hr))
        {
            throw ("CoInitializeSecurity failed.");
        }
#endif
    }

    ~CHNDCOMInit()
    {
#if defined(HNOS_WIN)
        CoUninitialize();
#endif
    }
};

/**
*@brief        wmi接口，使用前要求初始化COM(使用CHNDCOMInit类)
*/
class CHNWMIHandle : CHNNoCopyable
{
public:
    explicit CHNWMIHandle()
        :m_wbem(NULL)
        , m_locator(NULL)
    {
    }
    ~CHNWMIHandle()
    {
        Close();
    }

    /**
    *@brief     打开wmi接口
    *@param     machine     [in]    机器名
    *@param     user        [in]    用户名
    *@param     pass        [in]    密码
    *@return    是否成功
    */
    CHNResult<> Open(const char* machine = NULL, const char* user = NULL, const char* pass = NULL)
    {
        CHNResult<> result;
        if (m_wbem)
        {
            return result;
        }

        char path[MAX_PATH] = { 0 };

        HRESULT hr = CoCreateInstance(CLSID_WbemLocator,
            NULL, // IUnknown 
            CLSCTX_INPROC_SERVER,
            IID_IWbemLocator,
            (LPVOID *)&m_locator);
        if (FAILED(hr))
        {
            return result.SetSystemFail((int)hr);
        }

        if (machine == NULL)
        {
            machine = ".";
        }
        sprintf(path, "\\\\%s\\ROOT\\CIMV2", machine);
        hr = m_locator->ConnectServer(bstr_t(path),      //Object path of WMI namespace
            bstr_t(user),    //User name. NULL = current user
            bstr_t(pass),    //User password. NULL = current
            NULL,            //Locale. NULL indicates current
            0,               //Security flags
            NULL,            //Authority (e.g. Kerberos)
            NULL,            //Context object
            &m_wbem);        //pointer to IWbemServices proxy

        if (FAILED(hr))
        {
            return result.SetSystemFail((int)hr);
        }
        return result;
    }

    /**
    *@brief     关闭wmi接口
    */
    void Close()
    {
        if (m_wbem)
        {
            m_wbem->Release();
            m_wbem = NULL;
        }
        if (m_locator)
        {
            m_locator->Release();
            m_locator = NULL;
        }
    }

    /**
    *@brief     从wmi接口获取信息
    *@param     object  [in]    要获取的对象
    *@param     name    [in]    对象的信息名
    *@return    是否成功，成功后信息保存在 StrResult1()
    */
    CHNResult<std::string> GetPropertyString(const char* object, const wchar_t *name)
    {
        CHNResult<std::string> result;
        if (object == NULL || name == NULL)
        {
            return result.SetFail("params NULL.");
        }
        if (m_wbem == NULL)
        {
            return result.SetFail("not open wmi.");
        }

        IWbemClassObject *obj = NULL;
        HRESULT hr = m_wbem->GetObject(bstr_t(object), 0, 0, &obj, 0);
        if (FAILED(hr))
        {
            return result.SetSystemFail((int)hr);
        }
        CHNAutoObj autoRelease([&obj](){obj->Release(); });

        VARIANT var;
        hr = obj->Get(name, 0, &var, 0, 0);
        if (FAILED(hr))
        {
            return result.SetSystemFail((int)hr);
        }
        if (var.vt == VT_NULL)
        {
            VariantClear(&var);
            return result.SetFail("not this message or invalid arg or not permission.");
        }
        _bstr_t b = var.bstrVal;
        result.SetSucc((const char*)b);
        VariantClear(&var);
        return result;
    }

    /**
    *@brief     从wmi接口获取信息
    *@param     wql     [in]    查询语句,如：SELECT * FROM Win32_OperatingSystem
    *                               对应的wmic命令行命令为：wmic os get 列名
    *                               区别是：wql使用完整名Win32_OperatingSystem，而wmic命令采用短别名os
    *                               可以通过wmic /?查询所有短别名
    *                               可以通过wmic alias list brief查询所有短别名对应的完整名
    *@param     col     [in]    列名,如：Name
    *                               可以通过wmic os list brief 查询os的所有列名
    *                               其中os是Win32_OperatingSystem完整名的别名
    *@param     vecValue[out]   结果数组
    *@param     type    [in]    查询类型(可选，默认WQL）
    *@return    是否成功
    */
    CHNResult<> WQLQuery(const char *wql, const wchar_t* col, std::vector<std::string>& vecValue, const char* type = "WQL")
    {
        CHNResult<> result;
        vecValue.clear();
        if (type == NULL || wql == NULL)
        {
            return result.SetFail("params NULL.");
        }
        if (m_wbem == NULL)
        {
            return result.SetFail("not open wmi.");
        }
        HRESULT  hr = CoSetProxyBlanket(
            m_wbem,                       // the proxy to set
            RHN_C_AUTHN_WINNT,            // authentication service
            RHN_C_AUTHZ_NONE,             // authorization service
            NULL,                         // Server principal name
            RHN_C_AUTHN_LEVEL_CALL,       // authentication level
            RHN_C_IMP_LEVEL_IMPERSONATE,  // impersonation level
            NULL,                         // client identity 
            EOAC_NONE                     // proxy capabilities     
            );
        if (FAILED(hr))
        {
            return result.SetSystemFail((int)hr);
        }
        IEnumWbemClassObject* pEnumerator = NULL;
        hr = m_wbem->ExecQuery(
            bstr_t(type),
            bstr_t(wql),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pEnumerator);
        if (FAILED(hr))
        {
            return result.SetSystemFail((int)hr);
        }
        CHNAutoObj autoReleaseEm([&pEnumerator](){pEnumerator->Release(); });

        while (pEnumerator)
        {
            IWbemClassObject *pclsObj;
            ULONG uReturn = 0;
            hr = pEnumerator->Next(WBEM_INFINITE, 1,
                &pclsObj, &uReturn);
            if (FAILED(hr))
            {
                return result.SetSystemFail((int)hr);
            }
            if (0 == uReturn)
            {
                break;
            }
            CHNAutoObj autoReleaseCls([&pclsObj](){pclsObj->Release(); });

            VARIANT vtProp;
            hr = pclsObj->Get(col, 0, &vtProp, 0, 0);
            if (FAILED(hr))
            {
                return result.SetSystemFail((int)hr);
            }
            CHNAutoObj autoClearProp([&vtProp](){VariantClear(&vtProp); });

            char szPropNumber[16] = { 0 };
            switch (vtProp.vt)
            {
            case VT_I4:
                snprintf(szPropNumber, sizeof(szPropNumber) - 1, "%ld", vtProp.lVal);
                vecValue.push_back(szPropNumber);
                break;
            case VT_BOOL:
                vecValue.push_back(vtProp.boolVal ? "true" : "false");
                break;
            case VT_BSTR:
                vecValue.push_back((const char*)(_bstr_t)vtProp.bstrVal);
                break;
            default:
                return result.SetFail("unsupported type:%u", (unsigned int)vtProp.vt);
            }
        }
        return result;
    }

    /**
    *@brief     取进程信息
    *@param     pid     [in]    进程id
    *@param     name    [in]    进程信息名，如 L"ExecutablePath" L"CommandLine"
    *@return    是否成功，成功后信息保存在 StrResult1()
    */
    CHNResult<std::string> GetProcPropertyString(int pid, const wchar_t *name)
    {
        char queryhandle[64] = { 0 };
        sprintf(queryhandle, "Win32_Process.Handle=%d", pid);
        return GetPropertyString(queryhandle, name);
    }

protected:
    IWbemServices *m_wbem;
    IWbemLocator  *m_locator;
};

/**
*@brief      自动释放的gdi环境。
*            调用gdi函数时，整个操作必须在CHNAutoGdi对象的作用域中
*/
class CHNAutoGdi : CHNNoCopyable
{
public:
    /**
    *@brief     构造gdi环境。
    *@param     quality        [in]    图片质量(0,100]
    */
    explicit CHNAutoGdi(unsigned long quality = 100)
        :m_nStartedStatus(Gdiplus::GenericError)
        , m_quality(quality)
    {
        GdiInit();
        if (m_quality == 0 || m_quality > 100)
        {
            m_quality = 100;
        }
    }
    ~CHNAutoGdi() throw ()
    {
        if (Gdiplus::Ok == m_nStartedStatus)
        {
            Gdiplus::GdiplusShutdown(m_gdiplusToken);
        }
    }

    /**
    *@brief     按format格式保存srcpath图片到destpath
    *@param     srcpath     [in]    原始文件路径
    *@param     destpath    [in]    目标文件路径
    *@param     format      [in]    图片格式如：L"image/jpeg"    L"image/bmp"    L"image/tiff"
    *@return    是否成功
    */
    CHNResult<> SavePic(const char * srcpath, const char * destpath, const wchar_t* format = L"image/jpeg")
    {
        CHNResult<> result = GdiInit();
        if (!result)
        {
            return result;
        }
        std::wstring wcsSrcImg;
        result = CHNConvertHelper::HNA2W(wcsSrcImg, srcpath);
        if (!result)
            return result;
        Gdiplus::Image image(wcsSrcImg.c_str());
        return GdiSavePicImg(&image, destpath, format);
    }

    /**
    *@brief     将srcfile图片的宽和高分别缩放至wRatio和hRatio比例根据format保存到destfile。
    *@param     srcfile     [in]    原始文件路径
    *@param     destfile    [in]    目标文件路径
    *@param     wRatio      [in]    宽度缩放比例。比如0.1说明将宽度缩放至10%
    *@param     hRatio      [in]    高度缩放比例。比如0.1说明将高度缩放至10%
    *@param     format      [in]    图片格式如：L"image/jpeg"    L"image/bmp"    L"image/tiff"
    *@return    是否成功
    */
    CHNResult<> ScalePic(const char * srcfile, const char * destfile, double wRatio = 1.0, double hRatio = 1.0, const wchar_t* format = L"image/jpeg")
    {
        CHNResult<> result = GdiInit();
        if (!result)
        {
            return result;
        }
        if (wRatio <= 0 || hRatio <= 0 || wRatio > 1000 || hRatio > 1000)
        {
            return result.SetFail("ratio invalid. [0,1000]");
        }

        std::wstring wcsSrcImg;
        result = CHNConvertHelper::HNA2W(wcsSrcImg, srcfile);
        if (!result)
            return result;

        Gdiplus::Image image(wcsSrcImg.c_str());
        if (image.GetType() == Gdiplus::ImageTypeUnknown)
        {
            return result.SetFail("image(%s) == ImageTypeUnknown", srcfile);
        }
        double destw = image.GetWidth()  * wRatio;
        double desth = image.GetHeight() * hRatio;
        Gdiplus::Image* imageNew = image.GetThumbnailImage((unsigned int)destw, (unsigned int)desth, NULL, NULL);
        if (!imageNew)
        {
            return result.SetFail("GetThumbnailImage(%s > w %u,h %u) fail.", srcfile, (unsigned int)destw, (unsigned int)desth);
        }
        result = GdiSavePicImg(imageNew, destfile, format);
        delete imageNew;
        return result;
    }

    /**
    *@brief     将srcfile图片的旋转iRot并根据format保存到destfile。
    *@param     srcfile     [in]    原始文件路径
    *@param     destfile    [in]    目标文件路径
    *@param     iRot        [in]    旋转参数
    *@param     format      [in]    图片格式如：L"image/jpeg"    L"image/bmp"    L"image/tiff"
    *@return    是否成功
    */
    CHNResult<> RotatePic(const char * srcfile, const char * destfile, Gdiplus::RotateFlipType iRot, const wchar_t* format = L"image/jpeg")
    {
        CHNResult<> result = GdiInit();
        if (!result)
        {
            return result;
        }

        std::wstring wcsSrcImg;
        result = CHNConvertHelper::HNA2W(wcsSrcImg, srcfile);
        if (!result)
            return result;

        Gdiplus::Image image(wcsSrcImg.c_str());
        if (image.GetType() == Gdiplus::ImageTypeUnknown)
        {
            return result.SetFail("image(%s) == ImageTypeUnknown", srcfile);
        }

        Gdiplus::Status st = image.RotateFlip(iRot);
        if (Gdiplus::Ok != st)
        {
            return result.SetFail("RotateFlip(%s , iRot %d) fail.Status:%d", srcfile, (int)iRot, st);
        }
        return GdiSavePicImg(&image, destfile, format);
    }

public:
    /**
    *@brief     初始化GDI+
    */
    CHNResult<> GdiInit()
    {
        //判断是否已经初始化过
        CHNResult<> result;
        if (Gdiplus::Ok == m_nStartedStatus)
        {
            return result;
        }

        //GDI+初始化
        m_nStartedStatus = Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);
        if (Gdiplus::Ok != m_nStartedStatus)
        {
            return result.SetFail("GdiplusStartup fail(m_nStartedStatus:%d).", m_nStartedStatus);
        }
        return result;
    }
    /**
    *@brief     获取图片处理器的class id
    *@param     format       [in]    图片格式如：L"image/jpeg"    L"image/bmp"    L"image/tiff"
    *@param     clsid        [out]   输出class id
    *@return    是否成功
    */
    CHNResult<> GdiGetEncoderClsid(const wchar_t* format, CLSID* clsid)
    {
        CHNResult<> result = GdiInit();
        if (!result)
        {
            return result;
        }

        unsigned int num = 0;
        unsigned int size = 0;
        Gdiplus::Status st = Gdiplus::GetImageEncodersSize(&num, &size);
        if (st != Gdiplus::Ok || num == 0 || size == 0)
        {
            return result.SetFail("GetImageEncodersSize fail.st:%d", st);
        }
        Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
        if (pImageCodecInfo == NULL)
        {
            return result.SetFail("malloc ImageCodecInfo fail.st:%d", st);
        }
        CHNAutoObj autoFreeCodec([&pImageCodecInfo](){free(pImageCodecInfo); });

        st = Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);
        if (st != Gdiplus::Ok)
        {
            return result.SetFail("GetImageEncoders fail.st:%d", st);
        }
        for (unsigned int i = 0; i < num; ++i)
        {
            if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0)
            {
                *clsid = pImageCodecInfo[i].Clsid;
                return result;
            }
        }
        return result.SetFail("not find this format of codec");
    }

    /**
    *@brief     按format格式保存image图片到destpath
    *@param     image       [in]    image对象
    *@param     destpath    [in]    目标文件路径
    *@param     format      [in]    图片格式如：L"image/jpeg"    L"image/bmp"    L"image/tiff"
    *@return    是否成功
    */
    CHNResult<> GdiSavePicImg(Gdiplus::Image * image, const char * destpath, const wchar_t* format)
    {
        CHNResult<> result = GdiInit();
        if (!result)
        {
            return result;
        }
        std::wstring wcsDestImg;
        result = CHNConvertHelper::HNA2W(wcsDestImg, destpath);
        if (!result)
            return result;

        if (image->GetType() == Gdiplus::ImageTypeUnknown)
        {
            return result.SetFail("image type == ImageTypeUnknown");
        }
        CLSID encoderClsid;
        result = GdiGetEncoderClsid(format, &encoderClsid);
        if (!result)
        {
            return result;
        }
        ULONG quality = m_quality;
        Gdiplus::EncoderParameters encoderParameters;
        encoderParameters.Count = 1;
        encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
        encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
        encoderParameters.Parameter[0].NumberOfValues = 1;
        encoderParameters.Parameter[0].Value = &quality;
        Gdiplus::Status st = image->Save(wcsDestImg.c_str(), &encoderClsid, &encoderParameters);
        if (Gdiplus::Ok != st)
        {
            return result.SetFail("save file(%s) fail.st:%d", destpath, st);
        }
        return result;
    }
private:
    Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
    ULONG_PTR        m_gdiplusToken;
    Gdiplus::Status m_nStartedStatus;
    ULONG            m_quality;
};

#endif // __HELLONET_M_CORE_S_OS_WIN__