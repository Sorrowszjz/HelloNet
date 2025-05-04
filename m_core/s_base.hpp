#if defined(_MSC_VER) 
#pragma once
#endif


#ifndef __HELLONET_M_CORE_S_BASE__
#define __HELLONET_M_CORE_S_BASE__

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__) ) 
    #define HNOS_MAC
#elif defined(__ANDROID__) || defined(ANDROID)
    #define HNOS_ANDROID
    #define HNOS_LINUX
#elif defined(__CYGWIN__)
    #define HNOS_CYGWIN
#elif !defined(SAG_COM) && (!defined(WINAPI_FAMILY) || WINAPI_FAMILY==WINAPI_FAMILY_DESKTOP_APP) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
    #define HNOS_WIN64
#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
    #if defined(WINCE) || defined(_WIN32_WCE)
        #define HNOS_WINCE
    #elif defined(WINAPI_FAMILY)
        #if defined(WINAPI_FAMILY_PHONE_APP) && WINAPI_FAMILY==WINAPI_FAMILY_PHONE_APP
            #define HNOS_WINPHONE
            #define HNOS_WINRT
        #else
            #define HNOS_WINRT
        #endif
    #else
        #define HNOS_WIN32
    #endif
#elif defined(__sun) || defined(sun)
    #define HNOS_SOLARIS
#elif defined(hpux) || defined(__hpux)
    #define HNOS_HPUX
#elif defined(__ultrix) || defined(ultrix)
    #define HNOS_ULTRIX
#elif defined(sinix)
    #define HNOS_RELIANT
#elif defined(__native_client__)
    #define HNOS_NACL
#elif defined(__linux__) || defined(__linux)
    #define HNOS_LINUX
#elif defined(__FreeBSD__) || defined(__DragonFly__) || defined(__FreeBSD_kernel__)
    #ifndef __FreeBSD_kernel__
        #define HNOS_FREEBSD
    #endif
    #define HNOS_FREEBSD_KERNEL
    #define HNOS_BSD4
#elif defined(__NetBSD__)
    #define HNOS_NETBSD
    #define HNOS_BSD4
#elif defined(__OpenBSD__)
    #define HNOS_OPENBSD
    #define HNOS_BSD4
#elif defined(__bsdi__)
    #define HNOS_BSDI
    #define HNOS_BSD4
#elif defined(__INTERIX)
    #define HNOS_INTERIX
    #define HNOS_BSD4
#elif defined(__sgi)
    #define HNOS_IRIX
#elif defined(__osf__)
    #define HNOS_OSF
#elif defined(_AIX)
    #define HNOS_AIX
#elif defined(__Lynx__)
    #define HNOS_LYNX
#elif defined(__GNU__)
    #define HNOS_HURD
#elif defined(__DGUX__)
    #define HNOS_DGUX
#elif defined(__QNXNTO__)
    #define HNOS_QNX
#elif defined(_SEQUENT_)
    #define HNOS_DYNIX
#elif defined(_SCO_DS)
    #define HNOS_SCO
#elif defined(__USLC__) 
    #define HNOS_UNIXWARE
#elif defined(__svr4__) && defined(i386)
    #define HNOS_UNIXWARE
#elif defined(__INTEGRITY)
    #define HNOS_INTEGRITY
#elif defined(VXWORKS)
    #define HNOS_VXWORKS
#elif defined(__HAIKU__)
    #define HNOS_HAIKU
#elif defined(__MAKEDEPEND__)
#else
    #error    "THIS OS NOT SUPPORTETD."
#endif

#if defined(HNOS_WIN32) || defined(HNOS_WIN64) || defined(HNOS_WINCE) || defined(HNOC_WINRT) || defined(Q_OS_WIN) || defined(Q_CC_MSVC) 
    #define HNOC_WIN
    #if !defined(_WINDOWS)
        #define _WINDOWS
    #endif
#endif
#if defined(HNOC_MAC)
    #include <TargetConditionals.h>
    #if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
        #define HNOC_IOS
    #elif defined(TARGET_OS_MAC) && TARGET_OS_MAC
        #if defined(__LP64__) 
            #define HNOC_MAC64
        #else
            #define HNOC_MAC32
        #endif
    #endif
#endif

///////////////////////////////////////////////////////////////////////////////
// 识别程序是32位还是64位
#ifndef HN_VAR_64BIT
#if defined(__LP64__) || (defined(__x86_64__) && defined(__ILP32__)) || defined(_WIN64) || defined(__EMSCRIPTEN__)
#define HN_VAR_64BIT 1
#else
#define HN_VAR_64BIT 0
#endif
#endif // HN_VAR_64BIT

///////////////////////////////////////////////////////////////////////////////
// 识别大小端
#define HN_LITTLEENDIAN  0   //!< 小端。如果HN_ENDIAN==HN_LITTLEENDIAN，则机器为小端
#define HN_BIGENDIAN     1   //!< 大端。如果HN_ENDIAN==HN_BIGENDIAN，则机器为大端

//! 机器大小端
/*!
    GCC 4.6提供了用于检测目标计算机字节序的宏。 但是其他
     编译器可能没有这个。 用户可以将HN_ENDIAN定义为
     HN_LITTLEENDIAN或HN_BIGENDIAN。

    参考：
    \li https://gcc.gnu.org/onlinedocs/gcc-4.6.0/cpp/Common-Predefined-Macros.html
    \li http://www.boost.org/doc/libs/1_42_0/boost/detail/endian.hpp
*/
#ifndef HN_ENDIAN
// 使用GCC 4.6的宏进行检测
#  ifdef __BYTE_ORDER__
#    if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#      define HN_ENDIAN HN_LITTLEENDIAN
#    elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#      define HN_ENDIAN HN_BIGENDIAN
#    else
#      error Unknown machine endianess detected. User needs to define HN_ENDIAN.
#    endif // __BYTE_ORDER__
// 使用GLIBC的endian.h进行检测
#  elif defined(__GLIBC__)
#    include <endian.h>
#    if (__BYTE_ORDER == __LITTLE_ENDIAN)
#      define HN_ENDIAN HN_LITTLEENDIAN
#    elif (__BYTE_ORDER == __BIG_ENDIAN)
#      define HN_ENDIAN HN_BIGENDIAN
#    else
#      error Unknown machine endianess detected. User needs to define HN_ENDIAN.
#   endif // __GLIBC__
// 使用_LITTLE_ENDIAN和_BIG_ENDIAN宏进行检测
#  elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
#    define HN_ENDIAN HN_LITTLEENDIAN
#  elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
#    define HN_ENDIAN HN_BIGENDIAN
// 使用架构宏进行检测
#  elif defined(__sparc) || defined(__sparc__) || defined(_POWER) || defined(__powerHN__) || defined(__pHN__) || defined(__hpux) || defined(__hppa) || defined(_MIPSEB) || defined(_POWER) || defined(__s390__)
#    define HN_ENDIAN HN_BIGENDIAN
#  elif defined(__i386__) || defined(__alpha__) || defined(__ia64) || defined(__ia64__) || defined(_M_IX86) || defined(_M_IA64) || defined(_M_ALPHA) || defined(__amd64) || defined(__amd64__) || defined(_M_AMD64) || defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) || defined(__bfin__)
#    define HN_ENDIAN HN_LITTLEENDIAN
#  elif defined(_MSC_VER) && defined(_M_ARM)
#    define HN_ENDIAN HN_LITTLEENDIAN
#  elif defined(HN_DOXYGEN_RUNNING)
#    define HN_ENDIAN
#  else
#    error Unknown machine endianess detected. User needs to define HN_ENDIAN.   
#  endif
#endif // HN_ENDIAN

