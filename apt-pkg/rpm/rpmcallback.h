#include <apt-pkg/progress.h>
#include <rpm/rpmcli.h>

#if RPM_VERSION >= 0x040405
typedef unsigned long long rpmCallbackSize_t;
#else
typedef unsigned long rpmCallbackSize_t;
#endif

#if RPM_VERSION < 0x040000
void * rpmCallback(const Header h,
#else
void * rpmCallback(const void * arg, 
#endif
			const rpmCallbackType what,
                        const rpmCallbackSize_t amount,
                        const rpmCallbackSize_t total,
			const void * pkgKey, void * data);

// vim:sts=3:sw=3
