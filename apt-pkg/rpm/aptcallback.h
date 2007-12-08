#ifndef _APTRPM_RPMCALLBACK_H
#define _APTRPM_RPMCALLBACK_H

#include <apt-pkg/progress.h>
#include <rpm/rpmcli.h>

#if HAVE_RPM_RPMCB_H
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

#endif /* _APTRPM_RPMCALLBACK_H */
// vim:sts=3:sw=3