/*********************************************************************/
//seg1 包含各操作系统头文件
/*********************************************************************/
#if defined(HNOS_WIN)
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN            //防止winsock冲突，如果你的<windows.h>需要定义在HNquicklib.h之前，请自行在编译器选项中增加此项
    #endif
    #ifndef _WIN32_DCOM  
    #define _WIN32_DCOM                    //WinNT   4.0   or   Win95   w/DCOM  
    #endif  
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT   0x0600
    #endif
    #ifndef PSAPI_VERSION
    #define PSAPI_VERSION 1                //使xp可以使用Psapi.lib
    #endif
    #define NO_WARN_MBCS_MFC_DEPRECATION   //忽略警告：MFC的警告
    #pragma warning(disable:4996)          //忽略警告：sprintf等
    #pragma warning(disable:4100)          //忽略警告：未引用的形参
    #pragma warning(disable:4189)          //忽略警告：局部变量已初始化但不引用
    #pragma warning(disable:4800)          //忽略警告：性能警告
    //#define NOMINMAX                     //Windef.h中定义了max和min，和std::min std::max冲突，可以用这个宏禁止掉Windef.h的

    //库
    #pragma comment(lib, "Advapi32.lib")
    #pragma comment(lib, "Ws2_32.lib")
    #pragma comment(lib, "Crypt32.lib")
    #pragma comment(lib, "shlwapi.lib")
    #pragma comment(lib, "Userenv.lib")
    #pragma comment(lib, "Gdiplus.lib")  
    #pragma comment(lib, "DbgHelp.lib")
    #pragma comment(lib, "Shell32.lib")
    #pragma comment(lib, "urlmon.lib")
    #pragma comment(lib, "wininet.lib")
    #pragma comment(lib, "iphlpapi.lib")
    #pragma comment(lib, "Psapi.lib")
    #pragma comment(lib, "wbemuuid.lib")
    #pragma comment(lib, "comctl32.lib")
    #pragma comment(lib, "user32.lib")
    #pragma comment(lib, "ole32.lib")
    #pragma comment(lib, "version.lib")
	#pragma comment(lib, "Setupapi.lib")

    //头文件
    #include <Winsock2.h>
    #include <IPTypes.h>
    #include <MSWSock.h>
    #include <WS2tcpip.h>
    #include <Iphlpapi.h>
    //winhttp.h部分定义和wininet.h定义冲突。除少数例外，WinINet是WinHTTP的超集。
    //在两者之间进行选择时，除非计划在需要模拟和会话隔离的服务或类似服务的进程中运行，否则应使用WinINet。
    #include <WinInet.h>
    #ifndef __AFX_H__
    //因为mfc里面不能包含windows.h，所以如果在mfc程序里面使用HNlib库，
    //需要先 #include<afxwin.h>， 然后再 #include "HNquicklib.h"
    //建议的做法是将 #include "HNquicklib.h" 放在 stdafx.h 内容的最后
    #include <Windows.h>
    #endif
    #include <CommDlg.h>
    #include <ShellAPI.h>
    #include <sys/timeb.h>
    #include <process.h>
    #include <direct.h>
    #include <Shlwapi.h>
    #include <Shlobj.h>
    #include <io.h>
    #include <winsvc.h>
    #include <rHNsal.h>
    #include <netfw.h>
    #include <tlhelp32.h>
    #include <ShlDisp.h>
    #include <Wincrypt.h>   //Wincrypt.h 需要Windef.h中定义了max和min才能正常编译
    #include <Gdiplus.h>    //Gdiplus.h  需要Windef.h中定义了max和min才能正常编译
	#include <Setupapi.h>
	#include <initguid.h>
#ifndef _DBGHELP_
    #include <imagehlp.h>   //imagehlp.h 不能放在windows.h前面
#endif
    #include <winternl.h>
    #include <winperf.h>
    #include <Psapi.h>
    #include <objbase.h>
    #include <comdef.h>
    #include <wbemidl.h>
    #include <intrin.h>
#else
    #include <dlfcn.h>
    #include <unistd.h>
    #include <utime.h>
    #include <netdb.h>
    #include <dirent.h>
    #include <pthread.h>
    #include <ftw.h>
    #include <sys/time.h> 
    #include <sys/syscall.h>
    #include <sys/wait.h>
    #include <semaphore.h>
    #include <fcntl.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <sys/file.h>
    #include <net/if.h> 
    #include <sys/ioctl.h>
    #include <ifaddrs.h> 
    #include <arpa/inet.h>
    #include <netinet/tcp.h>
    #include <sys/select.h>
    #include <sys/utsname.h>
    #include <sys/statvfs.h>
    #include <sys/mount.h>
    #include <sys/param.h>
    #include <sys/mman.h>
    #include <sys/ipc.h>
#endif
#if defined(HNOS_LINUX)
    #include <sys/eventfd.h>
    #include <sys/epoll.h>
    #include <linux/sockios.h>
    #include <mntent.h>
    #include <endian.h>
	#include <linux/serial.h> //struct serial_struct
	#include <termios.h>
    #if !defined(HNOS_ANDROID)
        //麒麟系统没有这个头文件，UOS有
        //#include <cpuid.h>
    #endif
#endif
#if defined(HNOS_ANDROID)
    #include <android/ndk-version.h>
    #include <android/log.h>
    #include <sys/system_properties.h>
#endif
#if defined(HNOS_MAC)
    #include <mach/mach_init.h>
    #include <mach/message.h>
    #include <mach/kern_return.h>
    #include <mach/mach_host.h>
    #include <mach/mach_traps.h>
    #include <mach/mach_port.h>
    #include <mach/task.h>
    #include <mach/thread_act.h>
    #include <mach/thread_info.h>
    #include <mach/vm_map.h>
    #include <sys/ucred.h>
    #include <sys/sockio.h>
    #include <sys/event.h>
    #include <net/if_dl.h>
    #include <net/if_types.h>
#endif

//通用
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <time.h>  
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>   
#include <sys/stat.h>  
#include <math.h>
#include <float.h>
#include <limits.h>
#include <signal.h>
#if defined(_WIN32) && !defined(__MINGW32__) && \
  (!defined(_MSC_VER) || _MSC_VER<1600) && !defined(__WINE__)
    #include <BaseTsd.h>
    typedef __int8 int8_t;
    typedef unsigned __int8 uint8_t;
    typedef __int16 int16_t;
    typedef unsigned __int16 uint16_t;
    typedef __int32 int32_t;
    typedef unsigned __int32 uint32_t;
    typedef __int64 int64_t;
    typedef unsigned __int64 uint64_t;
#else
    #include <stdint.h>
#endif

//STL
#include <cassert>
#include <cstddef>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include <map>
#include <tuple>
#include <unordered_map>
#include <set>  
#include <memory>
#include <algorithm>
#include <functional>
#include <regex>
#include <locale>  
#include <fstream>  
#include <strstream>
#include <sstream>
#include <limits>
#include <stdexcept>
#include <array>
#include <regex>
#include <functional>
#include <random>
#if defined(_MSC_VER) && _MSC_VER < 1800
//低于vs2013可能不支持这些特性
#else
#include <condition_variable>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#endif

/*********************************************************************/
//seg2 定义了跨平台的宏
/*********************************************************************/

