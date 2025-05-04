#ifndef __HELLONET_M_CORE_S_OS_ANDROID__
#define __HELLONET_M_CORE_S_OS_ANDROID__

#include "../s_base.hpp"

//android下重写pthread_sigmask
//Android versions < 4.1 have a broken pthread_sigmask.    
static int HN__pthread_sigmask(int how, const sigset_t* set, sigset_t* oset) {
    static int workaround;
    int err;

    if (workaround) {
        return sigprocmask(how, set, oset);
    }
    else {
        err = pthread_sigmask(how, set, oset);
        if (err) {
            if (err == EINVAL && sigprocmask(how, set, oset) == 0) {
                workaround = 1;
                return 0;
            }
            else {
                return -1;
            }
        }
    }
    return 0;
}
# ifdef pthread_sigmask
# undef pthread_sigmask
# endif
# define pthread_sigmask(how, set, oldset) HN__pthread_sigmask(how, set, oldset)


#endif // __HELLONET_M_CORE_S_OS_ANDROID__