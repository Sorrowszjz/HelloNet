#ifndef __HELLONET_M_CORE__
#define __HELLONET_M_CORE__

#include "s_base.hpp"
#include "s_datatypes.hpp"
#include "s_codec.hpp"
#include "s_thread.hpp"

#if defined(HNOS_WIN)
#include "s_os_win/s_os_win.hpp"
#include "s_os_win/s_os_win_diskinfo.hpp"
#include "s_os_win/s_os_win_mfc.hpp"
#include "s_os_win/s_os_win_scm.hpp"
#elif defined(HNOS_ANDROID)
#include "s_os_android/s_os_android.hpp"
#elif defined(HNOS_MAC) 
#include "s_os_macos/s_os_macos.hpp"
#else
#include "s_os_linux/s_os_linux.hpp"
#endif

#include "s_console.hpp"
#include "s_datetime.hpp"
#include "s_stringutil.hpp"
#include "s_fileutil.hpp"
#include "s_logger.hpp"
#include "s_debug.hpp"
#include "s_unittest.hpp"
#include "s_service.hpp"

#endif // __HELLONET_M_CORE__