//补齐跨平台宏类型定义
#ifndef FALSE
#define FALSE       0
#endif
#ifndef TRUE
#define TRUE        1
#endif
#ifndef NULL
#define NULL        0
#endif
#ifndef MAX_PATH
#define MAX_PATH    260
#endif
#ifndef WM_USER
#define WM_USER             (0x0400)
#endif
#ifndef CODE_PAGE_GB18030
#define CODE_PAGE_GB18030   54936
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET      (-1)
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR        (-1)
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX          ((uint64_t) -1) /* 2^64-1 */
#endif
#ifndef HN_SIZE_T_MAX
#define HN_SIZE_T_MAX    ( (size_t) -1 ) 
#endif
#ifndef HN_MAX
#define HN_MAX(a,b)         (((a) > (b)) ? (a) : (b))
#endif
#ifndef HN_MIN
#define HN_MIN(a,b)         (((a) < (b)) ? (a) : (b))
#endif

//平台特定函数和类型
#if defined(HNOS_WIN)
#if !defined(HAVE_SSIZE_T) && !defined(_SSIZE_T_) && !defined(_SSIZE_T_DEFINED)
    typedef  intptr_t ssize_t;
    # define _SSIZE_T_
    # define _SSIZE_T_DEFINED
    # define HAVE_SSIZE_T
    #endif
    #ifndef F_OK
    #define F_OK        0    //access参数：仅检查文件是否存在
    #endif
    #ifndef X_OK
    #define X_OK        1    //access参数：检查执行权限。 MSVCRT下不要使用，可能会导致参数错误 
    #endif
    #ifndef W_OK
    #define W_OK        2    //access参数：检查写入权限。 
    #endif
    #ifndef R_OK
    #define R_OK        4    //access参数：检查读取权限。 
    #endif
    #ifndef SIGHUP
    #define SIGHUP      1    //信号
    #endif
    #ifndef SIGKILL
    #define SIGKILL     9    //信号
    #endif
    #ifndef SIGWINCH
    #define SIGWINCH    28    //信号
    #endif

    //TLS，注意！xp下禁止使用，windows动态库内禁止使用。如果你要使用TLS，最好只在linux下使用。
    #ifndef __thread
    #define __thread __declspec(thread)
    #endif

    //动态库导出
    #ifndef DLL_PUBLIC
    #ifdef  BUILD_DLL
    #define DLL_PUBLIC __declspec(dllexport)
    #else
    #define DLL_PUBLIC __declspec(dllimport)
    #endif
    #endif
#else
    #ifndef DLL_PUBLIC
    #define DLL_PUBLIC  __attribute__ ((visibility("default")))
    #endif
    #ifndef SOCKET
    #define SOCKET  int
    #endif
    #ifndef WINAPI
    #define WINAPI    
    #endif  
#endif 

//跨平台系统错误码
#ifndef ERROR_FILE_NOT_FOUND
#define ERROR_FILE_NOT_FOUND   ENOENT
#endif
#ifndef ERROR_PATH_NOT_FOUND
#define ERROR_PATH_NOT_FOUND   ENOENT
#endif
#ifndef ERROR_ALREADY_EXISTS
#define ERROR_ALREADY_EXISTS   EEXIST
#endif
#ifndef EILSEQ
#define EILSEQ              42
#endif

//hellonet库的错误码
#define HNERR_SUCC          (0)
#define HNERR_FAIL          (-1)
#define HNERR_TOOFEW        (-2)
#define HNERR_UNKNOWN       (-90001)
#define HNERR_PARAM         (-90002)
#define HNERR_TIMEOUT       (-90003)
#define HNERR_TYPE          (-90004)
#define HNERR_ABORTED       (-90005)
#define HNERR_NOTINITED     (-90006)
#define HNERR_NOTLOADLIB    (-90007)
#define HNERR_HANDESHAKE    (-90008)
#define HNERR_PARSE         (-90009)
#define HNERR_NOKEY         (-90010)
#define HNERR_KEYMATCH      (-90011)
#define HNERR_UNSUPP        (-90012)
#define HNERR_CLOSED        (-90013)
#define HNERR_UPGRADE       (-90014)
#define HNERR_SEND          (-90015)
#define HNERR_RECV          (-90016)
#define HNERR_UNCOMPLETE    (-90017)
#define HNERR_TOOBIG        (-90018)
#define HNERR_TOOSMALL      (-90019)
#define HNERR_EVENTQUEUE    (-90020)
#define HNERR_INVALID       (-90021)
#define HNERR_NOTEXECUTE    (-90022)

//hellonet库的宏定义
#define HN_MAXURL           (2048)           //url的最大长度
#define HN_MAXHTTPHEAD      (80*1024)        //http头的最大长度
#define HN_MAXPACK          (4096)           //网络数据包的默认缓冲区长度
#define HN_MAXSTACK         (16384)          //HNlib库在栈上分配的最大空间长度
#define HN_MAXHEAP          (1024*1024*10)   //HNlib库在堆上分配的最大空间长度
#define HN_LOCALHOST        "127.0.0.1"      //本地环回地址
#define HN_MSFORMATSTR      "SSS"            //毫秒格式化的扩展表示，长度必须为3

//系统命令集
#if defined(HNOS_WIN)
#define HN_CMD_SHUTDOWN     "shutdown /f /s /t 0"
#define HN_CMD_REBOOT       "shutdown /f /r /t 0"
#else
//关机指令android略有不同
#if defined(HNOS_ANDROID)
#define HN_CMD_SHUTDOWN "reboot -p"
#else
#define HN_CMD_SHUTDOWN "poweroff"
#endif

#define HN_CMD_REBOOT       "reboot"
#endif

//官方base64字符串
#define HN_BASE64_TABLE  ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/")
//自定义base64字符串
#define HN_MYBS64_TABLE  ("MBJxAb6fdtS+goXzapwRIvW5eHGLFEC02j4nZDrQ9lONY3/TU7KhVcqPyiks1mu8")

//按照windows的SYSTEMTIME定义跨平台可用的时间结构
typedef struct _SYSTEMTIME_WIN {
    unsigned short wYear;
    unsigned short wMonth;
    unsigned short wDayOfWeek;
    unsigned short wDay;
    unsigned short wHour;
    unsigned short wMinute;
    unsigned short wSecond;
    unsigned short wMilliseconds;
} SYSTEMTIME_WIN, *PSYSTEMTIME_WIN, *LPSYSTEMTIME_WIN;

/*********************************************************************/
//seg3 用宏实现的一些功能，主要是为了屏蔽平台差异
/*********************************************************************/

///////////////////////////////////////////////////////////////////////////////
// HN_ALIGN

//! 机器的数据对齐。
/*! param x对齐指针
    一些机器需要严格的数据对齐。 当前默认使用4个字节
    在32位平台上对齐，而在64位平台上对齐8字节。
    用户可以通过定义HN_ALIGN函数宏进行自定义。
*/
#ifndef HN_ALIGN
    #if HN_VAR_64BIT == 1
        #define HN_ALIGN(x) (((x) + static_cast<uint64_t>(7u)) & ~static_cast<uint64_t>(7u))
    #else
        #define HN_ALIGN(x) (((x) + 3u) & ~3u)
    #endif
#endif

///////////////////////////////////////////////////////////////////////////////
// HN_UINT64_C2

//! 通过一对32位integer构造一个64位literal。
/*!
    带有或不带有ULL后缀的64位literal易于产生编译器警告。
    UINT64_C()是C宏，它会导致编译问题。
    使用此宏通过一对32位整数定义64位常量。
*/
#ifndef HN_UINT64_C2
#define HN_UINT64_C2(high32, low32) ((static_cast<uint64_t>(high32) << 32) | static_cast<uint64_t>(low32))
#endif

///////////////////////////////////////////////////////////////////////////////
// HN_48BITPOINTER_OPTIMIZATION

//! 对于某些指针，仅使用低48位地址。
/*!
    这种优化利用了以下事实：当前的X86-64架构仅实现较低的48位虚拟地址。
    高16位可用于存储其他数据。
    使用此优化某些联合体或结构在64位体系结构中将其大小从24字节减小到16字节。
*/
#ifndef HN_48BITPOINTER_OPTIMIZATION
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#define HN_48BITPOINTER_OPTIMIZATION 1
#else
#define HN_48BITPOINTER_OPTIMIZATION 0
#endif
#endif // HN_48BITPOINTER_OPTIMIZATION

#if HN_48BITPOINTER_OPTIMIZATION == 1
#if HN_VAR_64BIT != 1
#error HN_48BITPOINTER_OPTIMIZATION can only be set to 1 when HN_VAR_64BIT=1
#endif
#define HN_SETPOINTER(type, p, x) (p = reinterpret_cast<type *>((reinterpret_cast<uintptr_t>(p) & static_cast<uintptr_t>(HN_UINT64_C2(0xFFFF0000, 0x00000000))) | reinterpret_cast<uintptr_t>(reinterpret_cast<const void*>(x))))
#define HN_GETPOINTER(type, p) (reinterpret_cast<type *>(reinterpret_cast<uintptr_t>(p) & static_cast<uintptr_t>(HN_UINT64_C2(0x0000FFFF, 0xFFFFFFFF))))
#else
#define HN_SETPOINTER(type, p, x) (p = (x))
#define HN_GETPOINTER(type, p) (p)
#endif


///////////////////////////////////////////////////////////////////////////////
//优化分支预测
#if (defined(__GNUC__) && __GNUC__ >= 3) || defined(__clang__)
#ifndef LIKELY
#define LIKELY(X)   __builtin_expect(!!(X), 1)
#endif    
#ifndef UNLIKELY
#define UNLIKELY(X) __builtin_expect(!!(X), 0)
#endif    
#else
#ifndef LIKELY
#define LIKELY(X) (X)
#endif    
#ifndef UNLIKELY
#define UNLIKELY(X) (X)
#endif    
#endif



//切换到目标文件夹，返回值：成功0 失败-1
#if defined(HNOS_WIN)
#define HNChDir(_DIRNAME)    ::_chdir(_DIRNAME)
#else
#define HNChDir(_DIRNAME)    ::chdir(_DIRNAME)
#endif

//休眠指定毫秒数，android下有被打断的可能，使用时注意
#if defined(_MSC_VER) && _MSC_VER < 1800
#define HNSleep(_MSEC)    ::Sleep(_MSEC)
#else
#define HNSleep(_MSEC)    std::this_thread::sleep_for(std::chrono::milliseconds(_MSEC))
//::usleep((_MSEC) * 1000)
#endif

//获取最后的错误码
#if defined(HNOS_WIN)
#define HNGetLastError()    GetLastError()
#else
#define HNGetLastError()    errno
#endif


//补齐window下的strcasecmp strncasecmp vsnprintf
#if defined(HNOS_WIN)
#ifndef strcasecmp
#define strcasecmp      _stricmp
#endif
#ifndef strncasecmp
#define strncasecmp     _strnicmp
#endif
#ifndef vsnprintf
#define vsnprintf       _vsnprintf
#endif
#endif

/**
*@brief     分配一段内存(数组)，用opBuffer指针指向这段内存
*@param     _TYPE_[in]    要new的类型，如char代表分配char数组
*@param     _LENGTH_[in]  需要的空间长度，实际分配的内存会保证大于这个长度
*@param     _CODE_[in]    使用这段内存的代码
*@note      内存分配方式：大于1024字节的内存在堆上分配，否则在栈上分配
*@note      使用这个宏分配的内存未初始化，需要注意
*/
#define HN_OPTIMIZE_NEWBUF(_TYPE_,_LENGTH_,_CODE_)                              \
do                                                                              \
{                                                                               \
    if(_LENGTH_>1024)                                                           \
        {                                                                       \
        _TYPE_ * opBuffer = new _TYPE_[_LENGTH_+2];                             \
        CHNAutoObj autoDelelteOpBuffer([&opBuffer](){HNDeleteArr(opBuffer);});  \
        _CODE_                                                                  \
        }                                                                       \
        else                                                                    \
        {                                                                       \
        _TYPE_ opBuffer[1024+2];                                                \
        _CODE_                                                                  \
        }                                                                       \
}                                                                               \
while(0);

/**
*@brief     处理c语言变长参数的通用方法，会自动优化<=1024短数据的内存使用性能
*@param     _FMT_[in]    变长的...参数之前的那个参数名称，一般是fmt
*@param     _CODE_[in]   格式化后使用这段格式化数据的代码
*@note      _CODE_里面可以使用两个变量：
*               opBuffer        格式化后的buffer，已自动添加\0（自动释放）
*               nOpBufferSize   格式化后的buffer长度
*/
#define HN_FORMAT_PROCESS(_FMT_,_CODE_)                                       \
do                                                                            \
{                                                                             \
    int nOpBufferSize = -1;                                                   \
    va_list argptrProcess;                                                    \
    va_start(argptrProcess, _FMT_);                                           \
    nOpBufferSize = (int)vsnprintf(NULL, 0, _FMT_, argptrProcess);            \
    va_end(argptrProcess);                                                    \
    assert(nOpBufferSize >= 0);                                               \
    if(nOpBufferSize > 1024)                                                  \
    {                                                                         \
        char * opBuffer = new char[nOpBufferSize+2];                          \
        CHNAutoObj autoDeleteOpBuffer([&opBuffer](){delete[] opBuffer;});     \
        va_start(argptrProcess, _FMT_);                                       \
        nOpBufferSize = (int)vsnprintf(opBuffer, nOpBufferSize+1, _FMT_, argptrProcess);  \
        va_end(argptrProcess);                                                \
        opBuffer[nOpBufferSize] = 0;                                          \
        _CODE_                                                                \
    }                                                                         \
    else                                                                      \
    {                                                                         \
        char opBuffer[1024+2];                                                \
        va_start(argptrProcess, _FMT_);                                       \
        nOpBufferSize = (int)vsnprintf(opBuffer, 1024+1, _FMT_, argptrProcess);         \
        va_end(argptrProcess);                                                \
        opBuffer[nOpBufferSize] = 0;                                          \
        _CODE_                                                                \
    }                                                                         \
}                                                                             \
while(0);

//释放资源
#if defined(HNOS_WIN)
    #define HNCloseHandle(h)    { if(h != NULL && h != INVALID_HANDLE_VALUE) { CloseHandle(h);    h = NULL; } }
    #define HNLocalFree(h)      { if(h != NULL)        { LocalFree(h);        h = NULL; } }
    #define HNFreeLibrary(h)    { if(h != NULL)        { FreeLibrary(h);    h = NULL; } }
    #define HNRegCloseKey(h)    { if(h != NULL)        { RegCloseKey(h);    h = NULL; } }
    #define HNCloseSocket(s)    { if(s != INVALID_SOCKET)    { ::closesocket(s);    s = (int)INVALID_SOCKET; } }
#else
    #define HNCloseSocket(s)    { if(s != INVALID_SOCKET)    { ::close(s);    s = INVALID_SOCKET; } }
#endif
#define        HNDelete(p)      { if(p != NULL)         { delete    p;    p    = NULL;    } }
#define        HNDeleteArr(p)   { if(p != NULL)         { delete [] p;    p    = NULL;    } }
#define        HNCloseFILE(fp)  { if(fp != NULL)        { fclose(fp);    fp    = NULL;    } }

///////////////////////////////////////////////////////////////////////////////
//! 主机字节序和网络字节序转换
/*!
    H：host，主机字节序。
    N：network，网络字节序
    数字：对应的数据字节长度
    如 HNHTON32，代表将32位的整数(int)从主机字节序转化为网络字节序
*/
#if HN_ENDIAN == HN_BIGENDIAN
    #define HNHTON16(val) val
    #define HNNTOH16(val) val
    #define HNHTON32(val) val
    #define HNNTOH32(val) val
    #define HNHTON64(val) val
    #define HNNTOH64(val) val
#else
    #define HNHTON16(val) HNSWAP16(val)
    #define HNNTOH16(val) HNSWAP16(val)
    #define HNHTON32(val) HNSWAP32(val)
    #define HNNTOH32(val) HNSWAP32(val)
    #define HNHTON64(val) HNSWAP64(val)
    #define HNNTOH64(val) HNSWAP64(val)
#endif

//交换两个整数
#define HNSWAP2INT(a,b)     do{ int t = (a); (a) = (b); (b) = t; t = 0; } while( 0 )

//32-bit integer manipulation macros (big endian)
#define HN_PUT_UINT32_BE(n,b,i)                             \
    do {                                                    \
        (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
        (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
        (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
        (b)[(i) + 3] = (unsigned char) ( (n)       );       \
        } while( 0 )

//32-bit integer manipulation macros (big endian)
#define HN_GET_UINT32_BE(n,b,i)                             \
    do {                                                    \
        (n) = ( (unsigned int) (b)[(i)    ] << 24 )         \
            | ( (unsigned int) (b)[(i) + 1] << 16 )         \
            | ( (unsigned int) (b)[(i) + 2] <<  8 )         \
            | ( (unsigned int) (b)[(i) + 3]       );        \
        } while( 0 )


/*********************************************************************/
//seg4 基础的C语言函数
/*********************************************************************/

//重写snprintf，因为<vs2015的_snprintf()在buf满时不会在字符串后面添加\0
#if defined(_MSC_VER) && _MSC_VER < 1900
#ifndef snprintf
static int snprintf(char* buf, size_t len, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = _vscprintf(fmt, ap);
    vsnprintf_s(buf, len, _TRUNCATE, fmt, ap);
    va_end(ap);
    return n;
}
#endif
#endif

//判断cpu是否为大端模式
//小端：x86 cpu,  arm cpu
//大端：power pc, 网络字节序 
//动态实现版本：{ const int n = 1;  if (*(char *)&n)  { return false; }  return true; }
inline static bool HNIsBigEndian()    
{
#if HN_ENDIAN == HN_BIGENDIAN
    return true;
#else
    return false;
#endif
}

//16位swap操作[按字节翻转]
inline static  uint16_t  HNSWAP16(uint16_t number)
{
#if defined(_MSC_VER) && _MSC_VER > 1310
    return _byteswap_ushort(number);
#elif defined(__GNUC__)
    return __builtin_bswap16(number);
#else
    return (number >> 8) | (number << 8);
#endif
}

//64位swap操作[按字节翻转]
inline static uint64_t  HNSWAP64(uint64_t number)
{
#if defined(_MSC_VER) && _MSC_VER > 1310
    return _byteswap_uint64(number);
#elif defined(__GNUC__)
    return __builtin_bswap64(number);
#else
    return  (((number) >> 56) |\
            (((number) & 0x00ff000000000000ll) >> 40) |\
            (((number) & 0x0000ff0000000000ll) >> 24) |\
            (((number) & 0x000000ff00000000ll) >> 8)  |\
            (((number) & 0x00000000ff000000ll) << 8)  |\
            (((number) & 0x0000000000ff0000ll) << 24) |\
            (((number) & 0x000000000000ff00ll) << 40) |\
            (((number) << 56)));
#endif
}

//8位翻转操作[按位翻转]
inline static unsigned char HNINVERT8(unsigned char number)
{
    unsigned char temp = 0;
    for (unsigned char i = 0; i < 8; i++)
    {
        if (number & (1 << i))
        {
            temp |= 1 << (7 - i);
        }
    }
    return temp;
}

//16位翻转操作[按位翻转]
inline static  unsigned short HNINVERT16( unsigned short number)
{
    unsigned short temp = 0;
    for (unsigned char i = 0; i < 16; i++)
    {
        if (number & (1 << i))
        {
            temp |= 1 << (15 - i);
        }
    }
    return temp;
}

//32位翻转操作[按位翻转]
inline static unsigned int HNINVERT32(unsigned int number)  
{
    unsigned int tmp[4];  
    tmp[0] = 0;
    for(unsigned char i=0;i< 32;i++)  
    {  
      if(number& (1 << i))  
        tmp[0]|=1<<(15 - i);  
    }  
    return tmp[0];  
}

/**
*@brief     获取字节数组的异或校验值
*@param     pszSrc    [in]    需做异或的报文
*@param     nSrcLen   [in]    报文长度。
*@return    异或校验值
*/
static unsigned char  HNXORSelf(const unsigned char *pszSrc, size_t nSrcLen)
{
    if (pszSrc == NULL || nSrcLen == 0)
    {
        return 0x00;
    }
    unsigned char ucXorCheckSum = 0x00;
    for (size_t i = 0; i < nSrcLen; i++)
    {
        ucXorCheckSum = ucXorCheckSum ^ pszSrc[i];
    }
    return ucXorCheckSum;
}

/**
*@brief     将szSrc与 szDest 异或运算，结果存放在 szDest 中。既可用做普通字节数组的异或，也可用作整数数组形式的异或。
*@param     szSrc    [in]    需做异或的报文1
*@param     szDest   [out]   需做异或的报文2
*@param     nSize    [in]    做异或报文的长度。
*@return    无
*/
static void  HNXOR2Array(const unsigned char *szSrc, unsigned char *szDest, size_t nSize)
{
    if (szSrc == NULL || szDest == NULL || nSize == 0)
    {
        return;
    }
    for (size_t i = 0; i<nSize; i++)
    {
        szDest[i] ^= szSrc[i];
    }
}

//实现inet_ntop，以支持xp机器
static const char* HN_INET_NTOP(int af, const void *src, char *dst, int size)
{
    if (src == NULL || dst == NULL || size <= 0)
        return NULL;
#if defined(HNOS_WIN)
    if (af == AF_INET)
    {
        struct sockaddr_in  si4;
        memset(&si4, 0, sizeof(struct sockaddr_in));
        si4.sin_family = af;
        si4.sin_addr = *(IN_ADDR*)src;
        DWORD dwSize = size - 1;
        if (0 != WSAAddressToStringA((LPSOCKADDR)&si4, sizeof(sockaddr_in), NULL, dst, &dwSize))
        {
            return NULL;
        }
        dst[dwSize] = 0;
        return dst;
    }
    else if (af == AF_INET6)
    {
        struct sockaddr_in6 si6;
        memset(&si6, 0, sizeof(struct sockaddr_in6));
        si6.sin6_family = af;
        si6.sin6_addr = *(IN6_ADDR*)src;
        DWORD dwSize = size - 1;
        if (0 != WSAAddressToStringA((LPSOCKADDR)&si6, sizeof(sockaddr_in6), NULL, dst, &dwSize))
        {
            return NULL;
        }
        dst[dwSize] = 0;
        return dst;
    }
    else
    {
        ::SetLastError(ERROR_NOT_SUPPORTED);
        return NULL;
    }
#else
    return inet_ntop(af, src, dst, size);
#endif
}

//实现inet_pton，以支持xp机器
static int HN_INET_PTON(int af, const char *csrc, void *dst)
{
#if defined(HNOS_WIN)
    char * src = NULL;
    if (csrc == NULL || dst == NULL || (src = strdup(csrc)) == NULL)
    {
        ::SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    if (af == AF_INET)
    {
        struct sockaddr_in  si4;
        si4.sin_family = AF_INET;
        int s = sizeof(si4);
        if (0 != WSAStringToAddressA(src, af, NULL, (LPSOCKADDR)&si4, &s))
        {
            free(src);
            return -1;
        }
        free(src);
        memcpy(dst, &si4.sin_addr, sizeof(si4.sin_addr));
        return 1;
    }
    else if (af == AF_INET6)
    {
        struct sockaddr_in6 si6;
        si6.sin6_family = AF_INET6;
        int s = sizeof(si6);
        if (0 != WSAStringToAddressA(src, af, NULL, (LPSOCKADDR)&si6, &s))
        {
            free(src);
            return -1;
        }
        free(src);
        memcpy(dst, &si6.sin6_addr, sizeof(si6.sin6_addr));
        return 1;
    }
    else
    {
        free(src);
        ::SetLastError(ERROR_NOT_SUPPORTED);
        return -1;
    }
#else
    return inet_pton(af, csrc, dst);
#endif
}

/**
*@brief     取进程id
*@return    进程pid
*/
inline static int HNGetPID()
{
#if defined(HNOS_WIN)
    return (int)::GetCurrentProcessId();
#else
    return ::getpid();
#endif
}

/**
*@brief     取当前内核线程id，用了TLS，一个线程只会取一次
*@return    当前线程id
*/
static int HNGetTID()
{
#if defined(HNOS_WIN)
    return (int)::GetCurrentThreadId();
#else
    static __thread int s_threadid = -1;
    if (s_threadid != -1)
        return s_threadid;
    s_threadid = ::syscall(SYS_gettid);
    return s_threadid;
#endif
}

/**
*@brief     根据系统错误码得到错误描述
*@param     nErrorNo        [in]    系统错误码
*@return    错误描述。
*/
static  std::string  HNGetSysErrorMsg(int nErrorNo)
{
    char pszErrMsgBuf[MAX_PATH] = { 0 };
    std::string strErrDesc = "NOT FIND ERR DESCRIPTION.";
#if defined(HNOS_WIN)
    LPVOID    lpMsgBuf = NULL;
    DWORD nFormatLen = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, nErrorNo, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&lpMsgBuf, 0, NULL);
    if (nFormatLen > 0)
    {
        strErrDesc.assign((char*)lpMsgBuf, nFormatLen);
    }
    HNLocalFree(lpMsgBuf);
#elif defined(HNOS_ANDROID) && defined(__NDK_MAJOR__) && (__NDK_MAJOR__<20)
    //printf("01--NDK too old, not support strerror_r.\n");
    char * pErrMsg = strerror(nErrorNo);
    if (pErrMsg != NULL)
    {
        strErrDesc = pErrMsg;
    }
#elif (defined(HNOS_MAC) )    || \
        (defined(HNOS_LINUX)    && (defined __USE_XOPEN2K && !defined __USE_GNU )) || \
        (defined(HNOS_ANDROID)  && (__ANDROID_API__ < 23  || !defined __USE_GNU ))
    //printf("02--POSIX\n");
    if (0 == strerror_r(nErrorNo, pszErrMsgBuf, sizeof(pszErrMsgBuf)))
    {
        strErrDesc = pszErrMsgBuf;
    }
#else
    //printf("03--GNU\n");
    char *pErrMsg = strerror_r(nErrorNo, pszErrMsgBuf, sizeof(pszErrMsgBuf));
    if (pErrMsg != NULL)
    {
        strErrDesc = pErrMsg;
    }
#endif
    snprintf(pszErrMsgBuf, sizeof(pszErrMsgBuf), "[CODE:%d]%s", nErrorNo, strErrDesc.c_str());
    return pszErrMsgBuf;
};

/*********************************************************************/
//seg5 基础的C++类
/*********************************************************************/

/**
*@brief      如果MT/MTd模式编译一个动态库，动态库没有用到任何浮点数，
*            但是导出的接口(带可变参数)可能被用户传入浮点数的情况下，
*            动态库里面执行 _vsnprintf(tmp,"%f",1.0f) 类似的浮点值操作会报错崩溃，原因是库里面并没有加载浮点例程
*            解决方案：在库里面随便写一个浮点数，强制编译器加载浮点例程。
*/
class  CHNUselessFloatLoadClass
{ 
public: CHNUselessFloatLoadClass(float f) :m_floatvalue(f){}
private:float m_floatvalue; 
};
static volatile CHNUselessFloatLoadClass G_HN_USELESS_FLOAT_VALUE_DO_NOT_USE(0.01f);

/**
*@brief        不可拷贝类，对于非值类型的对象的类都应该继承此类
*/
class CHNNoCopyable
{
protected:
    CHNNoCopyable() {}
    ~CHNNoCopyable() {}
private:
    CHNNoCopyable(const CHNNoCopyable&);
    CHNNoCopyable& operator=(const CHNNoCopyable&);
};

/**
*@brief     自动释放的对象。
*           调用时只需要传入回调函数，则出了对象作用域后时会自动执行回调函数释放资源。
*           比如一个自动关闭文件的示例：
*           FILE* fp = fopen("1.txt","rb");
*           HN_DEFER([&fp](void)
*           {
*               fclose(fp);
*           });
*           ...后续的业务代码
*/
class CHNAutoObj : CHNNoCopyable
{
public:
    explicit CHNAutoObj(std::function<void(void)> callback)
        :m_callback(callback){}
    ~CHNAutoObj() throw () { if(m_callback) m_callback(); }
private:
    CHNAutoObj(){}
    std::function<void(void)> m_callback;
};
#define HN_LINE_CAT(name,line)  name##line
#define HN_LINE_NAME(name,line) HN_LINE_CAT(name,line)
#define HN_DEFER(callback)      CHNAutoObj HN_LINE_NAME(HN_ON_SCOPE_EXIT,__LINE__)(callback)


/**
*@brief 标准内存分配器
*/
class CHNStdAllocator 
{
public:
    //是否需要主动释放
    static const bool kNeedFree = true;

    //分配
    void* Malloc(size_t size)
    { 
        //如果要分配0，需要调用者自己先处理
        if (size) 
            return std::malloc(size);
        else
            return NULL; 
    }

    //重新分配
    void* Realloc(void* originalPtr, size_t originalSize, size_t newSize) 
    {
        (void)originalSize;
        if (newSize == 0) 
        {
            std::free(originalPtr);
            return NULL;
        }
        return std::realloc(originalPtr, newSize);
    }

    //释放
    static void Free(void *ptr) 
    { 
        std::free(ptr); 
    }
};

/**
*@brief 内存池分配器，默认使用标准内存分配器
*       你可以自己定义自己的内存分配器
*/
template <typename BaseAllocator = CHNStdAllocator>
class CHNMemoryPoolAllocator : CHNNoCopyable
{
public:
    //是否需要主动释放,这里不需要主动调用Free
    static const bool kNeedFree = false;    

    /**
    *@brief     通过chunkSize进行构造
    *@param     chunkSize[in]       内存块大小
    *@param     baseAllocator[in]   分配器
    */
    CHNMemoryPoolAllocator(size_t chunkSize = kDefaultChunkCapacity, BaseAllocator* baseAllocator = 0) 
        : m_chunkHead(0)
        , m_chunk_capacity(chunkSize)
        , m_userBuffer(0)
        , m_baseAllocator(baseAllocator)
        , m_ownBaseAllocator(0)
    {
    }

    /**
    *@brief     通过用户提供的buffer构造
    *@param     buffer[in]          用户buffer
    *@param     size[in]            用户buffer长度，必须大于sizeof(HNChunkHeader)
    *@param     chunkSize[in]       内存块大小
    *@param     baseAllocator[in]   分配器
    */
    CHNMemoryPoolAllocator(void *buffer, size_t size, size_t chunkSize = kDefaultChunkCapacity, BaseAllocator* baseAllocator = 0) 
        : m_chunkHead(0)
        , m_chunk_capacity(chunkSize)
        , m_userBuffer(buffer)
        , m_baseAllocator(baseAllocator)
        , m_ownBaseAllocator(0)
    {
        assert(buffer != 0);
        assert(size > sizeof(HNChunkHeader));
        m_chunkHead = reinterpret_cast<HNChunkHeader*>(buffer);
        m_chunkHead->capacity = size - sizeof(HNChunkHeader);
        m_chunkHead->size = 0;
        m_chunkHead->next = 0;
    }

    /**
    *@brief     析构函数
    *           删除所有分配的块，不包括用户提供的buffer
    */
    ~CHNMemoryPoolAllocator() 
    {
        Clear();
        delete (m_ownBaseAllocator);
    }

    /**
    *@brief     删除所有分配的块，不包括用户提供的buffer
    */
    void Clear() 
    {
        while (m_chunkHead && m_chunkHead != m_userBuffer) 
        {
            HNChunkHeader* next = m_chunkHead->next;
            m_baseAllocator->Free(m_chunkHead);
            m_chunkHead = next;
        }
        if (m_chunkHead && m_chunkHead == m_userBuffer)
            m_chunkHead->size = 0; 
    }

    /**
    *@brief     计算所有分配块的容量
    *@return    容量
    */
    size_t Capacity() const 
    {
        size_t capacity = 0;
        for (HNChunkHeader* c = m_chunkHead; c != 0; c = c->next)
            capacity += c->capacity;
        return capacity;
    }

    /**
    *@brief     获取内存容量
    *@return    总字节
    */
    size_t Size() const 
    {
        size_t size = 0;
        for (HNChunkHeader* c = m_chunkHead; c != 0; c = c->next)
            size += c->size;
        return size;
    }

    /**
    *@brief     分配
    *@return    内存指针
    */
    void* Malloc(size_t size) 
    {
        if (!size)
            return NULL;

        size = HN_ALIGN(size);
        if (m_chunkHead == 0 || m_chunkHead->size + size > m_chunkHead->capacity)
            if (!AddChunk(m_chunk_capacity > size ? m_chunk_capacity : size))
                return NULL;

        void *buffer = reinterpret_cast<char *>(m_chunkHead) + HN_ALIGN(sizeof(HNChunkHeader)) + m_chunkHead->size;
        m_chunkHead->size += size;
        return buffer;
    }

    /**
    *@brief     重新分配
    *@return    内存指针
    */
    void* Realloc(void* originalPtr, size_t originalSize, size_t newSize) 
    {
        if (originalPtr == 0)
            return Malloc(newSize);

        if (newSize == 0)
            return NULL;

        originalSize = HN_ALIGN(originalSize);
        newSize = HN_ALIGN(newSize);

        // 如果新尺寸小于原始尺寸，请勿收缩
        if (originalSize >= newSize)
            return originalPtr;

        // 如果是最后一个分配并且有足够的空间，则只需对其进行扩展
        if (originalPtr == reinterpret_cast<char *>(m_chunkHead) + HN_ALIGN(sizeof(HNChunkHeader)) + m_chunkHead->size - originalSize) 
        {
            size_t increment = static_cast<size_t>(newSize - originalSize);
            if (m_chunkHead->size + increment <= m_chunkHead->capacity) 
            {
                m_chunkHead->size += increment;
                return originalPtr;
            }
        }

        // 重新分配过程：分配和复制内存，不释放原始缓冲区。
        if (void* newBuffer = Malloc(newSize)) {
            if (originalSize)
                memcpy(newBuffer, originalPtr, originalSize);
            return newBuffer;
        }
        else
            return NULL;
    }

    //! 释放内存块（分配器）不需要做任何事
    static void Free(void *ptr) { (void)ptr; }

private:

    /**
    *@brief     创建一个新的块。
    *@param     capacity[in]    块的容量（以字节为单位）
    *@return    是否成功
    */
    bool AddChunk(size_t capacity) 
    {
        if (!m_baseAllocator)
            m_ownBaseAllocator = m_baseAllocator = new BaseAllocator();
        if (HNChunkHeader* chunk = reinterpret_cast<HNChunkHeader*>(m_baseAllocator->Malloc(HN_ALIGN(sizeof(HNChunkHeader)) + capacity)))
        {
            chunk->capacity = capacity;
            chunk->size = 0;
            chunk->next = m_chunkHead;
            m_chunkHead =  chunk;
            return true;
        }
        else
            return false;
    }

    static const int kDefaultChunkCapacity = 64 * 1024; //!< 默认块容量。

    //! 用于附加到每个块的块头。
    /*! 块存储为单个链接列表。
    */
    struct  HNChunkHeader
    {
        size_t capacity;        //!< 块的容量（以字节为单位）（不包括标头本身）。
        size_t size;            //!< 当前已分配内存的大小（以字节为单位）。
        HNChunkHeader *next;    //!< 链表中的下一个块。
    };

    HNChunkHeader* m_chunkHead;  //!< 块链接列表的头。 只有头块服务分配。
    size_t m_chunk_capacity;     //!< 分配块时的最小容量。
    void * m_userBuffer;         //!< 用户提供的缓冲区。
    BaseAllocator* m_baseAllocator;      //!< 用于分配内存块的基本分配器。
    BaseAllocator* m_ownBaseAllocator;   //!< 此对象创建的基本分配器。
};

/**
*@brief  TLS线程局部存储
*/
class CHNTLS : CHNNoCopyable
{
public:
    explicit CHNTLS()
    {
#if defined(HNOS_WIN)
        m_tlsKey = ::TlsAlloc();
#else
        ::pthread_key_create(&m_tlsKey, NULL);
#endif
    }

    ~CHNTLS()
    {
#if defined(HNOS_WIN)
        ::TlsFree(m_tlsKey);
#else
        ::pthread_key_delete(m_tlsKey);
#endif
    }

    /**
    *@brief  将void*存入。
    */
    void Set(void* ptr)
    {
#if defined(HNOS_WIN)
        ::TlsSetValue(m_tlsKey, ptr);
#else
        ::pthread_setspecific(m_tlsKey, ptr);
#endif
    }

    /**
    *@brief  将void*取出。
    */
    void* Get()
    {
#if defined(HNOS_WIN)
        return (void*)::TlsGetValue(m_tlsKey);
#else
        return (void*)::pthread_getspecific(m_tlsKey);
#endif
    }

protected:
#if defined(HNOS_WIN)
    DWORD m_tlsKey;
#else
    pthread_key_t m_tlsKey;
#endif
};

///////////////////////////////////////////////////////////////////////////////////////
//新的结果类，大幅度节省空间/提高灵活性
///////////////////////////////////////////////////////////////////////////////////////

namespace hnlib_internal
{

/**
*@brief  用户错误信息存储的tls空间
*        这个类不要去使用。
*/
class CHNLocalErrInfo : CHNNoCopyable
{
public:
    explicit CHNLocalErrInfo()
    {
        //buffer = nullptr;
        m_tls.Set(NULL);
    }

    ~CHNLocalErrInfo()
    {
        char* ptr = (char*)m_tls.Get();
        if( ptr != nullptr) {
            HNDeleteArr(ptr);
        }
    }

    static CHNLocalErrInfo& obj()
    {
        static CHNLocalErrInfo o;
        return o;
    }

public:
    void SetErrDesc(const char* errdesc,size_t length)
    {
        char* ptr = (char*)m_tls.Get();
        if(ptr == NULL)
        {
            ptr = new char[MAX_PATH];
        }
        if(errdesc != NULL && length > 0 && length < MAX_PATH)
        {
            strncpy(ptr,errdesc,length);
            ptr[length] = 0;
        }
        else
        {
            ptr[0] = 0;
        }
        m_tls.Set((void*)ptr);
    }

    const char* GetErrDesc()
    {
        return (const char*)m_tls.Get();
    }

private:
    CHNTLS m_tls;
    //char* buffer;
};

}//hnlib_internal

/**
*@brief  模板+TLS实现的值语义的可附带任意类型用户数据函数执行结果类，大部分函数都返回此结果对象。
*        构造CHNResult对象时，对象默认为成功结果。
*        调用示例：auto ret = Func1();
*        判断是否成功(直接当作布尔变量使用即可)：if(ret) { //succ }
*        如果有用户数据，获取用户数据：auto mydata = ret.Get();
*@note   T代表可选的用户数据。
*/
template<typename T = bool>
class CHNResult
{
public:
    //默认构造一个成功的实例
    explicit CHNResult() 
        : m_errcode(0)
    {}

    //带用户数据构造一个成功的实例
    explicit CHNResult(const T& userdata)
        : m_errcode(0)
        , m_userdata(userdata)
    {}

    //带用户数据构造一个成功的实例
    explicit CHNResult(T&& userdata)
        : m_errcode(0)
        , m_userdata(std::move(userdata))
    {}

    //实现拷贝构造和拷贝赋值运算符，因为实现移动构造后拷贝构造不会自动生成了，而有些编译器没有实现返回值优化，导致可能还会调用拷贝构造
    CHNResult(const CHNResult& rhs)
        : m_errcode(rhs.m_errcode)
        , m_userdata(rhs.m_userdata)
    {}
    CHNResult& operator=(const CHNResult& rhs)
    {
        if (this == &rhs)
            return *this;
        m_errcode = rhs.m_errcode;
        m_userdata = rhs.m_userdata;
        return *this;
    }

    //实现移动构造，避免不必要的内存拷贝
    CHNResult(CHNResult && rhs)
        : m_errcode(rhs.m_errcode)
        , m_userdata(std::move(rhs.m_userdata))
    {}
    //移动赋值运算符
    CHNResult& operator=(CHNResult &&rhs)
    {
        if (this == &rhs)
            return *this;
        m_errcode = rhs.m_errcode;
        m_userdata = std::move(rhs.m_userdata);
        return *this;
    }

    /**
    *@brief 判断函数是否执行成功
    *       可直接将CHNResult对象当作bool变量使用
    */
    operator bool() const { return m_errcode == 0; }

    /**
    *@brief 判断函数是否执行成功
    *       兼容老版本函数
    */
    bool Succ()   const { return m_errcode == 0; }

    /**
    *@brief        函数失败的情况下：读取失败的错误码
    */
    int ErrCode() const { return m_errcode; }

    /**
    *@brief        函数失败的情况下：读取失败的错误描述
    */
    const char* ErrDesc() const { return hnlib_internal::CHNLocalErrInfo::obj().GetErrDesc();}

    /**
    *@brief 获取用户数据的引用
    */
    T& Get() { return m_userdata;}

    /**
    *@brief 移走用户数据，只能拿一次
    */
    T Take() { return std::move(m_userdata); }

public:
    /**
    *@brief 设置为成功
    */
    CHNResult& SetSucc()
    {
        m_errcode = 0;
        return *this;
    }

    /**
    *@brief 设置成功并附带用户数据
    */
    CHNResult& SetSucc(const T& userdata)
    {
        m_errcode = 0;
        m_userdata = userdata;
        return *this;
    }

    /**
    *@brief 设置成功并附带用户数据
    */
    CHNResult& SetSucc(T&& userdata)
    {
        m_errcode = 0;
        m_userdata = std::move(userdata);
        return *this;
    }

    /**
    *@brief 设置为失败【错误码-1，用户只关心错误描述】
    *@param szFmt[IN]   错误描述
    */
    CHNResult& SetFail(const char* const szFmt, ...)
    {
        m_errcode = -1;
        if(szFmt == NULL)
        {
            hnlib_internal::CHNLocalErrInfo::obj().SetErrDesc(NULL,0);
        }
        else
        {
            char szContent[MAX_PATH] = { 0 };
            va_list ap;
            va_start(ap, szFmt);
            vsnprintf(szContent, sizeof(szContent) - 1, szFmt, ap);
            va_end(ap);
            hnlib_internal::CHNLocalErrInfo::obj().SetErrDesc(szContent,strlen(szContent));
        }
        return *this;
    }

    /**
    *@brief 设置为失败【用户自定义错误码和错误描述】
    *@param errcode[IN] 错误码
    *@param szFmt[IN]   错误描述
    */
    CHNResult& SetFail(int errcode, const char* const szFmt, ...)
    {
        if(errcode == 0)
            errcode = -1;
        m_errcode = errcode;
        if(szFmt == NULL)
        {
            hnlib_internal::CHNLocalErrInfo::obj().SetErrDesc(NULL,0);
        }
        else
        {
            char szContent[MAX_PATH] = { 0 };
            va_list ap;
            va_start(ap, szFmt);
            vsnprintf(szContent, sizeof(szContent) - 1, szFmt, ap);
            va_end(ap);
            hnlib_internal::CHNLocalErrInfo::obj().SetErrDesc(szContent,strlen(szContent));
        }
        return *this;
    }

    /**
    *@brief 设置为失败：【使用系统的错误码和错误描述】
    *@note  这个函数会自动获取最后的系统错误 
    */
    CHNResult& SetSystemFail()
    {
        return this->SetSystemFail((int)HNGetLastError());
    }

    /**
    *@brief 设置为失败：【使用系统的错误码和错误描述】
    *@param errcode[IN] 系统错误码
    */
    CHNResult& SetSystemFail(int sysErrcode)
    {
        if(sysErrcode == 0)
            sysErrcode = -1;
        m_errcode = sysErrcode;
        std::string desc = HNGetSysErrorMsg(m_errcode);
        hnlib_internal::CHNLocalErrInfo::obj().SetErrDesc(desc.data(),desc.length());
        return *this;
    }

public:

    /**
    *@brief        将结果print出来，一般只用于调试
    */
    void Print()
    {
        if (m_errcode==0) 
        {
            printf("[succ]\n");
        }
        else 
        {
            printf("[fail][errcode:%d][errdesc:%s]\n", m_errcode, this->ErrDesc());
        }
    }

public:
    int m_errcode;  //是否成功（0成功，其他错误码）
    T   m_userdata; //用户数据
};

#endif // __HELLONET_M_CORE_S_BASE__